# 07 – Práctica de cambios (sección 9 de la guía, todos resueltos y probados)

| Archivo | Caso | Compilar y correr | Sale |
|---|---|---|---|
| `91_3procesos.c` | 3 procesos en vez de 5 | `gcc 91_3procesos.c -o p && ./p` | contador final = 3 |
| `92_palabras_argv.c` | palabras por argumento | `./p hola xyz ucsp SO mutex` | Pares intactos: SI |
| `93_letras_3veces.c` | cada letra 3 veces | `./p` | Trios intactos: SI |
| `94_dos_a_la_vez.c` | 2 procesos a la vez en la s.c. | `./p` | ENTRA de a 2 |
| `95_orden_inverso.c` | turnos 4 3 2 1 0 | `./p` | 4 3 2 1 0 ... |
| `96_pares_impares.c` | turnos 0 2 4 1 3 (cambia `orden[]`) | `./p` | 0 2 4 1 3 ... |
| `97_padre_espera_sem.c` | padre espera con semáforo | `./p` | los 5 avisaron |
| `98_suma_paralela.c` | suma 1..100 entre 5 | `./p` | 5050 |
| `99_resultado_casilla.c` | cada hijo su casilla | `./p` | 1 4 9 16 25 |
| `910_prodcons_3prod.c` | buffer 3, 3 productores | `./p` | suma = 3030 |
| `911_plantilla_terminales.c` | PLANTILLA para terminales | `./p c` → `./p t 1` / `./p t 2` / `./p t 3` (3 terminales) → `./p r` → `./p d` | contador = 9 |
| `912_mensaje_a_cada_hijo.c` | mensaje a un hijo específico (mtype) | `./p` | cada hijo recibe el suyo |
| `913_hijos_mensajes_padre.c` | hijos mandan resultado al padre | `./p` | hijo i mando i*10 |
| `914_scanf_fuera.c` | pedir datos (scanf ANTES de la s.c.) | `./p` y escribe 5 números | total = suma |
| `916_ver_valor_sem.c` | mostrar valor del semáforo | `./p` | adentro 0, al final 1 |

Todos los cambios están marcados con `/* CAMBIO */` en el código.

## ⚠️ SEM_UNDO: solo en el mutex

`SEM_UNDO` deshace las operaciones de un proceso cuando termina. Para un **mutex** está bien.
Pero en semáforos de **aviso, turnos o contadores** (fin, turnos, llenos/vacíos) **NO** se usa:
si un proceso hace `V` y muere antes de que el otro haga `P`, el aviso se borra y el otro se cuelga.
Por eso ej3, ej4, ej6, 95, 96, 910 usan flag `0` y el 97 usa `semVaviso` sin `SEM_UNDO`.

Si algo da `Invalid argument` o se cuelga: `ipcs` y borra con `ipcrm -S 0x1111` / `ipcrm -M 0x2222`.
