#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
int main (void)
{
    int pid;
    pid = fork();

    if(pid == 0)
    {
        //cod  del proceso hijo
        system("echo 'hijo'");
        system ("echo ");
        sleep (100); //pausa para inspeccionar terminal

    }

    //cod proceso padre
    else
    {
        system("echo 'padre'");
        system("echo ");
        sleep(100);
    }
    return 0;
}