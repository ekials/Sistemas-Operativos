#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/wait.h>

int main(void) {
    pid_t pid;
    int status; //info de estado que devuelve el hijo al terminar

    pid = fork(); //clona el proceso actual creando un proceso hijo

    if (pid == -1)
    {
        printf("can't fork, error occurred\n"); 
        exit(EXIT_FAILURE); 
    } 
    else if (pid == 0) 
    {
        // cod DEL HIJO
        printf("child process, pid = %u\n", getpid()); //hijo
        printf("parent of child process, pid = %u\n", getppid()); //padre

        // Arreglo de argumentos para execv. OBLIGATORIO terminar en NULL 
        char *argv_list[] = { "/usr/bin/sleep", "5", NULL }; 

        // execv reemplaza este proceso. Si tiene exito, las lineas de abajo NUNCA se ejecutan
        //execv(argv_list, argv_list); 
        execv(argv_list[0], argv_list);
        // Solo se ejecuta si execv() fallo (por ejemplo, ruta incorrecta) 
        perror("execv failed");  
        _exit(127); //finaliza de inmediato el hijo con cod 127
    } 
    else //padre >0
    {
        // Cod DEL PADRE
        printf("Parent Of parent process, pid = %u\n", getppid()); 
        printf("parent process, pid = %u\n", getpid()); 

        // waitpid() suspende al padre hasta que el hijo especifico con PID 'pid' cambie de estado 
        if (waitpid(pid, &status, 0) > 0) 
        {
            if (WIFEXITED(status) && !WEXITSTATUS(status)) //Macro WIFEXITED: Retorna verdadero si el hijo termino de forma normal
            { 
                //WEXISTATUS(status) == 0 indica ejecucion exitosa (cod salida 0)
                printf("program execution successful\n");  
            } 
            else if (WIFEXITED(status) && WEXITSTATUS(status)) 
            { 
                //Entra si el hijo termino pero devolvio un codigo de error distinto de 0
                if (WEXITSTATUS(status) == 127) 
                { 
                    printf("execv failed\n");
                }
            }
        }
    }
    return 0;
}