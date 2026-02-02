# 구성: C 신호 → C 소리 + Qt 화면

```
[ C: key_trigger 또는 다른 C 프로그램 ]
         │
         │  trigger_send_play_all("127.0.0.1")
         │  → 38473 + 38474 로 "PLAY" 전송
         ▼
    ┌────┴────┐
    │         │
    ▼         ▼
[ C: sound_trigger ]   [ Qt: test_spk_host ]
  포트 38473              포트 38474
  mpg123 alert.mp3       QML 창 "재생 중" 표시
```

## 역할

| 구성요소 | 언어 | 포트 | 역할 |
|----------|------|------|------|
| **trigger_send** | C | (클라이언트) | 다른 프로그램에 넣어 쓸 수 있는 "PLAY" 전송 모듈 |
| **key_trigger** | C | — | 키 입력 시 trigger_send 로 38473+38474 전송 |
| **sound_trigger** | C | 38473 | "PLAY" 수신 시 alert.mp3 재생 (mpg123) |
| **test_spk_host** | Qt | 38474 | "PLAY" 수신 시 QML 화면만 갱신 (소리 없음) |

## 실행 순서

1. **sound_trigger** 실행 (소리 재생 대기)
   ```bash
   cd /home/topst/test_spk
   ./sound_trigger/sound_trigger [경로/alert.mp3]
   ```
   기본 재생 파일: `./alert.mp3` 또는 환경변수 `TRIGGER_SOUND_FILE`

2. **Qt UI** 실행
   ```bash
   cd /home/topst/test_spk
   ./host/test_spk_host
   ```

3. **key_trigger** 실행 (또는 자신의 C 프로그램에서 trigger_send 호출)
   ```bash
   cd /home/topst/test_spk/key_trigger
   ./key_trigger
   ```
   키를 누르면 소리 + 화면 갱신이 동시에 발생.

## 빌드 (make 불필요: build.sh 사용)

**한 번에 전체 빌드:**

```bash
cd /home/topst/test_spk
./build.sh
```

**개별 빌드:**

- **trigger_send**: 별도 빌드 없음. key_trigger 빌드 시 함께 링크됨.
- **key_trigger**: `cd key_trigger && ./build.sh`
- **sound_trigger**: `cd sound_trigger && ./build.sh` (의존성: mpg123)
- **host (Qt)**: `cd host && ./build.sh`

## 다른 프로그램에 신호 넣기

`docs/trigger_send.md` 참고.  
`key_trigger/trigger_send.h` + `key_trigger/trigger_send.c` 복사 후, 이벤트 발생 시 `trigger_send_play_all("127.0.0.1")` 호출하면 됨.
