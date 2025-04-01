#include "allocator.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned char* memory_pool = NULL;
static size_t memory_pool_size = 0;
static AllocationStrategy current_strategy = FIRST_FIT;
static MemoryBlock* first_block = NULL;

static void* get_data_pointer(MemoryBlock* block) {
    return (void*)((unsigned char*)block + sizeof(MemoryBlock));
}

static MemoryBlock* get_block_header(void* data) {
    return (MemoryBlock*)((unsigned char*)data - sizeof(MemoryBlock));
}

void init_allocator(size_t size, AllocationStrategy strategy) {
    if (memory_pool != NULL) {
        cleanup_allocator();
    }
    
    memory_pool = (unsigned char*)malloc(size);
    memory_pool_size = size;
    current_strategy = strategy;
    
    first_block = (MemoryBlock*)memory_pool;
    first_block->size = size - sizeof(MemoryBlock);
    first_block->is_free = 1;
    first_block->next = NULL;
}

void cleanup_allocator(void) {
    if (memory_pool != NULL) {
        free(memory_pool);
        memory_pool = NULL;
        memory_pool_size = 0;
        first_block = NULL;
    }
}

static MemoryBlock* find_first_fit(size_t size) {
    MemoryBlock* current = first_block;
    
    while (current != NULL) {
        if (current->is_free && current->size >= size) {
            return current;
        }
        current = current->next;
    }
    
    return NULL;
}

static MemoryBlock* find_best_fit(size_t size) {
    MemoryBlock* current = first_block;
    MemoryBlock* best_block = NULL;
    size_t smallest_size = (size_t)-1; // Start with maximum size_t value
    
    while (current != NULL) {
        if (current->is_free && current->size >= size) {
            if (current->size < smallest_size) {
                smallest_size = current->size;
                best_block = current;
            }
        }
        current = current->next;
    }
    
    return best_block;
}

static void split_block(MemoryBlock* block, size_t size) {
    // Only split if block is large enough to be worth it (block + header + minimum usable size)
    if (block->size > size + sizeof(MemoryBlock) + 8) {
        MemoryBlock* new_block = (MemoryBlock*)((unsigned char*)block + sizeof(MemoryBlock) + size);
        
        new_block->size = block->size - size - sizeof(MemoryBlock);
        new_block->is_free = 1;
        new_block->next = block->next;
        
        block->size = size;
        block->next = new_block;
    }
}

static void coalesce_blocks(void) {
    MemoryBlock* current = first_block;
    
    while (current != NULL && current->next != NULL) {
        if (current->is_free && current->next->is_free) {
            current->size += sizeof(MemoryBlock) + current->next->size;
            current->next = current->next->next;
            // Don't move to next block - might be more to coalesce
        } else {
            current = current->next;
        }
    }
}

void* mem_alloc(size_t size) {
    if (size == 0 || memory_pool == NULL) {
        return NULL;
    }
    
    MemoryBlock* block = NULL;
    
    if (current_strategy == FIRST_FIT) {
        block = find_first_fit(size);
    } else {
        block = find_best_fit(size);
    }
    
    if (block == NULL) {
        return NULL;
    }
    
    split_block(block, size);
    block->is_free = 0;
    
    return get_data_pointer(block);
}

void* mem_calloc(size_t num, size_t size) {
    size_t total_size = num * size;
    void* ptr = mem_alloc(total_size);
    
    if (ptr != NULL) {
        memset(ptr, 0, total_size);
    }
    
    return ptr;
}

void* mem_realloc(void* ptr, size_t new_size) {
    if (ptr == NULL) {
        return mem_alloc(new_size);
    }
    
    if (new_size == 0) {
        mem_free(ptr);
        return NULL;
    }
    
    MemoryBlock* block = get_block_header(ptr);
    
    if (block->size >= new_size) {
        split_block(block, new_size);
        return ptr;
    }
    
    // Check if we can expand into the next block if it's free
    if (block->next != NULL && block->next->is_free && 
        (block->size + sizeof(MemoryBlock) + block->next->size) >= new_size) {
        block->size += sizeof(MemoryBlock) + block->next->size;
        block->next = block->next->next;
        
        split_block(block, new_size);
        return ptr;
    }
    
    // Need to allocate a new block and copy data
    void* new_ptr = mem_alloc(new_size);
    if (new_ptr == NULL) {
        return NULL;
    }
    
    memcpy(new_ptr, ptr, block->size);
    mem_free(ptr);
    
    return new_ptr;
}

void mem_free(void* ptr) {
    if (ptr == NULL || memory_pool == NULL) {
        return;
    }
    
    MemoryBlock* block = get_block_header(ptr);
    block->is_free = 1;
    
    coalesce_blocks();
}

void print_memory_map(void) {
    if (memory_pool == NULL) {
        printf("Memory allocator not initialized\n");
        return;
    }
    
    MemoryBlock* current = first_block;
    int block_num = 0;
    
    printf("\nMEMORY MAP:\n");
    printf("-----------------------------------------------------------------------------\n");
    printf("| %-5s | %-10s | %-10s | %-15s | %-15s |\n", 
           "Block", "Status", "Size", "Address", "Next Block");
    printf("-----------------------------------------------------------------------------\n");
    
    while (current != NULL) {
        printf("| %-5d | %-10s | %-10zu | 0x%-13lx | 0x%-13lx |\n", 
               block_num++, 
               current->is_free ? "FREE" : "ALLOCATED", 
               current->size, 
               (unsigned long)current, 
               (unsigned long)current->next);
        
        current = current->next;
    }
    
    printf("-----------------------------------------------------------------------------\n");
    printf("Pool size: %zu bytes, Free: %zu bytes (%.2f%%), Used: %zu bytes (%.2f%%)\n",
           memory_pool_size,
           get_free_memory(),
           (float)get_free_memory() / memory_pool_size * 100.0,
           get_used_memory(),
           (float)get_used_memory() / memory_pool_size * 100.0);
    printf("Fragmentation: %.2f%%\n\n", get_fragmentation() * 100.0);
}

size_t get_free_memory(void) {
    size_t free_memory = 0;
    MemoryBlock* current = first_block;
    
    while (current != NULL) {
        if (current->is_free) {
            free_memory += current->size;
        }
        current = current->next;
    }
    
    return free_memory;
}

size_t get_used_memory(void) {
    size_t used_memory = 0;
    MemoryBlock* current = first_block;
    
    while (current != NULL) {
        if (!current->is_free) {
            used_memory += current->size;
        }
        current = current->next;
    }
    
    return used_memory;
}

float get_fragmentation(void) {
    int free_block_count = 0;
    MemoryBlock* current = first_block;
    
    while (current != NULL) {
        if (current->is_free) {
            free_block_count++;
        }
        current = current->next;
    }
    
    if (free_block_count <= 1) {
        return 0.0;
    }
    
    float total_blocks = 0;
    current = first_block;
    while (current != NULL) {
        total_blocks++;
        current = current->next;
    }
    
    // Normalize to 0-1 range (1 free block means no fragmentation)
    return (free_block_count - 1) / (total_blocks - 1);
}
