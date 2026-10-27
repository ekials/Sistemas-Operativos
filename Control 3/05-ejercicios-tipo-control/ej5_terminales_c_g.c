/*
 * EJERCICIO 5: Estilo EXACTO de clase (c / g) pero con shared memory
 * -----------------------------------------------------------------
 * Enunciado: 5 procesos en 5 TERMINALES distintas (no fork) escriben su
 * nombre en un "muro" de memoria compartida, protegido con semaforo.
 * Como son programas independientes, se usan LLAVES FIJAS (no IPC_PRIVATE).
 *
 * gcc ej5_terminales_c_g.c -o muro
 *   ./muro c                 <- CREA semaforo + shm (una sola vez)
 *   ./muro g proceso1         <- en 5 terminales distintas, con nombres distintos
 *   ./muro g proceso2 ...
 *   ./muro r                 <- READ: muestra el muro
 *   ./muro d                 <- DELETE: borra semaforo y shm (verifica con ipcs)
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

#define SEM_KEY 0x1111
#define SHM_KEY 0x2222
#define TAM 1024

struct Muro { int n; char texto[TAM]; };

int main(int argc, char *argv[]) {
    if (argc < 2) { fprintf(stderr, "uso: %s c | g nombre | r | d\n", argv[0]); return 1; }
    char op = argv[1][0];
    int flags = (op == 'c') ? (0666 | IPC_CREAT) : 0666;

    int semid = semget(SEM_KEY, 1, flags);
    if (semid < 0) { perror("semget (corriste ./muro c primero?)"); exit(11); }
    int shmid = shmget(SHM_KEY, sizeof(struct Muro), flags);
    if (shmid < 0) { perror("shmget"); exit(21); }
    struct Muro *m = shmat(shmid, NULL, 0);
    if (m == (void *) -1) { perror("shmat"); exit(22); }

    if (op == 'c') {
        setSem(semid, 0, 1);
        memset(m, 0, sizeof(*m));
        printf("creado: semid=%d shmid=%d\n", semid, shmid);
    } else if (op == 'g') {
        if (argc < 3) { fprintf(stderr, "falta el nombre\n"); return 1; }
        srand(getpid());
        for (int k = 0; k < 3; k++) {              /* cada uno escribe 3 veces */
            semP(semid, 0);
            int len = strlen(m->texto);
            snprintf(m->texto + len, TAM - len, "[%d] %s (pid %d)\n", ++m->n, argv[2], getpid());
            printf("escribi: %s #%d\n", argv[2], m->n); fflush(stdout);
            sleep(rand() % 2);                     /* nadie mas entra mientras duermo */
            semV(semid, 0);
            sleep(rand() % 2);
        }
    } else if (op == 'r') {
        semP(semid, 0);
        printf("---- MURO (%d mensajes) ----\n%s", m->n, m->texto);
        semV(semid, 0);
    } else if (op == 'd') {
        shmdt(m);
        shmctl(shmid, IPC_RMID, NULL);
        semctl(semid, 0, IPC_RMID);
        printf("borrados\n");
        return 0;
    }
    shmdt(m);
    return 0;
}
