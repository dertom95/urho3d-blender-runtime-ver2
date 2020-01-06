sudo docker run -it -v $PWD:/var/sample -v $PWD/script:/var/sample/script -v $PWD/CMake:/var/sample/CMake -v $PWD/game:/var/sample/src/game  arnislielturks/urho3d:10 /var/sample/build_linux.sh

sudo chown -R ${USER:=$(/usr/bin/id -run)}:$USER ./build

if [[ ! -d "./build/bin/CoreData" ]]; then
    echo "Create coredata symlink"
    cd ./build/bin
    ln -s ../../bin/CoreData
    cd ../..
fi

