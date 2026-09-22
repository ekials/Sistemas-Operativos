#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/stat.h>

#define FIFO_PATH "/tmp/fifo_ej3"

int pipefd[2];

void handler_sigint(int signum) {
    char texto[] = "dato desde A\n";
    write(pipefd[1], texto, strlen(texto));
    write(1, "[A] SIGINT -> escrito en pipe hacia B\n", 38);
}

void proceso_b(int fd_pipe) {
    mkfifo(FIFO_PATH, 0666);
    printf("[B] PID: %d, esperando datos del pipe...\n", getpid());

    char buf[128];
    int n = read(fd_pipe, buf, sizeof(buf) - 1);
    if (n > 0) {
        buf[n] = '\0';
        printf("[B] recibio de A: %s", buf);
        printf("[B] reenviando a C por FIFO...\n");
        int fifo_fd = open(FIFO_PATH, O_WRONLY);
        write(fifo_fd, buf, n);
        close(fifo_fd);
    }
    exit(0);
}

int main(void) {
    printf("[A] PID: %d\n", getpid());
    pipe(pipefd);
    signal(SIGINT, handler_sigint);

    if (!fork()) {
        close(pipefd[1]);
        proceso_b(pipefd[0]);
    }

    close(pipefd[0]);
    signal(SIGCHLD, SIG_IGN);

    printf("[A] kill -2 %d\n", getpid());
    while (1) pause();

    return 0;
}