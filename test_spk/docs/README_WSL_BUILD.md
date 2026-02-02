# WSL에서 빌드해서 보드에 넣는 방법

WSL(Windows Subsystem for Linux)에 **gcc, make, Qt 개발 환경**을 설치하고, **host**와 **key_trigger**를 빌드한 뒤 보드(d3-g-topst-main 등)로 복사해서 쓰는 절차입니다.

---

## 1. WSL에서 필요한 패키지 설치

WSL 터미널(Ubuntu/Debian 기준)을 열고 아래를 실행합니다.

### 1-1. 기본 빌드 도구 + key_trigger용

```bash
sudo apt update
sudo apt install -y build-essential
```

- **build-essential**: gcc, g++, **make** 등 포함
- 이만 있으면 **key_trigger**, **sound_trigger**는 빌드 가능

**make만** 따로 설치하려면:

```bash
sudo apt install -y make
```

### 1-2. host(Qt) 빌드용 (선택)

**host**(test_spk_host)까지 WSL에서 빌드하려면 Qt5 개발 패키지를 추가로 설치합니다.

```bash
sudo apt install -y \
  qtbase5-dev \
  qtdeclarative5-dev \
  qtmultimedia5-dev \
  libqt5network5 \
  qtquickcontrols2-5-dev
```

- **qtbase5-dev**: Qt5 Core, Gui, Network, qmake, moc 등
- **qtdeclarative5-dev**: Qml, Quick
- **qtmultimedia5-dev**: QtMultimedia (QML에서 MediaPlayer 사용)
- **qtquickcontrols2-5-dev**: QML UI (필요 시)

배포판에 따라 패키지 이름이 다를 수 있습니다.

- Ubuntu 20.04/22.04: 위 이름 그대로 시도
- `qtbase5-dev`가 없으면: `qt5-qmake` + `qtbase5-dev` 검색 후 해당 패키지 설치

---

## 2. WSL에서 빌드하기

프로젝트 경로가 WSL 안에 있다고 가정합니다 (예: `/home/topst/test_spk` 또는 `/mnt/c/.../test_spk`).

### 2-1. 전체 빌드 (권장)

```bash
cd /home/topst/test_spk
./build.sh
```

key_trigger, sound_trigger, host 가 순서대로 빌드됩니다.

### 2-2. 개별 빌드

**key_trigger:**

```bash
cd /home/topst/test_spk/key_trigger
./build.sh
```

**sound_trigger:**

```bash
cd /home/topst/test_spk/sound_trigger
./build.sh
```

**host (Qt):**

```bash
cd /home/topst/test_spk/host
./build.sh
```

생성 파일: **`key_trigger/key_trigger`**, **`sound_trigger/sound_trigger`**, **`host/test_spk_host`**  
(실제 배포 시에는 **`output/`** 폴더에 모입니다)

---

## 3. 보드로 복사하기

WSL에서 빌드한 실행 파일을 보드로 보낼 때는 **WSL 터미널에서 scp**로 보내면 됩니다.

### 3-1. 보드 IP 확인

보드에서 `ip addr` 또는 `hostname -I`로 IP를 확인해 두세요 (예: 192.168.1.100).

### 3-2. scp로 복사

WSL 터미널에서:

```bash
cd /home/topst/test_spk

# 보드 사용자명과 IP를 실제 값으로 바세요
BOARD_USER=topst
BOARD_IP=192.168.1.100

# output 폴더에서 한 번에 복사 (권장)
scp output/test_spk_host ${BOARD_USER}@${BOARD_IP}:/home/topst/test_spk/host/
scp output/key_trigger ${BOARD_USER}@${BOARD_IP}:/home/topst/test_spk/key_trigger/
scp output/sound_trigger ${BOARD_USER}@${BOARD_IP}:/home/topst/test_spk/sound_trigger/
```

### 3-3. 보드에서 실행 권한 부여 후 실행

보드 쪽 터미널에서:

```bash
chmod +x /home/topst/test_spk/host/test_spk_host
chmod +x /home/topst/test_spk/key_trigger/key_trigger
chmod +x /home/topst/test_spk/sound_trigger/sound_trigger
```

실행:

```bash
cd /home/topst/test_spk
./host/test_spk_host
```

다른 터미널에서:

```bash
cd /home/topst/test_spk/key_trigger
./key_trigger
```

---

## 4. 한 번에 정리 (WSL에서 할 일만)

```bash
# 1) 패키지 설치 (최초 1회)
sudo apt update
sudo apt install -y build-essential
sudo apt install -y qtbase5-dev qtdeclarative5-dev qtmultimedia5-dev

# 2) 빌드 (make 불필요)
cd /home/topst/test_spk
./build.sh

# 3) 보드로 복사 (BOARD_IP 등은 실제 값으로)
scp output/test_spk_host topst@192.168.1.100:/home/topst/test_spk/host/
scp output/key_trigger topst@192.168.1.100:/home/topst/test_spk/key_trigger/
scp output/sound_trigger topst@192.168.1.100:/home/topst/test_spk/sound_trigger/
```

---

## 5. 주의사항

- **아키텍처**: WSL은 보통 x86_64(amd64)입니다. 보드가 **aarch64(ARM64)** 이면 WSL에서 빌드한 실행 파일은 보드에서 **실행되지 않습니다**. 그때는 보드와 같은 아키텍처로 빌드해야 합니다.
  - **보드가 ARM(aarch64)** 인 경우: PC에서 **크로스 컴파일**하거나, Yocto/빌드 서버에서 빌드한 실행 파일을 복사해야 합니다.
  - **보드가 x86_64** 인 경우: WSL에서 빌드한 것을 그대로 복사해서 사용하면 됩니다.

- **Qt 플러그인**: 보드에는 **실행용** Qt만 있어도 됩니다 (qtbase, qtmultimedia 등). WSL에서 빌드할 때 쓰는 **개발 패키지**는 보드에 설치할 필요 없습니다.

이 순서대로 하면 WSL에서 도구를 설치하고 빌드한 뒤, 보드에 복사해서 사용할 수 있습니다.
