#ifndef __MM_H__
#define __MM_H__

#define _XOPEN_SOURCE	500

typedef unsigned long long int uint64;
typedef uint64 Obj;

#define MAX_UINT64	((uint64)(-1))

/*
 * Los ultimos 3 bits de un objeto son flags que representan:
 *
 *   0x1: si este objeto es la continuacion de una estructura
 *        (una estructura es un arreglo de objetos,
 *        el primero tiene este flag en 0 y los siguientes
 *        en 1).
 *   0x2: si esta estructura ya fue alcanzada por el
 *        proceso de garbage collection.
 *   0x4: si este objeto es un handle que representa un
 *        puntero a otro objeto (en caso contrario se trata
 *        de un valor inmediato).
 *
 */

#define OBJ_FLAG_CONTINUE	((Obj)0x1)
#define OBJ_FLAG_REACH		((Obj)0x2)
#define OBJ_FLAG_HANDLE		((Obj)0x4)
#define OBJ_ALL_FLAGS		(OBJ_FLAG_CONTINUE | OBJ_FLAG_REACH | OBJ_FLAG_HANDLE)

#define OBJ_SET_FLAG(X, F) 		((X) = (X) | (F))
#define OBJ_UNSET_FLAG(X, F) 		((X) = (X) & ~(F))
#define OBJ_SET_FLAG_CONTINUE(X) 	OBJ_SET_FLAG(X, OBJ_FLAG_CONTINUE)
#define OBJ_UNSET_FLAG_CONTINUE(X) 	OBJ_UNSET_FLAG(X, OBJ_FLAG_CONTINUE)
#define OBJ_SET_FLAG_REACH(X) 		OBJ_SET_FLAG(X, OBJ_FLAG_REACH)
#define OBJ_UNSET_FLAG_REACH(X) 	OBJ_UNSET_FLAG(X, OBJ_FLAG_REACH)
#define OBJ_SET_FLAG_HANDLE(X) 		OBJ_SET_FLAG(X, OBJ_FLAG_HANDLE)
#define OBJ_UNSET_FLAG_HANDLE(X)	OBJ_UNSET_FLAG(X, OBJ_FLAG_HANDLE)

/* OBJ_PTR_TO_HANDLE toma un puntero a Obj,
 * y lo castea a un Obj que es un handle que
 * representa dicho puntero (setea OBJ_FLAG_HANDLE).
 *
 * OBJ_HANDLE_TO_PTR hace el trabajo inverso,
 * y apaga todas las flags.
 */
#define OBJ_PTR_TO_HANDLE(P)	(((Obj)(P)) | OBJ_FLAG_HANDLE)
#define OBJ_HANDLE_TO_PTR(H)	((Obj *)((H) & ~OBJ_ALL_FLAGS))

#define BLOCK_CAPACITY	10

/*
 * La memoria se organiza en bloques.
 * Cada bloque es un buffer de objetos.
 */
typedef struct _block {
	Obj buffer[BLOCK_CAPACITY];
	int block_size;
} Block;

/*
 * Un memory manager consta de muchos bloques.
 * La secuencia de bloques se guarda como un arreglo
 * de punteros a bloque (redimensionando al doble
 * del tamanyo en cuanto necesita crecer).
 *
 * Tambien se guarda un puntero al inicio de la pila
 * en el momento en el que comenzo la ejecucion del
 * programa. Esto es para poder visitar todos los
 * objetos alcanzables durante la recoleccion.
 *
 * Se guarda un valor umbral (expresado en bloques)
 * para determinar cuando debe hacerse garbage collection.
 * Cada vez que se hace gc, este valor se actualiza 
 * al doble de la cantidad de bloques requerida 
 * para almacenar las estructuras alcanzables.
 */
typedef struct _mm {
	Block **blocks;
	uint64 capacity;
	uint64 nblocks;
	uint64 *stack_bottom;
	uint64 gc_threshold;
} MM;

/* Funciones publicas */

void mm_init(MM *mm, uint64 *stack_bottom);
Obj mm_alloc(MM *mm, int obj_size);
void mm_set(Obj handle, int i, Obj value);
void mm_gc(MM *mm);
void mm_free_blocks(MM *mm);

/* Devuelve el valor que tenia el stack pointer
 * cuando comenzo a ejecutarse la funcion actual */
#define mm_stack_pointer_orig() __builtin_frame_address(0)

/* Devuelve el valor que tiene el stack pointer
 * actualmente */
uint64 *mm_stack_pointer_current(void);

/*
 * En el proceso de garbage collection se revisa todo el
 * contenido de la pila, que puede contener posiciones
 * no inicializadas.
 *
 * Con la opcion MM_VALGRIND_COMPATIBILITY se habilita
 * la funcionalidad para que el garbage collector
 * simule que todas esas posiciones estan inicializadas.
 * Esto evita que valgrind de falsos positivos
 * (para que no reporte accesos a posiciones sin
 * inicializar).
 */
#define MM_VALGRIND_COMPATIBILITY	1

#endif
