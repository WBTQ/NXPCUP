
# Copyright 2026 NXP
#
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.22.0)

include(${SdkRootDirPath}/cmake/extension/mcux.cmake)

project(nxpcup LANGUAGES C CXX ASM PROJECT_BOARD_PORT_PATH ${board_root}/${board}/demo_apps/led_blinky)


mcux_add_include(INCLUDES .)
include(board/CMakeLists.txt)
include(${SdkRootDirPath}/CMakeLists.txt)


mcux_add_source(
    SOURCES main.c hardware_init.c control_loop.c pixy.c
)
#include(${SdkRootDirPath}/${board_root}/${board}/demo_apps/led_blinky/reconfig.cmake OPTIONAL)

mcux_convert_binary(BINARY ${APPLICATION_BINARY_DIR}/${MCUX_SDK_PROJECT_NAME}.bin)
