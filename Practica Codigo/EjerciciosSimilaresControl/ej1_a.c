#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/stat.h>

#define FIFO_PATH "/tmp/fifo_ej1"

int pipefd[2];
int fifo_fd;

void handler_usr1(int signum)
{
    char texto[128];
    int n = snprintf(texto, sizeof(texto), "SIGUSR1 desde A (PID %d)", getpid());
    write(fifo_fd, texto, n);
    write(1, "[A] SIGUSR1 -> enviado por FIFO a B\n", 36);
}

void handler_usr2(int signum)
{
    char texto[128];
    int n = snprintf(texto, sizeof(texto), "SIGUSR2 desde A (PID %d)", getpid());
    write(pipefd[1], texto, n);
    write(1, "[A] SIGUSR2 -> enviado por PIPE a C\n", 36);
}

int main(void)
{
    printf("[A] PID: %d\n", getpid());
    mkfifo(FIFO_PATH, 0666);
    pipe(pipefd);

    pid_t pid_b = fork();
    
    if (pid_b == 0) 
    {
        close(pipefd[1]);
        pid_t pid_clon = fork();
    
        if (pid_clon == 0) 
        {
            char descriptor[16];
            snprintf(descriptor, sizeof(descriptor), "%d", pipefd[0]);
            char *args[] = {"./ej1_c", descriptor, NULL};
            execv("./ej1_c", args);
            perror("execv");
            exit(1);
        }
    
        close(pipefd[0]);
        printf("[B] PID: %d, abriendo FIFO ...\n", getpid());
        int fd = open(FIFO_PATH, O_RDONLY);
        char buf[128];
    
        while (1) 
        {
            int n = read(fd, buf, sizeof(buf) - 1);
            if (n > 0) {
                buf[n] = '\0';
                printf("[B] PID %d recibio (FIFO): %s\n", getpid(), buf);
            }
        }
    }
    
    close(pipefd[0]);
    fifo_fd = open(FIFO_PATH, O_WRONLY);
    signal(SIGCHLD, SIG_IGN);
    signal(SIGUSR1, handler_usr1);
    signal(SIGUSR2, handler_usr2);

    printf("[A] Listo. kill -USR1 %d | kill -USR2 %d\n", getpid(), getpid());
    
    while (1) pause();
    return 0;
}