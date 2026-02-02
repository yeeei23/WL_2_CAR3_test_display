#!/bin/sh
# test_spk 전체 빌드 및 결과물 수집 스크립트
set -e
ROOT="$(cd "$(dirname "$0")" && pwd)"
OUTPUT="$ROOT/output"

# 1. 크로스 컴파일 환경 설정 (SYSROOT 미설정 시 $HOME/sdk 기준)
export CC="${CC:-aarch64-telechips-linux-gcc}"
export CXX="${CXX:-aarch64-telechips-linux-g++}"
export SYSROOT="${SYSROOT:-$HOME/sdk/sysroots/cortexa72-telechips-linux}"
export CFLAGS="--sysroot=$SYSROOT -Wall -O2 -D_DEFAULT_SOURCE"

# output 폴더 초기화
rm -rf "$OUTPUT"
mkdir -p "$OUTPUT"

echo "=== test_spk build (Cross-compile for TOPST) ==="

# 2. key_trigger 빌드
echo "--- key_trigger ---"
cd "$ROOT/key_trigger"
make clean && make CC="$CC" CFLAGS="$CFLAGS"
cp key_trigger "$OUTPUT/"

# 3. sound_trigger 빌드
echo "--- sound_trigger ---"
cd "$ROOT/sound_trigger"
make clean && make CC="$CC" CFLAGS="$CFLAGS"
cp sound_trigger "$OUTPUT/"

# 4. host (Qt) 빌드
echo "--- host (Qt) ---"
cd "$ROOT/host"
# host/build.sh 내부에서 TARGET=test_spk_host 로 생성됨
sh "./build.sh"
cp test_spk_host "$OUTPUT/"

echo "=== All builds done! ==="
echo "Results are in: $OUTPUT"
ls -l "$OUTPUT"

# 최종 아키텍처 확인
echo "--- Architecture Check ---"
file "$OUTPUT"/*