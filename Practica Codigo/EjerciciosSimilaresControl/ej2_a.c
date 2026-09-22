#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/msg.h>

#define KEY_COLA 2222
#define TIPO_B 1

struct msgbuf {
    long mtype;
    char mtext[128];
};

int pipefd[2];
int msqid;

void handler_sigint(int signum) {
    struct msgbuf msg;
    msg.mtype = TIPO_B;
    snprintf(msg.mtext, sizeof(msg.mtext), "SIGINT desde A");
    msgsnd(msqid, &msg, strlen(msg.mtext) + 1, IPC_NOWAIT);
    write(1, "[A] SIGINT -> cola a B\n", 23);
}

void handler_sigterm(int signum) {
    char texto[] = "SIGTERM desde A\n";
    write(pipefd[1], texto, strlen(texto));
    write(1, "[A] SIGTERM -> pipe a C\n", 24);
}

int main(void) {
    printf("[A] PID: %d\n", getpid());
    pipe(pipefd);
    msqid = msgget(KEY_COLA, IPC_CREAT | 0666);

    signal(SIGINT, handler_sigint);
    signal(SIGTERM, handler_sigterm);

    pid_t pid_c = fork();
    if (pid_c == 0) {
        close(pipefd[1]);
        dup2(pipefd[0], STDIN_FILENO);
        close(pipefd[0]);

        char *args[] = { "./ej2_c", NULL };
        execv("./ej2_c", args);
        perror("execv");
        exit(1);
    }

    close(pipefd[0]);
    signal(SIGCHLD, SIG_IGN);

    printf("[A] kill -2 %d | kill -TERM %d\n", getpid(), getpid());
    while (1) pause();

    return 0;
}