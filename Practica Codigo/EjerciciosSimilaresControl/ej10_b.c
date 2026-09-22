#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    if (argc != 2) return 1;

    int fd = atoi(argv[1]);
    printf("[B] PID: %d, escuchando pipe (fd %d)\n", getpid(), fd);

    char buf[128];
    ssize_t n;

    while ((n = read(fd, buf, sizeof(buf) - 1)) > 0) {
        buf[n] = '\0';
        printf("[B] PID %d recibio (pipe): %s", getpid(), buf);
    }
    return 0;
}