
/*This is a program to illustrate
*the semaphore control, semctl(),
*system call capabilities.
*/

/*Include necessary header files.*/
#include    <stdio.h>
#include    <sys/types.h>
#include    <sys/ipc.h>
#include    <sys/sem.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

/*Start of main C language program*/
main()
{
    extern int errno;
    struct semid_ds semid_ds;
    int c, i, length;
    int uid, gid, mode;
    int retrn, semid, semnum, cmd, choice;
    ushort semvals[25];
    union semun  {
        int val;
        struct semid_ds *buf;
        ushort *array;
    } arg;

    /*Enter the semaphore ID.*/
    printf("Enter the semid = ");
    scanf("%d", &semid);

    /*Choose the desired command.*/
    printf("\nEnter the number for\n");
    printf("the desired cmd:\n");
    printf("GETVAL      =  1\n");
    printf("SETVAL      =  2\n");
    printf("GETPID      =  3\n");
    printf("GETNCNT     =  4\n");
    printf("GETZCNT     =  5\n");
    printf("GETALL      =  6\n");
    printf("SETALL      =  7\n");
    printf("IPC_STAT    =  8\n");
    printf("IPC_SET     =  9\n");
    printf("IPC_RMID    =  10\n");
    printf("Entry       =  ");
    scanf("%d", &cmd);

    /*Check entries.*/
    printf ("\nsemid =%d, cmd = %d\n\n",
        semid, cmd);

    /*Set the command and do the call.*/
    switch (cmd)
    {

    case 1: /*Get a specified value.*/
        printf("\nEnter the semnum = ");
        scanf("%d", &semnum);
        /*Do the system call.*/
        retrn = semctl(semid, semnum, GETVAL, arg);
        printf("\nThe semval = %d", retrn);
        break;
    case 2: /*Set a specified value.*/
        printf("\nEnter the semnum = ");
        scanf("%d", &semnum);
        printf("\nEnter the value = ");
        scanf("%d", &arg.val);
        /*Do the system call.*/
        retrn = semctl(semid, semnum, SETVAL, arg);
        break;
    case 3: /*Get the process ID.*/
        retrn = semctl(semid, 0, GETPID, arg);
        printf("\nThe sempid = %d", retrn);
        break;
    case 4: /*Get the number of processes
        waiting for the semaphore to
        become greater than its current
        value.*/
        printf("\nEnter the semnum = ");
        scanf("%d", &semnum);
        /*Do the system call.*/
        retrn = semctl(semid, semnum, GETNCNT, arg);
        printf("\nThe semncnt = %d", retrn);
        break;

    case 5: /*Get the number of processes
        waiting for the semaphore
        value to become zero.*/
        printf("\nEnter the semnum = ");
        scanf("%d", &semnum);
        /*Do the system call.*/
        retrn = semctl(semid, semnum, GETZCNT, arg);
        printf("\nThe semzcnt = %d", retrn);
        break;

    case 6: /*Get all of the semaphores.*/
        /*Get the number of semaphores in
          the semaphore set.*/
        arg.buf = & semid_ds;
        retrn = semctl(semid, 0, IPC_STAT, arg);
        if(retrn == -1)
            goto ERROR;
        length = arg.buf->sem_nsems;
        /*Get and print all semaphores in the
          specified set.*/
        arg.array = semvals;
        retrn = semctl(semid, 0, GETALL, arg);
        for (i = 0; i < length; i++)
        {
              printf("%d", semvals[i]);
              /*Separate each
                semaphore.*/
              printf(" ");
          }
          break;

      case 7: /*Set all semaphores in the set.*/
          /*Get the number of semaphores in
            the set.*/
          arg.buf = & semid_ds;
          retrn = semctl(semid, 0, IPC_STAT, arg);
          if(retrn == -1)
              goto ERROR;
          length = arg.buf->sem_nsems;
          printf("Length = %d\n", length);
          /*Set the semaphore set values.*/
          printf("\nEnter each value:\n");
          for(i = 0; i < length ; i++)
          {
              scanf("%d", &c);
              semvals[i] = c;
          }
          /*Do the system call.*/
          arg.array = semvals;
          retrn = semctl(semid, 0, SETALL, arg);
          break;

      case 8: /*Get the status for the semaphore set.*/
          /*Get and print the current status values.*/
          arg.buf = & semid_ds;
          retrn = semctl(semid, 0, IPC_STAT, arg);
          printf ("\nThe USER ID = %d\n",
              arg.buf->sem_perm.uid);
          printf ("The GROUP ID = %d\n",
              arg.buf->sem_perm.gid);
          printf ("The operation permissions = 0%o\n",
              arg.buf->sem_perm.mode);
          printf ("The number of semaphores in set = %d\n",
              arg.buf->sem_nsems);
          printf ("The last semop time = %d\n",
              arg.buf->sem_otime);
          printf ("The last change time  = %d\n",
              arg.buf->sem_ctime);
          break;

      case 9:    /*Select and change the desired
                    member of the data structure.*/
          /*Get the current status values.*/
          arg.buf = & semid_ds;
          retrn = semctl(semid, 0, IPC_STAT, arg.buf);
          if(retrn == -1)
              goto ERROR;
          /*Select the member to change.*/
          printf("\nEnter the number for the\n");
          printf("member to be changed:\n");
          printf("sem_perm.uid   = 1\n");
          printf("sem_perm.gid   = 2\n");
          printf("sem_perm.mode  = 3\n");
          printf("Entry          = ");
          scanf("%d", &choice);
          switch(choice){

          case 1: /*Change the user ID.*/
              printf("\nEnter USER ID = ");
              scanf ("%d", &uid);
              arg.buf->sem_perm.uid = uid;
              printf("\nUSER ID = %d\n",
                  arg.buf->sem_perm.uid);
              break;

          case 2: /*Change the group ID.*/
              printf("\nEnter GROUP ID = ");
              scanf("%d", &gid);
              arg.buf->sem_perm.gid = gid;
              printf("\nGROUP ID = %d\n",
                  arg.buf->sem_perm.gid);
              break;

          case 3: /*Change the mode portion of
                the operation
                            permissions.*/
              printf("\nEnter MODE in octal = ");
              scanf("%o", &mode);
              arg.buf->sem_perm.mode = mode;
              printf("\nMODE = 0%o\n",
                  arg.buf->sem_perm.mode);
              break;
          default: /* Invalid Input */
              exit(-1);
          }
          /*Do the change.*/
          retrn = semctl(semid, 0, IPC_SET, arg);
          break;
      case 10:    /*Remove the semid along with its
                    data structure.*/
          retrn = semctl(semid, 0, IPC_RMID, arg);
          break;
      default: /* Invalid Input */
          exit(-1);
      }
      /*Perform the following if the call is unsuccessful.*/
      if(retrn == -1)
      {
  ERROR:
         printf ("\nThe semctl call failed!, error number =  %d\n", errno);
          exit(0);
      }
      printf ("\n\nThe semctl system call was successful\n");
      printf ("for semid = %d\n", semid);
      exit (0);
  }

