# Custom Memory Allocator

A memory management system written in C that implements the functionality of malloc, calloc, realloc, and free using a statically allocated memory block.

This custom memory allocator provides a comprehensive solution for dynamic memory management without relying on the standard library allocation functions. It employs a configurable memory pool with sophisticated block tracking mechanisms to efficiently allocate, deallocate, and resize memory regions. The implementation supports both first-fit and best-fit allocation strategies to optimize memory usage based on different application requirements. Memory fragmentation is addressed through automatic block splitting and coalescing algorithms that maintain memory efficiency even under intensive use. The allocator includes robust error handling for boundary conditions and provides detailed memory visualization tools for debugging and optimization. Through extensive testing with various allocation patterns, the system demonstrates reliable performance across different memory usage scenarios, making it suitable for resource-constrained environments where memory management control is critical.

## Author

Kareem Alhwamdeh

## Building and Running

```bash
make
./memory_allocator
```

The program includes test cases demonstrating basic allocation, zero-initialized memory, resizing, allocation strategies, and fragmentation handling.
