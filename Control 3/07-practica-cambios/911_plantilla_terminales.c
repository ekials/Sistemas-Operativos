/*
 * 9.11 PLANTILLA para TERMINALES (sin fork): cada terminal es un proceso
 * Llave fija para que todos encuentren el mismo semaforo y shm.
 *
 * gcc 911_plantilla_terminales.c -o pt
 *   ./pt c            <- crea e inicializa (una vez)
 *   ./pt t 1          <- terminal 1  (trabaja)
 *   ./pt t 2          <- terminal 2
 *   ./pt t 3          <- terminal 3
 *   ./pt r            <- lee el resultado
 *   ./pt d            <- borra todo
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>

#define SEM_KEY 0x1111
#define SHM_KEY 0x2222

union semun { int val; struct semid_ds *buf; unsigned short *array; };

struct Compartido {          /* CAMBIA esto segun el ejercicio */
    int contador;
    int ultimo;
};

void semP(int id) { struct sembuf o = {0, -1, SEM_UNDO}; if (semop(id, &o, 1) < 0) { perror("semop P"); exit(13); } }
void semV(int id) { struct sembuf o = {0, +1, SEM_UNDO}; if (semop(id, &o, 1) < 0) { perror("semop V"); exit(14); } }

int main(int argc, char *argv[])
{
    if (argc < 2) { fprintf(stderr, "uso: %s c | t num | r | d\n", argv[0]); return 1; }
    char op = argv[1][0];
    int flags = (op == 'c') ? (0666 | IPC_CREAT) : 0666;

    int semid = semget(SEM_KEY, 1, flags);
    if (semid < 0) { perror("semget (corriste ./pt c?)"); exit(11); }
    int shmid = shmget(SHM_KEY, sizeof(struct Compartido), flags);
    if (shmid < 0) { perror("shmget (si dice Invalid argument: ipcrm -M 0x2222)"); exit(21); }
    struct Compartido *m = shmat(shmid, NULL, 0);
    if (m == (void *) -1) { perror("shmat"); exit(22); }

    if (op == 'c') {
        union semun u; u.val = 1;
        semctl(semid, 0, SETVAL, u);
        memset(m, 0, sizeof(*m));
        printf("creado: semid=%d shmid=%d\n", semid, shmid);
    } else if (op == 't') {
        int yo = (argc > 2) ? atoi(argv[2]) : getpid();
        srand(getpid());
        for (int k = 0; k < 3; k++) {
            semP(semid);                     /* ---- seccion critica ---- */
            m->contador++;
            m->ultimo = yo;
            printf("terminal %d -> contador = %d\n", yo, m->contador); fflush(stdout);
            sleep(rand() % 2);
            semV(semid);                     /* ------------------------- */
            sleep(rand() % 2);
        }
    } else if (op == 'r') {
        semP(semid);
        printf("contador = %d, ultimo en escribir = %d\n", m->contador, m->ultimo);
        semV(semid);
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
