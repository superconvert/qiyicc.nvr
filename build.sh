#!/bin/sh

#cd 3part/jsoncpp-master
#if [ ! -d "build/debug" ]; then
#    mkdir -p build/debug
#fi
#cd build/debug/
#cmake -DCMAKE_BUILD_TYPE=debug -DBUILD_STATIC_LIBS=ON -DBUILD_SHARED_LIBS=OFF -DARCHIVE_INSTALL_DIR=. -G "Unix Makefiles" ../..
#cd ../../../..

if [ ! -d "thrift" ]; then
    mkdir thrift
fi
cd thrift
thrift --gen py ../3part/nvr.thrift
# cob_style 生成带有异步处理的代码
thrift --gen cpp:cob_style ../3part/nvr.thrift
cd ..

if [ ! -d "build" ]; then
    mkdir build
fi

if [ ! -f "License.txt" ]; then
    touch License.txt 
fi

cd build
cmake ..
make
