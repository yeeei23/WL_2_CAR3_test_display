#!/bin/sh
# TOPST 보드용 Qt5 프로젝트 빌드 스크립트 (WSL + Telechips SDK 환경)
set -e

# 1. 경로 설정 (SYSROOT 미설정 시 $HOME/sdk 기준)
cd "$(dirname "$0")"
TARGET=test_spk_host
SYSROOT="${SYSROOT:-$HOME/sdk/sysroots/cortexa72-telechips-linux}"

# 2. MOC 설정 (PATH에서 moc 검색, SDK 내부 moc는 ARM용이라 호스트용을 사용)
MOC="${MOC:-$(command -v moc 2>/dev/null)}"
[ -z "$MOC" ] && MOC=$(command -v moc-qt5 2>/dev/null)

if [ -z "$MOC" ]; then
    echo "Error: MOC not found. Please run: sudo apt install qtbase5-dev-tools"
    exit 1
fi

# 3. 컴파일러 설정
# environment-setup을 source 했다면 $CXX에 복잡한 옵션이 이미 들어있습니다.
# 만약 변수가 비어있다면 기본 크로스 컴파일러명을 지정합니다.
CXX="${CXX:-aarch64-telechips-linux-g++}"

# 4. 인클루드 플래그 (중요!)
# 표준 라이브러리(stdlib.h)와 충돌을 피하기 위해 /usr/include 자체는 추가하지 않고
# Qt 세부 모듈만 콕 집어서 추가합니다.
INC_BASE="$SYSROOT/usr/include"

EXTRA_FLAGS="-DQT_OPENGL_ES_2 -DQT_NO_OPENGL_ES_3 \
    -I$INC_BASE/QtCore \
    -I$INC_BASE/QtGui \
    -I$INC_BASE/QtQuick \
    -I$INC_BASE/QtQml \
    -I$INC_BASE/QtNetwork \
    -I$INC_BASE/GLES2"
    
echo "=== TOPST Build Start ==="
echo "Using CXX: $CXX"
echo "Using MOC: $MOC"

# Step 1: MOC 생성
echo "Step 1: Generating MOC file..."
rm -f ./moc_TriggerServer.cpp
"$MOC" TriggerServer.h -o moc_TriggerServer.cpp

# Step 2: 개별 소스 파일 컴파일
# $CXX 변수 안에 이미 --sysroot 가 포함되어 있으므로 그대로 활용합니다.
echo "Step 2: Compiling..."
$CXX -std=c++11 -fPIC $EXTRA_FLAGS -c main.cpp -o main.o
$CXX -std=c++11 -fPIC $EXTRA_FLAGS -c TriggerServer.cpp -o TriggerServer.o
$CXX -std=c++11 -fPIC $EXTRA_FLAGS -c moc_TriggerServer.cpp -o moc_TriggerServer.o

# Step 3: 링크 (라이브러리 연결)
echo "Step 3: Linking..."
# 라이브러리 경로와 필요한 Qt5/GLES 모듈들을 연결합니다.
$CXX -o "$TARGET" main.o TriggerServer.o moc_TriggerServer.o \
    -L$SYSROOT/usr/lib \
    -lQt5Quick -lQt5Qml -lQt5Network -lQt5Gui -lQt5Core -lpthread -lGLESv2

echo "=== Build Completed Successfully! ==="
echo "Target: $(pwd)/$TARGET"
# 보드용 바이너리인지 확인 출력
file "$TARGET"