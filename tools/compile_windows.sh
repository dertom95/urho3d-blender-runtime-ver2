cd ..
sudo docker run -it -v $PWD:/var/sample -v $PWD/tools:/var/sample/build-folder/build-windows/bin/tool -v $PWD/script:/var/sample/script -v $PWD/CMake:/var/sample/CMake -v $PWD/game:/var/sample/src/game  dertom95/urho3d:00 /var/sample/tools/build_windows.sh

sudo chown -R ${USER:=$(/usr/bin/id -run)}:$USER ./build-folder/build-windows

cd build-folder/build-windows/bin
rm -Rf tool
#
#for d in */ ; do
#    echo "$d"
#    sudo docker run -it --rm  dertom95/urho3d:00  /Urho3D/build/bin/tool/PackageTool "$d" "$d.pak"  -c
#    echo sudo docker run -it --rm  dertom95/urho3d:00  /Urho3D/build/bin/tool/PackageTool "$d" "$d.pak"  -c
#done


#
#
#if [[ ! -d "./build-windows/bin/CoreData" ]]; then
#    echo "Create coredata symlink"
#    cd ./build-windows/bin
#    ln -s ../../bin/CoreData
#    cd ../..
#fi
 
