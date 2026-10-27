/*
 * COLA DE MENSAJES POSIX - EMISOR
 * ------------------------------
 * La cola tiene un NOMBRE que empieza con "/" (ej: "/mylib").
 * Linux la guarda en /dev/mqueue/ -> puedes verla con:  ls /dev/mqueue
 * y ver cuantos bytes tiene esperando con:            cat /dev/mqueue/mylib
 *
 * gcc mq_posix_send.c -o send -lrt
 * ./send "hola"            (terminal 1, puedes mandar varios)
 * ./recv                   (terminal 2, ver mq_posix_recv.c)
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>      /* O_CREAT, O_WRONLY */
#include <sys/stat.h>
#include <mqueue.h>

#define NOMBRE  "/mylib"
#define MAXMSG  10       /* cuantos mensajes caben en la cola */
#define TAMMSG  256      /* tamano maximo de cada mensaje     */

int main(int argc, char *argv[])
{
    if (argc < 2) { fprintf(stderr, "uso: %s \"mensaje\"\n", argv[0]); return 1; }

    struct mq_attr attr;
    attr.mq_flags   = 0;
    attr.mq_maxmsg  = MAXMSG;
    attr.mq_msgsize = TAMMSG;
    attr.mq_curmsgs = 0;

    /* abrir (o crear si no existe) para ESCRIBIR */
    mqd_t mq = mq_open(NOMBRE, O_CREAT | O_WRONLY, 0666, &attr);
    if (mq == (mqd_t) -1) { perror("mq_open"); exit(1); }

    /* mandar: (cola, texto, longitud, prioridad). +1 para enviar el '\0' */
    if (mq_send(mq, argv[1], strlen(argv[1]) + 1, 0) == -1) { perror("mq_send"); exit(2); }
    printf("enviado: %s\n", argv[1]);

    mq_close(mq);       /* cierro MI acceso; la cola y el mensaje SIGUEN existiendo */
    return 0;
}
