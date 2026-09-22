#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define KEY_COLA 1234
#define TIPO_MSG_B 1

struct msgbuf {
    long mtype;
    char mtext[128];
};

int pipefd[2];
int msqid;

/* Manejador de SIGINT (Signal 2): envia mensaje a la Cola hacia B */
void handler_sigint(int signum) {
    struct msgbuf msg;
    msg.mtype = TIPO_MSG_B;
    snprintf(msg.mtext, sizeof(msg.mtext),
             "SIGINT recibido por A (PID %d)", getpid());
    msgsnd(msqid, &msg, strlen(msg.mtext) + 1, IPC_NOWAIT);
    write(1, "[A] SIGINT -> mensaje enviado por COLA a B\n", 44);
}

/* Manejador de SIGTRAP (Signal 5): envia mensaje al Pipe hacia C */
void handler_sigtrap(int signum) {
    char texto[128];
    int n = snprintf(texto, sizeof(texto),
                     "SIGTRAP recibido por A (PID %d)", getpid());
    write(pipefd[1], texto, n);
    write(1, "[A] SIGTRAP -> mensaje enviado por PIPE a C\n", 45);
}

int main(void) {
    printf("[A] PID: %d\n", getpid());

    /* 1. Crear recursos IPC (Pipe y Cola System V) */
    if (pipe(pipefd) == -1) { 
        perror("pipe"); 
        exit(1); 
    }
    if ((msqid = msgget(KEY_COLA, IPC_CREAT | 0666)) < 0) {
        perror("msgget"); 
        exit(1);
    }

    /* 2. Registrar manejadores usando sigaction */
    struct sigaction sa_int, sa_trap;
    sa_int.sa_handler = handler_sigint;
    sa_int.sa_flags = SA_RESTART;
    sigemptyset(&sa_int.sa_mask);
    sigaction(SIGINT, &sa_int, NULL);

    sa_trap.sa_handler = handler_sigtrap;
    sa_trap.sa_flags = SA_RESTART;
    sigemptyset(&sa_trap.sa_mask);
    sigaction(SIGTRAP, &sa_trap, NULL);

    /* 3. Crear proceso B */
    pid_t pid_b = fork();
    if (pid_b == 0) {
        /* ===== PROCESO B ===== */
        close(pipefd[1]); /* No escribe en el pipe */

        pid_t pid_clon = fork();
        if (pid_clon == 0) {
            /* ===== CLON DE B -> Ejecuta proceso_c ===== */
            char descriptor[16];
            snprintf(descriptor, sizeof(descriptor), "%d", pipefd[0]);
            char *args[] = { "./proceso_c", descriptor, NULL };
            execv("./proceso_c", args);
            perror("execv");
            exit(1);
        }
        close(pipefd[0]); /* Cierra lectura pipe en B */

        printf("[B] PID: %d, esperando mensajes por la cola...\n", getpid());
        struct msgbuf recibido;
        while (1) {
            if (msgrcv(msqid, &recibido, sizeof(recibido.mtext),
                       TIPO_MSG_B, 0) < 0) {
                perror("msgrcv");
                break;
            }
            printf("[B] PID %d recibio (cola): %s\n",
                   getpid(), recibido.mtext);
        }
        exit(0);
    }

    /* ===== PROCESO A ===== */
    close(pipefd[0]); /* Cierra lectura pipe en A */
    signal(SIGCHLD, SIG_IGN); /* Previene procesos zombies */

    printf("[A] Esperando senales. Desde otra terminal ejecute:\n");
    printf("  kill -2 %d   (SIGINT  -> cola a B)\n", getpid());
    printf("  kill -5 %d   (SIGTRAP -> pipe a C)\n\n", getpid());

    while (1)
        pause();

    return 0;
}