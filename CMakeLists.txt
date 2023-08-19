# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(image-compression)

target_sources(app PRIVATE
    src/main.c
    src/fifo.h
    src/fifo.c
    src/img.h
    src/img.c
)
