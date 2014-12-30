#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <assert.h>

typedef unsigned long long int uint64;
typedef uint64 Obj;

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

#define OBJ_EMPTY	0

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
 */
typedef struct _mm {
	Block **blocks;
	uint64 capacity;
	uint64 nblocks;
	uint64 *stack_bottom;
} MM;

void mm_init_block(Block *b) {
	int i;
	for (i = 0; i < BLOCK_CAPACITY; i++) {
		b->buffer[i] = OBJ_EMPTY;
	}
	b->block_size = 0;
}

void mm_init(MM *mm, uint64 *stack_bottom) {
	mm->blocks = malloc(sizeof(Block *));
	mm->blocks[0] = malloc(sizeof(Block));
	mm_init_block(mm->blocks[0]);
	mm->capacity = 1;
	mm->nblocks = 1;
	mm->stack_bottom = stack_bottom;
}

/*
 * Inicializa una estructura.
 * Una estructura es un arreglo de objetos,
 * el primero de los cuales tiene el flag
 * OBJ_FLAG_CONTINUE en 0, y los siguientes en 1.
 */
void mm_init_structure(Obj *obj, int obj_size) {
	int i;
	OBJ_UNSET_FLAG_CONTINUE(obj[0]);
	for (i = 1; i < obj_size; i++){
		OBJ_SET_FLAG_CONTINUE(obj[i]);
	}
}

/*
 * Se fija si el arreglo de bloques esta lleno
 * y en tal caso lo redimensiona al doble del tamanyo.
 */
void mm_grow_blocks_if_full(MM *mm) {
	if (mm->nblocks < mm->capacity) {
		return;
	}
	Block **new_blocks = malloc(sizeof(Block *) * 2 * mm->capacity);
	memcpy(new_blocks, mm->blocks, sizeof(Block *) * mm->capacity);
	mm->capacity *= 2;
	free(mm->blocks);
	mm->blocks = new_blocks;
}

/*
 * Reserva una estructura del tamanyo indicado.
 * Si no hay mas espacio para almacenarla dentro de
 * los bloques del memory manager, se agrega un bloque
 * mas.
 *
 * Esta funcion no invoca al garbage collector.
 */
Obj mm_alloc(MM *mm, int obj_size) {
	assert(1 <= obj_size && obj_size <= BLOCK_CAPACITY);
	Block *last_block = mm->blocks[mm->nblocks - 1];
	if (last_block->block_size + obj_size >= BLOCK_CAPACITY) {
		/* Nota:
		 *   Se compara mediante ">=" con BLOCK_CAPACITY para
		 *   asegurarse de que el ultimo objeto del bloque
		 *   quede siempre libre, para poder determinar
		 *   el tamanyo de una estructura mirando unicamente
		 *   el OBJ_SET_FLAG_CONTINUE (sin tener en cuenta
		 *   si ya "me pase" del tamanyo del bloque).
		 */
		last_block = malloc(sizeof(Block));
		mm_init_block(last_block);
		mm_grow_blocks_if_full(mm);
		mm->nblocks += 1;
		mm->blocks[mm->nblocks - 1] = last_block;
	}
	Obj *obj = &last_block->buffer[last_block->block_size];
	mm_init_structure(obj, obj_size);
	last_block->block_size += obj_size;
	return OBJ_PTR_TO_HANDLE(obj);
}

/*
 * Setea el i-esimo slot de la estructura apuntada
 * por el handle, guardando en su lugar el valor "value",
 * pero respetando el flag OBJ_FLAG_CONTINUE
 * (que debe estar en 0 para el primer objeto de la
 * estructura, y en 1 para los objetos siguientes).
 */
void mm_set(Obj handle, int i, Obj value) {
	Obj *ptr = OBJ_HANDLE_TO_PTR(handle);
	if (i == 0) {
		OBJ_UNSET_FLAG_CONTINUE(value);
	} else {
		OBJ_SET_FLAG_CONTINUE(value);
	}
	ptr[i] = value;
}

void mm_free_blocks(MM *mm) {
	uint64 i;
	for (i = 0; i < mm->nblocks; i++) {
		free(mm->blocks[i]);
	}
	free(mm->blocks);
}

/*
 * Ordena el arreglo de bloques dado, de acuerdo con
 * la posicion de memoria en la que empieza el bloque
 * (BLOCK_START), de menor a mayor, usando quicksort.
 *
 * Se ordena este arreglo al comienzo de la etapa
 * de garbage collection, para poder determinar
 * rapidamente si un valor es un handle que apunta
 * al interior de alguno de los bloques.
 */
#define BLOCK_START(I)		((uint64)(&blocks[(I)]->buffer[0]))
#define BLOCK_SIZE_IN_BYTES(I)	(blocks[(I)]->block_size * sizeof(Obj))
void mm_sort_blocks(Block **blocks, int begin, int end) {
	if (begin + 1 >= end) {
		return;
	}

	uint64 pivot_index = random() % (end - begin) + begin;
	uint64 pivot_value = BLOCK_START(pivot_index);

	uint64 index_left = begin;
	uint64 index_right = end;

	while (index_left < index_right) {
		if (BLOCK_START(index_left) <= pivot_value) {
			index_left++;
		} else {
			Block *tmp = blocks[index_left];
			blocks[index_left] = blocks[index_right - 1];
			blocks[index_right - 1] = tmp;
			index_right--;
		}
	}

	mm_sort_blocks(blocks, begin, index_left);
	mm_sort_blocks(blocks, index_left, end);
}

/*
 * Determina si el objeto dado representa un handle
 * que apunta al interior de alguno de los bloques
 * administrados por el memory manager:
 *
 * - El OBJ_FLAG_HANDLE debe estar en 1.
 * - El puntero debe estar dentro de alguno de los
 *   bloques (busqueda binaria).
 * - El objeto apuntado debe ser el primero de la
 *   estructura. (En el esquema de manejo de memoria
 *   elegido no se permiten punteros "internos" a
 *   la mitad de una estructura).
 *
 * Pre: el arreglo de bloques de mm debe estar ordenado
 *      por BLOCK_START de menor a mayor.
 */
int mm_is_handle(MM *mm, Obj obj) {
	Block **blocks = mm->blocks;

	if (!(obj & OBJ_FLAG_HANDLE)) {
		/* not a handle */
		return 0;
	}
	
	if (BLOCK_START(0) > obj) {
		return 0;
	}

	uint64 index_left = 0;
	uint64 index_right = mm->nblocks;
	while (index_right - index_left > 1) {
		uint64 mid = index_left + (index_right - index_left) / 2;
		if (BLOCK_START(mid) <= obj) {
			index_left = mid;
		} else {
			index_right = mid;
		}
	}

	return BLOCK_START(index_left) <= obj
		&& obj < BLOCK_START(index_left) + BLOCK_SIZE_IN_BYTES(index_left)
		&& !(*OBJ_HANDLE_TO_PTR(obj) & OBJ_FLAG_CONTINUE);
}
#undef BLOCK_START
#undef BLOCK_SIZE_IN_BYTES

/*
 * Devuelve el tamanyo de la estructura apuntada
 * por el handle.
 *
 * Recordar que una estructura es un arreglo de
 * objetos, el primero de los cuales tiene el
 * OBJ_FLAG_CONTINUE en 0, y los siguientes en 1.
 */
int mm_structure_size(Obj handle) {
	Obj *p = OBJ_HANDLE_TO_PTR(handle);
	int size = 1;
	p++;
	while (*p & OBJ_FLAG_CONTINUE) {
		p++;
		size++;
	}
	return size;
}

/*
 * "Visita" el handle dado.
 * Primero determina si efectivamente apunta a una
 * estructura en el MM viejo.
 * Si la estructura no fue alcanzada anteriormente:
 * - Crea una copia de dicha estructura en el MM nuevo.
 * - Pisa la primera parte de la estructura vieja
 *   con el puntero al objeto nuevo, para poder
 *   actualizar las referencias en la etapa posterior
 *   de gc.
 * - Marca la estructura vieja como ya alcanzada.
 */
void mm_gc_visit(MM *old_mm, MM *new_mm, Obj *ptr) {
	if (!mm_is_handle(old_mm, *ptr)) {
		/* No es un handle */
		return;
	}

	Obj handle_src = *ptr;
	Obj *src = OBJ_HANDLE_TO_PTR(handle_src);
	if (!(*src & OBJ_FLAG_REACH)) {
		/* No fue alcanzado anteriormente */

		/* Creo una copia de la estructura */
		int size = mm_structure_size(handle_src);
		Obj handle_dst = mm_alloc(new_mm, size);
		Obj *dst = OBJ_HANDLE_TO_PTR(handle_dst);
		memcpy(dst, src, size * sizeof(Obj));

		/* Piso el primer objeto de la estructura vieja
		 * con el "forward pointer" */
		*src = handle_dst | OBJ_FLAG_REACH;
	}

	/* Actualizo la referencia a la estructura */
	*ptr = *src;
	OBJ_UNSET_FLAG_REACH(*ptr);
	if (handle_src & OBJ_FLAG_CONTINUE) {
		OBJ_SET_FLAG_CONTINUE(*ptr);
	}
}

/*
 * Hace una ronda de garbage collection usando
 * el metodo stop & copy:
 *
 * - Hace BFS partiendo de las raices (pila y registros
 *   del procesador) haciendo una copia de todas las
 *   estructuras alcanzables.
 *
 * - "Emparcha" todas las referencias a las estructuras
 *   viejas, para que apunten a sus copias nuevas.
 *
 * - Libera todos los bloques viejos.
 */
void mm_gc(MM *mm) {

	/* Ordena los bloques por la posicion donde comienzan */
	mm_sort_blocks(mm->blocks, 0, mm->nblocks);

	/* Guarda registros en la pila */
#include "save_registers.inc"

	/* Visita todas las estructuras alcanzables */
	uint64 dummy = 0;
	uint64 *stack_top = &dummy;

	MM _new_mm;
	MM *new_mm = &_new_mm;
	mm_init(new_mm, mm->stack_bottom);

	uint64 *p;
	for (p = stack_top; p < mm->stack_bottom; p++) {
		mm_gc_visit(mm, new_mm, p);
	}

	uint64 b;
	int i;
	for (b = 0; b < new_mm->nblocks; b++) {
		for (i = 0; i < new_mm->blocks[b]->block_size; i++) {
			mm_gc_visit(mm, new_mm, &new_mm->blocks[b]->buffer[i]);
		}
	}

	/* Restaura registros de la pila */
#include "restore_registers.inc"

	/* Libera la memoria del MM viejo */
	mm_free_blocks(mm);

	/* Ahora new_mm pasa a ser el MM actual */
	memcpy(mm, new_mm, sizeof(MM));
}

void test_mm_sort_blocks() 
{
	int n = 1000;
	int i;
	Block **bs;
	bs = malloc(sizeof(Block *) * n);
	for (i = 0; i < n; i++) {
		bs[i] = (void *)random();
	}

	printf("bloques:\n");
	for (i = 0; i < n; i++) {
		printf("\t%llu\n", (uint64)(&bs[i]->buffer[0]));
	}
	printf("\n");

	mm_sort_blocks(bs, 0, n);

	for (i = 0; i < n - 1; i++) {
		assert((uint64)(&bs[i]->buffer[0]) <= (uint64)(&bs[i + 1]->buffer[0]));
	}

	printf("bloques:\n");
	for (i = 0; i < n; i++) {
		printf("\t%llu\n", (uint64)(&bs[i]->buffer[0]));
	}
	printf("\n");
}

void test_mm_is_handle() {
	srandom(time(NULL));

	uint64 dummy = 0;
	MM mm;
	int i, j, k;
	int object_count = 0;
	mm_init(&mm, &dummy);
	free(mm.blocks);

	mm.nblocks = 5;
	mm.blocks = malloc(sizeof(Block *) * mm.nblocks);
	for (i = 0; i < mm.nblocks; i++) {
		mm.blocks[i] = malloc(sizeof(Block));
		mm_init_block(mm.blocks[i]);
		mm.blocks[i]->block_size = (i + 1 < BLOCK_CAPACITY) ? i + 1 : BLOCK_CAPACITY;
		object_count += mm.blocks[i]->block_size;
	}

	Obj *positives = malloc(sizeof(Obj) * object_count);
	j = 0;

	for (i = 0; i < mm.nblocks; i++) {
		for (k = 0; k < mm.blocks[i]->block_size; k++) {
			positives[j] = OBJ_PTR_TO_HANDLE(&mm.blocks[i]->buffer[k]);
			j++;
		}
	}
	
	mm_sort_blocks(mm.blocks, 0, mm.nblocks);

	printf("blocks: ");
	for (i = 0; i < mm.nblocks; i++) {
		printf("[%llu , +(%llu) , %llu) ",
			(uint64)(&mm.blocks[i]->buffer[0]),
 			(uint64)mm.blocks[i]->block_size * sizeof(Obj),
			(uint64)(&mm.blocks[i]->buffer[0]) + mm.blocks[i]->block_size * sizeof(Obj));
	}
	printf("\n");

	for (j = 0; j < object_count; j++) {
		printf("checking handle [+]: %llu\n", positives[j]);
		assert(mm_is_handle(&mm, positives[j]));
	}

	for (i = 0; i < mm.nblocks; i++) {
		Obj negative = OBJ_PTR_TO_HANDLE(&mm.blocks[i]->buffer[0] - sizeof(Obj));
		int is_negative = 1;
		for (j = 0; j < object_count; j++) {
			if (positives[j] == negative) {
				is_negative = 0;
			}
		}
		if (is_negative) {
			printf("checking handle [-]: %llu\n", negative);
			assert(!mm_is_handle(&mm, negative));
		}
	}

	for (i = 0; i < mm.nblocks; i++) {
		Obj negative = OBJ_PTR_TO_HANDLE(&mm.blocks[i]->buffer[mm.blocks[i]->block_size - 1] + sizeof(Obj));
		int is_negative = 1;
		for (j = 0; j < object_count; j++) {
			if (positives[j] == negative) {
				is_negative = 0;
			}
		}
		if (is_negative) {
			printf("checking handle [-]: %llu\n", negative);
			assert(!mm_is_handle(&mm, negative));
		}
	}

	for (i = 0; i < 1000; i++) {
		Obj negative = OBJ_PTR_TO_HANDLE(random());
		int is_negative = 1;
		for (j = 0; j < object_count; j++) {
			if (positives[j] == negative) {
				is_negative = 0;
			}
		}
		if (is_negative) {
			printf("checking handle [-]: %llu\n", negative);
			assert(!mm_is_handle(&mm, negative));
		}
	}

#undef NTESTS
	mm_free_blocks(&mm);
}

int main() {
	uint64 dummy = 0;
	MM mm;
	mm_init(&mm, &dummy);

	Obj handle_obj1 = mm_alloc(&mm, 2);
	Obj handle_obj2 = mm_alloc(&mm, 9);
	int i;
	for (i = 1; i < 1000; i++) {
		handle_obj2 = mm_alloc(&mm, 9);
	}

	mm_set(handle_obj1, 0, handle_obj2);
	mm_set(handle_obj1, 1, handle_obj2);
	mm_set(handle_obj2, 0, handle_obj1);

	printf("%llu\n", handle_obj1);
	printf("%llu\n", handle_obj2);
	printf("%llu\n", OBJ_HANDLE_TO_PTR(handle_obj1)[0]);
	printf("%llu\n", OBJ_HANDLE_TO_PTR(handle_obj1)[1]);
	printf("%llu\n", OBJ_HANDLE_TO_PTR(handle_obj2)[0]);

	mm_gc(&mm);

	mm_free_blocks(&mm);

	/*printf("-----\n");*/
	/*test_mm_sort_blocks();*/
	/*test_mm_is_handle();*/


	return 0;
}

