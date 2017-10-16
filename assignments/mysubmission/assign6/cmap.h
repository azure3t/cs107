/* File: cmap.h
 * ------------
 * Defines the interface for the CMap type.
 *
 * The CMap manages a collection of key-value pairs or "entries". The keys
 * are always of type char *, but the values can be of any desired type. The
 * main operations are to associate a value with a key and to retrieve
 * the value associated with a key. In order to work for all types of values,
 * all CMap values must be passed and returned via (void *) pointers.
 * Given its extensive use of untyped pointers, the CMap is a bit tricky
 * to use correctly as a client. Be diligent!
 *
 * CS107 jzelenski
 */

#ifndef _cmap_h
#define _cmap_h

#include <stddef.h>


 /**
  * Type: CleanupValueFn
  * --------------------
  * CleanupValueFn is the typename for a pointer to a client-supplied
  * cleanup function. The client passes a cleanup function to cmap_create
  * and the CMap will apply that function to a value that is being
  * removed/replaced/disposed. The cleanup function takes one void* pointer
  * that points to the value.
  *
  * The typedef allows the nickname "CleanupValueFn" to stand in for the
  * longer declaration. CleanupValueFn can be used as the declared type
  * for a variable, parameter, struct field, and so on.
  */
typedef void (*CleanupValueFn)(void *addr);


/**
 * Type: CMap
 * ----------
 * Defines the CMap type. The type is "incomplete", i.e. deliberately
 * avoids stating the field names/types for the struct CMapImplementation.
 * (That struct is completed in the implementation code not visible to
 * clients). The incomplete type forces the client to respect the privacy
 * of the representation. Client declare variables only of type CMap *
 * (pointers only, never of the actual struct) and cannot dereference
 * a CMap * nor attempt to read/write its internal fields. A CMap
 * is manipulated solely through the functions listed in this interface
 * (this is analogous to how you use a FILE *).
 */
typedef struct CMapImplementation CMap;


/**
 * Function: cmap_create
 * Usage: CMap *m = cmap_create(sizeof(int), 10, NULL)
 * ---------------------------------------------------
 * Creates a new empty CMap and returns a pointer to it. The pointer
 * is to storage allocated in the heap. When done with the CMap, the
 * client must call cmap_dispose to dispose of it. If allocation fails, an
 * assert is raised.
 *
 * The valuesz parameter specifies the size, in bytes, of the type of
 * values that will be stored in the CMap. For example, to store values of
 * type double, the client passes sizeof(double) for the valuesz. All values
 * stored in a given CMap must be of the same type. An assert is raised
 * if valuesz is zero.
 *
 * The capacity_hint parameter is an estimate of the number of entries
 * that will be stored in this CMap. The internal storage will be
 * optimized for this size. The capacity_hint is not a binding limit. 
 * If the number of entries exceeds original hint, the map accepts 
 * additional entries but performance may suffer. If capacity_hint is 0, 
 * an internal default value is used. (As a special case, our sample 
 * CMap interprets 0 capacity hint as a request to enable flexible resizing
 * that adapts to load and maintains fast performance for any number of
 * entries. This behavior is not expected in your implementation.)
 * The CMap has its best performance when the given capacity_hint is close 
 * to the number of entries added. If configured for a much-too-small capacity,
 * the CMap operations will degrade from constant to linear time. If 
 * configured for a much-too-large capacity, the CMap will consume 
 * excessive amounts of memory.
 *
 * The fn is a client callback that will be called on a value being
 * removed/replaced (via cmap_remove/cmap_put, respectively) and on every value
 * in the CMap when it is destroyed (via cmap_dispose). The client can use this
 * function to do any deallocation/cleanup required for the value, such as
 * freeing memory to which the value points (if the value is or contains a
 * pointer). The client can pass NULL for fn if values don't require any
 * cleanup.
 *
 * Asserts: zero elemsz, allocation failure
 * Assumes: cleanup fn is valid
 */  
CMap *cmap_create(size_t valuesz, size_t capacity_hint, CleanupValueFn fn);


/**
 * Function: cmap_dispose
 * Usage: cmap_dispose(m)
 * ----------------------
 * Disposes of the CMap. Calls the client's cleanup function on each value
 * and deallocates memory used for the CMap's storage, including the keys
 * that were copied. Operates in linear-time.
 */
void cmap_dispose(CMap *cm);


/**
 * Function: cmap_count
 * Usage: int count = cmap_count(m)
 * --------------------------------
 * Returns the number of entries currently stored in the CMap. Operates in
 * constant-time.
 */
int cmap_count(const CMap *cm);


/**
 * Function: cmap_put
 * Usage: cmap_put(m, "CS107", &val)
 * ---------------------------------
 * Associates the given key with a new value in the CMap. If there is an
 * existing value for the key, it is replaced with the new value. Before
 * being overwritten, the client's cleanup function is called on the old value.
 * addr is expected to be a valid pointer to a value. For example, if this 
 * CMap has been created for int values, addr should be the memory location 
 * where the desired int value is stored. The value at that location is 
 * copied into internal CMap storage. CMap makes a copy of the key string
 * and stores the copy internally with the value. Note that keys are 
 * compared case-sensitively, e.g. "binky" is not the same key as "BinKy". 
 * An assert is raised on allocation failure. Operates in constant-time (amortized)
 *
 * Asserts: allocation failure
 * Assumes: key is valid, address of valid value
 */ 
void cmap_put(CMap *cm, const char *key, const void *addr);


/**
 * Function: cmap_get
 * Usage: int val = *(int *)cmap_get(m, "CS107")
 * ---------------------------------------------
 * Searches the CMap for an entry with the given key and if found, returns
 * a pointer to the memory location where its associated value is stored.
 * If key is not found, then NULL is returned as a sentinel. The function
 * returns a pointer to the value within the CMap's storage. This pointer
 * provides direct access for a client to update a value "in-place".  
 * Note the pointer should be used with care. In particular, the pointer 
 * returned by cmap_get will become invalid during a call that removes this key.
 * Note that keys are compared case-sensitively, e.g. "binky" is not 
 * the same key as "BinKy". Operates in constant-time.
 *
 * Assumes: key is valid
 */
void *cmap_get(const CMap *cm, const char *key);


/**
 * Function: cmap_remove
 * Usage: cmap_remove(m, "CS107")
 * ------------------------------
 * Searches the CMap for an entry with the given key and if found, removes that
 * key and its associated value. If key is not found, no changes are made.
 * The client's cleanup function is called on the removed value and the copy of the
 * key string is deallocated. Note that keys are compared case-sensitively,
 * e.g. "binky" is not the same key as "BinKy". Operates in constant-time.
 *
 * Assumes: key is valid
 *
 * CS107 AUT1617 assign3 notes: You are not required to implement this
 * function. You may just leave a "stub" (a function where the body is
 * empty so it does nothing).
 */
void cmap_remove(CMap *cm, const char *key);


/**
 * Functions: cmap_first, cmap_next
 * Usage: for (const char *key = cmap_first(m); key != NULL; key = cmap_next(m, key))
 * ----------------------------------------------------------------------------------
 * These functions provide iteration over the CMap keys. The client
 * starts an iteration with a call to cmap_first which returns one of the keys
 * of the CMap or NULL if the CMap is empty. The client loop calls
 * cmap_next passing the previous key and receives the next key in the iteration
 * or NULL if there are no more keys. Keys are iterated in arbitrary order.
 * The argument to cmap_next is expected to be a key string as returned by a 
 * previous call to cmap_first/cmap_next. The CMap supports
 * multiple simultaneous iterations without cross-interference. The client
 * must not add/remove/rearrange CMap entries in the midst of iterating.
 * These functions operate in constant-time (amortized).
 *
 * Assumes: prevkey is valid
 */
const char *cmap_first(const CMap *cm);
const char *cmap_next(const CMap *cm, const char *prevkey);

#endif
