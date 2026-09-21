#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(void)
{
    pid_t pid = fork();

    if(pid < 0)
    {
        perror("Error al crear el proceso");
        exit(1);
    }
    else if(pid == 0)
    {
        //coid del hijo
        printf("[HIJO]: Proceso de hijo iniciado (PID : %d). Simulando tarea \n", getpid());
        sleep(2); //pausa de 2 segundos simulando procesamiento [11]
        printf("[HIJO]: Tarea finalizada, saliendo con estado 42...");
        exit(42); // devuelve el nro 42 al proceso padre[10]
    }
    else
    {
        //cod padre
        int estado;
        printf("[PADRE]: Esperando a que el hijo (PID : %d) termine..", pid);

        //wait() congela la ejecucion del padre hasya que el hijo llame a exit() [1,5]
        wait(&estado);

        //se verifica si el hijo termino normalmente y se extrae el codigo [6,12]
        if(WIFEXITED(estado))
        {
            printf("[PADRE]: El hijo termino, coidgo de salida capturado: %d\n", WEXITSTATUS(estado));
        }
        printf("[PADRE]: Recoleccion completada sin dejar zombies. \n");
    }

    return 0;
}