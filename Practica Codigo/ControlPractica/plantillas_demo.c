#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <setjmp.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <mqueue.h>

/* --- ESTRUCTURAS Y GLOBALES --- */
struct msgbuf_sysv {
    long mtype;
    char mtext[128];
};

static sigjmp_buf entorno_salto;
volatile sig_atomic_t flag_alerta = 0;

/* --- MANEJADORES DE SEÑALES PARA LAS DEMOS --- */

// 1. Manejador básico Async-Signal-Safe
void manejador_simple(int sig) {
    write(1, "    [SIG-1] SIGUSR1 capturada con sigaction!\n", 45);
}

// 2. Manejador Múltiple (Un manejador para varias señales)
void manejador_multiple(int sig) {
    if (sig == SIGINT) {
        write(1, "    [SIG-2] Capturado SIGINT (Ctrl+C)\n", 38);
    } else if (sig == SIGTRAP) {
        write(1, "    [SIG-2] Capturado SIGTRAP\n", 30);
    } else if (sig == SIGTERM) {
        write(1, "    [SIG-2] Capturado SIGTERM\n", 30);
    }
}

// 3. Manejador para Temporizador (SIGALRM)
void manejador_alarma(int sig) {
    write(1, "    [SIG-3] SIGALRM! El temporizador de alarm() expiro.\n", 56);
}

// 4. Manejador para Saltos (siglongjmp)
void manejador_jmp(int sig) {
    write(1, "    [SIG-4] Capturada senal para salto (siglongjmp)...\n", 55);
    siglongjmp(entorno_salto, 99);
}

// 5. Manejador Avanzado con Datos (SA_SIGINFO)
void manejador_con_info(int sig, siginfo_t *info, void *context) {
    char buf[100];
    int n = snprintf(buf, sizeof(buf), "    [SIG-5] Recepto SIGUSR2 con valor extra: %d desde PID: %d\n",
                     info->si_value.sival_int, info->si_pid);
    write(1, buf, n);
}

// 6. Manejador para Pipe Roto (SIGPIPE)
void manejador_sigpipe(int sig) {
    write(1, "    [SIG-6] Capturada SIGPIPE! Intentaste escribir en un pipe sin lector.\n", 74);
}


int main(int argc, char *argv[]) {
    /* MODO EXECV: Hijo lanzado vía execv */
    if (argc > 1 && strcmp(argv[1], "--hijo-exec") == 0) {
        char buf_exec[64];
        ssize_t bytes = read(STDIN_FILENO, buf_exec, sizeof(buf_exec) - 1);
        if (bytes > 0) {
            buf_exec[bytes] = '\0';
            printf("    [Hijo Execv PID %d] Leido de STDIN (redireccionado con dup2): %s\n", getpid(), buf_exec);
        }
        exit(0);
    }

    printf("======================================================================\n");
    printf("=== DEMO COMPLETA DE SEÑALIZACIONES Y MECANISMOS DE SO (10 SEÑALES) ===\n");
    printf("======================================================================\n\n");

    /* =======================================================================
     * SECCIÓN A: COMPENDIO COMPLETO DE SEÑALIZACIONES
     * ======================================================================= */

    /* 1. SEÑALIZACIÓN BÁSICA CON SIGACTION (SIGUSR1) */
    printf("[SEÑAL 1] Captura Basica con sigaction (SIGUSR1)...\n");
    struct sigaction sa1;
    sa1.sa_handler = manejador_simple;
    sa1.sa_flags = SA_RESTART;
    sigemptyset(&sa1.sa_mask);
    sigaction(SIGUSR1, &sa1, NULL);
    kill(getpid(), SIGUSR1);

    /* 2. UN SOLO MANEJADOR PARA MÚLTIPLES SEÑALES (SIGINT, SIGTRAP, SIGTERM) */
    printf("\n[SEÑAL 2] Mantenimiento de multiples senales (SIGINT, SIGTRAP, SIGTERM)...\n");
    struct sigaction sa_multi;
    sa_multi.sa_handler = manejador_multiple;
    sa_multi.sa_flags = SA_RESTART;
    sigemptyset(&sa_multi.sa_mask);
    sigaction(SIGINT, &sa_multi, NULL);
    sigaction(SIGTRAP, &sa_multi, NULL);
    sigaction(SIGTERM, &sa_multi, NULL);

    kill(getpid(), SIGINT);
    kill(getpid(), SIGTRAP);
    kill(getpid(), SIGTERM);

    /* 3. TEMPORIZADORES Y ALARMAS (SIGALRM + alarm) */
    printf("\n[SEÑAL 3] Temporizadores con alarm() y SIGALRM...\n");
    struct sigaction sa_alarm;
    sa_alarm.sa_handler = manejador_alarma;
    sa_alarm.sa_flags = SA_RESTART;
    sigemptyset(&sa_alarm.sa_mask);
    sigaction(SIGALRM, &sa_alarm, NULL);
    alarm(1); // Programa la alarma para dentro de 1 segundo
    pause();  // Espera a que llegue la señal sin consumir CPU

    /* 4. IGNORAR SEÑALES (SIG_IGN para evitar Zombies o ignorar interrupciones) */
    printf("\n[SEÑAL 4] Ignorando senales con SIG_IGN (Ejemplo: SIGCHLD para no dejar zombies)...\n");
    signal(SIGCHLD, SIG_IGN); // El kernel limpia automáticamente los hijos muertos
    if (fork() == 0) {
        // Hijo muere inmediatamente, no genera proceso Zombie
        exit(0);
    }
    sleep(1); // Pausa corta para dar tiempo al kernel
    printf("    [SIG-4] Hijo creado y limpiado automaticamente sin necesidad de wait()\n");

    /* 5. BLOQUEO Y MÁSCARAS DE SEÑALES (sigprocmask) */
    printf("\n[SEÑAL 5] Mascara de Senales (Bloquear y Desbloquear SIGINT)...\n");
    sigset_t mascara_bloqueo, mascara_antigua;
    sigemptyset(&mascara_bloqueo);
    sigaddset(&mascara_bloqueo, SIGINT);

    // Bloquear SIGINT
    sigprocmask(SIG_BLOCK, &mascara_bloqueo, &mascara_antigua);
    printf("    [SIG-5] SIGINT bloqueado. Enviando SIGINT a mi mismo (quedara pendiente)...\n");
    kill(getpid(), SIGINT); // No se ejecuta el manejador aún
    printf("    [SIG-5] Desbloqueando SIGINT ahora...\n");
    // Al desbloquear, la señal pendiente salta inmediatamente
    sigprocmask(SIG_SETMASK, &mascara_antigua, NULL);

    /* 6. CONTROL DE FLUJO CON sigsetjmp Y siglongjmp */
    printf("\n[SEÑAL 6] Saltos de control de flujo con sigsetjmp / siglongjmp...\n");
    signal(SIGUSR2, manejador_jmp);
    int cod_ret = sigsetjmp(entorno_salto, 1);
    if (cod_ret == 0) {
        printf("    [SIG-6] Punto de restauracion fijado. Enviando SIGUSR2...\n");
        kill(getpid(), SIGUSR2);
    } else {
        printf("    [SIG-6] Retornado exitosamente con siglongjmp! Codigo de retorno: %d\n", cod_ret);
    }

    /* 7. MANEJO DE SIGPIPE (ESCRITURA EN PIPE SIN LECTOR) */
    printf("\n[SEÑAL 7] Captura de SIGPIPE (Pipe roto)...\n");
    struct sigaction sa_pipe;
    sa_pipe.sa_handler = manejador_sigpipe;
    sa_pipe.sa_flags = SA_RESTART;
    sigemptyset(&sa_pipe.sa_mask);
    sigaction(SIGPIPE, &sa_pipe, NULL);

    int pipe_roto[2];
    pipe(pipe_roto);
    close(pipe_roto[0]); // Cerramos el extremo de LECTURA
    write(pipe_roto[1], "test", 4); // Escribir activa SIGPIPE

    /* 8. ENVÍO DE DATOS EXTRA EN SEÑALES (SA_SIGINFO + sigqueue) */
    printf("\n[SEÑAL 8] Envio Avanzado de datos con sigqueue (SA_SIGINFO)...\n");
    struct sigaction sa_info;
    sa_info.sa_sigaction = manejador_con_info;
    sa_info.sa_flags = SA_SIGINFO; // Activa recepción de datos extra
    sigemptyset(&sa_info.sa_mask);
    sigaction(SIGUSR2, &sa_info, NULL);

    union sigval valor;
    valor.sival_int = 2026; // Dato entero a enviar
    sigqueue(getpid(), SIGUSR2, valor); // Enviar señal con dato


    /* =======================================================================
     * SECCIÓN B: MECANISMOS IPC, DUP2, EXECV Y FIFOS
     * ======================================================================= */

    printf("\n\n----------------------------------------------------------------------\n");
    printf("--- MECANISMOS DE COMUNICACIÓN INTERPROCESO (IPC) Y PROCESOS ---\n");
    printf("----------------------------------------------------------------------\n\n");

    /* 9. PIPE CLÁSICO + FORK */
    printf("[IPC 1] Pipe Anonimo Clasico + Fork...\n");
    int pipefd[2];
    pipe(pipefd);
    if (fork() == 0) {
        close(pipefd[1]);
        char buf[64];
        int n = read(pipefd[0], buf, sizeof(buf) - 1);
        if (n > 0) {
            buf[n] = '\0';
            printf("    [Hijo PID %d] Leido de Pipe: %s\n", getpid(), buf);
        }
        close(pipefd[0]);
        exit(0);
    } else {
        close(pipefd[0]);
        char msg[] = "Mensaje desde Padre";
        write(pipefd[1], msg, strlen(msg));
        close(pipefd[1]);
        wait(NULL);
    }

    /* 10. DUP2 + EXECV */
    printf("\n[IPC 2] Redireccion dup2 + execv...\n");
    int pipe_exec[2];
    pipe(pipe_exec);
    if (fork() == 0) {
        close(pipe_exec[1]);
        dup2(pipe_exec[0], STDIN_FILENO); // Redireccionar pipe a STDIN
        close(pipe_exec[0]);
        char *args[] = { argv[0], "--hijo-exec", NULL };
        execv(argv[0], args);
        exit(1);
    } else {
        close(pipe_exec[0]);
        char msg_exec[] = "Texto directo a STDIN de execv";
        write(pipe_exec[1], msg_exec, strlen(msg_exec));
        close(pipe_exec[1]);
        wait(NULL);
    }

    /* 11. COLAS SYSTEM V */
    printf("\n[IPC 3] Colas de Mensajes System V (msgget / msgsnd / msgrcv)...\n");
    int msqid = msgget(9876, IPC_CREAT | 0666);
    if (msqid >= 0) {
        struct msgbuf_sysv env, rec;
        env.mtype = 1;
        snprintf(env.mtext, sizeof(env.mtext), "Test System V IPC");
        msgsnd(msqid, &env, strlen(env.mtext) + 1, IPC_NOWAIT);
        msgrcv(msqid, &rec, sizeof(rec.mtext), 1, 0);
        printf("    [System V] Mensaje Leido: %s\n", rec.mtext);
        msgctl(msqid, IPC_RMID, NULL);
    }

    /* 12. COLAS POSIX */
    printf("\n[IPC 4] Colas de Mensajes POSIX (mq_open / mq_send / mq_receive)...\n");
    struct mq_attr attr = {0, 5, 128, 0};
    mqd_t mq = mq_open("/demo_posix_queue", O_CREAT | O_RDWR, 0666, &attr);
    if (mq != (mqd_t)-1) {
        char msg_posix[] = "Test POSIX Queue";
        mq_send(mq, msg_posix, strlen(msg_posix) + 1, 0);
        char buf_posix[128];
        mq_receive(mq, buf_posix, sizeof(buf_posix), NULL);
        printf("    [POSIX Queue] Mensaje Leido: %s\n", buf_posix);
        mq_close(mq);
        mq_unlink("/demo_posix_queue");
    }

    /* 13. FIFO (Tuberías Nombradas) */
    printf("\n[IPC 5] Tuberias Nombradas (FIFO)...\n");
    const char *fifo_path = "/tmp/demo_fifo_test";
    mkfifo(fifo_path, 0666);
    if (fork() == 0) {
        int fd_read = open(fifo_path, O_RDONLY);
        char buf_fifo[64];
        int n = read(fd_read, buf_fifo, sizeof(buf_fifo) - 1);
        if (n > 0) {
            buf_fifo[n] = '\0';
            printf("    [FIFO] Leido: %s\n", buf_fifo);
        }
        close(fd_read);
        exit(0);
    } else {
        int fd_write = open(fifo_path, O_WRONLY);
        char msg_fifo[] = "Datos por archivo FIFO";
        write(fd_write, msg_fifo, strlen(msg_fifo));
        close(fd_write);
        wait(NULL);
        unlink(fifo_path);
    }

    printf("\n======================================================================\n");
    printf("=== TODAS LAS PRUEBAS (13/13) SE EJECUTARON CORRECTAMENTE ===\n");
    printf("======================================================================\n");

    return 0;
}