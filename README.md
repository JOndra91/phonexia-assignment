# Phonexia hiring assignment

Task is explained in `assignment_en.pdf`.

## Requirements

 * C++11 compatible compiler
 * CMake >= 3.0

## Build instructions

Update git submodules:

```
git submodule update --init --recursive
```

Use CMake to build executables:

```
mkdir build
cd build
cmake ..
make
```

Resulting executables are in `build/bin` directory.

## Run instructions

Run in build directory:
```
./bin/billing ../data.csv
```
