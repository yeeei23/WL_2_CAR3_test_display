#!/bin/sh
# make 없이 gcc로 빌드 (trigger_send 모듈 링크)
cd "$(dirname "$0")"
CC="${CC:-gcc}"
command -v "$CC" >/dev/null 2>&1 || CC=cc
command -v "$CC" >/dev/null 2>&1 || {
    echo "gcc/cc not found. See README_BUILD_ELSEWHERE.md"
    exit 1
}
CFLAGS="${CFLAGS:--Wall -Wextra -O2}"
$CC $CFLAGS -o key_trigger trigger.c trigger_send.c
