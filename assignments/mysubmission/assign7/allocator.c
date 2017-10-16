/*
 * File: allocator.c
 * Author: Tiantian Tang 
 * -----------------------------------------------------------------------------
 * A heap allocator that implmenets mymalloc, myfree and myrealloc. Implemented
 * based on segregated explicit free lists. 
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include "allocator.h"
#include "segment.h"
#pragma pack(1)

/*Struct definitions*/
/*----------------------------------------------------------------------------*/

// prologue is a 16 bytes allocated block consists of only header and footer
// created during init and never freed.
typedef struct{
    size_t header;
    size_t footer;
}prologue;

// eiplogue is a 8 byte allocated block consists of only header.
typedef struct{
    size_t header;
}epilogue;

// allocated block struct
typedef struct allocatedBlock allocatedBlock;
struct  allocatedBlock{
    size_t header;
};

// freed block struct
typedef struct freeBlock freeBlock;

struct freeBlock {
    size_t header; // header size:
    freeBlock *prev; // pree fereed block pointer
    freeBlock *next; // next freed block pointer
};


/*Macros*/
/*------------------------------------------------------------------*/

// Heap blocks are required to be aligned to 8-byte boundary
#define ALIGNMENT 8
#define NUM_BUCKETS 29 
#define MIN(x,y) ((x) < (y) ? (x) : (y))
#define MINBLKSZ 40
// Pack a size and allocated bit into header/footer.
// alloc bit is LSB, rest indicate size in bytes
#define PACK(size, alloc) ((size) | (alloc))

// Read and write at header/footer pointed by p
#define GET(p) (*(size_t *) (p))
#define PUT(p, val) (*(size_t *) (p) = (val))

// Return pointer to payload section for an allocated block
#define GET_PAYLOAD_PTR(blkptr) ((char *)blkptr + sizeof(size_t))

// Return pointer to header for a block given pointer to payload
#define GET_HEADER(ptr) ((char *) ptr - sizeof(size_t));

//Return size of block giving pointer to header/footer
#define GET_SIZE(ptr) (GET(ptr) & ~0x7)

//Return allocation bit given pointer to header/footer
#define GET_ALLOC(ptr) (GET(ptr) & 0x1)

// Compute address of footer, given pointer to a block
#define FTRP(headerptr) ((char *)headerptr + sizeof(size_t) + GET_SIZE(headerptr))

//Return address for left block,  given ptr to a  block's header
#define LEFT_BLK(headerptr) ( (char*)headerptr - sizeof(size_t) * 2 - GET_SIZE((char*)headerptr - sizeof(size_t)))

//Return address for right  block  given ptr to a block's header
#define RIGHT_BLK(headerptr) ((char*) headerptr + sizeof(size_t) * 2 + GET_SIZE(headerptr))

#define REALLOC_FACTOR 1.5 //

/*------------------------------------------------------------------*/

// Function declaration
freeBlock * coalesce(freeBlock * blkptr);

/*Global varibles*/
/*------------------------------------------------------------------*/
void *heap_listp; // points to start of most recent allocated page
size_t pagecnt; // record how many pages are currently allocated
int validatecnt;// how many times validat_heap() is called
freeBlock *freelist_arr[NUM_BUCKETS];

/*Inline functions*/
/*------------------------------------------------------------------*/

// Very efficient bitwise round of sz up to nearest multiple of mult
static inline size_t roundup(size_t sz, size_t mult)
{
    return (sz + mult-1) & ~(mult-1);
}

// Calculate log_2(num)
static inline size_t log_2(size_t num){
    size_t res = 0;
    while (num > 0){
        num >>= 1;
        res++;
    }
    return res;
}

// Set a block header and footer with payloadsz and allocation
static inline void set_block(void *blkptr, size_t payloadsz, size_t alloc){
    size_t headerval =  PACK(payloadsz, alloc);
    PUT(blkptr, headerval);
    PUT(FTRP(blkptr), headerval);
}

/*
 * Function: insert_node
 * -----------------------------------------------------------------------------
 * Given pointer to a freed block. Find correct size freelist to insert to
 * then insert to beginning. 
 *
 * bucket idx == 0, contain free block size up to 8
 * bucket idx == 1, contain free block (8, 16]
 * bucket idx == 2, contains free block size up to(16, 32]
 *
 * payload range each idx correspond to is (8*2^(idx - 1), 8*2^idx]
 *
 * */
void insert_node(freeBlock *blkptr){
    size_t bucketidx = MIN((log_2(GET_SIZE(blkptr)/ALIGNMENT)), NUM_BUCKETS - 1);
    freeBlock *head = freelist_arr[bucketidx];

    blkptr->prev = NULL;
    blkptr->next = head;
    if (head != NULL) head->prev = blkptr;
    freelist_arr[bucketidx] = blkptr;

    // coalsece newlly inserted node
    coalesce(blkptr);
}

/* Function: add_page
 * ----------------------------------------------------------------------------
 * Request page from OS for requested asize. Set up or update prologue and 
 * epilogue for the heap. Requested page will be setup as a giant free block
 * If request page is not initial page, will call coalesce to prime heap.
 */
bool add_page(size_t asize){
    freeBlock * bigpageblk;
    size_t numofpage = pagecnt/10;// Request some extra pages to reduce time
    size_t payloadsz;

    if (pagecnt == 0){ // initial page request
        numofpage  += (asize/(PAGE_SIZE - sizeof(prologue) - sizeof(epilogue) - MINBLKSZ)) + 1; 
        // denominator of first operatnd is the effective area for allocation when add page for the first time
    }else{
        numofpage  += (asize/(PAGE_SIZE -  MINBLKSZ)) + 1; 
        // denominator of first operatnd is the effective area for allocation  when adding addtioanl page
    }

    heap_listp = extend_heap_segment(numofpage);
    if (heap_listp == NULL) return false;  

    // Update epilogue
    epilogue *epi = (epilogue *) ((char *)heap_listp + numofpage * PAGE_SIZE - sizeof(epilogue));
    epi->header = PACK(0, 1); 

    if (pagecnt == 0){//initial page request
        //set up prologue 
        prologue *pro = heap_listp;
        pro->header = PACK(0, 1);
        pro->footer = PACK(0, 1);

        //newly allocated page is a giant freeBlock. set it and add to freelistp
        bigpageblk = (freeBlock *) ((char *)heap_listp + sizeof(prologue));
        payloadsz = numofpage * PAGE_SIZE - sizeof(prologue) - sizeof(epilogue) - sizeof(size_t) * 2;


        set_block(bigpageblk, payloadsz, 0);
        insert_node(bigpageblk);
    }else{
        bigpageblk = (freeBlock *) ((char *)heap_listp - sizeof(epilogue)); // freedblock starts at epilogue of previous add_page
        payloadsz =  (char *) epi - (char*) bigpageblk - sizeof(size_t) * 2;

        set_block(bigpageblk, payloadsz, 0);
        insert_node(bigpageblk);
    }

    pagecnt += numofpage;
    return true;
}

/* Function:myinit 
 * -----------------------------------------------------------------------------
 * The responsibility of the myinit function is to configure a new
 * empty heap. Typically this function will initialize the
 * segment (you decide the initial number pages to set aside, can be
 * zero if you intend to defer until first request) and set up the
 * global variables for the empty, ready-to-go state. The myinit
 * function is called once at program start, before any allocation 
 * requests are made. It may also be called later to wipe out the current
 * heap contents and start over fresh. This "reset" option is specifically
 * needed by the test harness to run a sequence of scripts, one after another,
 * without restarting program from scratch.
 */
bool myinit()
{
    heap_listp= init_heap_segment(0); 
    if (heap_listp == NULL) return false; // allocation failure 

    //Initialize global variables
    pagecnt = 0;
    validatecnt = 0;

    //initialize freelist_arr elements to NULL
    for (int i = 0; i < NUM_BUCKETS; i++){
        freelist_arr[i] = NULL; 
    }
    return true;
}



/* Function:find_fit 
 * -----------------------------------------------------------------------------
 * First fit to find free block with size >= asize. Search in corresponding free
 * list. If not, search innext larger free list.
 */
freeBlock *find_fit(size_t asize){
    int bucketidx = MIN((log_2(asize/ALIGNMENT)), NUM_BUCKETS - 1);
    freeBlock *blkptr; 

    for (size_t i = bucketidx; i < NUM_BUCKETS;i++){
        for (blkptr = freelist_arr[i]; blkptr != NULL; blkptr  = blkptr->next){
            if(asize <=  GET_SIZE(blkptr)){
                return blkptr;       
            }
        }
    }
    return NULL;
}


/* Fucntion: delete_node
 * -----------------------------------------------------------------------------
 * Give pointer to a freeBlock, delete that node from corresponidng freelist
 */
void delete_node(freeBlock *ptr){
    if (ptr == NULL) return;
    size_t bucketidx = MIN(log_2(GET_SIZE(ptr)/ALIGNMENT), NUM_BUCKETS - 1);

    if (ptr->prev == NULL){ // case1: node is at beginning
        freelist_arr[bucketidx]= ptr->next; // update head
        if (freelist_arr[bucketidx]!= NULL) freelist_arr[bucketidx]->prev = NULL; 
        // initialize head's prev to null
    }else if (ptr->next == NULL){//case 2 node is at the end
        ptr->prev->next = NULL;
    }else{//case3: node is in the middle
        ptr->prev->next = ptr->next;
        ptr->next->prev = ptr->prev;

    }
}


/* Fucntion: place
 * -----------------------------------------------------------------------------
 * Allocate a block with asize. Update its header and footer
 * Split free blockif neccessary.
 */
void place(freeBlock *blkptr, size_t asize){
    size_t csize = GET_SIZE(blkptr);

    if (csize - asize < MINBLKSZ){  
        set_block(blkptr, csize, 1);
        delete_node(blkptr);
    }else{ // if available free blk is big enough to be splitted 
        delete_node(blkptr);

        //allocate with asize
        set_block(blkptr, asize, 1);

        // Find starting address for remaining block and payload
        freeBlock *restblk = (freeBlock *) ((char*) blkptr + sizeof(size_t) * 2 + asize);
        size_t restsz = csize - asize - 2 * sizeof(size_t);
        set_block(restblk, restsz, 0);
        insert_node(restblk);
    }
}


/* Function:mymalloc
 * -----------------------------------------------------------------------------
 * Rounde up requestedsz to ensure 8 byes alignment then allocate in heap. 
 * Return a pointer to the allocated memory location.
 */
void *mymalloc(size_t requestedsz)
{
    size_t asize; //adjusted block size to ensure 8 bytes alignment
    freeBlock *blkptr; //pointer to allocated block

    // check for corner cases:
    if (requestedsz <= 0 || requestedsz > INT_MAX) return NULL;

    // Payload need to contain asize+ prev pointer + next pointer
    // otherwise, when free, no place to contain prev or next!
    asize = sizeof(size_t) * 2 + roundup(requestedsz, ALIGNMENT); 

    // blkptr = find_fit(asize);
    blkptr = find_fit(asize);
    if (blkptr != NULL){ // fit found and place it 
        place(blkptr, asize);
    }else{// no fit found
        if (add_page(asize)){
            //blkptr = find_fit(asize);
            blkptr = find_fit(asize);
            place(blkptr, asize);
        }else{
            return NULL; 
            // Heap reaches max. cant request more pages
        };

    }
    return GET_PAYLOAD_PTR(blkptr);
}


/*
 * Function: coalesce
 * -----------------------------------------------------------------------------
 * Given pointer to a free block. Try to merge with adjacent blocks
 * Will update freelist after coalesce
 *
 * Return the pointer to coalesced free block
 * */

freeBlock * coalesce(freeBlock * blkptr){

    size_t prev_alloc = GET_ALLOC(LEFT_BLK((blkptr)));
    size_t next_alloc = GET_ALLOC(RIGHT_BLK(blkptr));
    size_t payloadsz = GET_SIZE(blkptr);

    if (prev_alloc && next_alloc) return blkptr; //both left and right blocks are allocated

    if (prev_alloc && !next_alloc){ //merge with right block
        freeBlock * rightblkptr = (freeBlock *) RIGHT_BLK(blkptr);
        payloadsz += GET_SIZE(rightblkptr) + sizeof(size_t) * 2; //addition 16 bytes from merged header and footer

        delete_node(blkptr);
        delete_node(rightblkptr);

        set_block(blkptr, payloadsz, 0);

        insert_node(blkptr);
        return blkptr;
    }

    if (!prev_alloc && next_alloc){ //merge with left block
        freeBlock *leftblkptr = (freeBlock *) LEFT_BLK(blkptr);
        payloadsz += GET_SIZE(leftblkptr) + sizeof(size_t) * 2;//addition 16 bytes from merged header and footer

        delete_node(blkptr);
        delete_node(leftblkptr);


        set_block(leftblkptr, payloadsz, 0);
        insert_node(leftblkptr);
        return leftblkptr;
    }

    if (!prev_alloc && !next_alloc){ //merge with left and right block
        freeBlock *leftblkptr = (freeBlock *) LEFT_BLK(blkptr);
        freeBlock *rightblkptr = (freeBlock *) RIGHT_BLK(blkptr);

        payloadsz += GET_SIZE(leftblkptr) + GET_SIZE(rightblkptr) + sizeof(size_t) * 4;// additional bytes from merged headers and footers

        delete_node(blkptr);
        delete_node(leftblkptr);
        delete_node(rightblkptr);

        set_block(leftblkptr, payloadsz,0);
        insert_node(leftblkptr);
        return leftblkptr;
    }
    return NULL;

}

/*
 * Function: myfree
 * -----------------------------------------------------------------------------
 * Given pointer to payload of previously allocated block deallocat that block.
 *
 * Invalid requests like frees an already freed ptr, overruns past the end of 
 * an allocated block or other incorrect  usage. free will do nothing.
 */
void myfree(void *ptr)
{
    // Do nothing when ptr == NULL
    if (ptr == NULL) return;

    // Unpack
    allocatedBlock* blkptr = (allocatedBlock *)GET_HEADER(ptr);
    size_t payloadsz = GET_SIZE(blkptr);

    set_block(blkptr, payloadsz,0);

    // insert_node
    insert_node((freeBlock *)blkptr);
}


/*
 * Function: myrealloc
 * -----------------------------------------------------------------------------
 * Given pointer to payload of previously allocated block and new size.Change
 * the size of memory blok to new size. The content will be unchanged.
 * Return pointer to memory location
 */
void *myrealloc(void *oldptr, size_t newsz)
{
    // corner cases
    if (newsz <= 0 ) return NULL;
    if (oldptr == NULL) return mymalloc(newsz);
    if (newsz == 0) myfree(oldptr);

    int asize = sizeof(size_t) * 2 + roundup(newsz, 8);
    allocatedBlock * oldblk = (allocatedBlock *) ( (char *)oldptr - sizeof(size_t)); 
    int oldsize = GET_SIZE(oldblk);
    if (oldsize >= asize) return oldptr; // original block is big enough for relloc

    char buffer[oldsize]; // store old date in a buffer to prevent overwritten
    memcpy(buffer, oldptr, oldsize);
    myfree(oldptr); //free oldptr

    // Malloc to new location and copy original content
    void *newptr = mymalloc(newsz * REALLOC_FACTOR);
    memcpy(newptr, buffer, MIN(oldsize, asize));
    return newptr;
}


/*
 * Function: validate_addr
 * -----------------------------------------------------------------------------
 * validate if block pointer is within heap. used for heap_validate
 * Used for validate_heap
 */
bool validate_addr(freeBlock *ptr){
    void *blkptr = (void *) ptr;
    if (blkptr == NULL) return true;
    return blkptr > (void *)0x1070000000 && blkptr < (void *)(0x1070000000+ PAGE_SIZE * pagecnt);

}

/*
 * Function: check_freelist
 * -----------------------------------------------------------------------------
 * Iterate through non null freelist and print valudes. Used for validate_heap
 * Used for validate_heap
 */

bool check_freelist(){
    size_t bucketidx = 0;
    freeBlock *cur; 
    int invalidcnt = 0;

    printf("now checking  all free lists\n");
    printf("---------------------------------------\n");
    for (size_t i = bucketidx; i < NUM_BUCKETS;i++){

        printf("\t now checking  bucketidx %zu, list head is %p\n", i,(void *) freelist_arr[i]);
        for (cur = freelist_arr[i]; cur != NULL; cur = cur->next){

            if (!validate_addr(cur)){
                printf("\tinvalid cur addr: %p\n", (void *)cur);
                invalidcnt++;
            }
            if (!validate_addr(cur->next)){
                printf("\tinvalid cur->next addr: %p\n", (void *)cur->next);
                invalidcnt++;
            }
            if (!validate_addr(cur->prev)){
                printf("\tinvalid cur->prev addr: %p\n", (void *)cur->prev);
                invalidcnt++;
            }

            if (GET(cur) <= 0 || GET(cur) / 2 == 1 || GET(cur) > INT_MAX){
                printf("\tinvalid header: %zu\n", cur->header);
                invalidcnt++;
            }
            if (GET(cur) != GET(FTRP(cur))){
                printf("\theader footer not equal!");
                invalidcnt++;
            }
        }
    }
    if (invalidcnt == 0){
        printf("freelist valid !\n");
    }else{
        printf("**********OMG freelist invalid ********** !\n");
    }

    return invalidcnt == 0;
}


/*
 * Function: print_freelist
 * -----------------------------------------------------------------------------
 * Iterate through non null freelist and print valudes. Used for validate_heap
 */
void print_freelist(){
    size_t bucketidx = 0;
    freeBlock *cur; 

    printf("now printing all free lists\n");
    printf("---------------------------------------\n");
    for (size_t i = bucketidx; i < NUM_BUCKETS;i++){
        printf("\t now print bucketidx %zu, list head is %p\n", i,(void *) freelist_arr[i]);
        for (cur = freelist_arr[i]; cur != NULL; cur = cur->next){
            printf("\t\tcur: %p, cur->header: %zu, cur->prev: %p, cur->next: %p\n",
                    (void *)cur, cur->header, (void *)cur->prev, (void *) cur->next);

        }
    }

    printf("---------------------------------------\n");
}


/*
 * Function:validate_heap 
 * -----------------------------------------------------------------------------
 * My heap validator for debugging 
 */
bool validate_heap()
{
    validatecnt++;
    print_freelist();
    return check_freelist();
}

