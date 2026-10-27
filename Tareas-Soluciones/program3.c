#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>

#define MAX_BUF 1000
#define QUEUE2_KEY 5678
#define MSG_TYPE_FROM_P2 4

struct msg_buffer 
{
    long mtype;
    char mtext[MAX_BUF];
};

int main()
{
    printf("Prgrma 3 PID: %d\n", getpid());

    int msgid = msgget(QUEUE2_KEY, 0666 | IPC_CREAT);
    if (msgid < 0) 
    {
        perror("Prgrma 3 msgget");
        exit(1);
    }

    struct msg_buffer recibido;

    while (1) 
    {
        if (msgrcv(msgid, &recibido, sizeof(recibido.mtext), MSG_TYPE_FROM_P2, 0) == -1) 
        {
            perror("Prgrma 3 msgrcv");
            continue;
        }

        printf("Prgrma 3 Mensaje recibido de Programa 2\n");
        printf(">>> %s <<<\n", recibido.mtext);
    }

    return 0;
}
