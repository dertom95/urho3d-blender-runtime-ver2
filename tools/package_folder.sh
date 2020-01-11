#!/bin/bash

if [ -z "$1" ]; then
	echo ""
        echo "script to use Urho3D's PackageTool on whole Folder"
	echo ""
        echo "USAGE: ./package_folder.sh folder_with_folders_to_be_packed [outputfolder]"
	echo ""
	echo "if no output folder specified, then the result is placed in the input-folder"
	echo ""
        exit 1
fi

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

cd $1
for d in */ ; do
    FOLDERNAME=${d::-1}
    INFOLDER="$1/$FOLDERNAME"
    if [ -z "$2" ]; then
            OUTFOLDER="$1/$FOLDERNAME"
    else        
            OUTFOLDER="$2/$FOLDERNAME"
    fi
    $DIR/PackageTool "$INFOLDER" "$OUTFOLDER.pak"  -c
done

