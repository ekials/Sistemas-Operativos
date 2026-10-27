# 05 – Ejercicios tipo control (5 procesos + semáforos + shm)

Todos están **compilados y probados** (`gcc -Wall -Wextra` sin warnings). Cada archivo trae el
enunciado arriba y es **autocontenido**: puedes llevar un solo archivo y funciona.

| Archivo | Enunciado | Semáforos | Salida probada |
|---|---|---|---|
| `PLANTILLA.c` | Base para cualquier ejercicio | 1 mutex | `contador final = 5` |
| `ej1_contador_5procesos.c` | 5 procesos suman 100000 c/u a un contador en shm, con/sin semáforo | 1 mutex | sin: `336007` ❌ / con: `500000` ✅ |
| `ej2_letras_5procesos.c` | El código de clase con 5 procesos, guardando las letras en shm | 1 mutex | `hhSSxxmmuu...` pares intactos ✅ |
| `ej3_turnos_en_orden.c` | Los 5 escriben en orden fijo 0→1→2→3→4 durante 3 rondas | 5 (uno por proceso) | `0 1 2 3 4 0 1 2 3 4 ...` |
| `ej4_productor_consumidor.c` | 4 productores + 1 consumidor, buffer de 4 en shm | 3 (mutex, vacíos, llenos) | `suma = 5040` |
| `ej5_terminales_c_g.c` | Estilo de clase: 5 terminales escriben en un "muro" compartido | 1 mutex, key fija | 15 mensajes sin mezclarse |

## Receta para cualquier enunciado

1. ¿Qué datos comparten? → eso va en `struct Compartido` en la shm.
2. ¿Qué código toca esos datos? → eso es la sección crítica → `semP` / `semV`.
3. ¿Hay que esperar a alguien o seguir un orden? → semáforos extra que **empiezan en 0**.
4. ¿Procesos con `fork` o en terminales distintas?
   - `fork` → `IPC_PRIVATE` y todo se crea antes del fork (ej1–ej4).
   - Terminales → **llave fija** + modo `c` / `g` (ej5, como en clase).
5. Padre: `wait` N veces, imprime el resultado, borra todo.

## Patrones de semáforos

| Problema | Valores iniciales | Idea |
|---|---|---|
| Exclusión mutua | `mutex = 1` | `P(mutex)` … `V(mutex)` |
| Orden / turnos | `sem[0] = 1`, resto `0` | proceso i: `P(sem[i])` … `V(sem[i+1])` |
| Esperar que alguien termine | `listo = 0` | el que espera hace `P`, el que termina `V` |
| Máximo N a la vez | `sem = N` | igual que mutex pero con N |
| Productor-consumidor | `mutex=1, vacios=TAM, llenos=0` | prod: `P(vacios) P(mutex) … V(mutex) V(llenos)` |

⚠️ En productor-consumidor el orden importa: primero `P(vacios)` y **después** `P(mutex)`.
Al revés hay **deadlock** (el productor duerme con el mutex tomado).

## Variaciones que te podrían pedir (practica cambiando estos archivos)

- **ej1**: que cada hijo imprima cuánto sumó → agrega un arreglo `int parcial[5]` a la struct.
- **ej2**: imprimir las letras **3 veces** o que cada proceso tenga su palabra por `argv`.
- **ej3**: orden **al revés** (4→0) → `sem[4] = 1` y pasar a `(i + 4) % 5`.
- **ej3**: solo los pares primero (0, 2, 4) y luego los impares → cambia el "siguiente".
- **ej4**: 2 productores y 3 consumidores → cada consumidor saca `total / 3`.
- **ej5**: agregar opción `v` que muestre el valor del semáforo con `semctl(id, 0, GETVAL)`.
- **Cualquiera**: que el padre **espere** a que todos terminen usando un semáforo en vez de `wait`
  (semáforo `fin = 0`, cada hijo hace `V(fin)`, el padre hace `P(fin)` 5 veces).
