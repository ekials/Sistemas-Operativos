/*
//programa1c y programa2.c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(void) {
    int fd[2];
    pipe(fd); // fd es lectura, fd es escritura [cite: 156]

    pid_t pid = fork(); // Clona el proceso [cite: 156]

    if (pid == 0) {
        // CÓDIGO DEL HIJO: Ejecutará ./programa2 [cite: 156]

        close(fd[1]); // Cierra el extremo de escritura que no usará [cite: 156]

        // dup2 clona fd (lectura del pipe) sobre STDIN_FILENO (descriptor 0) [cite: 156]
        dup2(fd[0], STDIN_FILENO); // [cite: 156]
        close(fd[0]); // Cierra el descriptor original ya duplicado [cite: 156]

        char *args[] = {"./programa2", NULL}; // [cite: 156]

        // Carga ./programa2. Como los descriptores sobreviven a execv, STDIN ya apunta al pipe [cite: 144, 156]
        execv("./programa2", args); // [cite: 156]

        perror("execv"); // [cite: 156]
        exit(1); // [cite: 156]
    }

    // CÓDIGO DEL PADRE
    close(fd[0]); // Cierra el extremo de lectura que no usará [cite: 156]

    char mensaje[] = "Hola desde el proceso padre\n"; // [cite: 156]
    write(fd[1], mensaje, sizeof(mensaje) - 1); // Escribe en el pipe [cite: 156]

    close(fd[1]); // Cierra la escritura para enviar el fin de archivo (EOF) [cite: 156]

    wait(NULL); // Espera a que el hijo termine [cite: 156]

    return 0;
}
*/
//p3
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(void)
{
    int fd[2];
    pipe(fd);

    pid_t pid = fork();

    if (pid == 0) {
        // HIJO: ejecutará programa2

        close(fd[1]);

        char descriptor[20];
        sprintf(descriptor, "%d", fd[0]);

        char *args[] = {
            "./programa3",
            descriptor,
            NULL
        };
        printf("Descriptor del papa: %s\n",descriptor);
        execv("./programa3", args);

        perror("execv");
        exit(1);
    }

    // PADRE
    close(fd[0]);

    char mensaje[] = "Keiko y Pedrito\n";
    write(fd[1], mensaje, sizeof(mensaje) - 1);

    close(fd[1]);

    wait(NULL);

    return 0;
}
