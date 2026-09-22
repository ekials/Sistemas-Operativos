#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/msg.h>

#define KEY_COLA 4444
#define TIPO_B 1
#define TIPO_C 2

struct msgbuf {
    long mtype;
    char mtext[128];
};

int msqid;

void handler_sigint(int signum) {
    struct msgbuf m;
    m.mtype = TIPO_B;
    snprintf(m.mtext, sizeof(m.mtext), "para B (PID A %d)", getpid());
    msgsnd(msqid, &m, strlen(m.mtext) + 1, IPC_NOWAIT);
    write(1, "[A] SIGINT -> mensaje tipo 1 (B)\n", 33);
}

void handler_sigusr1(int signum) {
    struct msgbuf m;
    m.mtype = TIPO_C;
    snprintf(m.mtext, sizeof(m.mtext), "para C (PID A %d)", getpid());
    msgsnd(msqid, &m, strlen(m.mtext) + 1, IPC_NOWAIT);
    write(1, "[A] SIGUSR1 -> mensaje tipo 2 (C)\n", 34);
}

int main(void) {
    printf("[A] PID: %d\n", getpid());
    msqid = msgget(KEY_COLA, IPC_CREAT | 0666);

    signal(SIGINT, handler_sigint);
    signal(SIGUSR1, handler_sigusr1);

    if (!fork()) {
        struct msgbuf r;
        while (1) {
            msgrcv(msqid, &r, sizeof(r.mtext), TIPO_B, 0);
            printf("[B] PID %d recibio: %s\n", getpid(), r.mtext);
        }
    }

    if (!fork()) {
        struct msgbuf r;
        while (1) {
            msgrcv(msqid, &r, sizeof(r.mtext), TIPO_C, 0);
            printf("[C] PID %d recibio: %s\n", getpid(), r.mtext);
        }
    }

    signal(SIGCHLD, SIG_IGN);
    printf("[A] kill -2 %d | kill -USR1 %d\n", getpid(), getpid());
    while (1) pause();

    return 0;
}