#include <stdio.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define KEY_COLA 9999

struct msgbuf {
    long mtype;
    char mtext[128];
};

int main(void) {
    int msqid = msgget(KEY_COLA, IPC_CREAT | 0666);
    struct msgbuf m;
    printf("[C] PID: %d, esperando cola...\n", getpid());

    while (1) {
        msgrcv(msqid, &m, sizeof(m.mtext), 0, 0);
        printf("[C] recibio: %s\n", m.mtext);
    }
    return 0;
}