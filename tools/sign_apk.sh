#!/bin/bash

if [ -z "$2" ]; then
	echo ""
    echo "script to sign the unsigned release-apk and copy the result to the export-folder"
	echo ""
    echo "IMPORTANT: jarsigner and zipalign needs to be in PATH"
    echo "           the path to your keystore needs to be in ENV-Var: URHO3D_ANDROID_KEYSTORE"
    echo ""
    echo "USAGE: ./sign_apk.sh gamename keystore-aliasname"
    exit 1
fi

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

 GAMENAME="game-android.apk"

 path_to_executable=$(which jarsigner)
 if [ -x "$path_to_executable" ]; then
    echo "found jarsigner: $path_to_executable"
 else
    echo "No jarsigner in path. usually this can be found in JDK/bin/jarsigner"
    exit 1
 fi

path_to_executable=$(which zipalign)
 if [ -x "$path_to_executable" ]; then
    echo "found zipalign: $path_to_executable"
 else
    echo "No zipalign in path. usually this can be found in ANDROID-SDK/build-tools/[....]/zipalign"
    exit 1
 fi


if [ -z "$URHO3D_ANDROID_KEYSTORE" ]; then
   echo "There is no keystore specified in environment-variable: URHO3D_ANDROID_KEYSTORE"
   exit 1
fi

echo jarsigner -verbose -keystore $URHO3D_ANDROID_KEYSTORE $DIR/../build/android/apk/release/*.apk -signedjar $GAMENAME  $2

mkdir -p $DIR/../export
if [ -f "$DIR/../export/*.apk" ]; then
   rm $DIR/../export/*.apk
fi
   
jarsigner -verbose -keystore $URHO3D_ANDROID_KEYSTORE $DIR/../build/android/apk/release/*.apk -signedjar $DIR/../export/$GAMENAME-unaligned  $2
zipalign -v 4 $DIR/../export/$GAMENAME-unaligned $DIR/../export/$GAMENAME
rm $DIR/../export/$GAMENAME-unaligned
