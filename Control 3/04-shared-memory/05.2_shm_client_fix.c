/*
 * 05.2 shm_client (version corregida y comentada)
 * CLIENTE: busca el segmento que creo el server (misma key), lo lee
 * e indica que ya leyo poniendo '*' en la primera posicion.
 * Si corres el client antes que el server: "shmget: No such file or directory"
 */
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>   /* [FIX] exit */

#define SHMSZ 27

int main(void)
{
    int shmid;
    key_t key = 5678;
    char *shm, *s;

    /* 1) BUSCAR el segmento existente (sin IPC_CREAT) */
    if ((shmid = shmget(key, SHMSZ, 0666)) < 0) {
        perror("shmget");
        exit(1);
    }

    /* 2) PEGARLO */
    if ((shm = shmat(shmid, NULL, 0)) == (char *) -1) {
        perror("shmat");
        exit(1);
    }

    /* 3) LEER hasta el '\0' */
    for (s = shm; *s != '\0'; s++)               /* [FIX] '\0' en vez de NULL */
        putchar(*s);
    putchar('\n');

    /* 4) avisar al server: "ya lei" */
    *shm = '*';

    shmdt(shm);
    return 0;
}
