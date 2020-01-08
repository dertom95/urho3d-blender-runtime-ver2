#!/usr/bin/env bash

export MINGW_SYSROOT=/usr/x86_64-w64-mingw32
export MINGW_PREFIX=/usr/bin/x86_64-w64-mingw32

cd /var/sample/
echo dir
ls /usr/bin/x86_64*
echo ----
bash ./script/cmake_mingw.sh build-folder/build-windows -DURHO3D_HOME=/Urho3D/build-windows -DURHO3D_PACKAGING=1 -DURHO3D_PROFILING=0 -DMINGW_PREFIX=/usr/bin/x86_64-w64-mingw32  -DCMAKE_BUILD_TYPE=Release -DURHO3D_DEPLOYMENT_TARGET=generic || true
cd build-folder/build-windows
make -j $(nproc)
