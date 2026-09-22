#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <mqueue.h>

#define NOMBRE_COLA "/cola_ej5"

mqd_t qd;

void handler_sigint(int signum) {
    char texto[] = "SIGINT via POSIX mq";
    mq_send(qd, texto, strlen(texto), 0);
    write(1, "[A] SIGINT -> mq_send a B\n", 26);
}

int main(void) {
    printf("[A] PID: %d\n", getpid());
    struct mq_attr attr = { .mq_flags = 0, .mq_maxmsg = 10, .mq_msgsize = 256, .mq_curmsgs = 0 };

    qd = mq_open(NOMBRE_COLA, O_WRONLY | O_CREAT, 0660, &attr);
    signal(SIGINT, handler_sigint);

    if (!fork()) {
        mqd_t qd_lectura = mq_open(NOMBRE_COLA, O_RDONLY | O_CREAT, 0660, &attr);
        char buf[256];
        while (1) {
            ssize_t n = mq_receive(qd_lectura, buf, sizeof(buf), NULL);
            if (n > 0) {
                buf[n] = '\0';
                printf("[B] PID %d recibio (mq POSIX): %s\n", getpid(), buf);
            }
        }
    }

    signal(SIGCHLD, SIG_IGN);
    printf("[A] kill -2 %d\n", getpid());
    while (1) pause();

    return 0;
}