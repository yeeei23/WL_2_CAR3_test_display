/*
 * key_trigger: 한 줄 입력 시 "L 300 2" 형식(방향 거리 위험등급)을
 * 소리(38473) + Qt UI(38474)로 전송. 종료: Ctrl+C
 */

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include <unistd.h>
#include "trigger_send.h"

#define DEFAULT_HOST "127.0.0.1"
#define LINE_MAX 128

static void send_binary(uint8_t direction, uint32_t distance, uint8_t danger_level)
{
    trigger_send_binary(DEFAULT_HOST, TRIGGER_SOUND_PORT, distance, direction, danger_level);
    trigger_send_binary(DEFAULT_HOST, TRIGGER_UI_PORT, distance, direction, danger_level);
}

/* "L 100 3" 파싱 -> direction, distance, danger 반환. 성공 시 1, 실패 시 0 */
static int parse_line(const char *line, uint8_t *direction, uint32_t *distance, uint8_t *danger)
{
    unsigned int d1, d2;
    char dir;
    if (sscanf(line, " %c %u %u", &dir, &d1, &d2) != 3)
        return 0;
    dir = (char)toupper((unsigned char)dir);
    if (dir != 'L' && dir != 'R' && dir != 'F' && dir != 'N')
        return 0;
    if (d2 < 1 || d2 > 3)
        return 0;
    *direction = (uint8_t)dir;
    *distance = (uint32_t)d1;
    *danger = (uint8_t)d2;
    return 1;
}

int main(void)
{
    char line[LINE_MAX];
    int len = 0;

    printf("key_trigger: 한 줄 입력 후 엔터로 전송 (예: L 300 2). 종료: Ctrl+C\n");
    printf("형식: 방향(L/R/F/N) 거리(m) 위험등급(1~3). 바이너리(거리32+방향8+위험8bit) 전송.\n\n");
    fflush(stdout);

    while (fgets(line, (int)sizeof(line), stdin) != NULL) {
        len = (int)strlen(line);
        while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r'))
            line[--len] = '\0';
        if (len == 0)
            continue;
        {
            uint8_t dir, danger;
            uint32_t dist;
            if (parse_line(line, &dir, &dist, &danger))
                send_binary(dir, dist, danger);
        }
    }

    return 0;
}
