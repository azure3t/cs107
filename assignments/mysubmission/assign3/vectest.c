/* File: vectest.c
* ----------------
* A program to exercise some basic functionality of the CVector. You should
* supplement with additional tests of your own.  You may change/extend/replace
* this test program in any way you see fit.
* jzelenski
*/

#include "cvector.h"
#include <error.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


// Uncomment this line to test cvec_remove
//#define ENABLE_CVEC_REMOVE
// Uncomment this line to test cvec_replace
//#define ENABLE_CVEC_REPLACE


/* Function: verify_int
* ---------------------
* Used to compare a given int result with value expected and report on whether
* passed/failed.
*/
static void verify_int(int expected, int found, char *msg)
{
    printf("%s expect: %d found: %d. %s\n", msg, expected, found,
        (expected == found) ? "Seems ok." : "##### PROBLEM HERE #####");
}


/* Function: simple_cvec
* ----------------------
* Exercises the CVector storing integers. Exercises the operations to
* add elements (append/insert) and remove/change (remove/replace) and
* access elements (nth).
*/
static void simple_cvec()
{
    printf("\n----------------- Testing simple cvec ------------------ \n");
    CVector *cv = cvec_create(sizeof(int), 20, NULL); // large enough, no realloc
    printf("Created empty CVector.\n");
    verify_int(0, cvec_count(cv), "cvec_count");

    printf("\nAppending 10 numbers to CVector.\n");
    for (int i = 0; i < 10; i++)
        cvec_append(cv, &i);                       // 0|1|2|3|4|5|6|7|8|9
    verify_int(10, cvec_count(cv), "cvec_count");
    verify_int(5, *(int *)cvec_nth(cv, 5), "*value for cvec_nth(5)");

    printf("Contents are: ");
    for (int *cur = cvec_first(cv); cur != NULL; cur = cvec_next(cv, cur))
        printf("%d ", *cur);
    printf("\n");

    printf("\nNegate every other elem using pointer access.\n");
    for (int i = 0; i < cvec_count(cv); i += 2) // Loop by 2
        (*(int *) cvec_nth(cv, i)) *= -1;           //0|1|-2|3|-4|5|-6|7|-8|9
    verify_int(1, *(int *)cvec_nth(cv, 1), "*value for cvec_nth(1)");
    verify_int(-2, *(int *)cvec_nth(cv, 2), "*value for cvec_nth(2)");

#ifdef ENABLE_CVEC_REPLACE
    printf("\nUn-negate using replace function.\n");
    for (int i = 0; i < cvec_count(cv); i += 2) // Loop by 2
        cvec_replace(cv, &i, i);                   // 0|1|2|3|4|5|6|7|8|9
    verify_int(3, *(int *)cvec_nth(cv, 3), "*value for cvec_nth(3)");
    verify_int(4, *(int *)cvec_nth(cv, 4), "*value for cvec_nth(4)");
#else
    printf("\nUn-negate using pointer access.\n");
    for (int i = 0; i < cvec_count(cv); i += 2) // Loop by 2
        (*(int *) cvec_nth(cv, i)) *= -1;           //0|1|-2|3|-4|5|-6|7|-8|9
    verify_int(3, *(int *)cvec_nth(cv, 3), "*value for cvec_nth(3)");
    verify_int(4, *(int *)cvec_nth(cv, 4), "*value for cvec_nth(4)");
#endif

    printf("\nInsert new elem at indexes 3 and 6.\n");
    int val = 99;
    cvec_insert(cv, &val, 3);
    cvec_insert(cv, &val, 6);                    //0|1|2|99|3|4|99|5|6|7|8|9
    verify_int(12, cvec_count(cv), "cvec_count");
    verify_int(val, *(int *)cvec_nth(cv, 3), "*value for cvec_nth(3)");
    verify_int(6, *(int *)cvec_nth(cv, 8), "*value for cvec_nth(8)");

#ifdef ENABLE_CVEC_REMOVE
    printf("\nRemove at indexes 3 and 8.\n");
    cvec_remove(cv, 3);
    cvec_remove(cv, 8);                        //0|1|2|3|4|99|5|6|8|9
    verify_int(10, cvec_count(cv), "cvec_count");
    verify_int(3, *(int *)cvec_nth(cv, 3), "*value for cvec_nth(3)");
    verify_int(9, *(int *)cvec_nth(cv, 9), "*value for cvec_nth(9)");
#endif

    cvec_dispose(cv);
}


/* Function: cmp_char
* ------------------
* Comparator function used to compare two character elements within a cvector.
* Used for both sorting and searching in the array of characters. Returns
* negative value if first < second, positive if first > second, and zero if ==
*/
static int cmp_char(const void *p1, const void *p2)
{
    return (*(char *)p1 - *(char *)p2);
}


/* Function: sortsearch_test
* --------------------------
* Exercises the CVector storing chars. Tests sort, linear and binary search.
*/
static void sortsearch_test()
{
    char *jumbled = "xatmpdvyhglzjrknicoqsbuewf"; // alphabet permutation
    char *alphabet = "abcdefghijklmnopqrstuvwxyz";

    printf("\n----------------- Testing sort & search ------------------ \n");
    CVector *cv = cvec_create(sizeof(char), 4, NULL);
    for (int i = 0; i < strlen(jumbled); i++)
        cvec_append(cv, &jumbled[i]);

    printf("\nDoing linear searches on unsorted cvector.\n");
    char ch = '*';
    verify_int(0, cvec_search(cv, &jumbled[0], cmp_char, 0, false), "Linear search");
    verify_int(9, cvec_search(cv, &jumbled[9], cmp_char, 0, false), "Linear search");
    verify_int(-1, cvec_search(cv, &ch, cmp_char, 10, false), "Linear search");

    printf("\nSorting cvector.\n");
    cvec_sort(cv, cmp_char);	 // Sort into alpha order
    verify_int(alphabet[0], *(char *)cvec_nth(cv, 0), "*value for cvec_nth(0)");
    verify_int(alphabet[10], *(char *)cvec_nth(cv, 10), "*value for cvec_nth(10)");

    printf("\nDoing binary searches on sorted cvector.\n");
    verify_int(0, cvec_search(cv, &alphabet[0], cmp_char, 0, true), "Binary search");
    verify_int(20, cvec_search(cv, &alphabet[20], cmp_char, 10, true), "Binary search");
    verify_int(20, cvec_search(cv, &alphabet[20], cmp_char, 10, false), "Linear search");
    verify_int(-1, cvec_search(cv, &ch, cmp_char, 10, true), "Binary search");
    cvec_dispose(cv);
}



static int cmp_int(const void *p1, const void *p2)
{
    return (*(int *)p1) - (*(int *)p2);
}


/* Function: large_test
* ---------------------
* Generate a large CVector of ints. Uses a small allocation
* chunk size to force a lot of reallocation. Inserts in middle,
* deletes from middle, sorts, searches. Using same operations
* from simple but on a much larger vector to push the boundaries.
*/
static void large_test(int size)
{
    printf("\n----------------- Testing large CVector ------------------ \n");
    printf("(these operations can be slow. Have patience...)\n");

    printf("\nFilling CVector with numbers from 1 to %d in random order.\n", size);
    CVector *cv = cvec_create(sizeof(int), 4, NULL);
    for (int i = 0; i < size; i++)
        cvec_insert(cv, &i, rand() % (cvec_count(cv) + 1)); // insert i at random index

#ifdef ENABLE_CVEC_REMOVE
    printf("Deleting all odd numbers.\n");
    for (int i = 1; i < size; i+= 2)	{ // loop by 2
        int found = cvec_search(cv, &i, cmp_int, 0, false);
        cvec_remove(cv, found);
    }

    printf("Verifying only even numbers remain.\n");  // contains 0|2|4|6|...
    for (int i = 0; i < cvec_count(cv); i++) {
        if ((*(int *)cvec_nth(cv, i)) % 2 != 0) {
            verify_int(i*2, *(int *) cvec_nth(cv, i), "cvec_nth()");
            break; // stop at first sign of trouble
        }
    }

#endif

    printf("Sorting CVector.\n");
    //my add
    printf("cv length %d\n", cvec_count(cv));
    cvec_sort(cv, cmp_int);
    printf("Verifying CVector is in sorted order.\n");
    for (int i = 0; i < cvec_count(cv); i++) {
        if (i != *(int *) cvec_nth(cv, i)) {
            verify_int(i, *(int *) cvec_nth(cv, i), "cvec_nth()");
            break; // stop at first sign of trouble
        }
    }
    cvec_dispose(cv);
}



int main(int argc, char *argv[])
{
    simple_cvec();
    sortsearch_test();
    large_test(25000);
    return 0;
}
