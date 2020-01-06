sudo docker run -it -v $PWD:/var/sample -v $PWD/script:/var/sample/script -v $PWD/CMake:/var/sample/CMake -v $PWD/game:/var/sample/src/game -v $PWD/.hunter:/root/.hunter arnislielturks/urho3d:10 /var/sample/build_windows.sh

sudo chown -R ${USER:=$(/usr/bin/id -run)}:$USER ./build-windows


if [[ ! -d "./build-windows/bin/CoreData" ]]; then
    echo "Create coredata symlink"
    cd ./build-windows/bin
    ln -s ../../bin/CoreData
    cd ../..
fi
 
