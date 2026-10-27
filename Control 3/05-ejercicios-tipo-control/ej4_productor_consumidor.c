/*
 * EJERCICIO 4: Productor-Consumidor (buffer acotado) con 5 procesos
 * -----------------------------------------------------------------
 * Enunciado: 4 procesos PRODUCTORES generan 5 numeros cada uno y los ponen en
 * un buffer circular de tamano 4 en memoria compartida. 1 proceso CONSUMIDOR
 * saca los 20 numeros y los suma. Nadie debe escribir en el buffer lleno ni
 * leer del buffer vacio.
 *
 * 3 semaforos (clasico de libro, Silberschatz):
 *   MUTEX = 1        -> exclusion mutua al tocar el buffer
 *   VACIOS = TAM     -> cuantos huecos libres hay   (productor hace P)
 *   LLENOS = 0       -> cuantos elementos hay       (consumidor hace P)
 *
 * gcc ej4_productor_consumidor.c -o ej4   &&   ./ej4
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/wait.h>

union semun { int val; struct semid_ds *buf; unsigned short *array; };

void semP(int id, int num) {                 /* wait   */
    struct sembuf op = {num, -1, SEM_UNDO};
    if (semop(id, &op, 1) < 0) { perror("semop P"); exit(13); }
}
void semV(int id, int num) {                 /* signal */
    struct sembuf op = {num, +1, SEM_UNDO};
    if (semop(id, &op, 1) < 0) { perror("semop V"); exit(14); }
}
void setSem(int id, int num, int valor) {
    union semun u; u.val = valor;
    if (semctl(id, num, SETVAL, u) < 0) { perror("semctl"); exit(12); }
}

#define TAM    4
#define NPROD  4
#define POR_PROD 5
#define MUTEX  0
#define VACIOS 1
#define LLENOS 2

struct Compartido {
    int buf[TAM];
    int in, out;
};

int main(void) {
    int semid = semget(IPC_PRIVATE, 3, 0666 | IPC_CREAT);
    if (semid < 0) { perror("semget"); exit(11); }
    setSem(semid, MUTEX, 1);
    setSem(semid, VACIOS, TAM);
    setSem(semid, LLENOS, 0);

    int shmid = shmget(IPC_PRIVATE, sizeof(struct Compartido), 0666 | IPC_CREAT);
    if (shmid < 0) { perror("shmget"); exit(21); }
    struct Compartido *mem = shmat(shmid, NULL, 0);
    if (mem == (void *) -1) { perror("shmat"); exit(22); }
    memset(mem, 0, sizeof(*mem));

    /* 4 productores */
    for (int p = 0; p < NPROD; p++) {
        if (fork() == 0) {
            srand(getpid());
            for (int k = 0; k < POR_PROD; k++) {
                int item = (p + 1) * 100 + k;      /* ej: productor 2 -> 300,301,... */
                semP(semid, VACIOS);               /* espera hueco libre   */
                semP(semid, MUTEX);
                mem->buf[mem->in] = item;
                mem->in = (mem->in + 1) % TAM;
                printf("productor %d puso %d\n", p, item); fflush(stdout);
                semV(semid, MUTEX);
                semV(semid, LLENOS);               /* avisa: hay 1 mas      */
                usleep((rand() % 3) * 50000);
            }
            shmdt(mem); exit(0);
        }
    }

    /* 1 consumidor (el 5to proceso) */
    if (fork() == 0) {
        int suma = 0;
        for (int k = 0; k < NPROD * POR_PROD; k++) {
            semP(semid, LLENOS);                   /* espera que haya algo */
            semP(semid, MUTEX);
            int item = mem->buf[mem->out];
            mem->out = (mem->out + 1) % TAM;
            semV(semid, MUTEX);
            semV(semid, VACIOS);                   /* libera un hueco      */
            printf("   consumidor saco %d\n", item); fflush(stdout);
            suma += item;
        }
        printf("CONSUMIDOR: suma = %d (esperado 5040)\n", suma);
        shmdt(mem); exit(0);
    }

    for (int i = 0; i < NPROD + 1; i++) wait(NULL);
    shmdt(mem);
    shmctl(shmid, IPC_RMID, NULL);
    semctl(semid, 0, IPC_RMID);
    return 0;
}
