/*
 * 9.6 Primero pares (0,2,4) y luego impares (1,3) (ej3 modificado)
 * Para otro orden cambia solo el arreglo orden[]
 * gcc 96_pares_impares.c -o p96 && ./p96
 */
/*
 * EJERCICIO 3: 5 procesos que se turnan EN ORDEN (P0 -> P1 -> P2 -> P3 -> P4 -> P0 ...)
 * ---------------------------------------------------------------------------------
 * Enunciado: 5 procesos escriben en memoria compartida en ronda, siempre en el
 * mismo orden, durante 3 rondas. Salida esperada: 0 1 2 3 4 0 1 2 3 4 0 1 2 3 4
 *
 * Truco: un CONJUNTO de 5 semaforos (uno por proceso).
 *   sem[0] = 1  (el primero puede arrancar)    sem[1..4] = 0 (esperan)
 *   El proceso i hace P(sem[i]) ... trabaja ... V(sem[(i+1) % 5])
 *
 * gcc ej3_turnos_en_orden.c -o ej3   &&   ./ej3
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
    struct sembuf op = {num, -1, 0};
    if (semop(id, &op, 1) < 0) { perror("semop P"); exit(13); }
}
void semV(int id, int num) {                 /* signal */
    struct sembuf op = {num, +1, 0};
    if (semop(id, &op, 1) < 0) { perror("semop V"); exit(14); }
}
void setSem(int id, int num, int valor) {
    union semun u; u.val = valor;
    if (semctl(id, num, SETVAL, u) < 0) { perror("semctl"); exit(12); }
}

#define NPROC  5
#define RONDAS 3

struct Compartido {
    int  n;
    int  orden[NPROC * RONDAS];
};

int main(void) {
    int semid = semget(IPC_PRIVATE, NPROC, 0666 | IPC_CREAT);   /* 5 semaforos */
    if (semid < 0) { perror("semget"); exit(11); }
    int orden[NPROC] = {0, 2, 4, 1, 3};                 /* CAMBIO: el orden que pidan */
    int sig[NPROC];                                     /* sig[x] = a quien le paso el turno */
    for (int k = 0; k < NPROC; k++) sig[orden[k]] = orden[(k + 1) % NPROC];
    for (int i = 0; i < NPROC; i++) setSem(semid, i, i == orden[0] ? 1 : 0);

    int shmid = shmget(IPC_PRIVATE, sizeof(struct Compartido), 0666 | IPC_CREAT);
    if (shmid < 0) { perror("shmget"); exit(21); }
    struct Compartido *mem = shmat(shmid, NULL, 0);
    if (mem == (void *) -1) { perror("shmat"); exit(22); }
    memset(mem, 0, sizeof(*mem));

    for (int i = 0; i < NPROC; i++) {
        pid_t pid = fork();
        if (pid < 0) { perror("fork"); exit(1); }
        if (pid == 0) {
            for (int r = 0; r < RONDAS; r++) {
                semP(semid, i);                       /* espero MI turno */
                mem->orden[mem->n++] = i;
                printf("ronda %d: proceso %d (pid %d)\n", r, i, getpid());
                fflush(stdout);
                semV(semid, sig[i]);                  /* CAMBIO */         /* le paso el turno al siguiente */
            }
            shmdt(mem);
            exit(0);
        }
    }
    for (int i = 0; i < NPROC; i++) wait(NULL);

    printf("Orden guardado en shm: ");
    for (int k = 0; k < mem->n; k++) printf("%d ", mem->orden[k]);
    printf("\n");

    shmdt(mem);
    shmctl(shmid, IPC_RMID, NULL);
    semctl(semid, 0, IPC_RMID);
    return 0;
}
