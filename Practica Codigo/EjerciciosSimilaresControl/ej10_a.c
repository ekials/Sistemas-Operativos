#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define KEY_COLA 1010
#define TIPO_C 1

struct msgbuf {
    long mtype;
    char mtext[128];
};

int pipefd[2];
int msqid;

void handler_sigint(int s) {
    char t[] = "SIGINT para B (via pipe+execv)\n";
    write(pipefd[1], t, strlen(t));
    write(1, "[A] SIGINT -> pipe a B\n", 23);
}

void handler_sigtrap(int s) {
    struct msgbuf m;
    m.mtype = TIPO_C;
    snprintf(m.mtext, sizeof(m.mtext), "SIGTRAP para C (via cola)");
    msgsnd(msqid, &m, strlen(m.mtext) + 1, IPC_NOWAIT);
    write(1, "[A] SIGTRAP -> cola a C\n", 24);
}

int main(void) {
    printf("[A] PID: %d\n", getpid());
    pipe(pipefd);
    msqid = msgget(KEY_COLA, IPC_CREAT | 0666);

    struct sigaction sa1, sa2;
    sa1.sa_handler = handler_sigint;
    sa1.sa_flags = SA_RESTART;
    sigemptyset(&sa1.sa_mask);
    sigaction(SIGINT, &sa1, NULL);

    sa2.sa_handler = handler_sigtrap;
    sa2.sa_flags = SA_RESTART;
    sigemptyset(&sa2.sa_mask);
    sigaction(SIGTRAP, &sa2, NULL);

    if (!fork()) {
        close(pipefd[1]);
        char descriptor[16];
        snprintf(descriptor, sizeof(descriptor), "%d", pipefd[0]);
        char *args[] = { "./ej10_b", descriptor, NULL };
        execv("./ej10_b", args);
        perror("execv");
        exit(1);
    }

    if (!fork()) {
        struct msgbuf r;
        while (1) {
            msgrcv(msqid, &r, sizeof(r.mtext), TIPO_C, 0);
            printf("[C] PID %d recibio (cola): %s\n", getpid(), r.mtext);
        }
    }

    close(pipefd[0]);
    signal(SIGCHLD, SIG_IGN);

    printf("[A] kill -2 %d | kill -5 %d\n", getpid(), getpid());
    while (1) pause();

    return 0;
}