#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <setjmp.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define MAXSIZE 128
#define KEY 1234

struct msgbuf 
{
    long mtype;
    char mtext[MAXSIZE];
};

static sigjmp_buf punto_salto;
static int msqid;
static int contador[4] = {0, 0, 0, 0};

void enviar_mensaje(int tipo) 
{
    struct msgbuf sbuf;
    sbuf.mtype = tipo;
    snprintf(sbuf.mtext, MAXSIZE, "Mensaje tipo %d - PID %d - repeticion %d", tipo, getpid(), contador[tipo]);
    if (msgsnd(msqid, &sbuf, strlen(sbuf.mtext) + 1, IPC_NOWAIT) < 0)
        perror("msgsnd");
    else
        printf("[EMISOR] Mensaje tipo %d enviado a la cola: \"%s\"\n", tipo, sbuf.mtext);
}

void handler_sigint(int signum) 
{
    contador[1]++;
    printf("\n[F1] SIGINT recibido (%d). Van %d veces. Saltando\n", signum, contador[1]);
    siglongjmp(punto_salto, 1);
}

void handler_sigusr1(int signum) 
{
    contador[2]++;
    printf("\n[F2] SIGUSR1 recibido (%d). Van %d veces. Saltando\n", signum, contador[2]);
    siglongjmp(punto_salto, 2);
}

void handler_sigusr2(int signum) 
{
    contador[3]++;
    printf("\n[F3] SIGUSR2 recibido (%d). Van %d veces. Saltando\n", signum, contador[3]);
    siglongjmp(punto_salto, 3);
}

int main() 
{
    printf("PID de este proceso (emisor): %d\n", getpid());

    if ((msqid = msgget(KEY, IPC_CREAT | 0666)) < 0) 
    {
        perror("msgget");
        exit(1);
    }

    signal(SIGINT, handler_sigint);
    signal(SIGUSR1, handler_sigusr1);
    signal(SIGUSR2, handler_sigusr2);

    int origen = sigsetjmp(punto_salto, 1);

    if (origen == 0) 
    {
        printf("Esperando senales\n");
        printf("  kill -SIGINT  %d  - mensaje tipo 1\n", getpid());
        printf("  kill -SIGUSR1 %d  - mensaje tipo 2\n", getpid());
        printf("  kill -SIGUSR2 %d  - mensaje tipo 3\n\n", getpid());
    } 
    else 
    {
        enviar_mensaje(origen);
    }

    while (1) 
    {
        printf("Emisor activo - esperando senales\n");
        sleep(2);
    }

    return 0;
}
