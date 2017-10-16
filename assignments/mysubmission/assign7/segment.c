/* File: segment.c
 * ---------------
 * Handles low-level storage underneath the dynamic allocator. It reserves
 * the large memory segment using the OS-level mmap facility and then
 * opens it up on demand based on calls to extend.
 */

#include "segment.h"
#include <sys/mman.h>

// Place the heap at lower address, as default addresses are quite high and easily
// mistaken for stack addresses
#define HEAP_START_HINT (void *)0x1070000000L

// Entire segment is 8 GB
#define MAX_SEGMENT_SIZE (1L << 33)

// static variables track state of heap segment
static void * segment_start = NULL;
static size_t segment_size = 0;

void *heap_segment_start()
{
    return segment_start;
}

size_t heap_segment_size()
{
    return segment_size;
}


// Discard any previous segment by unmapping old segment
// Re-initialize by reserving new segment with mmap
void *init_heap_segment(size_t npages)
{
    if (segment_start != NULL) { // discard existing segment
        if (munmap(segment_start, MAX_SEGMENT_SIZE) == -1) return NULL;
        segment_start = NULL;
    }
    // reserve entire segment in advance
    if ((segment_start = mmap(HEAP_START_HINT, MAX_SEGMENT_SIZE, PROT_NONE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0)) == MAP_FAILED)
        return NULL; // allocation failure
    segment_size = 0;
    return extend_heap_segment(npages);
}


// Extend the segment and return the start address of new pages
void *extend_heap_segment(size_t npages)
{
    if (segment_start == NULL) return NULL; // init has not been called?

    void *previous_end = (char *)segment_start + segment_size;
    if (npages <= 0) return previous_end;
    size_t increment_size = npages*PAGE_SIZE;
    if (increment_size > MAX_SEGMENT_SIZE || (segment_size + increment_size) > MAX_SEGMENT_SIZE)
        return NULL;  // cannot extend beyond max size
    segment_size += increment_size;
    if (mprotect(previous_end, increment_size, PROT_READ|PROT_WRITE) == -1)
        return NULL;  // allocation failure
    return previous_end;
}

