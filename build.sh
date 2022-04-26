#!/bin/bash

if !([ -e "build" ] && [ -d "build" ])
then
    mkdir build;
fi


if (($#  > 0));
then
    cmake -S . -B build -DCMAKE_BUILD_TYPE=$1 -DCMAKE_TOOLCHAIN_FILE=vcpkg/scripts/buildsystems/vcpkg.cmake;
else
    cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE=vcpkg/scripts/buildsystems/vcpkg.cmake;
fi


if command -v mold &> /dev/null
then
    mold --run cmake --build build -j`nproc`
else
    cmake --build build -j`nproc`
fi

cp build/leaf .;
cp build/compile_commands.json .;
cd .;
