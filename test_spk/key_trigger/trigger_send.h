/**
 * trigger_send — 다른 프로그램에 이식하기 쉬운 "PLAY" 신호 전송
 *
 * 사용: trigger_send.c를 프로젝트에 복사하고, trigger_send_play() 호출.
 * 의존성: 표준 C, POSIX (socket, inet_pton 등).
 *
 * 기본 포트:
 *   38473 — 소리 재생 (sound_trigger)
 *   38474 — Qt UI 갱신 (test_spk_ui)
 */

#ifndef TRIGGER_SOUND_PORT
#define TRIGGER_SOUND_PORT 38473
#endif
#ifndef TRIGGER_UI_PORT
#define TRIGGER_UI_PORT 38474
#endif

#ifndef TRIGGER_SEND_H
#define TRIGGER_SEND_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define TRIGGER_BINARY_SIZE 6  /* 거리32 + 방향8 + 위험8 bit = 6 bytes */

/** 바이너리: 거리(32bit) 방향(8bit) 위험등급(8bit) 순서. 방향='L'/'R'/'F'/'N' 등 ASCII. 성공 시 0 */
int trigger_send_binary(const char *host, int port, uint32_t distance, uint8_t direction, uint8_t danger_level);

/** "PLAY\n" 를 host:port 로 전송. 성공 시 0, 실패 시 -1 (상대가 꺼져 있어도 -1) */
int trigger_send_play(const char *host, int port);

/** "PLAY <value>\n" 를 host:port 로 전송. value < 0 이면 "PLAY\n" 만 전송. QML에 거리(m) 등 숫자 전달용 */
int trigger_send_play_value(const char *host, int port, int value);

/** 임의 메시지 문자열을 "\n" 붙여 host:port 로 전송. 예: "L 300 2" -> "L 300 2\n". 성공 시 0 */
int trigger_send_message(const char *host, int port, const char *message);

/** 기본 호스트(127.0.0.1)로 소리(38473) + UI(38474) 둘 다 전송. 성공한 개수 반환 (0~2) */
int trigger_send_play_all(const char *host);

/** sound_trigger 없이 직접 gst-play-1.0으로 재생 (fork). 경로는 절대경로 권장. 성공 시 0 */
int trigger_play_local(const char *sound_path);

#ifdef __cplusplus
}
#endif

#endif /* TRIGGER_SEND_H */
