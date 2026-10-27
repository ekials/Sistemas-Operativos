/*
 * 11_Semaphore.c  (CODIGO DE LA CLASE DEL 20/10, version comentada y corregida)
 * -----------------------------------------------------------------------------
 * Idea: varios PROGRAMAS independientes (en terminales distintas) comparten
 * UN semaforo del sistema, identificado por la llave KEY = 0x1111.
 *
 * Cada programa imprime cada letra de un texto DOS veces. La impresion de
 * las dos letras esta protegida por el semaforo (seccion critica), asi que
 * aunque haya varios procesos, los pares NUNCA se rompen:
 *      bien:  hhxxoollaa...      mal (sin semaforo):  hxhxo...
 *
 * Compilar:  gcc 11_Semaphore_comentado.c -o sc
 * Uso:
 *   Terminal 1:  ./sc c "hola"     <- c = CREA el semaforo y lo pone en 1
 *   Terminal 2:  ./sc g "xyz"      <- g = solo OBTIENE el semaforo ya creado
 *   Terminal 3:  ./sc g "catolica"
 *   Si el texto empieza con 'a' (ej: ./sc g "ahora") el programa pide una
 *   tecla DENTRO de la seccion critica -> los demas se quedan BLOQUEADOS.
 *   Eso demuestra por que NUNCA se pide input dentro de la seccion critica.
 *
 * Ver semaforos:  ipcs -s      Borrar:  ipcrm -s <semid>
 *
 * Correcciones respecto al original (marcadas con [FIX]):
 *   - valida argc (el original da "Segmentation fault" si no le pasas args)
 *   - id inicializado y error si la opcion no es 'c' ni 'g'
 *   - srand(getpid()) para que cada proceso tenga distinto random
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

#define KEY 0x1111      /* "nombre" del semaforo: todos los procesos usan la misma */

/* Linux no define esta union, la tienes que declarar tu (lo pide semctl) */
union semun
{
    int val;                 /* para SETVAL  */
    struct semid_ds *buf;    /* para IPC_STAT / IPC_SET */
    unsigned short *array;   /* para GETALL / SETALL */
};

union semun u;

/* struct sembuf = {numero de semaforo, operacion, flags}
 * p: resta 1  -> "pedir" el recurso  (si vale 0, el proceso se BLOQUEA)
 * v: suma 1   -> "liberar" el recurso (despierta a uno que espera)    */
struct sembuf p = {0, -1, 0};
struct sembuf v = {0, +1, 0};

void semP(int id)                    /* wait / down / Lock.Acquire */
{
    if (semop(id, &p, 1) < 0)
    {
        perror("semop p");
        exit(13);
    }
}

void semV(int id)                    /* signal / up / Lock.Release */
{
    if (semop(id, &v, 1) < 0)
    {
        perror("semop v");
        exit(14);
    }
}

/* Crea el conjunto de semaforos (1 semaforo) si no existe.
 * 0666 = permisos rw para todos; IPC_CREAT = crear si no existe */
int getSemCreate()
{
    int i = semget(KEY, 1, 0666 | IPC_CREAT);
    if (i < 0)
    {
        perror("semget");
        exit(11);
    }
    return i;
}

/* Solo obtiene uno que YA existe (flag 0). Si nadie lo creo:
 * "semget: No such file or directory"  (lo que salio en clase) */
int getSem()
{
    int i = semget(KEY, 1, 0);
    if (i < 0)
    {
        perror("semget");
        exit(11);
    }
    return i;
}

/* Le da el valor inicial = cuantos procesos pueden entrar a la vez.
 * 1 -> mutex (solo 1 a la vez). 3 -> hasta 3 a la vez, etc. */
void controlSem(int id, int numRecursos)
{
    u.val = numRecursos;
    if (semctl(id, 0, SETVAL, u) < 0)
    {
        perror("semctl");
        exit(12);
    }
}

int main(int argc, char *argv[])
{
    char c;
    int id = -1;

    if (argc < 3) {                                            /* [FIX] */
        fprintf(stderr, "uso: %s c|g \"texto\"\n", argv[0]);
        return 1;
    }

    if (argv[1][0] == 'c') {
        id = getSemCreate();
        controlSem(id, 1);        /* mutex: valor 1 */
    } else if (argv[1][0] == 'g') {
        id = getSem();
    } else {                                                   /* [FIX] */
        fprintf(stderr, "opcion invalida: usa c (crear) o g (get)\n");
        return 1;
    }

    srand(getpid());                                           /* [FIX] */
    printf("sem:%d\n", id);

    char *s = argv[2];
    int l = strlen(s);
    for (int i = 0; i < l; ++i)
    {
        semP(id);                 /* ===== entra a la SECCION CRITICA ===== */
        putchar(s[i]);
        fflush(stdout);           /* sin fflush la letra se queda en el buffer */
        sleep(rand() % 2);        /* duerme 0 o 1 seg: provoca que otro quiera entrar */
        putchar(s[i]);
        fflush(stdout);
        if (argv[2][0] == 'a')    /* ojo!!! nunca pedir info al usuario en la c.s. */
            scanf(" %c", &c);
        semV(id);                 /* ===== sale de la SECCION CRITICA ===== */
        sleep(rand() % 2);
    }
    printf("\n");
    return 0;
}
