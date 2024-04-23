#!/bin/sh

# Make sure we are in the correct directory
# The following command changes to the directory where the script is located
cd "$(dirname "$0")"

# Create build directory
mkdir build

# Change to build directory
cd build

# Run cmake
cmake ..

sudo cmake --build .

sudo cmake --install .

echo "Done."