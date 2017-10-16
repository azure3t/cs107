/* File: bitstest.c
 * ----------------
 * A program to make simple calls to the functions in bits.c. This is merely
 * a trivial start on testing those functions and you will need to supplement
 * with further tests of your own. You may change/extend/replace this test
 * code in any way you see fit.
 */

#include "bits.h"
#include <assert.h>
#include <error.h>
#include <float.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

// Just a little helper to convert bool to string value when printing
static char *boolstr(bool b)
{
    return b ? "true" : "false";
}

// Just a little helper to convert bool to string value when printing
static char *sign(int num)
{
    return num == 0 ? "zero" : (num < 0 ? "negative" : "positive");
}

static void test_cmp_bits()
{
    printf("cmp_bits(%#x,%#x) = %s\n" , 0xa, 0x5, sign(cmp_bits(0xa, 0x5)));
    printf("cmp_bits(%#x,%#x) = %s\n" , 0xf, 0x1, sign(cmp_bits(0xf, 0x1)));
}

static void test_sudoku()
{
    int rows[] = {1, 2, 5}, cols[] = {2, 6, 7}, block[] = {1, 3, 4, 9};
    printf("\nis_single returned %s\n", boolstr(is_single(make_set(rows, 3), make_set(cols, 3), make_set(block, 4))));
    printf("is_single returned %s\n", boolstr(is_single(make_set(rows, 2), make_set(cols, 2), make_set(block, 3))));
    printf("is_single returned %s\n", boolstr(is_single(0, 0, 0)));
}

// The SFMT/UFMT is #defined to correct format for bitwidth setting of stype/utype
static void test_saturating()
{
    stype s1 = -9, s2 = 116, s3 = 127;
    utype u1 = 11, u2 = 96, u3 = 255;
    printf("\nSaturating stype is %s\n", SAT_NAME);
    printf(SFMT" + "SFMT" = "SFMT" (signed)\n", s1, s2, sat_add_signed(s1, s2));
    printf(SFMT" + "SFMT" = "SFMT" (signed)\n", s3, s3, sat_add_signed(s3, s3));
    printf(UFMT" + "UFMT" = "UFMT" (unsigned)\n", u1, u2, sat_add_unsigned(u1, u2));
    printf(UFMT" + "UFMT" = "UFMT" (unsigned)\n", u3, u3, sat_add_unsigned(u3, u3));
}

static void test_disassemble()
{
    // these constant arrays represented raw instruction bytes to be disassembled
    unsigned char imm[] =    {0x68, 0x10, 0x3f, 0x00, 0x00 }; // push 4-byte immediate
    unsigned char reg[] =    {0x53};                          // push register
    unsigned char ind[] =    {0xff, 0x32};                    // push register indirect
    unsigned char displ[] =  {0xff, 0x70, 0x08};              // push register indirect w/ displacement
    unsigned char scaled[] = {0xff, 0x74, 0x8d, 0xff};        // push register indirect w/ displacement & scaled index

    printf("\nDisassembling raw instructions:\n");
    disassemble(imm);
    disassemble(reg);
    disassemble(ind);
    disassemble(displ);
    disassemble(scaled);
}



int main(int argc, char *argv[])
{
    // use a bit set to select which tests to run
    enum { Cmpbits = 1<<1, Sudoku = 1<<2, Sat = 1<<3, Disassemble = 1<<4, All = (1<<5)-1 } which = All;
    if (argc > 1) {
        which = 1 << atoi(argv[1]);
        if (which < Cmpbits || which > Disassemble)  error(1,0, "argument is number from 1 to 4 to select which test");
    }
    if (which & Cmpbits) test_cmp_bits();
    if (which & Sudoku) test_sudoku();
    if (which & Sat) test_saturating();
    if (which & Disassemble) test_disassemble();
    return 0;
}

