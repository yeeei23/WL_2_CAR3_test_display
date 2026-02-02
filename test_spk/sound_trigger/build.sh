#!/bin/sh
cd "$(dirname "$0")"
CC="${CC:-gcc}"
command -v "$CC" >/dev/null 2>&1 || CC=cc
command -v "$CC" >/dev/null 2>&1 || {
    echo "gcc/cc not found. See README_BUILD_ELSEWHERE.md"
    exit 1
}
$CC ${CFLAGS:--Wall -Wextra -O2} -o sound_trigger sound_trigger.c
