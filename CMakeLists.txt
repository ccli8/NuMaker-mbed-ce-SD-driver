# Copyright (c) 2020 ARM Limited. All rights reserved.
# SPDX-License-Identifier: Apache-2.0

add_library(nusd STATIC EXCLUDE_FROM_ALL)

target_include_directories(nusd
    PUBLIC
        .
)

target_sources(nusd
    PRIVATE
        NuSDBlockDevice.cpp
        NuSDFlashSimBlockDevice.cpp
)

target_link_libraries(nusd PUBLIC mbed-storage-blockdevice)
