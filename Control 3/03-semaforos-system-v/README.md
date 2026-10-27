# 03 – Semáforos System V

| Archivo | Qué hace | ¿Importa? |
|---|---|---|
| `10_semaphore.c` | `fork()` + 1 semáforo mutex: padre imprime `abcd...`, hijo `ABCD...`, pares intactos | ⭐ base del control |
| `11_semaphoreinit.c` | Solo **crea** un semáforo (key `0x4444`) y lo pone en 1, y termina | ⭐ ejemplo mínimo |
| `11_semaphoreDemo_OLD.c` | Menú interactivo para probar **todas** las opciones de `semctl` | referencia ("menos old" = no es el foco) |

## Las 3 llamadas

```c
int id = semget(KEY, nsems, 0666 | IPC_CREAT);   // crear/obtener un CONJUNTO de nsems semáforos
semctl(id, num, SETVAL, u);                       // u.val = valor inicial del semáforo num
semop(id, &op, 1);                                // op = {num, -1 o +1, flags}
```

- `KEY`: número fijo para que procesos distintos encuentren el mismo semáforo.
  Con `fork` puedes usar `IPC_PRIVATE`.
- `nsems`: cuántos semáforos hay en el conjunto (1 para mutex; 5 para turnos; 3 para prod-cons).
- `union semun`: **tienes que declararla tú** (Linux no la trae).

## `SEM_UNDO` (diferencia entre el 10 y el 11)

- `10_semaphore.c` usa `{0, -1, SEM_UNDO}`: si el proceso muere con el semáforo tomado, el SO
  lo devuelve automáticamente → no se quedan todos bloqueados.
- `11_Semaphore.c` usa `{0, -1, 0}`: si haces Ctrl+C dentro de la sección crítica, el semáforo
  queda en 0 para siempre (hay que borrarlo con `ipcrm`).

**En el control usa `SEM_UNDO`.**

## Opciones de `semctl` (del `11_semaphoreDemo_OLD.c`)

| cmd | Qué hace |
|---|---|
| `GETVAL` | devuelve el valor actual |
| `SETVAL` | pone un valor (`u.val`) |
| `GETPID` | pid del último que hizo semop |
| `GETNCNT` | cuántos procesos esperan a que suba |
| `GETZCNT` | cuántos esperan a que llegue a 0 |
| `GETALL` / `SETALL` | leer / poner todos los del conjunto (`u.array`) |
| `IPC_STAT` / `IPC_SET` | info y permisos |
| `IPC_RMID` | **borrar** el semáforo |

Man page: https://man7.org/linux/man-pages/man2/semget.2.html
