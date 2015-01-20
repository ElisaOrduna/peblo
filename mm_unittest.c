#include "mm.h"

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <assert.h>

/* Funciones internas que no exporta mm.h */
int mm_is_handle(MM *mm, Obj obj);
void mm_sort_blocks(Block **blocks, int begin, int end);
void mm_init_block(Block *b);

void test_mm_sort_blocks(void) {
	printf("-- test mm_sort_blocks\n");
	int n = 1000;
	int i;
	Block **bs;
	bs = malloc(sizeof(Block *) * n);
	for (i = 0; i < n; i++) {
		bs[i] = (void *)random();
	}

	mm_sort_blocks(bs, 0, n);

	for (i = 0; i < n - 1; i++) {
		assert((uint64)(&bs[i]->buffer[0]) <= (uint64)(&bs[i + 1]->buffer[0]));
	}
	free(bs);
}

void test_mm_is_handle(void) {
	printf("-- test mm_is_handle\n");
	srandom(time(NULL));

	MM mm;
	int i, j, k;
	int object_count = 0;
	mm_init(&mm, mm_stack_pointer_orig());
	mm_free_blocks(&mm);

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

	for (j = 0; j < object_count; j++) {
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
			assert(!mm_is_handle(&mm, negative));
		}
	}

#undef NTESTS
	free(positives);
	mm_free_blocks(&mm);
}

#define MK_IMMEDIATE(X)		((X) << 3)
#define IMM_VALUE(X)		((X) >> 3)
void test_gc_build_structure(MM *mm, Obj handle_start) {
	Obj handle_prev = handle_start;
	Obj hdl;
	uint64 suma;
	int i;
	for (i = 1; i <= 1000; i++) {
		Obj handle_current = mm_alloc(mm, 2);
		mm_set(handle_current, 0, MK_IMMEDIATE(i));
		mm_set(handle_prev, 1, handle_current);
		handle_prev = handle_current;
	}
	mm_set(handle_prev, 1, MK_IMMEDIATE(9999));

	{
		suma = 0;
		hdl = handle_start;
		while (hdl & OBJ_FLAG_HANDLE) {
			suma += OBJ_HANDLE_TO_PTR(hdl)[0];
			hdl = OBJ_HANDLE_TO_PTR(hdl)[1];
		}
		assert(IMM_VALUE(suma) == 500544);
	}

	hdl = handle_start;
	for (i = 1; i <= 10; i++) {
		hdl = OBJ_HANDLE_TO_PTR(hdl)[1];
	}
	mm_set(handle_prev, 1, OBJ_HANDLE_TO_PTR(hdl)[1]);
	mm_set(hdl, 1, MK_IMMEDIATE(18));
}

void test_gc(void) {
	printf("-- test mm_gc\n");

	MM _mm;
	MM *mm = &_mm;
	mm_init(mm, mm_stack_pointer_orig());

	Obj handle_start = mm_alloc(mm, 2);
	mm_set(handle_start, 0, MK_IMMEDIATE(44));

	test_gc_build_structure(mm, handle_start);

	mm_gc(mm);

	{
		uint64 suma = 0;
		Obj hdl = handle_start;
		while (hdl & OBJ_FLAG_HANDLE) {
			suma += OBJ_HANDLE_TO_PTR(hdl)[0];
			hdl = OBJ_HANDLE_TO_PTR(hdl)[1];
		}
		assert(IMM_VALUE(suma) == 99);
	}

	test_gc_build_structure(mm, handle_start);
	mm_gc(mm);

	{
		uint64 suma = 0;
		Obj hdl = handle_start;
		while (hdl & OBJ_FLAG_HANDLE) {
			suma += OBJ_HANDLE_TO_PTR(hdl)[0];
			hdl = OBJ_HANDLE_TO_PTR(hdl)[1];
		}
		assert(IMM_VALUE(suma) == 99);
	}

	mm_free_blocks(mm);
}
#undef MK_IMMEDIATE
#undef IMM_VALUE

int main() {
	test_mm_sort_blocks();
	test_mm_is_handle();
	test_gc();

	return 0;
}

