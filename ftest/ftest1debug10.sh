#!/bin/bash
BUILD_DIR="../cmake-build-debug"
mkdir -p "$BUILD_DIR"
# Change to the build directory
cd "$BUILD_DIR"
# Run CMake to configure the build
cmake ..
make
"$BUILD_DIR/fluid/fluid" 10 "large.fld" "../outputftestdebug10.fld"