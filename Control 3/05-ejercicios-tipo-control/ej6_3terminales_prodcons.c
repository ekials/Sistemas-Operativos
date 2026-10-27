/*
 * EJERCICIO 6: 3 TERMINALES + shared memory + semaforos
 * -----------------------------------------------------
 * Enunciado: dos productores (terminales 1 y 2) mandan su texto letra por letra
 * a un buffer circular en memoria compartida. Un lector (terminal 3) saca las
 * letras y las imprime. El buffer es pequeno (5): si esta lleno, los productores
 * esperan; si esta vacio, el lector espera.
 *
 * gcc ej6_3terminales_prodcons.c -o t3
 *   ./t3 c                     <- crea shm + 3 semaforos (una vez)
 *   ./t3 l 2                   <- TERMINAL 3: lector, termina cuando 2 productores acabaron
 *   ./t3 p "hola mundo"        <- TERMINAL 1
 *   ./t3 p "SISTEMAS"          <- TERMINAL 2
 *   ./t3 d                     <- borra todo (verifica con ipcs)
 *
 * Semaforos (un conjunto de 3, llave fija porque son programas distintos):
 *   MUTEX=1  VACIOS=TAM  LLENOS=0
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
#define TAM     5
#define MUTEX   0
#define VACIOS  1
#define LLENOS  2
#define FIN     '\n'          /* cada productor manda '\n' al terminar */

union semun { int val; struct semid_ds *buf; unsigned short *array; };

struct Compartido {
    char buf[TAM];
    int  in, out;
};

void semP(int id, int n) { struct sembuf o = {n, -1, SEM_UNDO}; if (semop(id, &o, 1) < 0) { perror("semop P"); exit(13); } }
void semV(int id, int n) { struct sembuf o = {n, +1, SEM_UNDO}; if (semop(id, &o, 1) < 0) { perror("semop V"); exit(14); } }
void setSem(int id, int n, int v) { union semun u; u.val = v; if (semctl(id, n, SETVAL, u) < 0) { perror("semctl"); exit(12); } }

int main(int argc, char *argv[])
{
    if (argc < 2) { fprintf(stderr, "uso: %s c | p \"texto\" | l N | d\n", argv[0]); return 1; }
    char op = argv[1][0];
    int flags = (op == 'c') ? (0666 | IPC_CREAT) : 0666;

    int semid = semget(SEM_KEY, 3, flags);
    if (semid < 0) { perror("semget (hiciste ./t3 c?)"); exit(11); }
    int shmid = shmget(SHM_KEY, sizeof(struct Compartido), flags);
    if (shmid < 0) { perror("shmget"); exit(21); }
    struct Compartido *m = shmat(shmid, NULL, 0);
    if (m == (void *) -1) { perror("shmat"); exit(22); }

    if (op == 'c') {
        memset(m, 0, sizeof(*m));
        setSem(semid, MUTEX, 1);
        setSem(semid, VACIOS, TAM);
        setSem(semid, LLENOS, 0);
        printf("creado: semid=%d shmid=%d\n", semid, shmid);
    }
    else if (op == 'p') {                               /* ---- PRODUCTOR ---- */
        if (argc < 3) { fprintf(stderr, "falta el texto\n"); return 1; }
        char *s = argv[2];
        int l = strlen(s);
        srand(getpid());
        for (int i = 0; i <= l; i++) {
            char c = (i < l) ? s[i] : FIN;               /* al final manda FIN */
            semP(semid, VACIOS);                         /* espera hueco      */
            semP(semid, MUTEX);
            m->buf[m->in] = c;
            m->in = (m->in + 1) % TAM;
            semV(semid, MUTEX);
            semV(semid, LLENOS);                         /* avisa: hay 1 mas  */
            if (c != FIN) { printf("puse '%c'\n", c); fflush(stdout); }
            usleep((rand() % 3) * 100000);
        }
        printf("productor %d termino\n", getpid());
    }
    else if (op == 'l') {                               /* ---- LECTOR ---- */
        int productores = (argc > 2) ? atoi(argv[2]) : 1;
        int terminados = 0;
        printf("lector esperando a %d productores...\n", productores);
        while (terminados < productores) {
            semP(semid, LLENOS);                         /* espera que haya algo */
            semP(semid, MUTEX);
            char c = m->buf[m->out];
            m->out = (m->out + 1) % TAM;
            semV(semid, MUTEX);
            semV(semid, VACIOS);                         /* libera un hueco */
            if (c == FIN) terminados++;
            else { putchar(c); fflush(stdout); }
        }
        printf("\nlector: todos terminaron\n");
    }
    else if (op == 'd') {
        shmdt(m);
        shmctl(shmid, IPC_RMID, NULL);
        semctl(semid, 0, IPC_RMID);
        printf("borrados\n");
        return 0;
    }
    shmdt(m);
    return 0;
}
