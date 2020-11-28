#!/bin/bash

if [ -z "$1" ]; then
	echo "building packager - exports the builds in the 'builds'-folder and stores the result in the export-folder"
	echo ""
    echo "IMPORTANT: for signing android-apks, look that sign_apk.sh requirements are fulfilled"
    echo ""
	echo "export_game.sh [gamename] android-alias-name"
	exit 1
fi

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

cd $DIR/../build


GAME="$1"

rm -Rf $DIR/../export
mkdir $DIR/../export

if [ -z "$GAME" ]; then
	GAME="game"
fi

EXPORTED=""

for d in */ ; do
    FOLDERNAME=${d::-1}

    if [ "$FOLDERNAME" != "android" ]; then
	    if [ "$FOLDERNAME" == "native" ]; then
		    GAMENAME="$GAME-linux"
            elif [ "$FOLDERNAME" == "mingw" ]; then
		    GAMENAME="$GAME-windows"
	    else
		    GAMENAME="$GAME-$FOLDERNAME"
            fi

    	    echo "Package: $GAMENAME"
	    echo ""	    
	    cd $FOLDERNAME
	 
	    cp -r -L bin $GAMENAME
	    # copy files that should be packaged with your include
	    cp -r $DIR/../package_include/* $GAMENAME    
	    zip -r $GAMENAME.zip $GAMENAME
	    mv $GAMENAME.zip $DIR/../export
	    rm -Rf $GAMENAME
	    cd ..

        EXPORTED="$EXPORTED $FOLDERNAME"
     else
	     if [ "$2" != "" ]; then
           $DIR/sign_apk.sh $GAME $2 || true
           if [ $? -eq 0 ]; then
             EXPORTED="$EXPORTED android"
           fi
         fi
     fi
done

echo ""
echo "exported: $EXPORTED"
