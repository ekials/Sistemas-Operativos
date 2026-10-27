/*
 * COLA DE MENSAJES POSIX - RECEPTOR
 * --------------------------------
 * gcc mq_posix_recv.c -o recv -lrt
 * ./recv        -> recibe mensajes (se BLOQUEA si la cola esta vacia) hasta recibir "fin"
 *                  y al final borra la cola (mq_unlink)
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>

#define NOMBRE  "/mylib"
#define MAXMSG  10
#define TAMMSG  256

int main(void)
{
    struct mq_attr attr;
    attr.mq_flags = 0; attr.mq_maxmsg = MAXMSG; attr.mq_msgsize = TAMMSG; attr.mq_curmsgs = 0;
    mqd_t mq = mq_open(NOMBRE, O_CREAT | O_RDONLY, 0666, &attr);   /* abrir para LEER */
    if (mq == (mqd_t) -1) { perror("mq_open"); exit(1); }

    char buf[TAMMSG];       /* OJO: el buffer debe ser >= mq_msgsize o mq_receive falla */
    unsigned int prio;

    while (1) {
        ssize_t n = mq_receive(mq, buf, TAMMSG, &prio);   /* bloquea hasta que llegue algo */
        if (n == -1) { perror("mq_receive"); exit(2); }
        printf("recibido (%zd bytes, prio %u): %s\n", n, prio, buf);
        if (strcmp(buf, "fin") == 0) break;
    }

    mq_close(mq);
    mq_unlink(NOMBRE);      /* BORRA la cola (desaparece de /dev/mqueue) */
    printf("cola %s borrada\n", NOMBRE);
    return 0;
}
