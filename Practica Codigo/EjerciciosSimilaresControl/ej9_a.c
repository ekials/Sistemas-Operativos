#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#define FIFO_PATH "/tmp/fifo_ej9"

int fifo_fd;

void handler_sigint(int s) {
    char t[] = "dato desde A\n";
    write(fifo_fd, t, strlen(t));
    write(1, "[A] SIGINT -> FIFO a B\n", 23);
}

int main(void) {
    printf("[A] PID: %d\n", getpid());
    mkfifo(FIFO_PATH, 0666);

    if (!fork()) {
        execlp("./ej9_b", "./ej9_b", NULL);
    }

    fifo_fd = open(FIFO_PATH, O_WRONLY);
    signal(SIGINT, handler_sigint);
    signal(SIGCHLD, SIG_IGN);

    printf("[A] kill -2 %d\n", getpid());
    while (1) pause();

    return 0;
}