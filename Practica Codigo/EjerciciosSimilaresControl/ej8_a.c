#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

int pipe_b[2], pipe_c[2], pipe_d[2];

void handler_sigint(int s) { char t[] = "para B\n"; write(pipe_b[1], t, strlen(t)); }
void handler_sigusr1(int s) { char t[] = "para C\n"; write(pipe_c[1], t, strlen(t)); }
void handler_sigusr2(int s) { char t[] = "para D\n"; write(pipe_d[1], t, strlen(t)); }

void escuchar(char *nombre, int fd_lectura) {
    printf("[%s] PID %d escuchando...\n", nombre, getpid());
    char buf[64];
    while (1) {
        int n = read(fd_lectura, buf, sizeof(buf) - 1);
        if (n > 0) { buf[n] = '\0'; printf("[%s] recibio: %s", nombre, buf); }
    }
}

int main(void) {
    printf("[A] PID: %d\n", getpid());
    pipe(pipe_b); pipe(pipe_c); pipe(pipe_d);

    if (!fork()) { close(pipe_b[1]); escuchar("B", pipe_b[0]); }
    if (!fork()) { close(pipe_c[1]); escuchar("C", pipe_c[0]); }
    if (!fork()) { close(pipe_d[1]); escuchar("D", pipe_d[0]); }

    close(pipe_b[0]); close(pipe_c[0]); close(pipe_d[0]);

    signal(SIGINT, handler_sigint);
    signal(SIGUSR1, handler_sigusr1);
    signal(SIGUSR2, handler_sigusr2);
    signal(SIGCHLD, SIG_IGN);

    printf("[A] kill -2/-USR1/-USR2 %d\n", getpid());
    while (1) pause();

    return 0;
}