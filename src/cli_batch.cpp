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

string Strip0x(const string &hex)
{
    if (hex.size() > 1 && (hex.substr(0, 2) == "0x" || hex.substr(0, 2) == "0X")) {
        return hex.substr(2);
    }
    return hex;
}

void HexToBytes(const string &hex, uint8_t *result)
{
    for (uint32_t i = 0; i < hex.length(); i += 2) {
        string byteString = hex.substr(i, 2);
        uint8_t byte = (uint8_t)strtol(byteString.c_str(), NULL, 16);
        result[i / 2] = byte;
    }
}

const string delimiter = " ";

// prove creates a proof-of-space from the provided plot and challenge
int prove() { return 0; }

// verify the proof-of-space and return non-zero if there the program needs to exit
int verify(string id, string challenge, string proof)
{
    if (id.size() != 64) {
        std::cout << R"({"error":"invalid ID hex, should be 32 bytes"})" << std::endl;
    }

    if (challenge.size() != 64) {
        std::cout << R"({"error":"invalid challenge hex, should be 32 bytes"})" << std::endl;
    }

    if (proof.size() % 16) {
        std::cout << R"({"error":"invalid proof hex, should be a multiple of 8 bytes"})"
                  << std::endl;
    }

    // k can be implied from the proof
    uint8_t k = proof.size() / 16;
    std::cerr << "[verify]"
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
        std::cout << R"({"quality":")" << quality << R"("})" << std::endl;
    } else {
        std::cout << R"({"error":"verification failed"})" << std::endl;
    }

    return 0;
}

// plot will create a plot file which can be used to generate proofs of space
int plot() { return 0; }

// main starts our main program
int main()
{
    // read each line from stdin
    for (string line; std::getline(std::cin, line);) {
        // iterate over all tokens on the line
        size_t pos = 0;
        vector<string> tokens;
        while ((pos = line.find(delimiter)) != string::npos) {
            string token = line.substr(0, pos);
            tokens.push_back(token);
            line.erase(0, pos + delimiter.length());
        }

        // the line holds the remaining token
        tokens.push_back(line);
        if (tokens.size() < 2) {
            std::cerr << "expected at least 2 tokens, got:" << tokens.size() << std::endl;
            exit(1);
        }

        // decide the operation to take
        int code = 0;
        if (tokens[0] == "verify") {
            code = verify(tokens[1], tokens[2], tokens[3]);
        } else if (tokens[0] == "prove") {
            code = prove();
        } else if (tokens[0] == "plot") {
            code = plot();
        } else {
            std::cerr << "unsupported operation:" << tokens[0] << std::endl;
            exit(1);
        }

        // check if the operation went as expected
        if (code != 0) {
            exit(code);
        }
    }

    // string id = Strip0x("022fb42c08c12de3a6af053880199806532e79515f94e83461612101f9412f9e");
    // string proof = Strip0x(
    //     "0x99550b233d022598b09d4c8a7b057986f6775d80973a905f5a6251d628d186430cb4464b8c70ecc77101bd4d"
    //     "50ef2c016cc78682a13c4b796835431edeb2231a282229c9e7322614d10193b1b87daaac0e21af5b5acc9f73b7"
    //     "ddd1da2a46294a2073f2e2fc99d57f3278ea1fc0f527499267aaa3980f730cb2ea7aacc1fa3f460acca1254f92"
    //     "791612e6e9ab9c3aed5aea172d7056b03bbfdf5861372d5c0ceb09e109485412376e");
    // string challenge =
    //     Strip0x("0x4000000000000000000000000000000000000000000000000000000000000000");
    // if (id.size() != 64) {
    //     cout << "Invalid ID, should be 32 bytes" << endl;
    //     exit(1);
    // }
    // if (challenge.size() != 64) {
    //     cout << "Invalid challenge, should be 32 bytes" << endl;
    //     exit(1);
    // }
    // if (proof.size() % 16) {
    //     cout << "Invalid proof, should be a multiple of 8 bytes" << endl;
    //     exit(1);
    // }
    // uint8_t k = proof.size() / 16;
    // cout << "Verifying proof=" << proof << " for challenge=" << challenge
    //      << " and k=" << static_cast<int>(k) << endl
    //      << endl;
    // uint8_t id_bytes[32];
    // uint8_t challenge_bytes[32];
    // uint8_t *proof_bytes = new uint8_t[proof.size() / 2];
    // HexToBytes(id, id_bytes);
    // HexToBytes(challenge, challenge_bytes);
    // HexToBytes(proof, proof_bytes);

    // Verifier verifier = Verifier();
    // LargeBits quality = verifier.ValidateProof(id_bytes, k, challenge_bytes, proof_bytes, k * 8);

    // std::cerr << "called!" << quality << std::endl;
    // return 0;
}