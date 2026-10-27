/*
 * COLA DE MENSAJES SYSTEM V - RECEPTOR
 * ------------------------------------
 * gcc msgq_sysv_recv.c -o mrecv
 * ./mrecv 0    -> recibe CUALQUIER tipo, en orden de llegada
 * ./mrecv 2    -> recibe SOLO mensajes de tipo 2
 * Termina cuando recibe "fin" y borra la cola.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define KEY 0x3333

struct mensaje {
    long mtype;
    char texto[256];
};

int main(int argc, char *argv[])
{
    long tipo = (argc > 1) ? atol(argv[1]) : 0;

    int id = msgget(KEY, 0666 | IPC_CREAT);
    if (id < 0) { perror("msgget"); exit(1); }

    struct mensaje m;
    while (1) {
        /* bloquea hasta que llegue un mensaje del tipo pedido (0 = cualquiera) */
        if (msgrcv(id, &m, sizeof(m.texto), tipo, 0) < 0) { perror("msgrcv"); exit(2); }
        printf("recibido tipo %ld: %s\n", m.mtype, m.texto);
        if (strcmp(m.texto, "fin") == 0) break;
    }

    msgctl(id, IPC_RMID, NULL);      /* borrar la cola */
    printf("cola borrada\n");
    return 0;
}
