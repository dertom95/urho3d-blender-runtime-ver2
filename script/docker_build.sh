echo $PWD
platform=$1
shift
script/cmake_$platform.sh ./build/$platform "$@"
cd build/$platform
make -j8
