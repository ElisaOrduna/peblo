#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

typedef unsigned long long int Obj;

/*
 * The last 3 bits of an object represent:
 *
 *   0x1: whether a data array continues at this point
 *        (this flag is unset in all the places where
 *        a data array starts)
 *   0x2: whether it has been reached by the garbage
 *        collection process
 *   0x4: whether it is a handle representing a pointer
 *        to an Obj, or an immediate value
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

/* This macro takes a pointer to an Obj,
 * and casts it to an Obj representing that
 * pointer (with the OBJ_FLAG_HANDLE set to 1).
 */
#define OBJ_PTR_TO_HANDLE(P)	(((Obj)(P)) | OBJ_FLAG_HANDLE)
#define OBJ_HANDLE_TO_PTR(H)	((Obj *)((H) & ~OBJ_ALL_FLAGS))

#define OBJ_EMPTY	0

#define BLOCK_CAPACITY	10

typedef struct _block {
	Obj buffer[BLOCK_CAPACITY];
	int size;
	struct _block *prev_block, *next_block;
} Block;

typedef struct _mm {
	Block *first_block, *last_block;
} MM;

void mm_init_block(Block *b) {
	int i;
	for (i = 0; i < BLOCK_CAPACITY; i++) {
		b->buffer[i] = OBJ_EMPTY;
	}
	b->size = 0;
	b->prev_block = NULL;
	b->next_block = NULL;
}

void mm_init(MM *mm) {
	mm->first_block = malloc(sizeof(Block));
	mm->last_block = mm->first_block;
	mm_init_block(mm->first_block);
}

void mm_init_object(Obj *obj, int size) {
	int i;
	OBJ_UNSET_FLAG_CONTINUE(obj[0]);
	for (i = 1; i < size; i++){
		OBJ_SET_FLAG_CONTINUE(obj[i]);
	}
}

Obj mm_alloc(MM *mm, int size) {
	assert(1 <= size && size <= BLOCK_CAPACITY);
	if (mm->last_block->size + size > BLOCK_CAPACITY) {
		Block *b = malloc(sizeof(Block));
		mm_init_block(b);
		b->prev_block = mm->last_block;
		mm->last_block->next_block = b;
		mm->last_block = b;
	}
	Obj *obj = &mm->last_block->buffer[mm->last_block->size];
	mm_init_object(obj, size);
	mm->last_block->size += size;
	return OBJ_PTR_TO_HANDLE(obj);
}

/*
 * Sets the i-th slot of the data array pointed
 * by <handle> to <value>, preserving the
 * OBJ_FLAG_CONTINUE flag.
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

void mm_end(MM *mm) {
	Block *current_block = mm->first_block;
	while (current_block != NULL) {
		Block *b = current_block->next_block;
		free(current_block);
		current_block = b;
	}
}

void mm_gc(MM *mm) {
	/* TODO */
}

int main() {
	MM mm;
	mm_init(&mm);
	Obj handle_obj1 = mm_alloc(&mm, 2);
	Obj handle_obj2 = mm_alloc(&mm, 9);

	mm_set(handle_obj1, 0, handle_obj2);
	mm_set(handle_obj1, 1, handle_obj2);
	mm_set(handle_obj2, 0, handle_obj1);

	printf("%llu\n", handle_obj1);
	printf("%llu\n", handle_obj2);
	printf("%llu\n", OBJ_HANDLE_TO_PTR(handle_obj1)[0]);
	printf("%llu\n", OBJ_HANDLE_TO_PTR(handle_obj1)[1]);
	printf("%llu\n", OBJ_HANDLE_TO_PTR(handle_obj2)[0]);

	mm_end(&mm);
	return 0;
}

