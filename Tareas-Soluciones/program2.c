#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define MAX_MSG 1000
#define QUEUE1_KEY 1234
#define QUEUE2_KEY 5678
#define MSG_TYPE_FROM_P1 3
#define MSG_TYPE_TO_P3 4

struct msg_buffer {
    long mtype;
    char mtext[MAX_MSG];
};

int main()
{
    printf("Prgrma 2 PID: %d\n", getpid());

    int msgid1 = msgget(QUEUE1_KEY, 0666 | IPC_CREAT);
    if (msgid1 < 0) 
    {
        perror("Prgrma 2 msgget Queue1");
        exit(1);
    }

    int msgid2 = msgget(QUEUE2_KEY, 0666 | IPC_CREAT);
    if (msgid2 < 0) 
    {
        perror("Prgrma 2 msgget Queue2");
        exit(1);
    }

    struct msg_buffer recibido;

    while (1) 
    {
        if (msgrcv(msgid1, &recibido, sizeof(recibido.mtext), MSG_TYPE_FROM_P1, 0) == -1) 
        {
            perror("Prgrma 2 msgrcv");
            continue;
        }

        printf("Prgrma 2 Mensaje recibido de Programa 1: \"%s\"\n", recibido.mtext);

        struct msg_buffer reenvio;
        reenvio.mtype = MSG_TYPE_TO_P3;
        strncpy(reenvio.mtext, recibido.mtext, MAX_MSG);

        if (msgsnd(msgid2, &reenvio, sizeof(reenvio.mtext), 0) == -1) 
        {
            perror("Prgrma 2 msgsnd");
            continue;
        }

        printf("Prgrma 2 Mensaje reenviado a Programa 3 (type=%d): \"%s\"\n", MSG_TYPE_TO_P3, reenvio.mtext);
    }

    return 0;
}
