#!/bin/sh
# Small script to fetch the latest protobuf and installs it

set -ex
wget https://github.com/google/protobuf/releases/download/v3.0.0-beta-2/protobuf-cpp-3.0.0-beta-2.tar.gz
tar xvzf protobuf-cpp-3.0.0-beta-2.tar.gz
cd protobuf-3.0.0-beta-2 && ./configure --prefix=$HOME && make &&  make install

export LD_LIBRARY_PATH="$HOME/lib:$LD_LIBRARY_PATH"
export LIBRARY_PATH="$HOME/lib:$LIBRARY_PATH"

export PATH="$HOME/bin:$PATH"
export PKG_CONFIG_PATH="$HOME/lib/pkgconfig:$PKG_CONFIG_PATH"
export C_INCLUDE_PATH="$HOME/include:$C_INCLUDE_PATH"
export CPLUS_INCLUDE_PATH="$HOME/include:$CPLUS_INCLUDE_PATH"
