#!/bin/bash
set -e

# remove previous build
rm -rf build

# build in separate directory
mkdir -p build
cd build

# generate files
../configure.py

# compile
make

# test
export PYTHONPATH=$PWD
export LD_LIBRARY_PATH=$PWD/../../build/fakevim:$LD_LIBRARY_PATH
../test.py

