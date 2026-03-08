#!/bin/bash

# Copyright (c) 2024 Eclipse Foundation
# SPDX-License-Identifier: MIT

# Build script for STM32 NUCLEO-F429ZI Eclipse ThreadX application

set -e

BASEDIR=$(dirname "$(realpath "$0")")/..
BUILDDIR="${BASEDIR}/build"

echo "Building NUCLEO-F429ZI Eclipse ThreadX application..."
echo "Source: ${BASEDIR}"
echo "Build:  ${BUILDDIR}"

cmake -B"${BUILDDIR}" -GNinja \
    -DCMAKE_TOOLCHAIN_FILE="${BASEDIR}/../../cmake/arm-gcc-cortex-m4.cmake" \
    "${BASEDIR}"

cmake --build "${BUILDDIR}"

echo ""
echo "Build complete. Output:"
echo "  ELF: ${BUILDDIR}/app/nucleo_f429zi_threadx.elf"
echo "  BIN: ${BUILDDIR}/app/nucleo_f429zi_threadx.bin"
echo "  HEX: ${BUILDDIR}/app/nucleo_f429zi_threadx.hex"
