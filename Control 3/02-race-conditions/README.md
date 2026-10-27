# 02 – Race conditions

**Race condition** = varios procesos/hilos usan el mismo dato al mismo tiempo y el resultado
depende de quién llega primero. Es no determinístico.

| Archivo | Qué demuestra | Compilar |
|---|---|---|
| `10_raceCondition.c` | 2 procesos (`fork`) imprimen letras dobles **sin** semáforo → se mezclan | `gcc 10_raceCondition.c -o rc` |
| `badcnt.c` | 2 **hilos** suman 1.000.000 c/u a `cnt` → da menos de 2.000.000 | `gcc badcnt.c -o bad -lpthread` |
| `goodcnt.c` | **Solución** (Activity 2): mismo código con `sem_t mutex` → siempre 2.000.000 | `gcc goodcnt.c -o good -lpthread` |
| `semaphores.txt` | Apuntes del profe sobre semáforos POSIX (`sem_init`, `sem_wait`, `sem_post`) | – |

## Por qué falla `cnt++`

`cnt = cnt + 1` en realidad son 3 pasos: **leer → sumar → escribir**.

```
Hilo 1          Hilo 2          cnt
lee 0                            0
                lee 0            0
escribe 1                        1
                escribe 1        1   ← se perdió una suma
```

Probado aquí: `badcnt` dio `1140672` en vez de `2000000`; `goodcnt` dio `2000000`.

## `10_raceCondition.c` vs `10_semaphore.c`

Son **el mismo programa**, la única diferencia es que el segundo pone `semop P` antes de
los `putchar` y `semop V` después. Compáralos lado a lado: es la forma más clara de ver
qué agrega un semáforo.

- Sin semáforo: `aAaBbAB...` (pares rotos)
- Con semáforo: `aaAAbbBB...` (pares intactos)

## Dos tipos de semáforos (no confundir)

| | POSIX (`semaphore.h`) | System V (`sys/sem.h`) |
|---|---|---|
| Se usó en | `badcnt` / `goodcnt` / `semaphores.txt` | todos los códigos 10, 11 y el control |
| Crear | `sem_init(&s, 0, 1)` | `semget` + `semctl SETVAL` |
| P / V | `sem_wait` / `sem_post` | `semop` con -1 / +1 |
| Típico para | hilos | **procesos** (lo que entra) |
