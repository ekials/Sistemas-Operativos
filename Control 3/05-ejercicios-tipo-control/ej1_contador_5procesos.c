/*
 * EJERCICIO 1: Contador compartido con 5 procesos
 * ----------------------------------------------
 * Enunciado: crea 5 procesos hijos con fork(). Todos incrementan 100000 veces
 * un contador que esta en MEMORIA COMPARTIDA. Muestra que sin semaforo el
 * resultado sale mal (race condition) y con semaforo sale 500000.
 *
 * gcc ej1_contador_5procesos.c -o ej1
 * ./ej1 0   -> SIN semaforo (sale menos de 500000)
 * ./ej1 1   -> CON semaforo (siempre 500000)
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

#define NPROC 5
#define NITER 100000

int main(int argc, char *argv[]) {
    int usarSem = (argc > 1) ? atoi(argv[1]) : 1;

    /* semaforo: 1 solo, valor 1 (mutex). IPC_PRIVATE = llave unica,
       sirve porque los hijos heredan el id con fork() */
    int semid = semget(IPC_PRIVATE, 1, 0666 | IPC_CREAT);
    if (semid < 0) { perror("semget"); exit(11); }
    setSem(semid, 0, 1);

    /* memoria compartida: un solo int */
    int shmid = shmget(IPC_PRIVATE, sizeof(int), 0666 | IPC_CREAT);
    if (shmid < 0) { perror("shmget"); exit(21); }
    int *cont = shmat(shmid, NULL, 0);
    if (cont == (void *) -1) { perror("shmat"); exit(22); }
    *cont = 0;

    for (int i = 0; i < NPROC; i++) {
        pid_t pid = fork();
        if (pid < 0) { perror("fork"); exit(1); }
        if (pid == 0) {                         /* HIJO */
            for (int k = 0; k < NITER; k++) {
                if (usarSem) semP(semid, 0);
                int tmp = *cont;                /* leer   */
                tmp = tmp + 1;                  /* sumar  */
                *cont = tmp;                    /* escribir -> aqui se pisan */
                if (usarSem) semV(semid, 0);
            }
            shmdt(cont);
            exit(0);
        }
    }
    for (int i = 0; i < NPROC; i++) wait(NULL);

    printf("%s semaforo: contador = %d (esperado %d)\n",
           usarSem ? "CON" : "SIN", *cont, NPROC * NITER);

    shmdt(cont);
    shmctl(shmid, IPC_RMID, NULL);
    semctl(semid, 0, IPC_RMID);
    return 0;
}
