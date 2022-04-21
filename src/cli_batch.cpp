#include <ctime>
#include <iostream>
#include <set>

#include "../lib/include/picosha2.hpp"
#include "cxxopts.hpp"
#include "plotter_disk.hpp"
#include "prover_disk.hpp"
#include "verifier.hpp"

using std::string;
using std::vector;

// SplitLine will take the line and split it into tokens on delimiter
vector<string> SplitLine(string line, string delimiter)
{
    size_t pos = 0;
    vector<string> tokens;
    while ((pos = line.find(delimiter)) != string::npos) {
        string token = line.substr(0, pos);
        tokens.push_back(token);
        line.erase(0, pos + delimiter.length());
    }
    tokens.push_back(line);
    return tokens;
}

// HexToBytes decodes a string into a array of bytes
void HexToBytes(const string &hex, uint8_t *result)
{
    for (uint32_t i = 0; i < hex.length(); i += 2) {
        string byteString = hex.substr(i, 2);
        uint8_t byte = (uint8_t)strtol(byteString.c_str(), NULL, 16);
        result[i / 2] = byte;
    }
}

// Print uint8 vector to hex string
string Uint8VectorToHex(const vector<uint8_t> &v)
{
    string result;
    result.resize(v.size() * 2);
    const char letters[] = "0123456789abcdef";
    char *current_hex_char = &result[0];
    for (uint8_t b : v) {
        *current_hex_char++ = letters[b >> 4];
        *current_hex_char++ = letters[b & 0xf];
    }
    return result;
}

// plot creates a proof-of-space plot file
int plot(
    string filename,
    string id,
    string memo,
    string ks,
    string stripe_size,
    string tempdir,
    string tempdir2,
    string finaldir)
{
    if (id.size() != 64) {
        std::cerr << R"({"error":"invalid id hex, should be 32 bytes"})" << std::endl;
        return 0;
    }
    if (memo.size() % 2 != 0) {
        std::cerr << R"({"error":"invalid memo, should only be whole bytes hex"})" << std::endl;
        return 0;
    }

    int k = std::stoi(ks);
    if (k < 18) {
        std::cerr << R"({"error":"invalid k-value"})" << std::endl;
        return 0;
    }

    int sz = std::stoi(stripe_size);
    if (k < 0) {
        std::cerr << R"({"error":"invalid stripe_size"})" << std::endl;
    }

    std::cout << "[plot]"
              << " id: " << id << " memo: " << memo << " filename: " << filename
              << " stripe_size: " << stripe_size << " k: " << static_cast<int>(k) << std::endl;

    // decode hex strings
    std::vector<uint8_t> memo_bytes(memo.size() / 2);
    std::array<uint8_t, 32> id_bytes;
    HexToBytes(memo, memo_bytes.data());
    HexToBytes(id, id_bytes.data());

    try {
        DiskPlotter().CreatePlotDisk(
            tempdir,
            tempdir2,
            finaldir,
            filename,
            k,
            memo_bytes.data(),
            memo_bytes.size(),
            id_bytes.data(),
            id_bytes.size(),
            0,  // buffmegabytes
            0,  // num_buckets
            sz,
            0,  // num_threads
            ENABLE_BITFIELD);
    } catch (...) {
        std::cerr << R"({"error":"unexpected failure while plotting"})" << std::endl;
        return 0;
    }
    return 0;
}

// inspect simply returns information about the plot file
int inspect(string filename)
{
    // init a prover for the given file
    DiskProver prover(filename);
    vector<uint8_t> memo = prover.GetMemo();
    vector<uint8_t> id = prover.GetId();
    uint8_t size = prover.GetSize();
    std::cerr << R"({"id":")" << Uint8VectorToHex(id) << R"(","memo":")" << Uint8VectorToHex(memo)
              << R"(","size":")" << static_cast<int>(size) << R"("})" << std::endl;
    return 0;
}

// prove creates a proof-of-space from the provided plot and challenge
int prove(string filename, string challenge)
{
    if (challenge.size() != 64) {
        std::cerr << R"({"error":"invalid challenge hex, should be 32 bytes"})" << std::endl;
        return 0;
    }

    // decode hex string
    uint8_t challenge_bytes[32];
    HexToBytes(challenge, challenge_bytes);

    // init a prover for the given file
    DiskProver prover(filename);
    vector<uint8_t> memo = prover.GetMemo();
    vector<uint8_t> id = prover.GetId();
    uint8_t size = prover.GetSize();

    try {
        // check if their are any proves
        vector<LargeBits> qualities = prover.GetQualitiesForChallenge(challenge_bytes);

        // print prover output and start qualities array printing
        std::cerr << R"({"id":")" << Uint8VectorToHex(id) << R"(","memo":")"
                  << Uint8VectorToHex(memo) << R"(","size":")" << static_cast<int>(size)
                  << R"(","qualities":[)";
        std::cout << "[prove] id:" << Uint8VectorToHex(id) << " memo: " << Uint8VectorToHex(memo)
                  << " size:" << static_cast<int>(size) << std::endl;

        // print qualities so the client can decide which to get for full proof
        for (uint32_t i = 0; i < qualities.size(); i++) {
            if (i != 0) {
                std::cerr << ",";
            }
            std::cerr << R"(")" << qualities[i] << R"(")";
        }

        // end of qualities printing
        std::cerr << R"(]})" << std::endl;

        // the client should then send a line of input with qualities to the the full proof for.
        // if the client wants no qualities check it should send an invalid index. e.g: -1
        string line;
        std::getline(std::cin, line);
        std::cout << "[prove] index line:" << line << ";" << std::endl;
        vector<string> idxs = SplitLine(line, ",");

        // start proofs printing
        std::cerr << R"({"proofs":[)";
        for (std::size_t i = 0; i < idxs.size(); ++i) {
            int qualityidx = std::stoi(idxs[i]);
            if (qualityidx < 0 || qualityidx > (qualities.size() - 1)) {
                continue;  // ignore any out-of-range indexes
            }

            // get the full proof as bytes
            LargeBits proof = prover.GetFullProof(challenge_bytes, qualityidx, true);
            uint8_t *proof_data = new uint8_t[8 * size];
            proof.ToBytes(proof_data);

            // print the full proof, maybe with a comma and hex encoded
            if (i != 0) {
                std::cerr << ",";
            }
            std::cerr << R"(")" << Util::HexStr(proof_data, size * 8) << R"(")";
            delete[] proof_data;
        }

        std::cerr << R"(]})" << std::endl;
    } catch (const std::exception &ex) {
        std::cerr << R"({"error":"failed to prove: )" << ex.what() << R"("})" << std::endl;
        return 0;
    } catch (...) {
        std::cerr << R"({"error":"unexpected failure when proving"})" << std::endl;
        return 0;
    }

    return 0;
}

// verify the proof-of-space and return non-zero if there the program needs to exit
int verify(string id, string challenge, string proof)
{
    if (id.size() != 64) {
        std::cerr << R"({"error":"invalid ID hex, should be 32 bytes"})" << std::endl;
        return 0;
    }

    if (challenge.size() != 64) {
        std::cerr << R"({"error":"invalid challenge hex, should be 32 bytes"})" << std::endl;
        return 0;
    }

    if (proof.size() % 16) {
        std::cerr << R"({"error":"invalid proof hex, should be a multiple of 8 bytes"})"
                  << std::endl;
        return 0;
    }

    // k can be implied from the proof
    uint8_t k = proof.size() / 16;
    std::cout << "[verify]"
              << " id: " << id << " challenge: " << challenge << " proof: " << proof
              << " k: " << static_cast<int>(k) << std::endl;

    // decode hex into the variables we need
    uint8_t id_bytes[32];
    uint8_t challenge_bytes[32];
    uint8_t *proof_bytes = new uint8_t[proof.size() / 2];
    HexToBytes(id, id_bytes);
    HexToBytes(challenge, challenge_bytes);
    HexToBytes(proof, proof_bytes);

    // perform validation and assert quality
    LargeBits quality = Verifier().ValidateProof(id_bytes, k, challenge_bytes, proof_bytes, k * 8);
    if (quality.GetSize() == 256) {
        std::cerr << R"({"quality":")" << quality << R"("})" << std::endl;
    } else {
        std::cerr << R"({"error":"verification failed"})" << std::endl;
    }

    delete[] proof_bytes;
    return 0;
}

// main starts our main program
int main()
{
    // read each line from stdin
    for (string line; std::getline(std::cin, line);) {
        std::cout << "[main] input:" << line << ";" << std::endl;

        // iterate over all tokens on the line
        vector<string> tokens = SplitLine(line, " ");

        // the line holds the remaining token
        if (tokens.size() < 2) {
            std::cout << "expected at least 2 tokens, got:" << tokens.size() << std::endl;
            exit(1);
        }

        // decide the operation to take
        int code = 0;
        if (tokens[0] == "verify") {
            code = verify(tokens[1], tokens[2], tokens[3]);
        } else if (tokens[0] == "prove") {
            code = prove(tokens[1], tokens[2]);
        } else if (tokens[0] == "plot") {
            code = plot(
                tokens[1],
                tokens[2],
                tokens[3],
                tokens[4],
                tokens[5],
                tokens[6],
                tokens[7],
                tokens[8]);
        } else if (tokens[0] == "inspect") {
            code = inspect(tokens[1]);
        } else {
            std::cout << "unsupported operation:" << tokens[0] << std::endl;
            exit(1);
        }

        // check if the operation went as expected
        if (code != 0) {
            exit(code);
        }
    }
}
