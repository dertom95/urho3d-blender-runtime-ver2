#!/usr/bin/env bash

cd /var/sample/
./script/cmake_generic.sh build-folder/build-linux -DURHO3D_PACKAGING=1 -DURHO3D_HOME=/Urho3D/build -DCMAKE_BUILD_TYPE=Release -DURHO3D_PROFILING=0 -DURHO3D_DEPLOYMENT_TARGET=generic || true
cd build-folder/build-linux
make -j $(nproc)
