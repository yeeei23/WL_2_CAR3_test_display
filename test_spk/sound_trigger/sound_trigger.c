/*
 * sound_trigger: 포트 38473에서 "PLAY" 수신 시 alert.mp3 재생.
 * gst-play-1.0 사용 (mpg123 ALSA 미지원 보드 대응).
 * 사용: ./sound_trigger [경로/alert.mp3]
 * 환경변수 TRIGGER_SOUND_FILE 으로 파일 지정 가능.
 * 의존성: gst-play-1.0 (GStreamer)
 */

#define _POSIX_C_SOURCE 200809L

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>

#define LISTEN_PORT 38473
#define BUF_SIZE    64
#define BINARY_SIZE 6   /* 거리32 + 방향8 + 위험8 bit */
#define MSG_PLAY    "PLAY"

/* 바이너리(6바이트): 거리32(빅엔디안) 방향8 위험8. 유효하면 1 */
static int is_valid_binary(const unsigned char *buf, size_t len)
{
    if (len < BINARY_SIZE) return 0;
    char dir = (char)buf[4];
    int danger = buf[5];
    return (dir == 'L' || dir == 'R' || dir == 'F' || dir == 'N') && (danger >= 1 && danger <= 3);
}

/* 재생할 메시지인지: 바이너리 6바이트 또는 텍스트 "PLAY"/"L 300 2" 등 */
static int should_play(const char *buf, size_t len)
{
    if (!buf || len == 0) return 0;
    if (len == BINARY_SIZE && is_valid_binary((const unsigned char *)buf, len))
        return 1;
    if (len >= 4 && strncmp(buf, "PLAY", 4) == 0)
        return 1;
    if (len >= 4 && strncmp(buf, "play", 4) == 0)
        return 1;
    /* "L 300 2", "N 300 2" 등 방향+거리+등급 형식 */
    if (len >= 2) {
        char d = (char)(buf[0] >= 'a' && buf[0] <= 'z' ? buf[0] - 32 : buf[0]);
        if ((d == 'L' || d == 'R' || d == 'F' || d == 'N') && (buf[1] == ' ' || buf[1] == '\0'))
            return 1;
    }
    return 0;
}

static const char *get_sound_file(int argc, char *argv[])
{
    if (argc > 1 && argv[1][0] != '\0')
        return argv[1];
    {
        const char *e = getenv("TRIGGER_SOUND_FILE");
        if (e && e[0] != '\0')
            return e;
    }
    return "alert.mp3";
}

static void play_sound(const char *path)
{
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return;
    }
    if (pid > 0) {
        waitpid(pid, NULL, WNOHANG);
        return;
    }
    /* child: gst-play-1.0 --no-interactive path */
    execlp("gst-play-1.0", "gst-play-1.0", "--no-interactive", path, (char *)NULL);
    execl("/usr/bin/gst-play-1.0", "gst-play-1.0", "--no-interactive", path, (char *)NULL);
    _exit(127);
}

int main(int argc, char *argv[])
{
    const char *sound_file = get_sound_file(argc, argv);
    int listen_fd, client_fd;
    struct sockaddr_in addr;
    char buf[BUF_SIZE];
    ssize_t n;

    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0) {
        perror("socket");
        return 1;
    }

    {
        int on = 1;
        if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0) {
            perror("setsockopt");
            close(listen_fd);
            return 1;
        }
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(LISTEN_PORT);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(listen_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(listen_fd);
        return 1;
    }
    if (listen(listen_fd, 5) < 0) {
        perror("listen");
        close(listen_fd);
        return 1;
    }

    printf("sound_trigger: listening on port %d, sound file: %s (gst-play-1.0)\n", LISTEN_PORT, sound_file);
    fflush(stdout);

    for (;;) {
        client_fd = accept(listen_fd, NULL, NULL);
        if (client_fd < 0) {
            perror("accept");
            continue;
        }
        n = recv(client_fd, buf, BUF_SIZE - 1, 0);
        close(client_fd);
        if (n > 0) {
            int do_play = 0;
            if ((size_t)n >= BINARY_SIZE && is_valid_binary((const unsigned char *)buf, (size_t)n)) {
                do_play = 1;
            } else {
                buf[n] = '\0';
                while (n > 0 && (buf[n - 1] == '\n' || buf[n - 1] == '\r'))
                    buf[--n] = '\0';
                if (should_play(buf, (size_t)n))
                    do_play = 1;
            }
            if (do_play)
                play_sound(sound_file);
        }
    }

    close(listen_fd);
    return 0;
}
