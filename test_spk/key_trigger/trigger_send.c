/*
 * trigger_send — PLAY 신호 전송 구현.
 * 다른 C 프로그램에 이식 시: trigger_send.h + trigger_send.c 복사 후
 *   trigger_send_play("127.0.0.1", 38473);
 *   trigger_send_play("127.0.0.1", 38474);
 * 또는 trigger_send_play_all("127.0.0.1"); 한 번 호출.
 */

#define _POSIX_C_SOURCE 200809L

#include "trigger_send.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MSG_PLAY "PLAY\n"

int trigger_send_binary(const char *host, int port, uint32_t distance, uint8_t direction, uint8_t danger_level)
{
    int fd;
    struct sockaddr_in addr;
    ssize_t n;
    unsigned char buf[TRIGGER_BINARY_SIZE];

    if (!host || port <= 0)
        return -1;

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
        return -1;

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons((unsigned short)port);
    if (inet_pton(AF_INET, host, &addr.sin_addr) <= 0) {
        close(fd);
        return -1;
    }

    if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        close(fd);
        return -1;
    }

    /* 거리(32bit, network byte order) 방향(8bit) 위험등급(8bit) */
    uint32_t dist_be = htonl(distance);
    memcpy(buf, &dist_be, 4);
    buf[4] = direction;
    buf[5] = danger_level;

    n = send(fd, buf, TRIGGER_BINARY_SIZE, 0);
    close(fd);
    return (n == (ssize_t)TRIGGER_BINARY_SIZE) ? 0 : -1;
}

int trigger_send_play(const char *host, int port)
{
    int fd;
    struct sockaddr_in addr;
    ssize_t n;

    if (!host || port <= 0)
        return -1;

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
        return -1;

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons((unsigned short)port);
    if (inet_pton(AF_INET, host, &addr.sin_addr) <= 0) {
        close(fd);
        return -1;
    }

    if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        close(fd);
        return -1;
    }

    n = send(fd, MSG_PLAY, (size_t)(sizeof(MSG_PLAY) - 1), 0);
    close(fd);
    return (n == (ssize_t)(sizeof(MSG_PLAY) - 1)) ? 0 : -1;
}

int trigger_send_play_value(const char *host, int port, int value)
{
    int fd;
    struct sockaddr_in addr;
    ssize_t n;
    char buf[64];
    int len;

    if (!host || port <= 0)
        return -1;

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
        return -1;

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons((unsigned short)port);
    if (inet_pton(AF_INET, host, &addr.sin_addr) <= 0) {
        close(fd);
        return -1;
    }

    if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        close(fd);
        return -1;
    }

    if (value < 0)
        len = (int)snprintf(buf, sizeof(buf), "PLAY\n");
    else
        len = (int)snprintf(buf, sizeof(buf), "PLAY %d\n", value);
    if (len <= 0 || (size_t)len >= sizeof(buf)) {
        close(fd);
        return -1;
    }
    n = send(fd, buf, (size_t)len, 0);
    close(fd);
    return (n == (ssize_t)len) ? 0 : -1;
}

int trigger_send_message(const char *host, int port, const char *message)
{
    int fd;
    struct sockaddr_in addr;
    ssize_t n;
    char buf[256];
    int len;

    if (!host || port <= 0 || !message)
        return -1;

    len = (int)snprintf(buf, sizeof(buf), "%s\n", message);
    if (len <= 0 || (size_t)len >= sizeof(buf))
        return -1;

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
        return -1;

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons((unsigned short)port);
    if (inet_pton(AF_INET, host, &addr.sin_addr) <= 0) {
        close(fd);
        return -1;
    }

    if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        close(fd);
        return -1;
    }

    n = send(fd, buf, (size_t)len, 0);
    close(fd);
    return (n == (ssize_t)len) ? 0 : -1;
}

#ifndef TRIGGER_SOUND_PORT
#define TRIGGER_SOUND_PORT 38473
#endif
#ifndef TRIGGER_UI_PORT
#define TRIGGER_UI_PORT 38474
#endif

int trigger_send_play_all(const char *host)
{
    int n = 0;
    if (trigger_send_play(host, TRIGGER_SOUND_PORT) == 0)
        n++;
    if (trigger_send_play(host, TRIGGER_UI_PORT) == 0)
        n++;
    return n;
}

int trigger_play_local(const char *sound_path)
{
    pid_t pid;

    if (!sound_path || sound_path[0] == '\0')
        return -1;

    pid = fork();
    if (pid < 0)
        return -1;
    if (pid > 0) {
        waitpid(pid, NULL, WNOHANG);
        return 0;
    }
    /* child: PATH에서 gst-play-1.0 실행 (절대경로 의존 제거) */
    execlp("gst-play-1.0", "gst-play-1.0", "--no-interactive", sound_path, (char *)NULL);
    _exit(127);
}
