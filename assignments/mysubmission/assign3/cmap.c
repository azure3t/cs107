/*
 * File: cmap.c
 * Author: Tiantian Tang
 * ----------------------
 *
 */

#include "cmap.h"
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <stdbool.h>
// a suggested value to use when given capacity_hint is 0
#define DEFAULT_CAPACITY 1023

/* Type: struct CMapImplementation
 * -------------------------------
 * This definition completes the CMap type that was declared in
 * cmap.h. You fill in the struct with your chosen fields.
 */
struct CMapImplementation {
    size_t nbuckets; // capacity or number of buckets, proviede by user
    void **buckets; //points to first bucket in the bucket array which stores pointer to linkedlists
    size_t valuesz; //size of each value, provided by user
    size_t size; //number of <key,value pair stored in cmap
};


/* The NOT_YET_IMPLEMENTED macro is used as the body for all functions
 * to remind you about which operations you haven't yet implemented.
 * It wil report an error if a call is made to an not-yet-implemented
 * function (this is preferable to returning garbage or silently
 * ignoring the call).  Remove the call to this macro as you implement
 * each function and finally you can remove this macro and comment
 * when no longer needed.
 */
#define NOT_YET_IMPLEMENTED printf("%s() not yet implemented!\n", __func__); raise(SIGKILL); exit(107);



/* Function: hash
 * --------------
 * This function adapted from Eric Roberts' _The Art and Science of C_
 * It takes a string and uses it to derive a "hash code," which
 * is an integer in the range [0..nbuckets-1]. The hash code is computed
 * using a method called "linear congruence." A similar function using this
 * method is described on page 144 of Kernighan and Ritchie. The choice of
 * the value for the multiplier can have a significant effort on the
 * performance of the algorithm, but not on its correctness.
 * The computed hash value is stable, e.g. passing the same string and
 * nbuckets to function again will always return the same code.
 * The hash is case-sensitive, "ZELENSKI" and "Zelenski" are
 * not guaranteed to hash to same code.
 */
static int hash(const char *s, int nbuckets)
{
    const unsigned long MULTIPLIER = 2630849305L; // magic number
    unsigned long hashcode = 0;
    for (int i = 0; s[i] != '\0'; i++)
        hashcode = hashcode * MULTIPLIER + s[i];
    return hashcode % nbuckets;
}


CMap *cmap_create(size_t valuesz, size_t capacity_hint, CleanupValueFn fn)
{
    CMap *cm = malloc(sizeof(CMap));
    cm->nbuckets = capacity_hint == 0 ? DEFAULT_CAPACITY : capacity_hint;
    cm->valuesz = valuesz;
    cm->buckets = calloc(sizeof(void *) * cm->nbuckets, 1);
    cm->size = 0;
    return cm;

}

void cmap_dispose(CMap *cm)
{
    //find all the cell pointers using cmap_next
    void *cellptrs[cm->size];//array that contain pointers in each cell that will be freed
    int cnt = 0; // 
    for (const char *key = cmap_first(cm); key != NULL; key = cmap_next(cm, key)){
        //ptr to next cell is the stored in 8 bytes before key pointer
        void *cellptr = *(void **)((char *)key - sizeof(void *));
        cellptrs[cnt++] = cellptr;
    }
    //free each cellptr
    for (int i = 0; i < cnt; i++){
        if (cellptrs[i] != NULL) free(cellptrs[i]);
    }

    //free each bucket
    for (int i = 0; i < cm->nbuckets; i++){
        if (cm->buckets[i] != NULL)free(cm->buckets[i]);
    }
    //free cm->buckets
    free(cm->buckets);
    //finally free cm pointer
    free(cm);
}


int cmap_count(const CMap *cm)
{
    return cm->size;
}

// return ptr to malloced cell
void buildCell(void *firstCellptr, const char *key, const void* addr, size_t valuesz){
    // dereference firstCellptr to store pointer returned by malloc 
    *(void **)firstCellptr = calloc(sizeof(void*) + strlen(key) + 1 + valuesz, 1);
    // copy key
    strcpy(*(char **)firstCellptr + sizeof(void *), key);
    // copy value
    memcpy(*(char **)firstCellptr + sizeof(void *) + strlen(key) + 1, addr, valuesz);
}

// compare key input with key in the cell blob
// return 0 if same key
int sameKey(void *cur, const char *keyProvided){
    char *keyInMap = (char *) cur + sizeof(void *);
    return strncmp(keyInMap, keyProvided, strlen(keyProvided));
}

void cmap_put(CMap *cm, const char *key, const void *addr)
{
    int idx = hash(key, cm->nbuckets); //index of the bucket
    void *head = &cm->buckets[idx];//ptr to head pointer of linkedlist

    while (*(void **)head != NULL){
        if (sameKey(*(void **)head, key) == 0){//update value for same key
            void *dest = (char *) *(void **)head + sizeof(void *) + strlen(key) + 1;
            memcpy(dest, addr, cm->valuesz);
            return; 
        }
        head = *(void **) head;
    }

    // if key not found, append to end of linkedlist in that bucket
    buildCell(head, key, addr, cm->valuesz);
    cm->size++;

}

void *cmap_get(const CMap *cm, const char *key)
{

    int idx = hash(key, cm->nbuckets); //index of the bucket
    void *head = &cm->buckets[idx];///ptr to head pointer of linkedlist

    while (*(void **)head != NULL){
        if (sameKey(*(void **)head, key) == 0){
            void *valueptr = (char *) *(void **)head + sizeof(void *) + strlen(key) + 1;       
            return valueptr;
        }
        head = *(void **) head;
    }
    return NULL;
}

void cmap_remove(CMap *cm, const char *key)
{}//not required to implemented

const char *cmap_first(const CMap *cm)
{
    if (cm->size == 0) return NULL;
    for (int i = 0; i < cm->nbuckets; i++){ // loop through each bucket to find an arbitrary key
        void *head = &cm->buckets[i];
        if(*(void **)head != NULL){
            const char*keyptr =  (char *) *(void **)head + sizeof(void *);
            return keyptr; // return first key when found
        }
    }
    return NULL;

}

const char *cmap_next(const CMap *cm, const char *prevkey)
{

    int idx = hash(prevkey, cm->nbuckets); 
    void *head = &cm->buckets[idx];

    while (*(void **)head != NULL){
        if (sameKey(*(void **)head, prevkey) == 0){
            void *next = *(void **) *(void **)head;
            if (next != NULL){//there's still cell after cell with prevkey
                // calculate pointer to next key
                const char *nextkeyptr = (char *)next + sizeof(void *);
                return nextkeyptr;
            }else{// there's no cell after cell with prevkey 
                //try to get key from next non-empty bucket
                for (int i = idx + 1; i < cm->nbuckets; i++){
                    void *head2 = &cm->buckets[i];
                    if(*(void **)head2 != NULL){
                        const char*keyptr =  (char *) *(void **)head2 + sizeof(void *);
                        return keyptr; // return first key when looping thru the non-null bucket
                    }
                }

                return NULL;
            }

        }
        head = *(void **) head;
    }
    return NULL;
}
