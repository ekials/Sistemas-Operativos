/*
 * 9.2 Palabras por argumentos (ej2 modificado)
 * gcc 92_palabras_argv.c -o p92 && ./p92 hola xyz ucsp SO mutex
 */
/*
 * EJERCICIO 2: El codigo de clase pero con 5 procesos + shared memory
 * -------------------------------------------------------------------
 * Enunciado: 5 procesos hijos, cada uno con su palabra. Cada uno escribe
 * cada letra DOS veces en un buffer en memoria compartida (seccion critica
 * protegida con semaforo, igual que 11_Semaphore.c). Al final el padre
 * imprime el buffer: los pares nunca deben estar separados.
 *
 * gcc ej2_letras_5procesos.c -o ej2   &&   ./ej2
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
#define TAM   512

struct Compartido {
    int  pos;            /* siguiente posicion libre del buffer */
    char buf[TAM];
};

int main(int argc, char *argv[]) {
    if (argc < NPROC + 1) { fprintf(stderr, "uso: %s p1 p2 p3 p4 p5\n", argv[0]); return 1; }

    int semid = semget(IPC_PRIVATE, 1, 0666 | IPC_CREAT);
    if (semid < 0) { perror("semget"); exit(11); }
    setSem(semid, 0, 1);

    int shmid = shmget(IPC_PRIVATE, sizeof(struct Compartido), 0666 | IPC_CREAT);
    if (shmid < 0) { perror("shmget"); exit(21); }
    struct Compartido *mem = shmat(shmid, NULL, 0);
    if (mem == (void *) -1) { perror("shmat"); exit(22); }
    memset(mem, 0, sizeof(*mem));

    for (int i = 0; i < NPROC; i++) {
        pid_t pid = fork();
        if (pid < 0) { perror("fork"); exit(1); }
        if (pid == 0) {
            srand(getpid());
            char *s = argv[i + 1];      /* CAMBIO: palabra por argumento */
            for (int k = 0; s[k]; k++) {
                semP(semid, 0);                     /* ---- seccion critica ---- */
                mem->buf[mem->pos++] = s[k];
                putchar(s[k]); fflush(stdout);
                usleep((rand() % 3) * 100000);      /* 0-0.2 s: fuerza competencia */
                mem->buf[mem->pos++] = s[k];
                putchar(s[k]); fflush(stdout);
                semV(semid, 0);                     /* ------------------------- */
                usleep((rand() % 3) * 100000);
            }
            shmdt(mem);
            exit(0);
        }
    }
    for (int i = 0; i < NPROC; i++) wait(NULL);

    mem->buf[mem->pos] = '\0';
    printf("\nPADRE lee la shm: %s\n", mem->buf);

    /* verificacion: cada par debe ser igual */
    int ok = 1;
    for (int k = 0; k < mem->pos; k += 2)
        if (mem->buf[k] != mem->buf[k + 1]) ok = 0;
    printf("Pares intactos: %s\n", ok ? "SI" : "NO");

    shmdt(mem);
    shmctl(shmid, IPC_RMID, NULL);
    semctl(semid, 0, IPC_RMID);
    return 0;
}
