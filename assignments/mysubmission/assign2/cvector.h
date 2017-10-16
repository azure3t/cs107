/* File: cvector.h
 * ---------------
 * Defines the interface for the CVector type.
 *
 * The CVector manages a linear, indexed collection of homogeneous elements.
 * Think of it as an upgrade from the raw C array, with convenient dynamic
 * memory management (allocate, resize, deallocate), operations to insert/remove, 
 * and sort and search facilities. In order to work for all types of elements,
 * the size of each element is specified when creating the CVector, and
 * all CVector elements must be passed and returned via void* pointers. 
 * Given its extensive use of untyped pointers, the CVector is a bit tricky 
 * to use correctly as a client. Be diligent!
 *
 * CS107 jzelenski
 */

#ifndef _cvector_h
#define _cvector_h

#include <stdbool.h>	//  this header defines C99 bool type
#include <stddef.h> 	// size_t

/**
 * Type: CompareFn
 * ---------------
 * CompareFn is the typename for a pointer to a client-supplied 
 * comparator function. A CVector requires the client to provide a
 * comparator to sort or search the CVector. A comparator takes two 
 * const void* pointers, each of which points to an element of the type
 * stored in the CVector, and returns an integer. The integer indicates the 
 * ordering of the two elements using the same convention as strcmp:
 * 
 *   If element at addr1 < element at addr2, return a negative number
 *   If element at addr1 > element at addr2, return a positive number
 *   If element at addr1 = element at addr2, return zero
 * 
 * The typedef allows the nickname "CompareFn" to stand in for the
 * longer declaration. CompareFn can be used as the declared type 
 * for a variable, parameter, struct field, and so on.
 */
typedef int (*CompareFn)(const void *addr1, const void *addr2);


/** 
 * Type: CleanupElemFn
 * -------------------
 * CleanupElemFn is the typename for a pointer to a client-supplied 
 * cleanup function. The client passes a cleanup function to cvec_create 
 * and the CVector will apply that function to an element that is being 
 * removed/replaced/disposed. The cleanup function takes one void* pointer
 * that points to the element.
 *
 * The typedef allows the nickname "CleanupElemFn" to stand in for the
 * longer declaration. CleanupElemFn can be used as the delared type
 * for a variable, parameter, struct field, and so on.
 */
typedef void (*CleanupElemFn)(void *addr);


/**
 * Type: CVector
 * -------------
 * Defines the CVector type. The type is "incomplete", i.e. deliberately
 * avoids stating the field names/types for the struct CVectorImplementation. 
 * (That struct is completed in the implementation code not visible to
 * clients). The incomplete type forces the client to respect the privacy
 * of the representation. Client declare variables only of type CVector* 
 * (pointers only, never of the actual struct) and cannot dereference 
 * a CVector* nor attempt to read/write its internal fields. A CVector 
 * is manipulated solely through the functions listed in this interface
 * (this is analogous to how you use a FILE*).
 */
typedef struct CVectorImplementation CVector;


/**
 * Function: cvec_create
 * Usage: CVector *v = cvec_create(sizeof(int), 10, NULL)
 * ------------------------------------------------------
 * Creates a new empty CVector and returns a pointer to it. The pointer 
 * is to storage allocated in the heap. When done with the CVector, the
 * client should call cvec_dispose to dispose of it. If allocation fails,
 * an assert is raised.
 *
 * The elemsz parameter specifies the size, in bytes, of the type of elements
 * that will be stored in the CVector. For example, to store elements of type
 * double, the client passes sizeof(double) for the elemsz. All elements
 * stored in a given CVector must be of the same type. An assert is 
 * raised if elemsz is zero.
 *
 * The capacity_hint parameter allows the client to tune the resizing behavior.
 * The CVector's internal storage will initially be allocated to hold the 
 * number of elements hinted. This capacity_hint is not a binding limit. 
 * If the initially allocated capacity is outgrown, the CVector enlarges its
 * capacity. If intending to store many elements, specifying a large capacity_hint 
 * will result in an appropriately large initial allocation and fewer resizing 
 * operations later. For a small vector, a small capacity_hint will result in 
 * several smaller allocations and potentially less waste. If capacity_hint 
 * is 0, an internal default value is used. 
 * 
 * The fn parameter is a client callback function to cleanup an element. This
 * function will be called on an element being removed/replaced (via 
 * cvec_remove/cvec_replace respectively) and on every element in the CVector
 * when it is destroyed (via cvec_dispose). The client can use this function 
 * to do any deallocation/cleanup required for the element, such as freeing any
 * memory to which the element points (if the element itself is or contains a
 * pointer). The client can pass NULL for fn if elements don't require any cleanup.
 *
 * Asserts: zero elemsz, allocation failure
 * Assumes: cleanup fn is valid
 */
CVector *cvec_create(size_t elemsz, size_t capacity_hint, CleanupElemFn fn);


/**
 * Function: cvec_dispose
 * Usage: cvec_dispose(v)
 * ----------------------
 * Disposes of the CVector. Calls the client's cleanup function on each element
 * and deallocates memory used for the CVector's storage. Operates in linear-time.
 */
void cvec_dispose(CVector *cv);


/**
 * Function: cvec_count
 * Usage: int count = cvec_count(v)
 * --------------------------------
 * Returns the number of elements currently stored in the CVector.
 * Operates in constant-time.
 */
int cvec_count(const CVector *cv);


/**
 * Function: cvec_nth
 * Usage: int num = *(int *)cvec_nth(v, 0)
 * ---------------------------------------
 * Accesses the element at a given index in the CVector. Returns a
 * pointer to a memory location where the element value is stored.
 * Valid indexes are 0 to count-1. An assert is raised if index is out
 * of bounds. The return value is a pointer into the CVector's storage
 * so it must be used with care. In particular, a pointer returned by 
 * cvec_nth can become invalid during any call that adds, removes, or
 * rearranges elements within the CVector. The CVector could have been 
 * designed without this direct access, but it is useful and efficient to offer 
 * it, despite its potential pitfalls. Operates in constant-time.
 *
 * Asserts: invalid index
 */ 
void *cvec_nth(const CVector *cv, int index);


/**
 * Function: cvec_insert
 * Usage: cvec_insert(v, &elem, 0)
 * -------------------------------
 * Inserts a new element into the CVector, placing it at the given index
 * and shifting up other elements to make room. An assert is raised if index 
 * is less than 0 or greater than the count. addr is expected to be a valid
 * pointer to an element. For example, if this CVector has been created for 
 * int elements, addr should be the memory location where the desired int 
 * value is stored. The value at that location is copied into internal 
 * CVector storage. The capacity is enlarged if necessary, an assert is raised
 * on allocation failure. Operates in linear-time.
 *
 * Asserts: invalid index, allocation failure
 * Assumes: address of valid elem
 */
void cvec_insert(CVector *cv, const void *addr, int index);


/**
 * Function: cvec_append
 * Usage: cvec_append(v, &elem)
 * ----------------------------
 * Appends a new element to the end of the CVector. addr is expected to be a
 * valid pointer to an element. The element value is copied from the memory 
 * location pointed to by addr. The capacity is enlarged if necessary, an 
 * assert is raised on allocation failure. Operates in constant-time (amortized).
 *
 * Asserts: allocation failure
 * Assumes: address of valid elem
 */
void cvec_append(CVector *cv, const void *addr);
  
  
/**
 * Function: cvec_replace
 * Usage: cvec_replace(v, &elem, 0)
 * ---------------------------------
 * Overwrites the element at the given index with a new element. Before 
 * being overwritten, the client's cleanup function is called on the old
 * element. addr is expected to be a valid pointer to an element. The new 
 * element value is copied from the memory location pointed to by addr and 
 * replaces the old element at index. An assert is raised if index is 
 * out of bounds. Operates in constant-time.
 *
 * Asserts: invalid index
 * Assumes: address of valid elem
 *
 * CS107 SPR1617 assign3 notes: You are not required to implement this
 * function. You may just leave a "stub" (a function where the body is
 * empty so it does nothing).
 */
void cvec_replace(CVector *cv, const void *addr, int index);


/**
 * Function: cvec_remove
 * Usage: cvec_remove(v, 0)
 * ------------------------
 * Removes the element at the given index from the CVector and shifts
 * other elements down to close the gap. The client's cleanup function is 
 * called on the element being removed. An assert is raised if index is  
 * out of bounds. Operates in linear-time.
 *
 * Asserts: invalid index
 *
 * CS107 SPR1617 assign3 notes: You are not required to implement this
 * function. You may just leave a "stub" (a function where the body is
 * empty so it does nothing).
 */
void cvec_remove(CVector *cv, int index);
  
  
/**
 * Function: cvec_search
 * Usage: int found = cvec_search(v, &key, cmp_students, 0, false)
 * ---------------------------------------------------------------
 * Searches the CVector for an element matching a key element. keyaddr is 
 * expected to be a valid pointer to the key element. For example, if
 * this CVector has been created for int elements, keyaddr should be
 * the memory location where the key int value is stored.
 * It uses the provided cmp callback to compare elements.
 * The search considers all elements from start index to end. 
 * To search the entire CVector, specify a start index of 0. 
 * The sorted parameter allows the client to specify that elements
 * are currently stored in sorted order, in which case cvec_search uses a
 * faster binary search. If sorted is false, linear search is used instead.
 * If a match is found, the index of a matching element is returned;
 * else -1 is returned. If more than one match exists, any of the
 * matching indexes can be returned. This function does not 
 * re-arrange/modify elements within the CVector or modify the key. 
 * Operates in linear-time or logarithmic-time (if sorted).
 * 
 * An assert is raised if start is less than 0 or greater than the
 * CVector's count (although searching from count will never find anything, 
 * allowing this case means client can search an empty CVector from 0 without 
 * getting an assert). 
 *
 * Asserts: invalid start index
 * Assumes: address of valid key, cmp fn is valid
 */  
int cvec_search(const CVector *cv, const void *keyaddr, CompareFn cmp, int start, bool sorted);


/**
 * Function: cvec_sort
 * Usage: cvec_sort(v, cmp_student)
 * --------------------------------
 * Rearranges elements in the CVector into ascending order according to the 
 * client's provided cmp callback. Operates in NlgN-time.
 *
 * Assumes: cmp fn is valid
 */  
void cvec_sort(CVector *cv, CompareFn cmp);


/**
 * Functions: cvec_first, cvec_next
 * Usage: for (void *cur = cvec_first(v); cur != NULL; cur = cvec_next(v, cur))
 * ----------------------------------------------------------------------------
 * These functions provide iteration over the CVector elements. The client
 * starts an iteration with a call to cvec_first which returns a pointer
 * to the first (0th) element of the CVector or NULL if the CVector is empty.
 * The client loop calls cvec_next passing the pointer to the previous
 * element and receives a pointer to the next element in the iteration
 * or NULL if there are no more elements. Elements are iterated in order
 * of increasing index (from 0 to N-1). A pointer returned by cvec_first
 * or cvec_next points to the memory location where the element value is stored
 * within the internals of the CVector.
 * The argument to cvec_next is expected to be a valid pointer to an element 
 * as returned by a previous call to cvec_first/cvec_next. CVector supports
 * multiple simultaneous iterations without cross-interference. The client
 * must not add/remove/rearrange CVector elements in the midst of iterating.
 * The functions operate in constant-time.
 *
 * Assumes: address of prev is valid
 */  
void *cvec_first(const CVector *cv);
void *cvec_next(const CVector *cv, const void *prev);

#endif
