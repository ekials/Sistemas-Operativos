#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <setjmp.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#define FIFO_PATH "/tmp/fifo_ej6"

static sigjmp_buf punto;
int pipefd[2];
int fifo_fd;

void handler_sigint(int signum) { siglongjmp(punto, 1); }
void handler_sigusr1(int signum) { siglongjmp(punto, 2); }

int main(void) {
    printf("[A] PID: %d\n", getpid());
    pipe(pipefd);
    mkfifo(FIFO_PATH, 0666);

    signal(SIGINT, handler_sigint);
    signal(SIGUSR1, handler_sigusr1);

    if (!fork()) {
        close(pipefd[1]);
        int fifo_r = open(FIFO_PATH, O_RDONLY | O_NONBLOCK);
        char buf[128];
        while (1) {
            int n = read(pipefd[0], buf, sizeof(buf) - 1);
            if (n > 0) { buf[n] = '\0'; printf("[B] via PIPE: %s\n", buf); }

            n = read(fifo_r, buf, sizeof(buf) - 1);
            if (n > 0) { buf[n] = '\0'; printf("[B] via FIFO: %s\n", buf); }

            usleep(100000);
        }
    }

    close(pipefd[0]);
    fifo_fd = open(FIFO_PATH, O_WRONLY | O_NONBLOCK);

    int origen = sigsetjmp(punto, 1);
    if (origen == 0) {
        printf("[A] Esperando senales...\n");
    } else if (origen == 1) {
        char t[] = "dato por PIPE (SIGINT)\n";
        write(pipefd[1], t, strlen(t));
    } else if (origen == 2) {
        char t[] = "dato por FIFO (SIGUSR1)\n";
        write(fifo_fd, t, strlen(t));
    }

    signal(SIGCHLD, SIG_IGN);
    while (1) pause();

    return 0;
}