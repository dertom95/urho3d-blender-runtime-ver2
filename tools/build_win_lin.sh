#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

cd $DIR/..
#rm -Rf build/*

echo "native"
script/dockerized.sh linux 
echo "mingw"
script/dockerized.sh mingw
