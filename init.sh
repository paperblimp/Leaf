#!/bin/bash

git clone --depth 1 https://github.com/Microsoft/vcpkg.git
cd vcpkg
./bootstrap-vcpkg.sh --disableMetrics
./vcpkg install imgui[glfw-binding,opengl3-binding,docking-experimental] ffmpeg[openh264,avcodec,ffmpeg] glfw3 glm stb nlohmann-json fmt termcolor boost-dll boost-serialization boost-filesystem boost-date-time
cd ..
cp vcpkg/installed/x64-linux/tools/ffmpeg/ffmpeg .