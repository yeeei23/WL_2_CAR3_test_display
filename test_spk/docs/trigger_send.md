# trigger_send — 다른 프로그램에 이식하기 쉬운 신호 전송

C 프로그램에서 **"PLAY" 신호**를 소리(sound_trigger) / Qt UI 쪽으로 보낼 때 쓰는 작은 모듈입니다.  
다른 프로젝트에 **복사해서 붙이기**만 하면 됩니다.

## 필요한 파일 (위치: `key_trigger/`)

- `key_trigger/trigger_send.h`
- `key_trigger/trigger_send.c`

## API

```c
#include "trigger_send.h"

/* 한 곳으로만 전송 */
trigger_send_play("127.0.0.1", 38473);  /* 소리 */
trigger_send_play("127.0.0.1", 38474);  /* Qt UI */

/* Qt UI(38474)에 숫자와 함께 전송 → QML에 "400m", "300m" 등 표시. value < 0 이면 "PLAY\n"만 전송 */
trigger_send_play_value("127.0.0.1", 38474, 400);

/* 소리(38473) + Qt UI(38474) 둘 다 전송. 성공한 개수(0~2) 반환 */
int n = trigger_send_play_all("127.0.0.1");
```

## 이식 방법

1. `trigger_send.h`, `trigger_send.c` 를 자신의 프로젝트로 복사.
2. 소스에 `#include "trigger_send.h"` 추가.
3. 빌드 시 `trigger_send.c` 를 같이 컴파일.
4. 신호를 보내고 싶은 시점에 `trigger_send_play(host, port)` 또는 `trigger_send_play_all(host)` 호출.

예 (다른 프로그램 안에서):

```c
#include "trigger_send.h"

void on_some_event(void) {
    trigger_send_play_all("127.0.0.1");
}
```

## 포트

| 포트   | 용도           | 수신 프로그램   |
|--------|----------------|------------------|
| 38473  | 소리 재생      | sound_trigger     |
| 38474  | Qt UI 갱신     | test_spk_host(Qt) |

다른 포트를 쓰려면 `trigger_send.c` 에서  
`TRIGGER_SOUND_PORT`, `TRIGGER_UI_PORT` 를 컴파일 전에 정의하면 됩니다.
