/*
 * EJERCICIO 7: LOS 3 COMBINADOS - shared memory + semaforo + cola de mensajes
 * ---------------------------------------------------------------------------
 * Enunciado: un SERVIDOR (terminal 1) y dos CLIENTES (terminales 2 y 3).
 *  - Cada cliente escribe su texto en una "pizarra" en memoria compartida
 *    (protegida con semaforo porque varios escriben ahi).
 *  - Luego le AVISA al servidor por una COLA DE MENSAJES en que posicion
 *    escribio (el mensaje lleva el dato chico; la shm el dato grande).
 *  - El servidor recibe los avisos, lee la pizarra en esa posicion y lo muestra.
 *
 * gcc ej7_combinado_shm_sem_msg.c -o comb
 *   ./comb s 2                 <- TERMINAL 1: servidor, crea todo, espera 2 avisos, borra todo
 *   ./comb c proceso1 "hola"   <- TERMINAL 2
 *   ./comb c proceso2 "chau"   <- TERMINAL 3
 *
 * Ver mientras corre:  ipcs   (aparecen las 3 cosas: -q colas, -m shm, -s semaforos)
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/msg.h>

#define SEM_KEY 0x1111
#define SHM_KEY 0x2222
#define MSG_KEY 0x3333
#define NSLOTS  10

union semun { int val; struct semid_ds *buf; unsigned short *array; };

struct Pizarra {                 /* lo que va en shared memory */
    int  usados;
    char texto[NSLOTS][64];
};

struct Aviso {                   /* lo que va por la cola de mensajes */
    long mtype;                  /* obligatorio, > 0 */
    int  slot;                   /* en que posicion escribio */
    int  pid;
};

void semP(int id) { struct sembuf o = {0, -1, SEM_UNDO}; if (semop(id, &o, 1) < 0) { perror("semop P"); exit(13); } }
void semV(int id) { struct sembuf o = {0, +1, SEM_UNDO}; if (semop(id, &o, 1) < 0) { perror("semop V"); exit(14); } }

int main(int argc, char *argv[])
{
    if (argc < 3) { fprintf(stderr, "uso: %s s N | c nombre \"texto\"\n", argv[0]); return 1; }
    char op = argv[1][0];
    int flags = (op == 's') ? (0666 | IPC_CREAT) : 0666;

    int semid = semget(SEM_KEY, 1, flags);
    int shmid = shmget(SHM_KEY, sizeof(struct Pizarra), flags);
    int msqid = msgget(MSG_KEY, flags);
    if (semid < 0 || shmid < 0 || msqid < 0) { perror("get (corre primero el servidor)"); exit(11); }
    struct Pizarra *p = shmat(shmid, NULL, 0);
    if (p == (void *) -1) { perror("shmat"); exit(22); }

    if (op == 's') {                                    /* ======= SERVIDOR ======= */
        union semun u; u.val = 1;
        semctl(semid, 0, SETVAL, u);
        memset(p, 0, sizeof(*p));
        int n = atoi(argv[2]);
        printf("servidor listo (sem=%d shm=%d msg=%d), espero %d avisos\n", semid, shmid, msqid, n);
        fflush(stdout);

        for (int k = 0; k < n; k++) {
            struct Aviso a;
            if (msgrcv(msqid, &a, sizeof(a) - sizeof(long), 1, 0) < 0) { perror("msgrcv"); exit(31); }
            semP(semid);                                 /* leo la shm protegido */
            printf("aviso de pid %d -> pizarra[%d] = \"%s\"\n", a.pid, a.slot, p->texto[a.slot]);
            semV(semid);
            fflush(stdout);
        }
        shmdt(p);
        shmctl(shmid, IPC_RMID, NULL);
        semctl(semid, 0, IPC_RMID);
        msgctl(msqid, IPC_RMID, NULL);
        printf("servidor: todo borrado\n");
        return 0;
    }

    if (op == 'c') {                                    /* ======= CLIENTE ======= */
        if (argc < 4) { fprintf(stderr, "falta el texto\n"); return 1; }
        semP(semid);                                     /* ---- seccion critica ---- */
        int slot = p->usados;
        if (slot >= NSLOTS) { semV(semid); fprintf(stderr, "pizarra llena\n"); return 1; }
        snprintf(p->texto[slot], sizeof(p->texto[slot]), "%s: %s", argv[2], argv[3]);
        p->usados++;
        sleep(1);                                        /* simula trabajo: el otro cliente espera */
        semV(semid);                                     /* -------------------------- */

        struct Aviso a = {1, slot, getpid()};           /* aviso al servidor */
        if (msgsnd(msqid, &a, sizeof(a) - sizeof(long), 0) < 0) { perror("msgsnd"); exit(32); }
        printf("cliente %s escribio en slot %d y aviso al servidor\n", argv[2], slot);
        shmdt(p);
    }
    return 0;
}
