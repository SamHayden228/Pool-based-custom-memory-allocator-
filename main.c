
#include <assert.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#define POOL_SIZE 4096

static unsigned char pool_buffer_15[POOL_SIZE];
static unsigned char pool_buffer_180[POOL_SIZE];
typedef struct Pool_Free_Node Pool_Free_Node;
typedef struct Pool Pool;
struct Pool {
	unsigned char* buf;
	size_t buf_len;
	size_t chunk_size;
	Pool_Free_Node* head;
};

static Pool pool_15;
static Pool pool_180;

struct Pool_Free_Node {
	Pool_Free_Node* next;
};

void pool_free_all(Pool* p); // This procedure will be covered later in this article

void pool_init(Pool* p, void* backing_buffer, size_t backing_buffer_length,
	size_t chunk_size, size_t chunk_alignment) {

	// Allign chunk size with required alignment 
	if ((chunk_size % chunk_alignment != 0)) {
		chunk_size += chunk_alignment - (chunk_size % chunk_alignment);
	}

	// Assert that the parameters passed are valid
	assert(chunk_size >= sizeof(Pool_Free_Node) &&
		"Chunk size is too small");
	assert(backing_buffer_length >= chunk_size &&
		"Backing buffer length is smaller than the chunk size");

	// Store the adjusted parameters
	p->buf = (unsigned char*)backing_buffer;
	p->buf_len = backing_buffer_length;
	p->chunk_size = chunk_size;
	p->head = NULL; // Free List Head

	// Set up the free list for free chunks
	pool_free_all(p);
}



void* pool_alloc(size_t size) {
	// Get latest free node
	Pool* p;
	if (size <= 15) {
		p = &pool_15;
	}
	else if (size <= 180) {
		p = &pool_180;
	}
	else {
		return NULL;
	}
	Pool_Free_Node* node = p->head;

	if (node == NULL) {
		assert(0 && "Pool allocator has no free memory");
		return NULL;
	}

	// Pop free node
	p->head = p->head->next;

	// Zero memory by default
	return (void*)node;
}

void pool_free(void* ptr) {
	if (!ptr) return;

	
	Pool* p = NULL;
	uintptr_t addr = (uintptr_t)ptr;

	if (addr >= (uintptr_t)pool_15.buf && addr < (uintptr_t)pool_15.buf + POOL_SIZE) {
		p = &pool_15;
	}
	else if (addr >= (uintptr_t)pool_180.buf && addr < (uintptr_t)pool_180.buf + POOL_SIZE) {
		p = &pool_180;
	}

	// Turn new chunk into new haed
	Pool_Free_Node* node = (Pool_Free_Node*)ptr;
	node->next = p->head;
	p->head = node;
	
}

void pool_free_all(Pool* p) {
	size_t chunk_count = p->buf_len / p->chunk_size;
	size_t i;


	// Set all chunks to be free
	for (i = 0; i < chunk_count; i++) {
		void* ptr = &p->buf[i * p->chunk_size];
		Pool_Free_Node* node = (Pool_Free_Node*)ptr;
		// Push free node onto thte free list
		node->next = p->head;
		p->head = node;
	}
}

int main() {
	pool_init(&pool_15, pool_buffer_15, POOL_SIZE, 15,8);
	pool_init(&pool_180, pool_buffer_180, POOL_SIZE, 180,8);
	void* a = pool_alloc(4);
	printf("Allocated at: %p\n", a);
	pool_free(a);
	printf("Allocated at: %p\n", a);

	return 0;

}	
