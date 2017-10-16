/* File: maptest.c
* ----------------
* A small program to exercise some basic functionality of the CMap. You should
* supplement with additional tests of your own.  You may change/extend/replace
* this test program in any way you see fit.
* jzelenski
*/

#include "cmap.h"
#include <assert.h>
#include <ctype.h>
#include <error.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/* Function: verify_int
* ---------------------
* Used to compare a given result with what was expected and report on whether
* passed/failed.
*/
static void verify_int(int expected, int found, char *msg)
{
    printf("%s expect: %d found: %d. %s\n", msg, expected, found,
        (expected == found) ? "Seems ok." : "##### PROBLEM HERE #####");
}

static void verify_ptr(void *expected, void *found, char *msg)
{
    printf("%s expect: %p found: %p. %s\n", msg, expected, found,
        (expected == found) ? "Seems ok." : "##### PROBLEM HERE #####");
}

static void verify_int_ptr(int expected, int *found, char *msg)
{
    if (found == NULL)
        printf("%s found: %p %s\n", msg, found, "##### PROBLEM HERE #####");
    else
        verify_int(expected, *found, msg);
}


void simple_cmap()
{
    char *words[] = {"apple", "pear", "banana", "cherry", "kiwi", "melon", "grape", "plum"};
    char *extra = "strawberry";
    int len, nwords = sizeof(words)/sizeof(words[0]);
    CMap *cm = cmap_create(sizeof(int), nwords, NULL);

    printf("\n----------------- Testing simple cmap ------------------ \n");
    printf("Created empty CMap.\n");
    verify_int(0, cmap_count(cm), "cmap_count");
    verify_ptr(NULL, cmap_get(cm, "nonexistent"), "cmap_get(\"nonexistent\")");

    printf("\nAdding %d keys to CMap.\n", nwords);
    for (int i = 0; i < nwords; i++) {
        len = strlen(words[i]);
        cmap_put(cm, words[i], &len); // associate word w/ its strlen
    }
    verify_int(nwords, cmap_count(cm), "cmap_count");
    verify_int_ptr(strlen(words[0]), cmap_get(cm, words[0]), "cmap_get(\"apple\")");

    printf("\nAdd one more key to CMap.\n");
    len = strlen(extra);
    cmap_put(cm, extra, &len);
    verify_int(nwords+1, cmap_count(cm), "cmap_count");
    verify_int_ptr(strlen(extra), cmap_get(cm, extra), "cmap_get(\"strawberry\")");

    printf("\nReplace existing key in CMap.\n");
    len = 2*strlen(extra);
    cmap_put(cm, extra, &len);
    verify_int(nwords+1, cmap_count(cm), "cmap_count");
    verify_int_ptr(len, cmap_get(cm, extra), "cmap_get(\"strawberry\")");
/*
    printf("\nRemove key from CMap.\n");
    cmap_remove(cm, words[0]);
    verify_int(nwords, cmap_count(cm), "cmap_count");
    verify_ptr(NULL, cmap_get(cm, words[0]), "cmap_get(\"apple\")");
*/
    printf("\nUse iterator to count keys.\n");
    int nkeys = 0;
    for (const char *key = cmap_first(cm); key != NULL; key = cmap_next(cm, key))
        nkeys++;
    verify_int(cmap_count(cm), nkeys, "Number of keys");

    cmap_dispose(cm);
}


/* Function: frequency_test
* -------------------------
* Runs a test of the CMap to count letter frequencies from a file.
* Each key is single-char string, value is count (int).
* Reads file char-by-char, updates map, prints total when done.
*/
static void frequency_test()
{
    printf("\n----------------- Testing frequency ------------------ \n");
    CMap *counts = cmap_create(sizeof(int), 26, NULL);

    int val, zero = 0;
    char buf[2];
    buf[1] = '\0';  // null terminator for string of one char

   // initialize map to have entries for all lowercase letters, count = 0
    for (char ch = 'a'; ch <= 'z'; ch++) {
        buf[0] = ch;
        cmap_put(counts, buf, &zero);
    }
    FILE *fp = fopen("/afs/ir/class/cs107/samples/assign1/gettysburg_frags", "r"); 
    assert(fp != NULL);
    while ((val = getc(fp)) != EOF) {
        if (isalpha(val)) { // only count letters
            buf[0] = tolower(val);
            (*(int *)cmap_get(counts, buf))++;
        }
    }
    fclose(fp);

    int total = 0;
    for (const char *key = cmap_first(counts); key != NULL; key = cmap_next(counts, key))
        total += *(int *)cmap_get(counts, key);
    printf("Total of all frequencies = %d\n", total);
    // correct count should agree with shell command
    // tr -c -d "[:alpha:]" < /afs/ir/class/cs107/samples/assign1/gettysburg_frags | wc -c
    cmap_dispose(counts);
}

int main(int argc, char *argv[])
{
    simple_cmap();
    frequency_test();
    return 0;
}
