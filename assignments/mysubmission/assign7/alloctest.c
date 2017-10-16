/*
 * Files: alloctest.c
 * ------------------
 * Reads and interprets text-based script files containing a sequence of
 * allocator requests. Runs the allocator on the script, validating for
 * for correctness and then evaulating allocator's utilization and throughput.
 *
 * jzelenski, updated Wed Nov 19 14:45:18 PST 2014
 */

#define _GNU_SOURCE
#include <dirent.h>
#include <limits.h>
#include <stdio.h>
#include <errno.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <valgrind/callgrind.h>

#include "allocator.h"
#include "fcyc.h"
#include "segment.h"

// Alignment requirement
#define ALIGNMENT 8

// Returns true if p is ALIGNMENT-byte aligned
#define IS_ALIGNED(p)  ((((uintptr_t)p) % ALIGNMENT) == 0)

// default path for test scripts
#define DEFAULT_SCRIPT_DIR "/afs/ir/class/cs107/samples/assign7/"
#define MAX_SCRIPTS 100

// Throughput (in Kreq/sec) of the reference malloc on standard myth on samples
// This constant is the stable target allocator throughput is ranked against
#define TARGET_THRUPUT      12000

// struct for a single allocator request
typedef struct {
    enum {ALLOC=1, FREE, REALLOC} op;	// type of request
    int id;		        // id for free() to use later
    size_t size;        // num bytes for alloc/realloc request
    int lineno;         // which line in file
} request_t;

// struct for facts about a single malloc'ed node
typedef struct {
    void *ptr;
    size_t size;
} block_t;

// struct for info for one script file
typedef struct {
    char name[128];		// short name of script
    request_t *ops;	    // array of requests read from script
    int num_ops;		// number of requests
    int num_ids;		// number of distinct block ids
    block_t *blocks;    // array of blocks returned by malloc when executing
} script_t;

// packs the params to the speed function to be timed by fcyc.
// timed function must take single void * client data pointer
typedef struct {
    script_t *script;
    double *utilization;
} perfdata_t;

// Result from executing a script
typedef struct {
    char name[128];     // short name of script
    int num_ops;		// number of ops (malloc/free/realloc) in script
    bool valid;		    // was the script processed correctly by the allocator?
    double secs;		// number of secs needed to execute the script
    double utilization;	// mem utilization  (percent of heap storage in use)
    int tput;           // expressed in Kreq/sec
} result_t;

typedef enum { Correctness = 1, Performance = 2 } flags_t;

static void get_scripts(char *path, char files[][PATH_MAX], int max, int *pcount);
static void parse_script(char *filename, script_t *script);
static void run_scripts(char paths[][PATH_MAX], int n, flags_t flags);
static bool eval_correctness(script_t *script);
static void eval_performance(void *data);
static bool verify_block(void *ptr, size_t size, script_t *script, int lineno);
static bool verify_payload(void *ptr, size_t size, int id, script_t *script, int lineno, char *op);
static void print_table(result_t result[], int n, flags_t which);
static void usage();
static void fatal_error(char *format, ...);
static void allocator_error(script_t *script, int lineno, char* format, ...);
static const char *mybasename(const char *path);
static int cmpbase(const void *one, const void *two);
static char *endswith(char *str, const char *suffix);


int main(int argc, char *argv[])
{
    char paths[MAX_SCRIPTS][PATH_MAX];
    flags_t flags = Correctness | Performance; // default is to test both
    char c;
    int nscripts = 0;

    CALLGRIND_TOGGLE_COLLECT ;// turn off profiling while we do the setup work, later turn on during simulation
    while ((c = getopt(argc, argv, "f:pc")) != EOF) {
        switch (c) {
            case 'f':
                get_scripts(optarg, paths, sizeof(paths)/sizeof(paths[0]), &nscripts);
                break;
            case 'p':
                flags = Performance;
                break;
            case 'c':
                flags = Correctness;
                break;
            default:
                usage();
        }
    }
    if (optind < argc) usage();
    if (nscripts == 0)
        get_scripts(DEFAULT_SCRIPT_DIR, paths, sizeof(paths)/sizeof(paths[0]), &nscripts);
    qsort(paths, nscripts, sizeof(paths[0]), cmpbase); // sort by filename
    setvbuf(stdout, NULL, _IONBF, 0); // disable stdout buffering, all printfs display to terminal immediately
    run_scripts(paths, nscripts, flags);
    return 0;
}


/* Function: get_scripts
 * ---------------------
 * Given a path, determine if file or directory, and adds script file(s) to the array.
 */
static void get_scripts(char *path, char files[][PATH_MAX], int max, int *pcount)
{
    struct stat st;
    if (stat(path, &st) != 0)
        fatal_error("Could not open script file \"%s\".\n", path);
    if (!S_ISDIR(st.st_mode)) {  // path is single file, not directory
        strcpy(files[(*pcount)++], path);
        return;
    }

    DIR *dirp = opendir(path);
    if (!dirp)
        fatal_error("Could not open directory \"%s\".\n", path);
    printf("Reading scripts from %s...", path);
    struct dirent *dp;
    int before = *pcount;
    while ((dp = readdir(dirp)) != NULL && *pcount < max) { // read files from dir one-by-one
        // take all non-hidden files with .script extension
        if (dp->d_name[0] != '.' && endswith(dp->d_name,".script"))
            sprintf(files[(*pcount)++], "%s/%s", path, dp->d_name);
    }
    closedir(dirp);
    printf(" found %d.\n", *pcount - before);
}


/* Function: run_scripts
 * ---------------------
 * Runs a set of scripts against the allocator.  It loops script-by-script.
 * For each script, runs once for correctness (unless flags are perf only)
 * and if had no correctness errors, runs a performance trial on the same script.
 * Records results into an array, which is printed at end.
 */
static void run_scripts(char paths[][PATH_MAX], int n, flags_t which)
{
    result_t result[n];

    for (int i = 0; i < n; i++) {
        script_t script;
        parse_script(paths[i], &script);
        strcpy(result[i].name, script.name);
        result[i].num_ops = script.num_ops;
        printf("Evaluating allocator on %s....", script.name);
        result[i].valid = !(which & Correctness) || eval_correctness(&script);
        if (result[i].valid && (which & Performance)) {
            perfdata_t pd = {.script = &script, .utilization = &result[i].utilization};
            result[i].secs = fsecs(eval_performance, &pd);
            result[i].tput = result[i].num_ops/(result[i].secs*1e3);
        } else {
            result[i].secs = result[i].utilization = 0;
        }
        printf("done.\n");
        free(script.ops);
        free(script.blocks);
    }
    print_table(result, n, which); // display results
}


/* Function: read_line
 * --------------------
 * Reads one line from file and stores in buf. Skips lines that are all-white or beginning with
 * comment char #. Increments pass-by-ref counter of number of lines read/skipped. Removes
 * trailing newline. Returns true if did read valid line, false otherwise.
 */
static bool read_line(char buf[], size_t bufsz, FILE *fp, int *pnread)
{
    while (true) {
        if (fgets(buf, bufsz, fp) == NULL) return false;
        (*pnread)++;
        if (buf[strlen(buf)-1] == '\n') buf[strlen(buf)-1] ='\0'; // remove trailing newline
        char ch;
        if (sscanf(buf, " %c", &ch) == 1 && ch != '#') // scan first non-white char, check not #
            return true;
    }
}


/*
 * Fuction: parse_script
 * ---------------------
 * Parse a script file and store sequence of requests for later execution.
 */
static void parse_script(char *path, script_t *script)
{
    FILE *fp = fopen(path, "r");
    if (fp == NULL)
        fatal_error("Could not open script file \"%s\".\n", path);

    strncpy(script->name, mybasename(path), sizeof(script->name)-1); // copy basename (up to limit)
    script->name[sizeof(script->name)-1] = '\0';    // null terminate
    char *ext = endswith(script->name, ".script");   // truncate file extension
    if (ext) *ext = '\0';
    script->ops = NULL;
    script->num_ops = 0;
    int lineno = 0, nallocated = 0, maxid = 0;
    char buf[1024];

    for (int i = 0; read_line(buf, sizeof(buf), fp, &lineno) ; i++) {
        if (i == nallocated) {
            nallocated += 500;
            script->ops = realloc(script->ops, nallocated*sizeof(request_t));
            if (!script->ops)
                fatal_error("Libc heap exhausted. Cannot continue.\n");
        }
        script->ops[i].lineno = lineno;
        char request;
        script->ops[i].op = script->ops[i].size = 0;
        int nscanned = sscanf(buf, " %c %d %zu", &request, &script->ops[i].id, &script->ops[i].size);
        if (request == 'a' && nscanned == 3)
            script->ops[i].op = ALLOC;
        else if (request == 'r' && nscanned == 3)
            script->ops[i].op = REALLOC;
        else if (request == 'f' && nscanned == 2)
            script->ops[i].op = FREE;
        if (!script->ops[i].op || script->ops[i].id < 0 || script->ops[i].size > INT_MAX)
            fatal_error("Malformed request '%s' line %d of %s\n", buf, lineno, script->name);
        if (script->ops[i].id > maxid) maxid = script->ops[i].id;
        script->num_ops = i+1;
    }
    fclose(fp);

    script->num_ids = maxid + 1;
    script->blocks = calloc(script->num_ids, sizeof(block_t));
    if (!script->blocks)
        fatal_error("Libc heap exhausted. Cannot continue.\n");
}


/* Function: eval_correctness
 * --------------------------
 * Check the allocator for correctness on given script. Interprets the earlier parsed
 * script operation-by-operation and reports if it detects any "obvious"
 * errors (returning blocks outside the heap, unaligned, overlapping blocks, etc.)
 */
static bool eval_correctness(script_t *script)
{
    if (!myinit()) {
        allocator_error(script, 0, "myinit() returned false");
        return false;
    }
    if (!validate_heap()) { // check heap consistency after init
        allocator_error(script, 0, "validate_heap() returned false, called after myinit");
        return false;
    }
    memset(script->blocks, 0, script->num_ids*sizeof(script->blocks[0]));

    for (int req = 0; req < script->num_ops; req++) {
        int id = script->ops[req].id;
        size_t requested_size = script->ops[req].size;
        size_t old_size = script->blocks[id].size;
        void *p, *newp, *oldp = script->blocks[id].ptr;

        switch (script->ops[req].op) {

            case ALLOC:
                if ((p = mymalloc(requested_size)) == NULL && requested_size != 0) {
                    allocator_error(script, script->ops[req].lineno, "malloc returned NULL");
                    return false;
                }
                // Test new block for correctness: must be properly aligned
                // and must not overlap any currently allocated block.
                if (!verify_block(p, requested_size, script, script->ops[req].lineno))
                    return false;

                // Fill new block with the low-order byte of new id
                // can be used later to verify data copied when realloc'ing
                memset(p, id & 0xFF, requested_size);
                script->blocks[id] = (block_t){.ptr = p, .size = requested_size};
                break;

            case REALLOC:
                if (!verify_payload(oldp, old_size, id, script, script->ops[req].lineno, "realloc-ing"))
                    return false;
                if ((newp = myrealloc(oldp, requested_size)) == NULL && requested_size != 0) {
                    allocator_error(script, script->ops[req].lineno, "realloc returned NULL");
                    return false;
                }

                old_size = script->blocks[id].size;
                script->blocks[id].size = 0;
                if (!verify_block(newp, requested_size, script, script->ops[req].lineno))
                    return false;
                // Verify new block contains the data from the old block
                for (size_t j = 0; j < (old_size < requested_size ? old_size : requested_size); j++) {
                    if (*((unsigned char *)newp + j) != (id & 0xFF)) {
                        allocator_error(script, script->ops[req].lineno, "realloc did not preserve the data from old block");
                        return false;
                    }
                }
                // Fill new block with the low-order byte of new id
                memset(newp, id & 0xFF, requested_size);
                script->blocks[id] = (block_t){.ptr = newp, .size = requested_size};
                break;

            case FREE:
                old_size = script->blocks[id].size;
                p = script->blocks[id].ptr;
                // verify payload intact before free
                if (!verify_payload(p, old_size, id, script, script->ops[req].lineno, "freeing"))
                    return false;
                script->blocks[id] = (block_t){.ptr = NULL, .size = 0};
                myfree(p);
                break;
        }

        if (!validate_heap()) { // check heap consistency after each request
            allocator_error(script, script->ops[req].lineno, "validate_heap() returned false, called in-between requests");
            return false;   // stop at first sign of error
        }
    }

    // verify payload is still intact for any block still allocated
    for (int id = 0;  id < script->num_ids;  id++)
        if (!verify_payload(script->blocks[id].ptr, script->blocks[id].size, id, script, -1, "at exit"))
            return false;
    return true;
}


/* Function: eval_performance
 * --------------------------
 * This is almost same code as function above, but unifying the two clutters
 * the code path adds performance penalties to the time trial, which needed to
 * be avoided. This interprets the script with no additional overhead
 * (e.g. no checking for validity), measures time, and tracks the high water mark
 * of the heap segment to report on memory utilization.  The function
 * takes a void* client pointer, since that is what is required to work
 * with the timing trial code. This client data provides the script to exeucute.
 */
static void eval_performance(void *data)
{
    perfdata_t *pd = (perfdata_t *)data;
    size_t peak_payload_size = 0, cur_payload_size = 0, max_segment_size = 0;
    script_t *script = pd->script;

    myinit();
    memset(script->blocks, 0, script->num_ids*sizeof(script->blocks[0]));

    CALLGRIND_TOGGLE_COLLECT;	// turn on valgrind profiler here
    for (int line = 0; line < script->num_ops;  line++) {
        int id = script->ops[line].id;
        size_t requested_size = script->ops[line].size;

        switch (script->ops[line].op) {

            case ALLOC:
                script->blocks[id].ptr = mymalloc(requested_size);
                script->blocks[id].size = requested_size;
                cur_payload_size += requested_size;
                if (requested_size) ((char *)script->blocks[id].ptr)[0] = ((char *)script->blocks[id].ptr)[requested_size-1] = 0xab;
                break;

            case REALLOC:
                script->blocks[id].ptr = myrealloc(script->blocks[id].ptr, requested_size);
                cur_payload_size += (requested_size - script->blocks[id].size);
                script->blocks[id].size = requested_size;
                if (requested_size) ((char *)script->blocks[id].ptr)[0] = ((char *)script->blocks[id].ptr)[requested_size-1] = 0xcd;
                break;

            case FREE:
                myfree(script->blocks[id].ptr);
                cur_payload_size -= script->blocks[id].size;
                script->blocks[id] = (block_t){.ptr = NULL, .size = 0};
                break;
        }

        // peak util is ratio of inuse/segment, reset when either changes (numerator or denom)
        if (heap_segment_size() > max_segment_size || (cur_payload_size > peak_payload_size) ) {
            max_segment_size = heap_segment_size();
            peak_payload_size = cur_payload_size;
        } 
     }
 
    *pd->utilization = ((double)peak_payload_size)/max_segment_size;
    CALLGRIND_TOGGLE_COLLECT;  // turn off profiler here
}



/* Function: verify_block
 * ----------------------
 * Does some simple checks on the block returned by allocator to try to
 * verify correctness.  If any problem shows up, reports an allocator error
 * with details and line from script file. The checks it performs are:
 *  -- verify block address is correctly aligned
 *  -- verify block address is within heap segment
 *  -- verify block address + size doesn't overlap any existing allocated block
 */
static bool verify_block(void *ptr, size_t size, script_t *script, int lineno)
{
    // address must be ALIGNMENT-byte aligned
    if (!IS_ALIGNED(ptr)) {
        allocator_error(script, lineno, "New block (%p) not aligned to %d bytes",
                        ptr, ALIGNMENT);
        return false;
    }
    if (ptr == NULL && size == 0) return true;

    // block must lie within the extent of the heap
    void *end = (char *)ptr + size;
    void *heap_end = (char *)heap_segment_start() + heap_segment_size();
    if (ptr < heap_segment_start() || end > heap_end) {
        allocator_error(script, lineno, "New block (%p:%p) not within heap segment (%p:%p)",
                        ptr, end, heap_segment_start(), heap_end);
        return false;
    }
    // block must not overlap any other blocks
    for (int i = 0; i < script->num_ids; i++) {
        if (script->blocks[i].ptr == NULL || script->blocks[i].size == 0) continue;
        void *other_start = script->blocks[i].ptr;
        void *other_end = (char *)other_start + script->blocks[i].size;
        if ((ptr >= other_start && ptr < other_end) || (end > other_start && end < other_end) ||
            (ptr < other_start && end >= other_end)){
            allocator_error(script, lineno, "New block (%p:%p) overlaps existing block (%p:%p)",
                            ptr, end, other_start, other_end);
            return false;
        }
    }
    return true;
}


/* Function: verify_payload
 * ------------------------
 * When a block is allocated, the payload is filled with a simple repeating pattern
 * Later when realloc'ing or freeing that block, check the payload to verify those
 * contents are still intact, otherwise raise allocator error.
 */
static bool verify_payload(void *ptr, size_t size, int id, script_t *script, int lineno, char *op)
{
    for (size_t i = 0; i < size; i++) {
        if (*((unsigned char *)ptr + i) != (id & 0xFF)) {
            allocator_error(script, lineno, "invalid payload data detected when %s address %p", op, ptr);
            return false;
        }
    }
    return true;
}

/* Function: print_result
 * ----------------------
 * Prints row in result table with name of script and result of execution.
 */
static void print_result(result_t *st, flags_t which, bool is_total)
{
    printf("%-20s %-7s ", st->name, !is_total && (which & Correctness) ? (st->valid ? "Y" : "N") : "" );
    if (st->valid && (which & Performance))
        printf("%7.0f%% %12d %14.6f %10d", st->utilization*100, st->num_ops, st->secs, st->tput);
    else
        printf("%7s %12s %14s %10s","-","-","-","-");
    printf("\n");
}


/* Function: print_table
 * ---------------------
 * This prints table of individual script results, displays overall
 * average utilization and throughput.
 */
static void print_table(result_t result[], int n, flags_t which)
{
    char *dashes = "-------------------------------------------------------------------------------";
    result_t total = {.name = "Aggregate", .valid = true, .num_ops = 0, .secs = 0, .utilization = 0};
    int failures = 0;

    // Print the individual results for each script
    printf("\n script name     correct?    utilization    requests       secs       Kreq/sec\n%s\n", dashes);
    for (int i = 0; i < n; i++) {
        print_result(&result[i], which, false);
        if (!result[i].valid)
            failures++;
        else {
            total.secs += result[i].secs;
            total.num_ops += result[i].num_ops;
            total.utilization += result[i].utilization;
            total.tput += result[i].tput;
        }
    }
    printf("%s\n\n", dashes);
    sprintf(total.name, "%s %d of %d", "Aggregate" , n-failures, n);

    // Print the aggregate results for all scripts
    total.valid = (failures != n);  // if at least one script succeeded, total has some validity
    total.utilization /= n;
    total.tput /= n;
    print_result(&total, which, true);
    double rel_tput = (double)total.tput/TARGET_THRUPUT;
    if (which & Performance)
        printf("\t%.0f%% (utilization) %.0f%% (throughput, expressed relative to target %d Kreq/sec)\n",total.utilization*100, rel_tput*100, TARGET_THRUPUT);
    if (failures != 0)
        printf("%d script%s exited with correctness errors.\n", failures, (failures > 1 ? "s" : ""));
    printf("\n");
}

// minor path/string handling helpers
static char *endswith(char *str, const char *suffix) {
    char *tail = str + strlen(str) - strlen(suffix);
    return strlen(str) >= strlen(suffix) && strcmp(tail, suffix) == 0 ? tail : NULL;
}
static const char *mybasename(const char *path) { return strrchr(path, '/') ? strrchr(path, '/') + 1 : path; }
static int cmpbase(const void *a, const void *b) { return strcmp(mybasename(a), mybasename(b)); }

// fatal_error - Report an error and exit
static void fatal_error(char *format, ...)
{
    fprintf(stdout, "\nFATAL ERROR: ");
    va_list args;
    va_start(args, format);
    vfprintf(stdout, format, args);
    va_end(args);
    exit(107);
}

// Report errors from invoking student's allocator functions (non-fatal)
static void allocator_error(script_t *script, int lineno, char* format, ...)
{
    va_list args;
    fprintf(stdout, "\nALLOCATOR ERROR [%s, line %d]: ", script->name, lineno);
    va_start(args, format);
    vfprintf(stdout, format, args);
    va_end(args);
    fprintf(stdout,"\n");
}

static void usage()
{
   fprintf(stderr, "Usage: %s [-f <file-or-dir>]\n", program_invocation_short_name);
   fprintf(stderr, "\t-c                Run only the correctness tests (no checks for performance).\n");
   fprintf(stderr, "\t-p                Run only the performance tests (no checks for correctness).\n");
   fprintf(stderr, "\t-f <file-or-dir>  Use <file> as script or read all script files from <dir>.\n");
   fprintf(stderr, "Without -f option, reads scripts from default path: %s\n", DEFAULT_SCRIPT_DIR);
   exit(107);
}
