#!/bin/sh
# Small script to fetch the latest protobuf and installs it
set -ex

URL=https://github.com/google/protobuf/releases/download/v3.0.0-beta-2/protobuf-cpp-3.0.0-beta-2.tar.gz
FILE=$(basename $URL)
DIR=protobuf-3.0.0-beta-2
DIST=$HOME/.dist
DISTFILE=$DIST/$FILE

[ -d $DIST ] || mkdir $DIST

if [[ -f $DISTFILE ]]; then
  # not first run
  curl -o $DISTFILE -z $DISTFILE -L $URL
else
  # first run
  curl -o $DISTFILE -L $URL
fi

tar xvzf $DISTFILE
cd $DIR && ./configure --prefix=$HOME && make &&  make install
