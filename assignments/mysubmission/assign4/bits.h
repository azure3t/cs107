/**
 * File: bits.h
 * ------------
 * Defines the interface for required functions for bits problem set.
 *
 * CS107
 */

#ifndef _bits_h
#define _bits_h

#include <stdbool.h>
#include <stdio.h>

/* Macro: SAT_CHOICE
 * -----------------
 * This #define controls the bitwidth of the stype and utype variables for
 * the saturating arithmetic functions. The options for SAT_CHOICE are
 * 1 for char, 2 for short, 3 for int, 4 for long, or 5 for long long.  
 * Be sure to test your saturating fucntions on all bitwidths. You can
 * manually change the setting in this header file, but the easier way 
 * to test different bitwidths is using the special Makefile targets by
 * typename e.g. make int or make short. These targets set SAT_CHOICE 
 * and rebuild, without having to edit this header file at all.
 */
#ifndef SAT_CHOICE
#define SAT_CHOICE 1
#endif

 
/* Function: cmp_bits
 * ------------------
 * Compares the count of "on" bits between parameters a and b. Returns a negative result
 * if the bitwise representation of a has fewer 1s than b, a positive result if a has more
 * 1s than b, and zero if both have the same number of 1s.
 */
int cmp_bits(int a, int b);


/* Function: make_set
 * ------------------
 * Create a bit vector set from the given array of values. Returns an unsigned short that
 * represents a bit vector set of the values from the array. The bits in positions 1-9 of
 * the returned result mark the set membership, the remaining bits are zeros.
 * Precondition: every number in the values array is the range 1-9
 * Implementation can assume client is responsible for meeting precondition.
 */
unsigned short make_set(int values[], int nvalues);

/* Function: is_single
 * -------------------
 * This function is given three sets in form as returned by make_set above. These represent
 * the set of digits already used in the row, column, and block for a given cell. Returns true
 * if already used digits admit one and only one possible digit for this cell and false otherwise.
 * Precondition: parameters represent well-formed bit vector sets, as returned by make_set
 * Implementation can assume client is responsible for meeting precondition.
 */
bool is_single(unsigned short used_in_row, unsigned short used_in_col, unsigned short used_in_block);


/* Typedefs: stype, utype
 * ----------------------
 * Use the setting for SAT_CHOICE to configure the stype/utype typedefs
 * and the printf format to match the type. Feel free to completely ignore the
 * use/abuse of the preprocessor shown below.
 */

#if SAT_CHOICE == 1
    #define BASETYPE char
    #define FMT "hh"
#elif SAT_CHOICE == 2
    #define BASETYPE short
    #define FMT "h"
#elif SAT_CHOICE == 3
    #define BASETYPE int
    #define FMT ""
#elif SAT_CHOICE == 4
    #define BASETYPE long
    #define FMT "l"
#elif SAT_CHOICE == 5
    #define BASETYPE long long
    #define FMT "ll"
#else
   #error Must define proper SAT_CHOICE (1-5) to set width for saturating arithmetic
#endif

typedef signed BASETYPE stype;
typedef unsigned BASETYPE utype;

#define SFMT "%" FMT "d"
#define UFMT "%" FMT "u"

#define STRINGIFY(x) #x
#define STR(x) STRINGIFY(x)
#define SAT_NAME STR(BASETYPE)

/* Function: sat_add_signed, sat_add_unsigned
 * ------------------------------------------
 * These functions implement saturating arithmetic operators for signed and
 * unsigned types. The function returns the sum a + b if it doesn't
 * overflow/underflow; otherwise returns a value pegged to the maximum/minimum.
 */
stype sat_add_signed(stype a, stype b);
utype sat_add_unsigned(utype a, utype b);


/* Function: disassemble
 * ---------------------
 * Given a pointer to a sequence of binary bytes representing a machine
 * instruction, this function will disassemble and print the equivalent
 * assembly instruction.
 * Precondition: raw_instr points to a sequence of bytes that represents
 * a well-formed instance of one 'pushq' instruction of one of the varieties
 * specifically listed in the assignment writeup.
 * Implementation can assume client is responsible for meeting precondition.
 */
void disassemble(const unsigned char *raw_instr);



#endif
