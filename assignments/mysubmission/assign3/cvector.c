/*
 * File: cvector.c
 * Author: Tiantian Tang 
 * ----------------------
 *
 */
#include <assert.h>
#include "cvector.h"
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>

// a suggested value to use when given capacity_hint is 0
#define DEFAULT_CAPACITY 16

/* Type: struct CVectorImplementation
 * ----------------------------------
 * This definition completes the CVector type that was declared in
 * cvector.h. You fill in the struct with your chosen fields.
 */
struct CVectorImplementation {
    void *elems; //store pointer to the first element
    size_t nelems;//capacity of the  Cvector
    size_t elemsz;//size of individual elements in bytes
    size_t size;// count of current elemtns stored in Cvector
};




/* The NOT_YET_IMPLEMENTED macro is used as the body for all functions
 * to remind you about which operations you haven't yet implemented.
 * It will report a fatal error if a call is made to an not-yet-implemented
 * function (this is preferable to returning garbage or silently
 * ignoring the call).  Remove the call to this macro as you implement
 * each function and finally you can remove this macro and comment
 * when no longer needed.
 */
#define NOT_YET_IMPLEMENTED printf("%s() not yet implemented!\n", __func__); raise(SIGKILL); exit(107);


CVector *cvec_create(size_t elemsz, size_t capacity_hint, CleanupElemFn fn)
{
    CVector *cv = malloc(sizeof(CVector));
    cv->nelems = capacity_hint <= 0 ? DEFAULT_CAPACITY : capacity_hint;// CVector capacity
    cv->elems = malloc(sizeof(elemsz)*cv->nelems);//points to 0th elem in array always
    cv->elemsz = elemsz;// element size in byte
    cv->size = 0;// CVector size
    return cv;

};

void cvec_dispose(CVector *cv)
{
    free(cv->elems);
    free(cv);
}

int cvec_count(const CVector *cv)
{
    return cv->size;
}

void *cvec_nth(const CVector *cv, int index)
{
    assert(index >= 0 && index < cv->size);
    void *nth = (char *)cv->elems + index * cv->elemsz;
    return nth;
}

void doubleCap(CVector *cv){
    // if CVector is filled, double the capacity
    if (cv->size == cv->nelems){
        cv->elems = realloc(cv->elems,(size_t) 2 * cv->nelems * cv->elemsz);
        cv->nelems = 2 * cv->nelems;
    } 

}
void cvec_insert(CVector *cv, const void *addr, int index)
{
    assert(index >= 0 && index <= cv->size);
    doubleCap(cv);
    // address of new element after insertion
    void *dest = (char *) cv->elems + index * cv->elemsz;

    // move old elemes to the right
    if (index <= cv->size - 1){
        void *destPlusOne = (char *)dest + cv->elemsz;
        int bitsToMove = cv->elemsz * (cv->size - index); // bits need to move to the right
        char tmp[bitsToMove];//tmp char array to hold old value
        memcpy(tmp, dest, bitsToMove);//copy old bits to tmp
        memcpy(destPlusOne, tmp, bitsToMove);//copy from tmp to right of original idx
    }
    // insert new element
    memcpy(dest, addr, cv->elemsz);
    cv->size++;
}

void cvec_append(CVector *cv, const void *addr)
{
    cvec_insert(cv, addr, cv->size);
}



void cvec_replace(CVector *cv, const void *addr, int index)
{}//not required for sp17

void cvec_remove(CVector *cv, int index)
{}//not required for sp17


int cvec_search(const CVector *cv, const void *key, CompareFn cmp, int start, bool sorted)
{
    assert(start >= 0 && start <= cv->size);   
    char *cur = (char *)cv->elems + start * cv->elemsz;
    if (!sorted){
        //linear search if unsorted
        for (; cur != NULL; cur = cvec_next(cv, cur)){
            if (cmp(cur, key) == 0) return (int)((cur - (char *)cv->elems) / cv->elemsz);
        }
        return -1;
    }else{
        //binary search if sorted
        void *res = bsearch(key, cur, cv->size - start, cv->elemsz, cmp);
        return res == NULL ? -1 : (int)(((char *)res - (char *)cv->elems) / cv->elemsz);
    }

}

void cvec_sort(CVector *cv, CompareFn cmp)
{
    qsort(cv->elems, cv->size, cv->elemsz, cmp);
}

void *cvec_first(const CVector *cv)
{
    if (cv->size == 0) return NULL;
    return cv->elems;
}

void *cvec_next(const CVector *cv, const void *prev)
{
    //return NULL if already at end of CVector
    if ( ((char *)prev - (char *)cv->elems)/cv->elemsz + 1 == 
            cv->size) return NULL;
    return (char *)prev + cv->elemsz;

}
