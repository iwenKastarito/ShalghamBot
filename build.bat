@echo off
echo Building Chess Game...

REM Check if VCPKG is installed
if not exist "vcpkg\vcpkg.exe" (
    echo Installing VCPKG and SFML...
    git clone https://github.com/Microsoft/vcpkg.git
    cd vcpkg
    call .\bootstrap-vcpkg.bat
    .\vcpkg install sfml:x64-windows
    
    echo Installing Qt5...
    .\vcpkg install qt5-base:x64-windows
    cd ..
)

REM Create build directory
if not exist "build" mkdir build
cd build

REM Configure and build
cmake .. -DCMAKE_TOOLCHAIN_FILE=../vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build . --config Release

echo Build complete!
echo Run SFML version with: .\build\Release\chess.exe

REM Check if Qt version was built
if exist "Release\qtchess.exe" (
    echo Run Qt version with: .\build\Release\qtchess.exe
) 