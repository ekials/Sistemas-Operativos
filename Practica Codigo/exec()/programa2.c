#include <stdio.h>
#include <unistd.h>

int main(void) {
    char buffer[100];
    ssize_t n;

    // Lee directamente de la Entrada Estándar (STDIN_FILENO / descriptor 0) [cite: 156]
    n = read(STDIN_FILENO, buffer, sizeof(buffer) - 1); // [cite: 156]

    if (n > 0) {
        buffer[n] = '\0'; // Agrega el terminador nulo [cite: 156]
        printf("Programa 2 recibió: %s", buffer); // [cite: 156]
    }

    return 0;
}