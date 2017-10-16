/*
 * File: bits.c
 * Author: TIANTIAN TANG
 * ----------------------
 *
 */

#include "bits.h"
#include <assert.h>
#include <math.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
/* The NOT_YET_IMPLEMENTED macro is used as the body for all functions
 * to remind you about which operations you haven't yet implemented.
 * It wil report an error if a call is made to an not-yet-implemented
 * function (this is preferable to returning garbage or silently
 * ignoring the call).  Remove the call to this macro as you implement
 * each function and finally you can remove this macro and comment
 * when no longer needed.
 */
#define NOT_YET_IMPLEMENTED printf("%s() not yet implemented!\n", __func__); raise(SIGKILL); exit(107);

/* Function: cmp_bit
 * --------------------------
 * Given integer a, b. Find compare which integer has more on bit in 
 * their binary representation. If a has more on bits than b, return 
 * a positive number, same return 0, fewer return negativ enumber
 */
int cmp_bits(int a, int b)
{
    int acount = 0;
    int bcount = 0;
    while (a){
        acount += a & 1; 
        a >>= 1;
    }
    while (b){
        bcount += b & 1;
        b >>= 1;
    }
    return acount - bcount;

}
/* Function: make_set
 * ----------------
 * create a bit vector set from array. Arguments are  array of integer, 
 * array count. Return an unsinged short that represents a bit vecotr set
 * from the array. bits in position 1-9 of returned sult mark the set
 * membership. other bits are zero
 */

unsigned short make_set(int values[], int nvalues)
{
    unsigned short res = 0;
    for (int i = 0; i < nvalues; i++){
        res |= 1 << values[i];
    }
    return res;
}

/* Function: is_single
 * ------------------
 * arguments are array of integers. Return true if there is only one
 * umber in 1-9 that is not in given array.
 */
bool is_single(unsigned short used_in_row, unsigned short used_in_col, unsigned short used_in_block)
{
    unsigned short tmp = used_in_row | used_in_col | used_in_block;
    // 1023 has binary representation 111111111, 9 bits are on
    // 127 has binary represetnation 1111111, 7 bits are on
    // we want to check if tmp has 8 bits set excactly
    // so utilize cmp_bits define before. 
    return cmp_bits(tmp,1023) < 0 && cmp_bits(tmp,127) > 0;
}

/* Function: sat_add_unsigned
 * --------------------------
 * Add two unsinged number of various integer type.
 * return their sum or MAX if saturated
 */
utype sat_add_unsigned(utype a, utype b)
{
    utype res = a + b;
    // overflow happens when result is less than operand 
    if (res < a || res < b){
        res = (utype) 0xffff; // MAX value for unsigned number has bit pattern of all 1s.
    }
    return res;

}

/* Function: sat_add_signed
 * --------------------------
 * Add two singed number of various integer type.
 * return their sum or MAX/MIN if saturated
 */

stype sat_add_signed(stype a, stype b)
{
    stype res = a + b;

    // overflow happens when
    // 1. sum of two positive numbers yiels a negative result
    if (a > 0 && b > 0 && res < 0){
        // MAX value pattern for signed number looks like 01111...
        // which can be obtain by 1111..11 ^ 1000..00
        res  = (stype) 0xffff;
        stype mask = (stype) 0x1; 
        // creating a max bits pattern looks like 1000000....
        while ( (stype)(mask << 1) != 0){
            mask <<= 1;
        }
        return  res^mask;
    }
    //2. sum of two negative number yields a positive result
    if (a < 0 && b < 0 && res > 0){
        // MIN value pattern for signed number looks like 10000....
        // which can be obtained by 1111..111 & 10000..00
        res = (stype) 0xffff;
        stype mask = (stype) 0x1; 
        // creating a max bits pattern looks like 1000000....
        while ( (stype)(mask << 1) != 0){
            mask <<= 1;
        }
        return res&mask;
    }

    // otherwise, result has not overflowed
    return res;
}

#define MAX_BYTES 5

/* Function: print_hex_bytes
 * --------------------------
 * Given the pointer to a sequence of bytes and a count, and this
 * function will print count number of raw bytes in hex starting at
 * the bytes address, filling in with spaces to align at the width
 * of the maximum number of bytes printed (5 for push).
 */
void print_hex_bytes(const unsigned char *bytes, int nbytes)
{
    for (int i = 0; i < MAX_BYTES; i++)
        if (i < nbytes)
            printf("%02x ", bytes[i]);
        else
            printf("  ");
    printf("\t");
}

// Each register name occupies index of array according to number
const char *regnames[] = {"%rax", "%rcx", "%rdx", "%rbx", "%rsp", "%rbp", "%rsi", "%rdi"};

/* Function: disassemble 
 * --------------------------
 * Given the pointer to a sequence of assembly instructions of various
 * length. Print out instruction in hex and human readable
 * disassebled instruction.
 */

void disassemble(const unsigned char *raw_instr)
{
    int mask1 = 0x68;//first byte mask for pushq(5)
    int mask2 = 0x50;//first byte mask for pushq(1)
    int mask3 = 0xff;//first byte mask for pushq(2, 3, 4)
    int mask4 = 0x30;//seconde byte mask for pushq(2)
    int mask5 = 0x74;//second byte mask for pushq(4)
    int mask6 = 0x70; //second byte maks for pushq(3)
    unsigned char firstByte = *raw_instr;
    unsigned char secondByte = *(raw_instr + 1);
    unsigned char thirdByte = *(raw_instr + 2);
    unsigned char fourthByte = *(raw_instr + 3);  
    
    //pushq(5) 
    if ((mask1 ^ firstByte) == 0){
        print_hex_bytes(raw_instr, 5);
        printf("pushq $0x%x%x\n", *(raw_instr + 2), *(raw_instr + 1));
        return;
    }

    //pushq(1)
    if (((mask2 >> 3) ^ (firstByte >> 3)) == 0){
        print_hex_bytes(raw_instr, 1);
        printf("pushq %s\n", regnames[firstByte & 07]);
        return;
    }

    if ((mask3 ^ firstByte) == 0){
        //pushq(2): last 3 bits in seoncdByte decide which  register
        if ((secondByte ^ mask4 ) >= 0 && (secondByte^mask4) <= 7 ){
            print_hex_bytes(raw_instr, 2);
            printf("pushq (%s)\n", regnames[(int) secondByte & 0x07]);       
            return;
        }
        // pushq(4): 
        if ((*(raw_instr + 1) ^ mask5) == 0){
            print_hex_bytes(raw_instr, 4);
            int scaledIdx = 1 << (thirdByte >> 6);
            printf("pushq 0x%x(%s, %s, %d)\n", fourthByte, regnames[(thirdByte & 0x07)], regnames[(thirdByte & 0x38) >> 3], scaledIdx);       
            return;
        } 
        //pushq(3): last 3 bits in seoncd byte are regi
        if ((secondByte ^ mask6 ) >= 0 && (secondByte ^ mask6) <= 7 ){
            print_hex_bytes(raw_instr, 3);
            printf("pushq 0x%x(%s)\n",thirdByte, regnames[secondByte & 07]);       
            return;
        }
    }
}



