#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include <stddef.h>

typedef struct MemoryBlock {
    size_t size;
    unsigned char is_free;
    struct MemoryBlock* next;
} MemoryBlock;

typedef enum {
    FIRST_FIT,
    BEST_FIT
} AllocationStrategy;

void init_allocator(size_t size, AllocationStrategy strategy);
void cleanup_allocator(void);
void* mem_alloc(size_t size);
void* mem_calloc(size_t num, size_t size);
void* mem_realloc(void* ptr, size_t new_size);
void mem_free(void* ptr);
void print_memory_map(void);
size_t get_free_memory(void);
size_t get_used_memory(void);
float get_fragmentation(void);

#endif // ALLOCATOR_H
