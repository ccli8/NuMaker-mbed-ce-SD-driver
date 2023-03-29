# Copyright (c) 2020 ARM Limited. All rights reserved.
# SPDX-License-Identifier: Apache-2.0

add_library(nusd INTERFACE)

target_include_directories(nusd
    INTERFACE
        .
)

target_sources(nusd
    INTERFACE
        NuSDBlockDevice.cpp
        NuSDFlashSimBlockDevice.cpp
)
