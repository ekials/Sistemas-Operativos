#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>

void handler_sigusr2(int signum) {
    int fd[2];
    pipe(fd);

    if (!fork()) {
        close(fd[1]);
        dup2(fd[0], STDIN_FILENO);
        close(fd[0]);

        char *args[] = { "./procesador", NULL };
        execv("./procesador", args);
        perror("execv");
        exit(1);
    }

    close(fd[0]);
    char mensaje[] = "dato generado tras SIGUSR2\n";
    write(fd[1], mensaje, strlen(mensaje));
    close(fd[1]);
    wait(NULL);
}

int main(void) {
    printf("[A] PID: %d\n", getpid());
    signal(SIGUSR2, handler_sigusr2);
    printf("[A] kill -USR2 %d\n", getpid());

    while (1) pause();
    return 0;
}