/*
 * 9.7 El padre espera a los hijos con un semaforo (no solo wait)
 * OJO: el aviso usa V SIN SEM_UNDO; con SEM_UNDO se pierde y el padre se cuelga
 * gcc 97_padre_espera_sem.c -o p97 && ./p97
 */
/*
 * PLANTILLA para el control: N procesos + semaforos System V + shared memory
 * -------------------------------------------------------------------------
 * Copia este archivo, cambia la parte marcada "TU EJERCICIO AQUI" y listo.
 *
 * Compilar:  gcc PLANTILLA.c -o plantilla
 * Ejecutar:  ./plantilla
 * Revisar:   ipcs -s   (semaforos)    ipcs -m   (shared memory)
 * Limpiar:   ipcrm -s <semid>          ipcrm -m <shmid>
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

#define SEM_KEY 0x1111      /* llave del semaforo (como en clase)       */
#define SHM_KEY 0x2222      /* llave de la memoria compartida           */
#define NPROC   5           /* numero de procesos hijos                 */

/* En Linux esta union NO viene en los headers: hay que declararla */
union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};

/* ---------------- lo que vive en la memoria compartida ---------------- */
struct Compartido {
    int  contador;
    char buffer[256];
    int  pos;
};

/* ---------------------------- semaforos ---------------------------- */
/* semP = wait = Lock.A()   (resta 1; si esta en 0, se bloquea)
 * semV = signal = Lock.R() (suma 1; despierta a alguien que espera)
 * num  = cual semaforo del conjunto (0 si solo hay uno)
 * SEM_UNDO: si el proceso muere, el SO deshace su operacion (evita deadlock) */
void semP(int id, int num) {
    struct sembuf op = {num, -1, SEM_UNDO};
    if (semop(id, &op, 1) < 0) { perror("semop P"); exit(13); }
}
void semV(int id, int num) {
    struct sembuf op = {num, +1, SEM_UNDO};
    if (semop(id, &op, 1) < 0) { perror("semop V"); exit(14); }
}

/* crea un conjunto de 'n' semaforos y pone cada uno en 'valor' */
int crearSem(int n, int valor) {
    int id = semget(SEM_KEY, n, 0666 | IPC_CREAT);
    if (id < 0) { perror("semget"); exit(11); }
    union semun u;
    u.val = valor;
    for (int i = 0; i < n; i++)
        if (semctl(id, i, SETVAL, u) < 0) { perror("semctl"); exit(12); }
    return id;
}

/* ------------------------- shared memory ------------------------- */
int crearShm(size_t tam) {
    int id = shmget(SHM_KEY, tam, 0666 | IPC_CREAT);
    if (id < 0) { perror("shmget"); exit(21); }
    return id;
}
void *pegarShm(int shmid) {                       /* "attach" */
    void *p = shmat(shmid, NULL, 0);
    if (p == (void *) -1) { perror("shmat"); exit(22); }
    return p;
}

/* CAMBIO: V SIN SEM_UNDO para avisos (con SEM_UNDO el aviso se deshace cuando el hijo muere) */
void semVaviso(int id, int num) {
    struct sembuf op = {num, +1, 0};
    if (semop(id, &op, 1) < 0) { perror("semop V"); exit(14); }
}

/* borra semaforo y shm (SIEMPRE al final, si no quedan en ipcs) */
void limpiar(int semid, int shmid) {
    semctl(semid, 0, IPC_RMID);
    shmctl(shmid, IPC_RMID, NULL);
}

/* ------------------------------ main ------------------------------ */
int main(void) {
    int semid = crearSem(2, 1);   /* CAMBIO: 2 semaforos: 0 = mutex, 1 = fin */
    { union semun u; u.val = 0; semctl(semid, 1, SETVAL, u); }   /* fin empieza en 0 */                       /* 1 semaforo, valor 1 = mutex */
    int shmid = crearShm(sizeof(struct Compartido));
    struct Compartido *mem = pegarShm(shmid);
    memset(mem, 0, sizeof(*mem));                     /* inicializar ANTES del fork */

    for (int i = 0; i < NPROC; i++) {
        pid_t pid = fork();
        if (pid < 0) { perror("fork"); exit(1); }
        if (pid == 0) {                               /* ======= HIJO i ======= */
            srand(getpid());                          /* cada hijo su propio random */

            /* ---------- TU EJERCICIO AQUI ---------- */
            semP(semid, 0);                           /* entra a la seccion critica */
            mem->contador++;
            printf("hijo %d (pid %d) -> contador = %d\n", i, getpid(), mem->contador);
            fflush(stdout);
            semV(semid, 0);                           /* sale de la seccion critica */
            /* --------------------------------------- */

            semVaviso(semid, 1);                      /* CAMBIO: aviso al padre que termine */
            shmdt(mem);
            exit(0);                                  /* IMPORTANTE: el hijo termina aqui */
        }
    }

    for (int i = 0; i < NPROC; i++) semP(semid, 1);   /* CAMBIO: espera con semaforo */
    printf("PADRE: los %d avisaron que terminaron\n", NPROC);
    for (int i = 0; i < NPROC; i++) wait(NULL);       /* igual recojo zombies */

    printf("PADRE: contador final = %d (esperado %d)\n", mem->contador, NPROC);
    shmdt(mem);
    limpiar(semid, shmid);
    return 0;
}
