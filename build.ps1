
if (!(Test-Path "./build")) {
    mkdir build
}


$build_mode = ""

if ($args.Length -eq 1) {
    $build_mode = $($args[0])
} else {
    $build_mode = "Debug"
}

$($build_mode)

Remove-Item .\build\$($build_mode)\leaf.exe -Force

cmake -S . -B build -A x64 -DVCPKG_TARGET_TRIPLET=x64-windows "-DCMAKE_TOOLCHAIN_FILE=.\vcpkg\scripts\buildsystems\vcpkg.cmake" -DBoost_NO_WARN_NEW_VERSIONS=1

cmake --build build -j4 --config $($build_mode)
Copy-Item .\vcpkg\installed\x64-windows\tools\ffmpeg\* .\build\$($build_mode)

Copy-Item .\icons  .\build\$($build_mode) -R -Force
Copy-Item .\fonts  .\build\$($build_mode) -R -Force
