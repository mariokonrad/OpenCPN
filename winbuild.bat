@echo off

rem builds the software in the current directory

rmdir /q/s build

mkdir build

cd build

cmake -G "Visual Studio 10" -DCMAKE_BUILD_TYPE=Debug %~dp0
cmake --build .

rem cpack -G NSIS -C Debug
rem cpack -G ZIP  -C Debug

rem cmake --build . --config Release
rem cpack -G NSIS -C Release
rem cpack -G ZIP  -C Release

cd ..
