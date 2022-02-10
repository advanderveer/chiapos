package posp

// #cgo LDFLAGS: -L. -lstdc++ -luint128 -lfse
// #cgo CXXFLAGS: -std=c++17 -I. -I../../lib/include -I../_deps/cxxopts-src/include -I../_deps/gulrak-src/include/ghc -I../../lib/FiniteStateEntropy/lib -I../src -I../test
// #include "posp.h"
// #include "../src/b3/blake3.c"
// #include "../src/b3/blake3_portable.c"
// #include "../src/b3/blake3_dispatch.c"
// #include "../src/b3/blake3_avx2_x86-64_unix.S"
// #include "../src/b3/blake3_avx512_x86-64_unix.S"
// #include "../src/b3/blake3_sse41_x86-64_unix.S"
// #include "../src/chacha8.c"
import "C"

func ValidateProof() {
	C.ValidateProof()
}
