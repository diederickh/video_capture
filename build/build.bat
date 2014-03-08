echo off
set d=%CD%
set bd=win-vs2012-x86_64
set bits=%1
set type=%2
set cmake_bt="Release"
set cmake_gen="Visual Studio 11 Win64"
set cmake_opt=""

if "%bits%" == "" (
  echo "Usage: release.bat [32,64] [debug, release]"
  exit /b 2
)

if "%type%" == "" (
  echo "Usage: release.bat [32,64] [debug, release]"
  exit /b 2
)

if "%bits%" == "32" (
   set bd=win-vs2012-i386
   set cmake_gen="Visual Studio 11"
   set cmake_opt="-DREMOXLY_USE_32BIT=1"
)

if "%type%" == "debug" (
   set bd="%bd%d"
   set cmake_bt="Debug"
)

if not exist "%d%\%bd%" (       
   mkdir %d%\%bd%
)


cd %d%\%bd%
cmake -DCMAKE_BUILD_TYPE=%cmake_bt% -G %cmake_gen% %cmake_opt% ..\
:: cmake --build . --target install --config %cmake_bt%
cmake --build .  
cd Debug
:: videocapture.exe
cd %d%

