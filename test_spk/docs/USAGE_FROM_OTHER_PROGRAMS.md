# 다른 프로그램에서 QML 조종 / 소리 출력하기

다른 프로그램(자기 앱, 스크립트 등)에서 **test_spk**의 **QML UI**를 갱신하거나 **소리**를 재생하려면,  
**바이너리** 또는 **텍스트** 형식으로 TCP 전송하면 됩니다.

## 바이너리 형식 (권장)

**6바이트**: 거리(32bit, 빅엔디안) + 방향(8bit) + 위험등급(8bit) 순서.

| 오프셋 | 크기 | 설명 |
|--------|------|------|
| 0-3 | 32bit | 거리(m), network byte order |
| 4 | 8bit | 방향: 'L'(0x4C), 'R'(0x52), 'F'(0x46), 'N'(0x4E) |
| 5 | 8bit | 위험등급: 1~3 |

- **소리**: `127.0.0.1:38473` 으로 6바이트 전송
- **QML**: `127.0.0.1:38474` 로 6바이트 전송

## 텍스트 형식 (호환)

- **소리 재생**: `127.0.0.1:38473` 으로 "PLAY\n" 전송 → sound_trigger가 alert.mp3 재생
- **QML 조종(화면 갱신)**: `127.0.0.1:38474` 로 "PLAY\n" 또는 **"L 400 2\n"** 전송 → test_spk_host가 QML 갱신.

**사전 조건**:  
실행 중이어야 할 것 — `./sound_trigger/sound_trigger` (소리용), `./host/test_spk_host` (QML용).

---

## 1. C/C++ — trigger_send API 사용 (권장)

`key_trigger/trigger_send.h` 와 `key_trigger/trigger_send.c` 를 자기 프로젝트에 복사한 뒤, 아래처럼 호출하면 됩니다.

### 소리만 재생

```c
#include "trigger_send.h"

/* 소리만: sound_trigger(38473)에 PLAY 전송 */
void play_sound(void) {
    trigger_send_play("127.0.0.1", 38473);
}
```

### QML만 갱신 (화면만 조종)

```c
#include "trigger_send.h"

/* QML만: test_spk_host(38474)에 PLAY 전송 → QML "재생 중" 등 갱신 */
void refresh_qml_ui(void) {
    trigger_send_play("127.0.0.1", 38474);
}

/* QML에 숫자(거리 m)까지 전달 → 화면에 "400m", "300m" 등 표시 */
void refresh_qml_ui_with_distance(int meters) {
    trigger_send_play_value("127.0.0.1", 38474, meters);
}
```

### 소리 + QML 동시

```c
#include "trigger_send.h"

/* 소리(38473) + QML(38474) 둘 다 전송. 성공한 개수(0~2) 반환 */
void play_and_refresh_ui(void) {
    int n = trigger_send_play_all("127.0.0.1");
    (void)n;  /* 0=둘 다 꺼짐, 1=하나만 수신, 2=둘 다 수신 */
}
```

### 빌드

- 소스에 `#include "trigger_send.h"` 추가
- `trigger_send.c` 를 같이 컴파일·링크

```bash
gcc -o my_app my_app.c trigger_send.c
```

---

## 2. C/C++ — 소켓으로 직접 보내기

trigger_send를 쓰지 않고, 직접 TCP로 "PLAY\n" 만 보내도 됩니다.

```c
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

void send_play(int port) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_port   = htons((unsigned short)port),
    };
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);

    if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) == 0) {
        send(fd, "PLAY\n", 5, 0);
    }
    close(fd);
}

/* 소리 */
void play_sound(void)   { send_play(38473); }

/* QML UI 갱신 */
void refresh_qml(void)  { send_play(38474); }
```

---

## 3. Python

```python
import socket

def send_play(port):
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        try:
            s.connect(("127.0.0.1", port))
            s.sendall(b"PLAY\n")
        except OSError:
            pass

# 소리 재생
def play_sound():
    send_play(38473)

# QML UI 갱신
def refresh_qml():
    send_play(38474)

# 둘 다
def play_and_refresh():
    send_play(38473)
    send_play(38474)
```

---

## 4. 셸 (bash / nc)

```bash
# 소리만
echo -n "PLAY" | nc -q0 127.0.0.1 38473

# QML만
echo -n "PLAY" | nc -q0 127.0.0.1 38474

# 둘 다 (한 줄에)
( echo -n "PLAY"; echo -n "PLAY" ) | ( nc -q0 127.0.0.1 38473 & nc -q0 127.0.0.1 38474 )
```

`printf "PLAY\n" | nc ...` 도 가능합니다. 프로토콜은 **"PLAY\n"** (5바이트) 전송입니다.

---

## 5. QML에 숫자(거리 m) 보내기

QML 쪽(38474)으로 **"PLAY 숫자\n"** 를 보내면, 화면에 **"400m"**, **"300m"** 처럼 그 숫자가 표시됩니다.

- **프로토콜**: `"PLAY\n"` → 숫자 없음, QML은 "재생 중..." 표시. `"PLAY 400\n"` → QML은 "400m" 표시.
- **C API**: `trigger_send_play_value("127.0.0.1", 38474, 400);` — value < 0 이면 "PLAY\n"만 전송.

```c
/* 거리 400m 표시 */
trigger_send_play_value("127.0.0.1", 38474, 400);

/* 거리 300m 표시 */
trigger_send_play_value("127.0.0.1", 38474, 300);
```

Python/셸에서 직접 보낼 때: `s.sendall(b"PLAY 400\n")` 또는 `echo "PLAY 400" | nc ...`

---

## 6. 포트 정리

| 포트  | 용도           | 수신 쪽              | 호출 시 동작           |
|-------|----------------|----------------------|------------------------|
| 38473 | 소리 재생      | sound_trigger        | alert.mp3 재생         |
| 38474 | QML 조종/갱신  | test_spk_host (Qt)   | QML 화면(재생 중 / 400m 등) 갱신 |

다른 호스트에서 제어하려면 `127.0.0.1` 대신 해당 호스트 IP를 쓰면 됩니다 (같은 기기면 127.0.0.1).

---

## 7. 정리

- **QML 조종**: `trigger_send_play("127.0.0.1", 38474)` 또는 위와 같이 38474로 "PLAY\n" 전송.
- **소리 출력**: `trigger_send_play("127.0.0.1", 38473)` 또는 38473으로 "PLAY\n" 전송.
- **둘 다**: `trigger_send_play_all("127.0.0.1")` 한 번 호출.

C/C++에서는 `trigger_send.h` + `trigger_send.c` 를 복사해 쓰는 방식이 가장 간단합니다.
