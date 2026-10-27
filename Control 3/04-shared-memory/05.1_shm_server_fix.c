/*
 * 05.1 shm_server (version corregida y comentada)
 * SERVER: crea un segmento de memoria compartida, escribe "abc...z" y espera
 * hasta que el cliente cambie el primer caracter a '*' (senal de "ya lei").
 *
 * gcc 05.1_shm_server_fix.c -o server   &&   ./server        (terminal 1)
 * gcc 05.2_shm_client_fix.c -o client   &&   ./client        (terminal 2)
 *
 * [FIX] respecto al original: faltaban <stdlib.h> y <unistd.h> (warnings de
 * exit/sleep que salen en la captura de clase), main sin "int", *s = NULL
 * (NULL es un puntero, para un char va '\0'), y no se borraba el segmento.
 */
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>   /* [FIX] exit  */
#include <unistd.h>   /* [FIX] sleep */

#define SHMSZ 27      /* 26 letras + '\0' */

int main(void)
{
    char c;
    int shmid;
    key_t key = 5678;           /* "nombre" del segmento: el cliente usa el mismo */
    char *shm, *s;

    /* 1) CREAR el segmento (IPC_CREAT) de SHMSZ bytes con permisos 0666 */
    if ((shmid = shmget(key, SHMSZ, IPC_CREAT | 0666)) < 0) {
        perror("shmget");
        exit(1);
    }

    /* 2) PEGAR (attach) el segmento a mi espacio de direcciones */
    if ((shm = shmat(shmid, NULL, 0)) == (char *) -1) {
        perror("shmat");
        exit(1);
    }

    /* 3) ESCRIBIR: es un puntero normal, se usa como un arreglo */
    s = shm;
    for (c = 'a'; c <= 'z'; c++)
        *s++ = c;
    *s = '\0';                                  /* [FIX] */

    /* 4) ESPERAR a que el cliente marque '*' (espera activa con sleep:
     *    sincronizacion "casera", lo correcto seria con un semaforo) */
    while (*shm != '*')
        sleep(1);

    /* 5) despegar y BORRAR el segmento                      [FIX] */
    shmdt(shm);
    shmctl(shmid, IPC_RMID, NULL);
    printf("server: el cliente leyo, segmento borrado\n");
    return 0;
}
