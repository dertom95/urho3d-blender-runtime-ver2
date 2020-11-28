#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

PROJECT_DIR=$(cd "${0%/*}/.." || exit 1; pwd)

echo "DIR: $DIR"
rm -Rf $DIR/../android/launcher-app/build
rm -Rf $DIR/../android/launcher-app/.cxx
mkdir -p $DIR/../android/launcher-app/src/main/assets
rm -Rf $DIR/../android/launcher-app/src/main/assets/*
$DIR/../tools/package_folder.sh $DIR/../bin $DIR/../android/launcher-app/src/main/assets
cd $DIR/..
#script/dockerized.sh android ./gradlew -P ANDROID_ABI=armeabi-v7a build 
#        "dertom95/urho3d-android:latest" 

echo docker run -it --rm -h fishtank \
        -e HOST_UID="$(id -u)" -e HOST_GID="$(id -g)" -e PROJECT_DIR="$PROJECT_DIR" $NEW_URHO3D_HOME \
        --mount type=bind,source="$PROJECT_DIR",target="$PROJECT_DIR" \
        -v $PROJECT_DIR/android/launcher-app:/Urho3D/android/launcher-app \
	--mount source="$(id -u).urho3d_home_dir",target=/home/urho3d \
	--name "dockerized-android-build" \
dertom95/urho3d-android:latest $@
docker run -it --rm -h fishtank \
        -e HOST_UID="$(id -u)" -e HOST_GID="$(id -g)" -e PROJECT_DIR="$PROJECT_DIR" $NEW_URHO3D_HOME \
        --mount type=bind,source="$PROJECT_DIR",target="$PROJECT_DIR" \
        -v $PROJECT_DIR/android/launcher-app:/Urho3D/android/launcher-app \
	--mount source="$(id -u).urho3d_home_dir",target=/home/urho3d \
	--name "dockerized-android-build" \
dertom95/urho3d-android:latest $@
mkdir -p $DIR/../build/android
cp -r $DIR/../android/launcher-app/build/outputs/apk $DIR/../build/android
