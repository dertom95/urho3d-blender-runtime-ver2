#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

cd $DIR/..
rm -Rf build/*

echo "linux"
script/dockerized.sh linux
echo "mingw"
script/dockerized.sh mingw
echo "web"
script/dockerized.sh web
echo "android"
script/android_packRes_clearbuild_build_copy.sh
echo "rpi"
script/dockerized.sh rpi
echo "arm64"
script/dockerized.sh arm
