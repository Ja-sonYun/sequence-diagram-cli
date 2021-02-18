#ifndef MEM_H
#define MEM_H
#include <stdio.h>
#include <stdlib.h>
// bit control
#define UNFLAG_R(flag, len) flag >> len << len // unflag bits by length from front bit

// memory
#define CHECK_MEM_ERR(v) \
    if (!v) \
    { \
        fprintf(stderr, "Insufficient memory"); \
        exit(EXIT_FAILURE); \
    }

#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
  (byte & 0x80 ? '1' : '0'), \
  (byte & 0x40 ? '1' : '0'), \
  (byte & 0x20 ? '1' : '0'), \
  (byte & 0x10 ? '1' : '0'), \
  (byte & 0x08 ? '1' : '0'), \
  (byte & 0x04 ? '1' : '0'), \
  (byte & 0x02 ? '1' : '0'), \
  (byte & 0x01 ? '1' : '0')
// #define print_uint32_t(head, bin) printf(head BYTE_TO_BINARY_PATTERN" "BYTE_TO_BINARY_PATTERN" "BYTE_TO_BINARY_PATTERN" "BYTE_TO_BINARY_PATTERN"\n", \
//     BYTE_TO_BINARY(bin>>24), BYTE_TO_BINARY(bin>>16), BYTE_TO_BINARY(bin>>8), BYTE_TO_BINARY(bin));
#define print_uint32_t(head, bin) NULL


// malloc
static inline void* malloc_s(size_t t)
{
    void *n = malloc(t);
    CHECK_MEM_ERR(n);
    return n;
}

// calloc
static inline void* calloc_s(size_t num, size_t size)
{
    void *n = calloc(num, size);
    CHECK_MEM_ERR(n);
    return n;
}

// realloc
static inline void* realloc_s(void *ptr, size_t size)
{
    void *n = realloc(ptr, size);
    CHECK_MEM_ERR(n);
    return n;
}
#endif
