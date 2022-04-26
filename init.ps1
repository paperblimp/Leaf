git clone --depth 1 https://github.com/Microsoft/vcpkg.git
Set-Location vcpkg
.\bootstrap-vcpkg.bat -disableMetrics 
.\vcpkg install --triplet=x64-windows imgui[glfw-binding,opengl3-binding,docking-experimental] ffmpeg[openh264,avcodec,ffmpeg] glfw3 glm stb nlohmann-json fmt termcolor boost-dll boost-serialization boost-filesystem boost-date-time
Set-Location ..
