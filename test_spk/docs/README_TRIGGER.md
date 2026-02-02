# 외부 C 프로그램에서 신호 → 소리 + Qt 화면

C 프로그램에서 **"PLAY" 신호**를 보내면 **C(sound_trigger)**가 소리를 재생하고 **Qt(test_spk_host)**가 화면만 갱신하는 구성입니다.

## 현재 구조 (3개 프로그램)

```
[ C: key_trigger 또는 trigger_send를 쓴 다른 프로그램 ]
         │  "PLAY" → 38473 + 38474
    ┌────┴────┐
    ▼         ▼
[ C: sound_trigger ]   [ Qt: test_spk_host ]
  포트 38473              포트 38474
  mpg123 alert.mp3       QML "재생 중" 표시
```

- **trigger_send**: 다른 C 프로그램에 이식하기 쉬운 "PLAY" 전송 모듈 (`key_trigger/` 안의 trigger_send.c, trigger_send.h).
- **key_trigger**: 키 입력 시 trigger_send로 38473(소리) + 38474(Qt UI)에 전송.
- **sound_trigger**: C. 포트 38473에서 "PLAY" 수신 시 `mpg123 alert.mp3` 실행.
- **test_spk_host**: Qt. 포트 38474에서 "PLAY" 수신 시 QML 화면만 갱신 (소리 없음).

자세한 구성·실행 순서는 **docs/README_ARCHITECTURE.md** 참고.

### 빌드 요약 (Yocto 등 make 미사용 환경: build.sh 사용)

**한 번에 전체 빌드:**

```bash
cd /home/topst/test_spk
./build.sh
```

**개별 빌드:**

| 대상 | 경로 | 명령 |
|------|------|------|
| key_trigger | key_trigger/ | `./build.sh` |
| sound_trigger | sound_trigger/ | `./build.sh` (실행 시 **mpg123** 필요) |
| Qt UI (host) | host/ | `./build.sh` |

(make가 있는 환경에서는 각 디렉터리에서 `make` 사용 가능)

### 실행 순서

1. `./sound_trigger/sound_trigger` (test_spk 디렉터리에서, alert.mp3 경로 지정 가능)
2. `./host/test_spk_host`
3. `./key_trigger/key_trigger` (키 입력 시 소리+화면 동시 반응)

## 요구 사항

- Qt5 (Quick, Gui, Qml, **Network**, Multimedia 플러그인)
- C 컴파일러 (gcc/clang)
- `test_spk` 디렉터리에 `alert.mp3`, `test_spk.qml` 존재

## 1. Qt 호스트 빌드

**build.sh (권장, make 불필요, Yocto 등):**

```bash
cd /home/topst/test_spk/host
./build.sh
```

(make가 있으면: qmake 또는 `make -f Makefile.gcc`)

**`Could not find qmake spec 'linux-g++'` 오류가 나는 경우**  
(내장/크로스 환경 등에서 mkspec이 다를 때)

**1) 사용 가능한 스펙이 뭐가 있는지 확인하는 방법**

아래 중 하나만 실행하면 됩니다.

```bash
# 방법 A: qmake가 아는 mkspecs 폴더 바로 열기
qmake -query QT_HOST_DATA
```
위 명령 출력이 예를 들어 `/usr/lib`이면, 그 아래에 `mkspecs` 폴더가 있는지 봅니다:
```bash
ls /usr/lib/mkspecs
```
(다른 경로가 나왔으면 그 경로 기준으로 `.../mkspecs` 를 열어보면 됩니다.)

```bash
# 방법 B: 한 번에 목록만 보기 (host 디렉터리에서)
cd /home/topst/test_spk/host
../scripts/list_specs.sh
```

나온 이름 중 하나(예: `linux-aarch64-gnu-g++`)를 골라서 **2)**처럼 쓰면 됩니다.

**2) 스펙 이름을 지정해서 빌드**

(위에서 본 이름으로 `linux-???-g++` 형태를 넣으면 됩니다.)
```bash
qmake -spec linux-aarch64-gnu-g++ test_spk_host.pro
make
```
**CMake 사용 (make 필요):**

```bash
cd /home/topst/test_spk/host
mkdir -p build && cd build
cmake ..
make
```

**make가 있는 환경:** `make -f Makefile.gcc` 또는 qmake + make

실행 파일: `host/test_spk_host`

## 2. C 키 트리거 빌드

**build.sh (권장, make 불필요):**

```bash
cd /home/topst/test_spk/key_trigger
./build.sh
```

make가 있으면: `make`

**`gcc: command not found` 인 경우** — 이 머신에는 C 컴파일러가 없습니다. **다른 PC나 WSL**에서 빌드한 뒤 실행 파일만 복사해서 쓰면 됩니다.

```bash
# 빌드 가능한 환경(PC, WSL 등)에서:
gcc -Wall -O2 -o key_trigger trigger.c
# 생성된 key_trigger 파일을 이 머신의 test_spk/key_trigger/ 로 복사 (scp, USB 등)
```

자세한 절차는 `docs/README_BUILD_ELSEWHERE.md` 참고.

실행 파일: `key_trigger/key_trigger`

## 3. 실행 방법

**순서:**

1. **sound_trigger** 실행 (포트 38473, 소리 재생)
   ```bash
   cd /home/topst/test_spk
   ./sound_trigger/sound_trigger [경로/alert.mp3]
   ```

2. **Qt UI** 실행 (포트 38474, 화면 갱신)
   ```bash
   cd /home/topst/test_spk
   ./host/test_spk_host
   ```

3. **key_trigger** 실행
   ```bash
   cd /home/topst/test_spk/key_trigger
   ./key_trigger
   ```
   키를 누르면 소리 + Qt 화면이 동시에 반응합니다. 종료: **Ctrl+C**

## 포트

- 기본 포트: **38473** (localhost만 사용)
- 변경하려면:
  - 호스트: `host/main.cpp`의 `triggerPort`, `host/TriggerServer.cpp` 참고
  - C 쪽: `key_trigger/trigger.c`의 `SERVER_PORT` 매크로

## 다른 C 프로그램에서 트리거하는 방법

동일 포트로 `PLAY` 문자열만 보내면 됩니다.

```c
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

void play_alert(void) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr = { .sin_family = AF_INET, .sin_port = htons(38473) };
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
    if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) == 0) {
        send(fd, "PLAY\n", 5, 0);
        close(fd);
    }
}
```

호스트(`test_spk_host`)가 먼저 떠 있어야 합니다.
