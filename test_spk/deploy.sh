#!/bin/sh
# TOPST 보드로 빌드 결과물 전송 및 권한 설정 스크립트
# sshpass로 비밀번호 'topst' 자동 입력 (미설치 시: sudo apt install sshpass)

# 1. 인자 확인 (TARGET_PATH 미설정 시 원격 홈 아래 test_spk)
TARGET_IP=$1
TARGET_PATH="${TARGET_PATH:-~/test_spk}"
REMOTE_USER="${REMOTE_USER:-topst}"  # 보드 로그인 계정 (환경변수 USER와 구분)
REMOTE_PASS="${REMOTE_PASS:-topst}"  # 보드 로그인 비밀번호

if [ -z "$TARGET_IP" ]; then
    echo "Usage: ./deploy.sh <IP_ADDRESS>"
    echo "Example: ./deploy.sh 192.168.137.24"
    exit 1
fi

# 2. sshpass 존재 확인
if ! command -v sshpass >/dev/null 2>&1; then
    echo "Error: 'sshpass' not found. Install with: sudo apt install sshpass"
    exit 1
fi

# 3. output 폴더 존재 확인
if [ ! -d "./output" ]; then
    echo "Error: 'output' directory not found. Please run ./build.sh first."
    exit 1
fi

echo "Connecting to $TARGET_IP..."

# 4. 보드에 목적지 디렉토리 생성, 배포 대상 3개 파일만 기존 것 삭제
export SSHPASS="$REMOTE_PASS"
sshpass -e ssh -o StrictHostKeyChecking=no ${REMOTE_USER}@${TARGET_IP} "mkdir -p ${TARGET_PATH} && rm -f ${TARGET_PATH}/key_trigger ${TARGET_PATH}/sound_trigger ${TARGET_PATH}/test_spk_host"

# 5. 파일 전송 (SCP)
echo "Sending files to ${TARGET_PATH}..."
sshpass -e scp -o StrictHostKeyChecking=no ./output/* ${REMOTE_USER}@${TARGET_IP}:${TARGET_PATH}/

# 6. 실행 권한 부여
echo "Setting execution permissions..."
sshpass -e ssh -o StrictHostKeyChecking=no ${REMOTE_USER}@${TARGET_IP} "chmod +x ${TARGET_PATH}/*"

echo "=== Deployment Completed! ==="
echo "Files are located at ${TARGET_IP}:${TARGET_PATH}"