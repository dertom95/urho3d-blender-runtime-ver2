#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

cd $DIR/..
rm -Rf build/*

echo "native"
script/dockerized.sh native
echo "mingw"
script/dockerized.sh mingw
echo "web"
script/dockerized.sh web
echo "android"
script/android_packRes_clearbuild_build_copy.sh
