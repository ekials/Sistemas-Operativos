# 06 – Colas de mensajes (pasar información por mensaje)

Shared memory = una pizarra que todos leen/escriben (necesita semáforo).
Cola de mensajes = un **buzón**: uno manda, otro recibe, y el SO ya sincroniza
(si no hay mensajes, `receive` **se bloquea solo**; no hace falta semáforo).

Hay 2 versiones. El profe tiene las dos (`04.1 mq_s.c`/`04.2 mq_c.c` y `mq_s_sysV.c`/`mq_c_sysV.c`, `ipc_msgq_*`).

## 1) POSIX – la del nombre `"/mylib"`

| Archivo | Uso |
|---|---|
| `mq_posix_send.c` | `gcc mq_posix_send.c -o send -lrt` → `./send "hola"` |
| `mq_posix_recv.c` | `gcc mq_posix_recv.c -o recv -lrt` → `./recv` (recibe hasta `"fin"` y borra la cola) |

```c
mqd_t mq = mq_open("/mylib", O_CREAT | O_WRONLY, 0666, &attr);  // abrir/crear
mq_send(mq, texto, strlen(texto)+1, prioridad);                 // mandar
mq_receive(mq, buf, TAMMSG, &prio);                             // recibir (bloquea)
mq_close(mq);                                                   // cerrar MI acceso
mq_unlink("/mylib");                                            // BORRAR la cola
```

**Lo de "/mylib que no puedes acceder pero por ahí ves el mensaje":**
el nombre empieza con `/` pero **no es un archivo de tu carpeta**. Linux lo guarda en un
sistema de archivos especial:

```bash
ls /dev/mqueue            # aparece: mylib
cat /dev/mqueue/mylib     # QSIZE:13 ... -> bytes esperando en la cola
```

(Si `/dev/mqueue` no existe en tu WSL: `sudo mkdir -p /dev/mqueue && sudo mount -t mqueue none /dev/mqueue`)

⚠️ Errores típicos POSIX:
- Olvidar `-lrt` al compilar → `undefined reference to mq_open`.
- El buffer de `mq_receive` es más chico que `mq_msgsize` → `Message too long`.
- El nombre **debe** empezar con `/`.
- `mq_close` no borra; borra `mq_unlink`.

## 2) System V – misma familia que `semget` / `shmget`

| Archivo | Uso |
|---|---|
| `msgq_sysv_send.c` | `./msend 1 "hola"` → manda con **tipo** 1 |
| `msgq_sysv_recv.c` | `./mrecv 0` (cualquier tipo) o `./mrecv 2` (solo tipo 2) |

```c
struct mensaje { long mtype; char texto[256]; };   // SIEMPRE empieza con long mtype > 0
int id = msgget(KEY, 0666 | IPC_CREAT);
msgsnd(id, &m, tamaño_SIN_el_long, 0);
msgrcv(id, &m, sizeof(m.texto), tipo, 0);          // tipo 0 = el primero que haya
msgctl(id, IPC_RMID, NULL);                         // borrar
```

Ver/borrar: `ipcs -q`, `ipcrm -q <msqid>`.

**El tipo (`mtype`) sirve para mandar a alguien en particular**: el proceso 3 hace
`msgrcv(..., 3, ...)` y solo recibe los mensajes de tipo 3.

## Comparación rápida

| | Shared memory | Cola de mensajes |
|---|---|---|
| Idea | pizarra compartida | buzón |
| ¿Necesita semáforo? | **Sí** | No (el SO bloquea solo) |
| Bueno para | datos grandes, muchos lectores | avisos, datos chicos, orden |
| System V | `shmget/shmat` | `msgget/msgsnd/msgrcv` |
| POSIX | `shm_open/mmap` | `mq_open/mq_send/mq_receive` |

Si te piden **combinar las 3 cosas**, ve `05-ejercicios-tipo-control/ej7_combinado_shm_sem_msg.c`:
los clientes escriben en shm (con semáforo) y avisan al servidor por la cola.
