# Chia Proof of Space

![Build](https://github.com/Chia-Network/chiapos/workflows/Build/badge.svg)
![PyPI](https://img.shields.io/pypi/v/chiapos?logo=pypi)
![PyPI - Format](https://img.shields.io/pypi/format/chiapos?logo=pypi)
![GitHub](https://img.shields.io/github/license/Chia-Network/chiapos?logo=Github)

[![Total alerts](https://img.shields.io/lgtm/alerts/g/Chia-Network/chiapos.svg?logo=lgtm&logoWidth=18)](https://lgtm.com/projects/g/Chia-Network/chiapos/alerts/)
[![Language grade: Python](https://img.shields.io/lgtm/grade/python/g/Chia-Network/chiapos.svg?logo=lgtm&logoWidth=18)](https://lgtm.com/projects/g/Chia-Network/chiapos/context:python)
[![Language grade: C/C++](https://img.shields.io/lgtm/grade/cpp/g/Chia-Network/chiapos.svg?logo=lgtm&logoWidth=18)](https://lgtm.com/projects/g/Chia-Network/chiapos/context:cpp)

Chia's proof of space is written in C++. Includes a plotter, prover, and
verifier.

This fork is maintainted to provide a "batch" provider that exposes a very simple
text protocol for other processes to interact with. It makes it more performant to verify and
prove from other porcesses. This is for research purposes only.

## C++ Usage Instructions

### Compile

```bash
# Requires cmake 3.14+

mkdir -p build && cd build
cmake ../
cmake --build . -- -j 6
```

## Static Compilation With glibc

### Statically compile ProofOfSpace

```bash
mkdir -p build && cd build
cmake -DBUILD_PROOF_OF_SPACE_STATICALLY=ON ../
cmake --build . -- -j 6
```

### Run tests

```bash
./RunTests
```

### CLI usage

```bash
./ProofOfSpace -k 25 -f "plot.dat" -m "0x1234" create
./ProofOfSpace -k 25 -f "final-plot.dat" -m "0x4567" -t TMPDIR -2 SECOND_TMPDIR create
./ProofOfSpace -f "plot.dat" prove <32 byte hex challenge>
./ProofOfSpace -k 25 verify <hex proof> <32 byte hex challenge>
./ProofOfSpace -f "plot.dat" check <iterations>
```

### Batch CLI usage

With the default plot as produced above, the following can be send over stdin to produce 3 proves.

```bash
prove plot.dat 1000000000000000000000000000000000000000000000000000000000000000
```

An below is an example for verifying a proof

```bash
verify 022fb42c08c12de3a6af053880199806532e79515f94e83461612101f9412f9e 4000000000000000000000000000000000000000000000000000000000000000 99550b233d022598b09d4c8a7b057986f6775d80973a905f5a6251d628d186430cb4464b8c70ecc77101bd4d50ef2c016cc78682a13c4b796835431edeb2231a282229c9e7322614d10193b1b87daaac0e21af5b5acc9f73b7ddd1da2a46294a2073f2e2fc99d57f3278ea1fc0f527499267aaa3980f730cb2ea7aacc1fa3f460acca1254f92791612e6e9ab9c3aed5aea172d7056b03bbfdf5861372d5c0ceb09e109485412376e
```

And a plot command looks like this:

```bash
plot plot.dat 022fb42c08c12de3a6af053880199806532e79515f94e83461612101f9412f9e 0102030405 21 0 . . .
```

super tiny plot from the tests:

```bash
plot tiny.dat 022fb42c08c12de3a6af053880199806532e79515f94e83461612101f9412f9e 0102030405 18 4000 . . .
```

### Benchmark

```bash
time ./ProofOfSpace -k 25 create
```

### Hellman Attacks usage

There is an experimental implementation which implements some of the Hellman
Attacks that can provide significant space savings for the final file.

```bash
./HellmanAttacks -k 18 -f "plot.dat" -m "0x1234" create
./HellmanAttacks -f "plot.dat" check <iterations>
```

## Python

Finally, python bindings are provided in the python-bindings directory.

### Install

```bash
python3 -m venv .venv
. .venv/bin/activate
pip3 install .
```

### Run python tests

Testings uses pytest. Linting uses flake8 and mypy.

```bash
py.test ./tests -s -v
```

## ci Building

The primary build process for this repository is to use GitHub Actions to
build binary wheels for MacOS, Linux (x64 and aarch64), and Windows and publish
them with a source wheel on PyPi. See `.github/workflows/build.yml`. CMake uses
[FetchContent](https://cmake.org/cmake/help/latest/module/FetchContent.html)
to download [pybind11](https://github.com/pybind/pybind11). Building is then
managed by [cibuildwheel](https://github.com/joerick/cibuildwheel). Further
installation is then available via `pip install chiapos` e.g.

## Contributing and workflow

Contributions are welcome and more details are available in chia-blockchain's
[CONTRIBUTING.md](https://github.com/Chia-Network/chia-blockchain/blob/main/CONTRIBUTING.md).

The main branch is usually the currently released latest version on PyPI.
Note that at times chiapos will be ahead of the release version that
chia-blockchain requires in it's main/release version in preparation for a
new chia-blockchain release. Please branch or fork main and then create a
pull request to the main branch. Linear merging is enforced on main and
merging requires a completed review. PRs will kick off a GitHub actions ci build
and analysis of chiapos at
[lgtm.com](https://lgtm.com/projects/g/Chia-Network/chiapos/?mode=list). Please
make sure your build is passing and that it does not increase alerts at lgtm.
