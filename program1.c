#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <setjmp.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define FIFO_PATH "/tmp/fifoLab"
#define MAX_BUF 1000
#define QUEUE1_KEY 1234
#define MSG_TYPE_TO_P2 3

struct msg_buffer {
    long mtype;
    char mtext[MAX_BUF];
};

int fd;
char buf[MAX_BUF] = "";

static sigjmp_buf punto_salto2;
static sigjmp_buf punto_salto10;

void manejador2(int signo)
{
    printf("\nPrgrma 1 Se recibio Signal 2 (senal %d)\n", signo);
    siglongjmp(punto_salto2, 1);
}

void manejador10(int signo)
{
    printf("\nPrgrma 1 Se recibio Signal 10 (senal %d)\n", signo);
    siglongjmp(punto_salto10, 1);
}

int main(void)
{
    printf("[Programa 1] PID: %d\n", getpid());

    signal(SIGINT, manejador2);
    signal(SIGUSR1, manejador10);

    while (1) 
    {

        if (sigsetjmp(punto_salto2, 1) != 0) 
        {
            fd = open(FIFO_PATH, O_RDONLY);
            if (fd < 0) perror("Prgrma 1");
            else 
            {
                ssize_t leidos = read(fd, buf, MAX_BUF - 1);
                close(fd);
                if (leidos > 0) 
                {
                    buf[leidos] = '\0';
                    buf[strcspn(buf, "\n")] = '\0';
                    printf("Prgrma 1 Dato leido del FIFO: \"%s\"\n", buf);
                }
            }
            continue;
        }

        if (sigsetjmp(punto_salto10, 1) != 0) 
        {
            int msgid = msgget(QUEUE1_KEY, 0666 | IPC_CREAT);
            if (msgid < 0) 
            {
                perror("Prgrma 1 msgget");
            } 
            else 
            {
                struct msg_buffer mensaje;
                mensaje.mtype = MSG_TYPE_TO_P2;
                strncpy(mensaje.mtext, buf, MAX_BUF);

                if (msgsnd(msgid, &mensaje, sizeof(mensaje.mtext), 0) == -1) perror("Prgrma 1 msgsnd");
                else printf("Prgrma 1 Mensaje enviado a Programa 2 (type=%d): \"%s\"\n", MSG_TYPE_TO_P2, buf);
            }
            continue;
        }

        pause();
    }

    return 0;
}
