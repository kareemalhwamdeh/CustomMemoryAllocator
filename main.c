#include "allocator.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Simple test function
void basic_test(void) {
    printf("Basic Test\n");
    
    // Allocate memory
    int* arr1 = (int*)mem_alloc(5 * sizeof(int));
    if (arr1) {
        printf("Array 1\n");
        for (int i = 0; i < 5; i++) {
            arr1[i] = i * 10;
        }
        print_memory_map();
    }
    
    // Allocate more memory
    int* arr2 = (int*)mem_alloc(10 * sizeof(int));
    if (arr2) {
        printf("Array 2\n");
        for (int i = 0; i < 10; i++) {
            arr2[i] = i * 5;
        }
        print_memory_map();
    }
    
    // Free first array
    printf("Free 1\n");
    mem_free(arr1);
    print_memory_map();
    
    // Allocate a smaller block that should fit in the freed space
    char* str = (char*)mem_alloc(12);
    if (str) {
        printf("String\n");
        sprintf(str, "Hello World");
        printf("%s\n", str);
        print_memory_map();
    }
    
    // Free all allocations
    mem_free(arr2);
    mem_free(str);
    print_memory_map();
}

// Test calloc functionality
void calloc_test(void) {
    printf("\nCalloc\n");
    
    // Allocate and initialize array to zero
    int* arr = (int*)mem_calloc(5, sizeof(int));
    if (arr) {
        printf("Zeros:\n");
        for (int i = 0; i < 5; i++) {
            printf("arr[%d] = %d\n", i, arr[i]);
        }
        print_memory_map();
        mem_free(arr);
    }
}

// Test realloc functionality
void realloc_test(void) {
    printf("\nRealloc\n");
    
    // Allocate initial array
    int* arr = (int*)mem_alloc(5 * sizeof(int));
    if (arr) {
        printf("Small:\n");
        for (int i = 0; i < 5; i++) {
            arr[i] = i * 10;
            printf("arr[%d] = %d\n", i, arr[i]);
        }
        print_memory_map();
        
        // Grow the array
        printf("Bigger\n");
        arr = (int*)mem_realloc(arr, 10 * sizeof(int));
        if (arr) {
            // Set values for new elements
            for (int i = 5; i < 10; i++) {
                arr[i] = i * 10;
            }
            for (int i = 0; i < 10; i++) {
                printf("arr[%d] = %d\n", i, arr[i]);
            }
            print_memory_map();
            
            // Shrink the array
            printf("Smaller\n");
            arr = (int*)mem_realloc(arr, 3 * sizeof(int));
            if (arr) {
                for (int i = 0; i < 3; i++) {
                    printf("arr[%d] = %d\n", i, arr[i]);
                }
                print_memory_map();
                mem_free(arr);
            }
        }
    }
}

// Test allocation strategy (first-fit vs best-fit)
void strategy_test(void) {
    printf("\nStrategy\n");
    
    // Clean up previous allocations
    cleanup_allocator();
    
    // Test first-fit strategy
    printf("\nFirst-fit\n");
    init_allocator(1024, FIRST_FIT);
    
    void* ptr1 = mem_alloc(100);
    void* ptr2 = mem_alloc(200);
    void* ptr3 = mem_alloc(300);
    print_memory_map();
    
    // Free the middle block to create a hole
    printf("Free middle\n");
    mem_free(ptr2);
    print_memory_map();
    
    // Allocate a smaller block, which should use the first available hole (ptr2's space)
    printf("Small block\n");
    void* ptr4 = mem_alloc(50);
    print_memory_map();
    
    // Clean up
    mem_free(ptr1);
    mem_free(ptr3);
    mem_free(ptr4);
    cleanup_allocator();
    
    // Test best-fit strategy
    printf("\nBest-fit\n");
    init_allocator(1024, BEST_FIT);
    
    ptr1 = mem_alloc(100);
    ptr2 = mem_alloc(200);
    ptr3 = mem_alloc(300);
    print_memory_map();
    
    // Free the first and middle blocks to create holes of different sizes
    printf("Two holes\n");
    mem_free(ptr1); // 100 bytes hole
    mem_free(ptr2); // 200 bytes hole
    print_memory_map();
    
    // Allocate a block that fits in both holes, should use the smaller hole (ptr1's space) with best-fit
    printf("Small hole\n");
    ptr4 = mem_alloc(50);
    print_memory_map();
    
    // Clean up
    mem_free(ptr3);
    mem_free(ptr4);
}

// Fragmentation test
void fragmentation_test(void) {
    printf("\nFragmentation\n");
    
    // Clean up previous allocations
    cleanup_allocator();
    init_allocator(1024, FIRST_FIT);
    
    // Allocate and free in a pattern that causes fragmentation
    void* ptrs[10];
    
    // Allocate 10 small blocks
    for (int i = 0; i < 10; i++) {
        ptrs[i] = mem_alloc(40);  // 40 bytes each
        printf("Block %d\n", i);
    }
    print_memory_map();
    
    // Free alternate blocks to create fragmentation
    for (int i = 0; i < 10; i += 2) {
        mem_free(ptrs[i]);
        printf("Free %d\n", i);
    }
    print_memory_map();
    
    // Try to allocate a larger block, which won't fit in any single hole
    void* large_ptr = mem_alloc(100);
    if (large_ptr == NULL) {
        printf("Failed: fragmented\n");
    } else {
        printf("Success: 100B\n");
        mem_free(large_ptr);
    }
    
    // Coalescing should happen after all blocks are freed
    for (int i = 1; i < 10; i += 2) {
        mem_free(ptrs[i]);
    }
    print_memory_map();
    
    // Now the large allocation should succeed
    large_ptr = mem_alloc(400);
    if (large_ptr != NULL) {
        printf("Success: 400B\n");
        print_memory_map();
        mem_free(large_ptr);
    }
}

// Stress test with random allocations and frees
void stress_test(void) {
    printf("\nStress\n");
    
    // Clean up previous allocations
    cleanup_allocator();
    init_allocator(4096, BEST_FIT); // Larger pool for stress test
    
    const int NUM_PTRS = 100;
    void* ptrs[NUM_PTRS];
    
    // Initialize array
    for (int i = 0; i < NUM_PTRS; i++) {
        ptrs[i] = NULL;
    }
    
    srand((unsigned int)time(NULL));
    
    // Perform random allocations and frees
    for (int round = 0; round < 1000; round++) {
        int idx = rand() % NUM_PTRS;
        
        // Free existing allocation with 40% probability
        if (ptrs[idx] != NULL && (rand() % 100 < 40)) {
            mem_free(ptrs[idx]);
            ptrs[idx] = NULL;
        } 
        // Otherwise allocate or reallocate
        else {
            size_t size = (size_t)(rand() % 200) + 1; // 1-200 bytes
            
            if (ptrs[idx] == NULL) {
                // New allocation
                ptrs[idx] = mem_alloc(size);
            } else {
                // Reallocation
                void* new_ptr = mem_realloc(ptrs[idx], size);
                if (new_ptr != NULL) {
                    ptrs[idx] = new_ptr;
                }
            }
        }
        
        // Occasionally show memory map (every 200 operations)
        if (round % 200 == 0) {
            printf("\nProgress: %d/1000\n", round);
            print_memory_map();
        }
    }
    
    // Clean up any remaining allocations
    for (int i = 0; i < NUM_PTRS; i++) {
        if (ptrs[i] != NULL) {
            mem_free(ptrs[i]);
        }
    }
    
    printf("\nFinal:\n");
    print_memory_map();
}

int main(void) {
    printf("Memory Allocator\n");
    printf("Kareem\n\n");
    
    init_allocator(1024, FIRST_FIT);
    
    basic_test();
    calloc_test();
    realloc_test();
    strategy_test();
    fragmentation_test();
    stress_test();
    
    cleanup_allocator();
    
    printf("Done\n");
    return 0;
}
