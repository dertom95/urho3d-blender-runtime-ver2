cd ..
echo $PWD
sudo docker run -it -v $PWD:/var/sample -v $PWD/script:/var/sample/script -v $PWD/CMake:/var/sample/CMake -v $PWD/game:/var/sample/src/game  dertom95/urho3d:00 /var/sample/tools/build_linux.sh

sudo chown -R ${USER:=$(/usr/bin/id -run)}:$USER ./build-folder/build-linux

#if [[ ! -d "./build/bin/CoreData" ]]; then
#    echo "Create coredata symlink"
#    cd ./build/bin
#    ln -s ../../bin/CoreData
#    cd ../..
#fi

