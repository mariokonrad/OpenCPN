@echo off

rem builds the software in the current directory

rmdir /q/s build
mkdir build
cd build

goto release_build_vs2010

:debug_build_vs2010
echo "BUILD: debug, visual studio 2010"
cmake -G "Visual Studio 10" -DCMAKE_BUILD_TYPE=Debug %~dp0
cmake --build .
cmake --build . --target unittest
rem cpack -G NSIS -C Debug
rem cpack -G ZIP  -C Debug
goto end

:release_build_vs2010
echo "BUILD: release, visual studio 2010"
cmake -G "Visual Studio 10" -DCMAKE_BUILD_TYPE=Release %~dp0
cmake --build . --config Release
cpack -G NSIS -C Release
cpack -G ZIP  -C Release
goto end

:debug_build_vs2012
echo "BUILD: debug, visual studio 2012"
cmake -G "Visual Studio 11" -DCMAKE_BUILD_TYPE=Debug %~dp0
cmake --build .
rem cmake --build . --target unittest
rem cpack -G NSIS -C Debug
rem cpack -G ZIP  -C Debug
goto end


:end
cd ..

