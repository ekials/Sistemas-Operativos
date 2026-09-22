#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define FIFO_PATH "/tmp/fifo_ej9"
#define KEY_COLA 9999

struct msgbuf {
    long mtype;
    char mtext[128];
};

int main(void) {
    printf("[B] PID: %d\n", getpid());
    int fifo_fd = open(FIFO_PATH, O_RDONLY);
    int msqid = msgget(KEY_COLA, IPC_CREAT | 0666);
    char buf[128];

    while (1) {
        int n = read(fifo_fd, buf, sizeof(buf) - 1);
        if (n > 0) {
            buf[n] = '\0';
            printf("[B] recibio de A: %s", buf);

            struct msgbuf m;
            m.mtype = 1;
            strncpy(m.mtext, buf, sizeof(m.mtext));
            msgsnd(msqid, &m, strlen(m.mtext) + 1, IPC_NOWAIT);
            printf("[B] reenviado a C por cola\n");
        }
    }
    return 0;
}