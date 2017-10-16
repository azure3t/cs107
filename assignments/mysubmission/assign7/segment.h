/* File: segment.h
 * ---------------
 * A simple interface to a low-level memory allocator. These functions
 * allocate/extend a large segment of memory. Your allocator will call these
 * functions to manage the segment which is parceled out in response to
 * malloc requests. The segment is allocated in page-size chunks.
 * There is an upper bound on the total segment size. If you attempt to extend
 * the segment beyond that bound, NULL is returned to indicate failure.
 */

#ifndef _SEGMENT_H_
#define _SEGMENT_H_
#include <stddef.h> // for size_t

/* Constants
 * ---------
 * The segment manager allocates memory in chunks called "pages".
 * PAGE_SIZE is the number of bytes in a single page of memory.
 * You can assume the page size is fixed to this constant value.
 */
#define PAGE_SIZE 4096


/* Function: init_heap_segment
 * ---------------------------
 * This function is called to initialize the heap segment and allocate the
 * segment to hold npages, each of PAGE_SIZE bytes. The parameter npages can
 * be 0, in which case the heap segment is configured with no pages yet
 * allocated. After initialization, the extend_heap_segment function is used
 * to further grow the segment. If init_heap_segment is called again, it discards
 * the current heap segment and re-configures for npages. The function returns
 * the base address of the heap segment if successful or NULL if the initialization
 * failed. The base address of the heap segment is always page-aligned.
 */
void *init_heap_segment(size_t npages);



/* Function: extend_heap_segment
 * -----------------------------
 * This function is called to extend the size of the existing heap segment. The
 * segment must have been previously initialized using init_heap_segment. If
 * extend is successful, the existing heap segment is enlarged to include an
 * additional npages and the starting address of those new pages is
 * returned (this address was previously the end of the heap segment). If the heap
 * cannot be extended, NULL is returned. The address returned is always page-aligned.
 */
void *extend_heap_segment(size_t npages);


/* Functions: heap_segment_start, heap_segment_size
 * ------------------------------------------------
 * heap_segment_start returns the base address of the current heap segment
 * (NULL if no segment has been initialized).
 * heap_segment_size returns the current segment size in bytes.
 * The segment size will always be a multiple of PAGE_SIZE.
 * The start and size define the current extent of the heap segment.
 */
void *heap_segment_start(void);
size_t heap_segment_size(void);


#endif
