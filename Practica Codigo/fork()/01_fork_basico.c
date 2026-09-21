#include <stdio.h>
#include <unistd.h> //para fork(), getpid(), getppid()

int main (void)
{
    pid_t pid;
    printf("Antes del fork (solo existe el proceso padre) \n");

    pid = fork(); //proceso hijo

    if(pid == 0)
    {
        //cod del hijo
        printf("[HIJO] -> Mi PID es: %d | El PID de mi Padre es: %d\n", getpid(), getppid());
    }
    else if(pid > 0)
    {
        //codigo del padre
        printf("[PADRE] -> Mi PID es: %d | Creé al hijo con PID: %d\n", getpid(), pid);
    
    }
    else
    {
        //error
        perror("fallo en el fork");
        return 1;
    }
    return 0;
}