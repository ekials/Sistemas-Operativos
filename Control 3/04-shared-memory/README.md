# 04 – Shared memory

Después de `fork()`, padre e hijo **NO comparten variables** (cada uno tiene su copia).
La shared memory es un pedazo de RAM que varios procesos **sí** ven igual.

| Archivo | Qué hace |
|---|---|
| `05.1_shm_server.c` / `05.2_shm_client.c` | Originales del profe ("códigos 5.1 y 5.2") |
| `05.1_shm_server_fix.c` / `05.2_shm_client_fix.c` | Comentados y sin los warnings de la captura |
| `shm-posix-producer-orig.c` / `shm-posix-consumer-orig.c` | Mismo concepto con la API **POSIX** (libro Silberschatz) |

## 5.1 / 5.2 – cómo funciona

```bash
gcc 05.1_shm_server_fix.c -o server && ./server     # terminal 1
gcc 05.2_shm_client_fix.c -o client && ./client     # terminal 2 → abcdefghijklmnopqrstuvwxyz
```

1. **Server** crea el segmento (`key = 5678`, 27 bytes), escribe `a..z` y se queda esperando.
2. **Client** busca el mismo segmento (misma key), lo lee y pone `*` en la primera posición.
3. Server ve el `*` y termina.

Ese `while (*shm != '*') sleep(1);` es sincronización **casera** (espera activa).
**En el control la sincronización va con semáforo**: eso es exactamente lo que juntan los ejercicios.

## Las 4 llamadas (System V)

```c
int id  = shmget(key, tamaño, 0666 | IPC_CREAT);   // crear/obtener
void *p = shmat(id, NULL, 0);                       // pegar a mi memoria (devuelve puntero)
shmdt(p);                                           // despegar
shmctl(id, IPC_RMID, NULL);                         // borrar
```

**Truco:** guarda una `struct` en la shm y castea el puntero:

```c
struct Compartido { int contador; char buf[256]; };
struct Compartido *mem = shmat(id, NULL, 0);
mem->contador++;   // ¡dentro de semP / semV!
```

## Warnings de la captura de clase

`implicit declaration of function 'exit'` → faltaba `#include <stdlib.h>`.
`comparison between pointer and integer` → `*s != NULL`; para un `char` se compara con `'\0'`.

## System V vs POSIX

| | System V (5.1/5.2) | POSIX (producer/consumer) |
|---|---|---|
| Identificador | key numérica | nombre (`"OS"`) |
| Crear | `shmget` | `shm_open` + `ftruncate` |
| Pegar | `shmat` | `mmap` |
| Borrar | `shmctl IPC_RMID` | `shm_unlink` |
| Compilar | normal | `-lrt` |
