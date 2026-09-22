#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Uso: %s descriptor_pipe\n", argv[0]);
        return 1;
    }

    int fd = atoi(argv[1]);
    printf("[C] PID: %d, escuchando el pipe (fd %d)...\n", getpid(), fd);

    char buffer[128];
    ssize_t n;
    while ((n = read(fd, buffer, sizeof(buffer) - 1)) > 0) {
        buffer[n] = '\0';
        printf("[C] PID %d recibio (pipe): %s\n", getpid(), buffer);
    }

    close(fd);
    return 0;
}