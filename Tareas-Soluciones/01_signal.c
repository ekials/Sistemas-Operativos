#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <setjmp.h>

//  https://people.cs.pitt.edu/~alanjawi/cs449/code/shell/UnixSignals.htm

/* 
SIGHUP	1	Exit	Hangup
SIGINT	2	Exit	Interrupt
SIGQUIT	3	Core	Quit
SIGILL	4	Core	Illegal Instruction
SIGTRAP	5	Core	Trace/Breakpoint Trap
SIGABRT	6	Core	Abort
SIGEMT	7	Core	Emulation Trap
SIGFPE	8	Core	Arithmetic Exception
SIGKILL	9	Exit	Killed
SIGBUS	10	Core	Bus Error
SIGSEGV	11	Core	Segmentation Fault
SIGSYS	12	Core	Bad System Call
SIGPIPE	13	Exit	Broken Pipe
SIGALRM	14	Exit	Alarm Clock
SIGTERM	15	Exit	Terminated
SIGUSR1	16	Exit	User Signal 1
SIGUSR2	17	Exit	User Signal 2
SIGCHLD	18	Ignore	Child Status
SIGPWR	19	Ignore	Power Fail/Restart
SIGWINCH	20	Ignore	Window Size Change
SIGURG	21	Ignore	Urgent Socket Condition
SIGPOLL	22	Ignore	Socket I/O Possible
SIGSTOP	23	Stop	Stopped (signal)
SIGTSTP	24	Stop	Stopped (user)
SIGCONT	25	Ignore	Continued
SIGTTIN	26	Stop	Stopped (tty input)
SIGTTOU	27	Stop	Stopped (tty output)
SIGVTALRM	28	Exit	Virtual Timer Expired
SIGPROF	29	Exit	Profiling Timer Expired
SIGXCPU	30	Core	CPU time limit exceeded
SIGXFSZ	31	Core	File size limit exceeded
SIGWAITING	32	Ignore	All LWPs blocked
SIGLWP	33	Ignore	Virtual Interprocessor Interrupt for Threads Library
SIGAIO	34	Ignore	Asynchronous I/O

*/

// se guarda el punto de recuperacion (jump)
static sigjmp_buf punto_salto;

// Cont global para la F2
static int contador_usr1 = 0;

//F1: SIGINT -> hace el salto simple 
void sighandler_sigint(int signum) {
   printf("\n[F1] Caught SIGINT (signal %d). Saltando al inicio\n", signum);
   siglongjmp(punto_salto, 1);
}

//F2: SIGUSR1 -> lleva un contador y luego salta
void sighandler_sigusr1(int signum) {
   contador_usr1++;
   printf("\n[F2] Caught SIGUSR1 (signal %d). Van %d veces recibida. Saltando\n",
          signum, contador_usr1);
   siglongjmp(punto_salto, 2);
}

// F3: SIGUSR2 -> no salta, termina el programa
void sighandler_sigusr2(int signum) {
   printf("\n[F3] Caught SIGUSR2 (signal %d). Terminando el programa (sin salto)\n", signum);
   exit(0);
}

int main () {
   printf("PID de este proceso: %d\n", getpid());

   signal(SIGINT, sighandler_sigint);
   signal(SIGUSR1, sighandler_sigusr1);
   signal(SIGUSR2, sighandler_sigusr2);

   int origen = sigsetjmp(punto_salto, 1);

   if (origen == 0) {
      printf("Programa iniciado.\n");
      printf("Desde otra terminal prueba:\n");
      printf("  kill -SIGUSR1 %d   -> cuenta y salta\n", getpid());
      printf("  kill -SIGUSR2 %d   -> termina el programa\n", getpid());
      printf("O Ctrl+C aqui mismo -> SIGINT, salta\n\n");
   } else if (origen == 1) {
      printf("Salto realizado desde SIGINT (F1)\n");
   } else if (origen == 2) {
      printf("Salto realizado desde SIGUSR1 (F2)\n");
   }

   while(1) {
      printf("Going to sleep for a second\n");
      sleep(1); 
   }

   return(0);
}
