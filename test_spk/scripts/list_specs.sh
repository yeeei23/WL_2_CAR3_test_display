#!/bin/sh
# 사용 가능한 qmake 스펙 목록만 출력 (뭐가 있는지 확인할 때 사용)
QTDATA=$(qmake -query QT_HOST_DATA 2>/dev/null || true)
if [ -z "$QTDATA" ]; then
    echo "qmake에서 QT_HOST_DATA를 못 찾았습니다."
    exit 1
fi
MKSPECS="$QTDATA/mkspecs"
if [ ! -d "$MKSPECS" ]; then
    echo "mkspecs 폴더가 없습니다: $MKSPECS"
    echo "시스템에서 mkspecs 검색 중... (FIND_ROOT=${FIND_ROOT:-/usr})"
    find "${FIND_ROOT:-/usr}" -path "*mkspecs*" -type d 2>/dev/null | head -20
    exit 1
fi
echo "사용 가능한 qmake 스펙 (아래 이름 중 하나를 -spec 뒤에 쓰면 됨):"
echo "  예: qmake -spec <이름> test_spk_host.pro"
echo ""
ls -1 "$MKSPECS"
