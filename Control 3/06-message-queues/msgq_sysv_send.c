/*
 * COLA DE MENSAJES SYSTEM V - EMISOR  (misma familia que semget/shmget: usa KEY)
 * ----------------------------------
 * gcc msgq_sysv_send.c -o msend
 * ./msend 1 "hola"     -> manda "hola" con TIPO 1
 * ./msend 2 "chau"     -> manda "chau" con TIPO 2
 * Ver la cola:  ipcs -q      Borrar:  ipcrm -q <msqid>
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define KEY 0x3333

/* el mensaje SIEMPRE empieza con un long mtype > 0 (el "tipo") */
struct mensaje {
    long mtype;
    char texto[256];
};

int main(int argc, char *argv[])
{
    if (argc < 3) { fprintf(stderr, "uso: %s tipo \"texto\"\n", argv[0]); return 1; }

    int id = msgget(KEY, 0666 | IPC_CREAT);
    if (id < 0) { perror("msgget"); exit(1); }

    struct mensaje m;
    m.mtype = atol(argv[1]);
    strncpy(m.texto, argv[2], sizeof(m.texto) - 1);
    m.texto[sizeof(m.texto) - 1] = '\0';

    /* el tamano NO incluye el long mtype, solo el contenido */
    if (msgsnd(id, &m, strlen(m.texto) + 1, 0) < 0) { perror("msgsnd"); exit(2); }
    printf("enviado tipo %ld: %s\n", m.mtype, m.texto);
    return 0;
}
