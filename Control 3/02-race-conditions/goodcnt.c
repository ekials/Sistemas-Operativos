/*
 * goodcnt.c  -> SOLUCION de la "Activity 2" de semaphores.txt
 * badcnt.c con un semaforo POSIX como mutex: siempre da 2*NITER.
 *
 * gcc goodcnt.c -o goodcnt -lpthread     &&   ./goodcnt
 */
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>

#define NITER 1000000

int cnt = 0;
sem_t mutex;                       /* semaforo global (lo comparten los hilos) */

void *Count(void *a)
{
    int i, tmp;
    for (i = 0; i < NITER; i++)
    {
        sem_wait(&mutex);          /* P: entra a la seccion critica */
        tmp = cnt;
        tmp = tmp + 1;
        cnt = tmp;
        sem_post(&mutex);          /* V: sale */
    }
    return NULL;
}

int main(void)
{
    pthread_t tid1, tid2;

    sem_init(&mutex, 0, 1);        /* 0 = compartido entre hilos, valor inicial 1 */

    if (pthread_create(&tid1, NULL, Count, NULL)) { printf("ERROR hilo 1\n"); exit(1); }
    if (pthread_create(&tid2, NULL, Count, NULL)) { printf("ERROR hilo 2\n"); exit(1); }

    pthread_join(tid1, NULL);
    pthread_join(tid2, NULL);

    if (cnt < 2 * NITER)
        printf("BOOM! cnt is [%d], should be %d\n", cnt, 2 * NITER);
    else
        printf("OK! cnt is [%d]\n", cnt);

    sem_destroy(&mutex);
    return 0;
}
