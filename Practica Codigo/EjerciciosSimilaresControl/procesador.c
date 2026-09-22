#include <stdio.h>
#include <unistd.h>

int main(void) {
    char buf[128];
    ssize_t n = read(STDIN_FILENO, buf, sizeof(buf) - 1);
    if (n > 0) {
        buf[n] = '\0';
        printf("[procesador] PID %d recibio: %s", getpid(), buf);
    }
    return 0;
}