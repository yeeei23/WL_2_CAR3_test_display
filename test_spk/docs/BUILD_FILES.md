# 빌드에 포함되는 파일

`./build.sh` 실행 시 어떤 소스/헤더가 컴파일·링크되는지 정리한 문서입니다.

---

## 1. key_trigger (바이너리: `output/key_trigger`)

| 역할 | 경로 | 비고 |
|------|------|------|
| 소스 (컴파일) | `key_trigger/trigger.c` | 진입점, `trigger_send_play_all()` 호출 |
| 소스 (컴파일·링크) | `key_trigger/trigger_send.c` | PLAY 신호 전송 구현 |
| 헤더 (include) | `key_trigger/trigger_send.h` | trigger.c, trigger_send.c에서 사용 |
| 빌드 스크립트 | `key_trigger/Makefile` 또는 `key_trigger/build.sh` | 루트 build.sh는 Makefile 사용 |

- **결과**: `trigger.c` + `trigger_send.c` 한 번에 링크 → `key_trigger`

---

## 2. sound_trigger (바이너리: `output/sound_trigger`)

| 역할 | 경로 | 비고 |
|------|------|------|
| 소스 (컴파일·링크) | `sound_trigger/sound_trigger.c` | 단일 소스, trigger_send 미사용 |
| 빌드 스크립트 | `sound_trigger/Makefile` 또는 `sound_trigger/build.sh` | 루트 build.sh는 Makefile 사용 |

- **결과**: `sound_trigger.c` 단일 컴파일 → `sound_trigger`

---

## 3. host (Qt, 바이너리: `output/test_spk_host`)

| 역할 | 경로 | 비고 |
|------|------|------|
| 소스 (컴파일) | `host/main.cpp` | QML 로드, TriggerServer 사용 |
| 소스 (컴파일) | `host/TriggerServer.cpp` | TCP 서버, PLAY 수신 |
| 헤더 (include + MOC 입력) | `host/TriggerServer.h` | main.cpp, TriggerServer.cpp에서 include; MOC가 이 파일에서 moc_TriggerServer.cpp 생성 |
| 생성 파일 (MOC) | `host/moc_TriggerServer.cpp` | build 시 `moc TriggerServer.h`로 생성, 그다음 컴파일 |
| 빌드 스크립트 | `host/build.sh` | 루트 build.sh가 이 스크립트 호출 |

- **컴파일 순서**: MOC로 `moc_TriggerServer.cpp` 생성 → main.cpp, TriggerServer.cpp, moc_TriggerServer.cpp 컴파일 → 링크
- **결과**: `main.o` + `TriggerServer.o` + `moc_TriggerServer.o` → `test_spk_host`

---

## 4. 루트 빌드 스크립트

| 역할 | 경로 |
|------|------|
| 전체 빌드·수집 | `build.sh` |

- key_trigger → make (Makefile)
- sound_trigger → make (Makefile)
- host → `host/build.sh` 실행
- 최종 바이너리 복사: `output/key_trigger`, `output/sound_trigger`, `output/test_spk_host`

---

## 5. 요약: 소스/헤더만 (생성물 제외)

```
key_trigger     : key_trigger/trigger.c
                + key_trigger/trigger_send.c
                + key_trigger/trigger_send.h (include)

sound_trigger  : sound_trigger/sound_trigger.c

host           : host/main.cpp
                + host/TriggerServer.cpp
                + host/TriggerServer.h (include + MOC 입력)
                + host/moc_TriggerServer.cpp (빌드 시 MOC가 생성)
```

**trigger_send** 모듈(trigger_send.c, trigger_send.h)은 **key_trigger** 디렉터리 안에 있으며, key_trigger 빌드에만 포함됩니다.

**실행 시에만 참조**되고 빌드에는 포함되지 않는 파일:
- `test_spk.qml` — test_spk_host 실행 시 QML 엔진이 로드
- `alert.mp3` — sound_trigger 실행 시 재생 파일로 사용
