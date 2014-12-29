#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <assert.h>

typedef unsigned long long int uint64;
typedef uint64 Obj;

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
	int block_size;
} Block;

typedef struct _mm {
	Block **blocks;
	uint64 capacity;
	uint64 nblocks;
} MM;

void mm_init_block(Block *b) {
	int i;
	for (i = 0; i < BLOCK_CAPACITY; i++) {
		b->buffer[i] = OBJ_EMPTY;
	}
	b->block_size = 0;
}

void mm_init(MM *mm) {
	mm->blocks = malloc(sizeof(Block *));
	mm->blocks[0] = malloc(sizeof(Block));
	mm_init_block(mm->blocks[0]);
	mm->capacity = 1;
	mm->nblocks = 1;
}

void mm_init_object(Obj *obj, int obj_size) {
	int i;
	OBJ_UNSET_FLAG_CONTINUE(obj[0]);
	for (i = 1; i < obj_size; i++){
		OBJ_SET_FLAG_CONTINUE(obj[i]);
	}
}

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

Obj mm_alloc(MM *mm, int obj_size) {
	assert(1 <= obj_size && obj_size <= BLOCK_CAPACITY);
	Block *last_block = mm->blocks[mm->nblocks - 1];
	if (last_block->block_size + obj_size > BLOCK_CAPACITY) {
		last_block = malloc(sizeof(Block));
		mm_init_block(last_block);
		mm_grow_blocks_if_full(mm);
		mm->nblocks += 1;
		mm->blocks[mm->nblocks - 1] = last_block;
	}
	Obj *obj = &last_block->buffer[last_block->block_size];
	mm_init_object(obj, obj_size);
	last_block->block_size += obj_size;
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
	uint64 i;
	for (i = 0; i < mm->nblocks; i++) {
		free(mm->blocks[i]);
	}
	free(mm->blocks);
}

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

/* mm with its blocks sorted */
int mm_is_potential_handle(MM *mm, Obj obj) {
	Block **blocks = mm->blocks;

	if (!(obj & OBJ_FLAG_HANDLE)) { /* not a handle */
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
		&& obj < BLOCK_START(index_left) + BLOCK_SIZE_IN_BYTES(index_left);
}
#undef BLOCK_START
#undef BLOCK_SIZE_IN_BYTES

void mm_gc(MM *mm) {
	mm_sort_blocks(mm->blocks, 0, mm->nblocks);
	/* TODO */
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

void test_mm_is_potential_handle() {
	srandom(time(NULL));

	MM mm;
	int i, j, k;
	int object_count = 0;
	mm_init(&mm);
	free(mm.blocks);

	mm.nblocks = 5;
	mm.blocks = malloc(sizeof(Block *) * mm.nblocks);
	for (i = 0; i < mm.nblocks; i++) {
		mm.blocks[i] = malloc(sizeof(Block));
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
		assert(mm_is_potential_handle(&mm, positives[j]));
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
			assert(!mm_is_potential_handle(&mm, negative));
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
			assert(!mm_is_potential_handle(&mm, negative));
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
			assert(!mm_is_potential_handle(&mm, negative));
		}
	}

#undef NTESTS
	mm_end(&mm);
}

int main() {
	int i;
	MM mm;
	mm_init(&mm);
	Obj handle_obj1 = mm_alloc(&mm, 2);
	Obj handle_obj2 = mm_alloc(&mm, 9);
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

	mm_end(&mm);

	printf("-----\n");
	/*test_mm_sort_blocks();*/
	test_mm_is_potential_handle();

	return 0;
}

