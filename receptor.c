#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define MAXSIZE 128
#define KEY 1234

struct msgbuf {
    long mtype;
    char mtext[MAXSIZE];
};

void die(char *s) {
    perror(s);
    exit(1);
}

int main() {
    int msqid;
    struct msgbuf rcvbuffer;

    if ((msqid = msgget(KEY, IPC_CREAT | 0666)) < 0)
        die("msgget");

    printf("Receptor listo. PID: %d\n", getpid());
    printf("Esperando mensajes de la cola (key=1234)...\n\n");

    while (1) {
        if (msgrcv(msqid, &rcvbuffer, MAXSIZE, 0, 0) < 0)
            die("msgrcv");

        printf("========================================\n");
        printf("Mensaje recibido - Tipo: %ld\n", rcvbuffer.mtype);
        printf("Contenido: %s\n", rcvbuffer.mtext);

        if (rcvbuffer.mtype == 1)
            printf("-> Procesando TIPO 1 (SIGINT)\n");
        else if (rcvbuffer.mtype == 2)
            printf("-> Procesando TIPO 2 (SIGUSR1)\n");
        else if (rcvbuffer.mtype == 3)
            printf("-> Procesando TIPO 3 (SIGUSR2)\n");
        else
            printf("-> Tipo desconocido\n");

        printf("Esperando 5 segundos...\n");
        printf("========================================\n\n");
        sleep(5);
    }

    return 0;
}
