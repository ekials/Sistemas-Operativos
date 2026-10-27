/*
 * 9.12 Mandar un mensaje a un proceso ESPECIFICO usando el tipo (mtype)
 * El padre manda a cada hijo un mensaje con tipo = i+1; el hijo i solo recibe el suyo.
 * gcc 912_mensaje_a_cada_hijo.c -o p912 && ./p912
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/wait.h>

#define NPROC 5
struct Msg { long mtype; char texto[64]; };

int main(void)
{
    int msqid = msgget(IPC_PRIVATE, 0666 | IPC_CREAT);
    if (msqid < 0) { perror("msgget"); exit(1); }

    for (int i = 0; i < NPROC; i++) {
        if (fork() == 0) {
            struct Msg m;
            msgrcv(msqid, &m, sizeof(m.texto), i + 1, 0);   /* SOLO tipo i+1 */
            printf("hijo %d recibio: %s\n", i, m.texto); fflush(stdout);
            exit(0);
        }
    }
    for (int i = NPROC - 1; i >= 0; i--) {                  /* mando en desorden a proposito */
        struct Msg m; m.mtype = i + 1;
        snprintf(m.texto, sizeof(m.texto), "hola hijo %d", i);
        msgsnd(msqid, &m, strlen(m.texto) + 1, 0);
    }
    for (int i = 0; i < NPROC; i++) wait(NULL);
    msgctl(msqid, IPC_RMID, NULL);
    return 0;
}
