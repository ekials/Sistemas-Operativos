#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#define FIFO_PATH "/tmp/fifo_ej3"

int main(void) {
    printf("[C] PID: %d, esperando FIFO...\n", getpid());
    int fd = open(FIFO_PATH, O_RDONLY);
    char buf[128];
    int n = read(fd, buf, sizeof(buf) - 1);
    if (n > 0) {
        buf[n] = '\0';
        printf("[C] recibio de B: %s", buf);
    }
    close(fd);
    return 0;
}