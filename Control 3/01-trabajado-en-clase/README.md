# 01 – Trabajado en clase (20/10)

Temas de la clase del 20/10: comandos `ipcs`, el man de `semget`, "códigos 11 menos old",
race conditions y "códigos 5.1 y 5.2". Este es el código que se corrió en clase.

| Archivo | Qué es |
|---|---|
| `11_Semaphore.c` | Original del profe, tal cual |
| `11_Semaphore_comentado.c` | El mismo, comentado línea por línea y con 3 bugs corregidos (`[FIX]`) |

## Qué hace

Varios **programas independientes** (cada uno en su terminal) comparten **un solo semáforo** del
sistema, identificado por la llave `KEY = 0x1111`. Cada programa imprime cada letra de su texto
**dos veces**, y esas dos impresiones están dentro de la **sección crítica**.

```
semP(id);          // pide el semáforo (si otro lo tiene, ESPERA aquí)
putchar(s[i]);
sleep(rand()%2);   // tarda a propósito para que otro quiera entrar
putchar(s[i]);
semV(id);          // lo suelta
```

**Resultado:** las letras de distintos procesos se intercalan, pero los **pares nunca se rompen**:

```
./sc c "hola"   y   ./sc g "xyz"   a la vez  →  hhooxxllyyaazz   ✅
sin semáforo                                 →  hxhoxl...         ❌
```

## Cómo correrlo (lo que se ve en la captura de clase)

```bash
gcc 11_Semaphore_comentado.c -o sc
./sc c "hola"          # terminal 1: c = CREA el semáforo y lo inicializa en 1
./sc g "xyz"           # terminal 2: g = OBTIENE el que ya existe
./sc g "catolica"      # terminal 3
ipcs -s                # ver el semáforo (key 0x00001111, nsems 1)
```

## Partes clave

| Función | Llamada al sistema | Para qué |
|---|---|---|
| `getSemCreate()` | `semget(KEY, 1, 0666 \| IPC_CREAT)` | crea un conjunto de **1** semáforo |
| `getSem()` | `semget(KEY, 1, 0)` | solo lo busca; si no existe → error |
| `controlSem(id, 1)` | `semctl(id, 0, SETVAL, u)` | valor inicial = 1 → **mutex** |
| `semP(id)` | `semop` con `{0, -1, 0}` | resta 1 (bloquea si está en 0) |
| `semV(id)` | `semop` con `{0, +1, 0}` | suma 1 (despierta a otro) |

`struct sembuf = {número de semáforo, operación, flags}`.

## El "ojo!!!" del profe

```c
if (argv[2][0] == 'a') scanf(" %c", &c);   // DENTRO de la sección crítica
```

Si corres `./sc g "ahora"`, ese proceso se queda esperando una tecla **teniendo el semáforo**, y
todos los demás procesos se quedan **bloqueados** hasta que presiones algo.
**Lección:** nunca pidas input (ni hagas nada lento) dentro de la sección crítica.

## Bugs del original (te pueden servir si te dan código "para arreglar")

1. **No valida `argc`** → `./sc` sin argumentos = `Segmentation fault` (sale en la captura).
2. **`id` sin inicializar** si la opción no es `c` ni `g`.
3. **No hay `srand`** → todos los procesos usan la misma secuencia "random".
4. Ambos `perror` dicen `"semop p"` (el de V debería decir `"semop v"`).

## Por qué con `c` y `g` y no `fork`

Como son **programas distintos** (no padre/hijo), no pueden heredar el `id`. Por eso usan una
**llave fija** (`0x1111`): cualquiera que conozca la llave encuentra el mismo semáforo.
Con `fork()` en cambio puedes usar `IPC_PRIVATE` porque los hijos heredan el id
(ver `05-ejercicios-tipo-control`).
