#include <stdio.h>
#include <unistd.h>

int main(void) {
    char buffer[128];
    ssize_t n = read(STDIN_FILENO, buffer, sizeof(buffer) - 1);
    if (n > 0) {
        buffer[n] = '\0';
        printf("[C] PID %d recibio por stdin (en realidad pipe): %s", getpid(), buffer);
    }
    return 0;
}