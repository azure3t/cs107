/* File: allocator.h
 * -----------------
 * Interface file for the custom heap allocator.
 */
#ifndef _ALLOCATOR_H
#define _ALLOCATOR_H

#include <stdbool.h> // for bool
#include <stddef.h>  // for size_t


/* Function: myinit
 * ----------------
 * This must be called by a client before making any allocation
 * requests.  The function returns true if initiaization was successful,
 * false otherwise. The myinit function can be called to reset
 * the heap to an empty state. When running against a whole suite of
 * of test scripts, our test harness calls myinit before starting
 * each new script.
 */
bool myinit(void);

/* Function: mymalloc
 * ------------------
 * Custom version of malloc.
 */
void *mymalloc(size_t size);


/* Function: myrealloc
 * -------------------
 * Custom version of realloc.
 */
void *myrealloc(void *ptr, size_t size);


/* Function: myfree
 * ----------------
 * Custom version of free.
 */
void myfree(void *ptr);


/* Function: validate_heap
 * -----------------------
 * This is the hook for your heap consistency checker. Returns true
 * if all is well, false on any problem.
 */
bool validate_heap(void);



#endif
