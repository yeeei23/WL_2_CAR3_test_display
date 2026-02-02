# 다른 곳에서 빌드해서 여기(test_spk)에 넣는 방법

host와 key_trigger를 **빌드 가능한 다른 환경**(PC, 빌드 서버 등)에서 빌드한 뒤, **실행할 보드/디렉터리**(여기, test_spk)로 실행 파일만 가져와서 쓰는 과정입니다.

---

## 로컬에서 gcc/g++로 바로 빌드하고 싶을 때

**여기(test_spk가 있는 머신)**에 **gcc/g++**와 **Qt5 개발 패키지**가 있으면, 다른 곳에서 빌드할 필요 없이 로컬에서 바로 빌드할 수 있습니다.

**build.sh 사용 (make 불필요, Yocto 등):**

```bash
cd /home/topst/test_spk
./build.sh
```

key_trigger, sound_trigger, host 가 한 번에 빌드됩니다. 개별 빌드는 `key_trigger/build.sh`, `sound_trigger/build.sh`, `host/build.sh` 를 각각 실행하면 됩니다.

로컬에 Qt 개발 패키지가 없거나, 아키텍처가 달라서(예: x86에서 빌드해 ARM 보드에 넣기) 로컬 빌드가 어렵다면, 아래처럼 다른 곳에서 빌드한 뒤 실행 파일만 복사하면 됩니다.

---

## 전체 흐름

1. **빌드하는 쪽**: 소스 복사 → host / key_trigger 빌드 → 실행 파일만 확인  
2. **넣는 쪽(여기)**: 실행 파일을 정해진 위치에 복사  
3. **실행**: test_spk 디렉터리에서 호스트 실행, 다른 터미널에서 key_trigger 실행  

---

## 1단계: 빌드하는 쪽(다른 환경)에서 할 일

### 1-1. 소스 가져오기

이 프로젝트의 아래 디렉터리/파일을 빌드 머신으로 복사합니다.

- **host 쪽**  
  - `host/main.cpp`  
  - `host/TriggerServer.cpp`  
  - `host/TriggerServer.h`  
  - `host/test_spk_host.pro` (qmake 사용 시)  
  - 또는 `host/CMakeLists.txt` (cmake 사용 시)

- **key_trigger 쪽**  
  - `key_trigger/trigger.c`  
  - `key_trigger/Makefile`  

(스크립트 `build.sh`, `scripts/list_specs.sh`는 빌드 시 선택 사항.)

### 1-2. 전체 빌드 (build.sh, make 불필요)

```bash
cd /path/to/test_spk
./build.sh
```

key_trigger, sound_trigger, host 가 순서대로 빌드됩니다.

**개별 빌드:**

- **host**: `cd host && ./build.sh` → `host/test_spk_host`
- **key_trigger**: `cd key_trigger && ./build.sh` → `key_trigger/key_trigger`
- **sound_trigger**: `cd sound_trigger && ./build.sh` → `sound_trigger/sound_trigger`

### 1-3. 가져갈 파일 확인

빌드가 끝나면 아래 두 개를 "여기"로 복사할 대상입니다.

| 빌드 결과물                | 복사할 때 이름(권장) |
|----------------------------|----------------------|
| host 쪽 실행 파일          | `test_spk_host`      |
| key_trigger 쪽 실행 파일   | `key_trigger`       |
| sound_trigger 쪽 실행 파일 | `sound_trigger`      |

---

## 2단계: 여기(test_spk)에 넣기

**실행할 쪽** 디렉터리 구조가 아래와 같다고 가정합니다.

```
/home/topst/test_spk/
├── test_spk.qml
├── alert.mp3
├── host/
│   └── test_spk_host      ← host 실행 파일
├── key_trigger/
│   └── key_trigger        ← key_trigger 실행 파일
└── sound_trigger/
    └── sound_trigger      ← sound_trigger 실행 파일
```

### 해야 할 일

1. **host 실행 파일 넣기**  
   - 빌드 머신에서 만든 `test_spk_host`를 **여기**의 `host/` 안에 둡니다.  
   - 경로 예: `.../test_spk/host/test_spk_host`

2. **key_trigger 실행 파일 넣기**  
   - `key_trigger`를 **여기**의 `key_trigger/` 안에 둡니다.

3. **sound_trigger 실행 파일 넣기**  
   - `sound_trigger`를 **여기**의 `sound_trigger/` 안에 둡니다.

4. **실행 권한**  
   - 보드/리눅스에서 한 번만 실행:
     ```bash
     chmod +x /home/topst/test_spk/host/test_spk_host
     chmod +x /home/topst/test_spk/key_trigger/key_trigger
     chmod +x /home/topst/test_spk/sound_trigger/sound_trigger
     ```

복사 방법은 상황에 맞게 쓰면 됩니다 (USB, scp, rsync 등).

- 예 (scp로 보드에 넣는 경우):
  ```bash
  scp host/test_spk_host 사용자@보드IP:/home/topst/test_spk/host/
  scp key_trigger/key_trigger 사용자@보드IP:/home/topst/test_spk/key_trigger/
  scp sound_trigger/sound_trigger 사용자@보드IP:/home/topst/test_spk/sound_trigger/
  ```

---

## 3단계: 여기(test_spk)에서 실행

실행은 **항상 test_spk 디렉터리가 작업 디렉터리**인 상태에서 합니다. (test_spk.qml, alert.mp3를 찾기 위해)

1. **호스트(QML 창) 실행**

   ```bash
   cd /home/topst/test_spk
   ./host/test_spk_host
   ```

   QML 창이 떠 있고, 포트 38473에서 대기합니다.

2. **다른 터미널에서 키 트리거 실행**

   ```bash
   cd /home/topst/test_spk/key_trigger
   ./key_trigger
   ```

   이 터미널에서 키를 누르면 QML 쪽에서 alert.mp3가 재생됩니다. 종료는 Ctrl+C.

---

## 정리 체크리스트

| 단계 | 할 일 |
|------|--------|
| 빌드 쪽 | host 소스로 `test_spk_host` 빌드 |
| 빌드 쪽 | key_trigger 소스로 `key_trigger` 빌드 |
| 넣는 쪽 | `test_spk_host` → `test_spk/host/test_spk_host` 로 복사 |
| 넣는 쪽 | `key_trigger` → `test_spk/key_trigger/key_trigger` 로 복사 |
| 넣는 쪽 | `chmod +x` 로 실행 권한 부여 |
| 실행 | `cd test_spk` 후 `./host/test_spk_host` 실행 |
| 실행 | 다른 터미널에서 `./key_trigger/key_trigger` 실행 |

이 순서대로 하면 "다른 곳에서 빌드해서 여기에 넣고" 실행하는 과정을 수행할 수 있습니다.
