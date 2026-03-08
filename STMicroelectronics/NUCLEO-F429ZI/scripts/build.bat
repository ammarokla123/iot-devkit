@echo off

REM Copyright (c) 2024 Eclipse Foundation
REM SPDX-License-Identifier: MIT

REM Build script for STM32 NUCLEO-F429ZI Eclipse ThreadX application

set BASEDIR=%~dp0..
set BUILDDIR=%BASEDIR%\build

echo Building NUCLEO-F429ZI Eclipse ThreadX application...

cmake -B"%BUILDDIR%" -GNinja ^
    -DCMAKE_TOOLCHAIN_FILE="%BASEDIR%\..\..\cmake\arm-gcc-cortex-m4.cmake" ^
    "%BASEDIR%"

cmake --build "%BUILDDIR%"

echo.
echo Build complete.
echo   ELF: %BUILDDIR%\app\nucleo_f429zi_threadx.elf
echo   BIN: %BUILDDIR%\app\nucleo_f429zi_threadx.bin
echo   HEX: %BUILDDIR%\app\nucleo_f429zi_threadx.hex
