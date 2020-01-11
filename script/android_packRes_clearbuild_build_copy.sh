#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

if [ -z "$1" ]; then
	ABI = "armeabi-v7a"
else
	ABI = "$1"
fi

echo "DIR: $DIR"
rm -Rf $DIR/../android/launcher-app/build
rm -Rf $DIR/../android/launcher-app/.cxx
rm -Rf $DIR/../android/launcher-app/src/main/assets/*
$DIR/../tools/package_folder.sh $DIR/../bin $DIR/../android/launcher-app/src/main/assets
cd $DIR/..
script/project_dockerized.sh android export URHO3D_HOME="$ABI" && ./gradlew -P ANDROID_ABI=$ABI build 

mkdir -p $DIR/../build/android
cp -r $DIR/../android/launcher-app/build/outputs/apk $DIR/../build/android
