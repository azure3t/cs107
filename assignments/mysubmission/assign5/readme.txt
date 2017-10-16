File: readme.txt
Author: Tiantian Tang 


0) Explain the tactics you used to suppress/avoid/disable explosions.
---------------------------------------------------------------------
From objdump we know that the function that explode the bomb is called explode_bomb.
so I just set a breakpoint there in gdb to avoid explosions:


1) level_1 contains an instruction mov $<hex>,%edi
Explain how this instruction fits into the operation of level_1. What is this
hex value and for what purpose is it being moved?  Why can this instruction
reference %edi instead of the full %rdi register?
--------------------------------------------------------------------------------
hex = 0x403038. this is a pointer to string literal stored in data section in 
memory. since any pointer is size 8 byte. %edi will be big enough to contain it.


2) level_2 contains a call to sscanf, but it is not preceded by a mov into %rdi.
How then is the first argument being passed to sscanf?  What are the other
arguments being passed to this sscanf call?
--------------------------------------------------------------------------------
first argument was passed to %rdi previously by read_input(). 

second argument passed to sscanf is the string format pointer 0x4032c3
which stored in $esi: instruction is as folowing
0x401c62 <level_2+14>   mov    $0x4032c3,%esi

3) level_3 calls the binky function. Translate the assembly instructions for the
binky function into an equivalent C version.
--------------------------------------------------------------------------------

/*
 *Function: binky
 *---------------
 *return 0xffffff060c183060 by generating inttermediate value:
 *0000000000000000000 0011111 0011111 0011111 0011111 0011111 0011111
 *then revert the bit of intermediate value 
 *function takes in 3 arguments. numofOnes is the num of ones in pattern
 *001111, chunkWidth is num of bits in pattern 001111, doubleLooptimes
 *is double the times that pattern exist in intermediate value
 */
int binky(int doubleLooptimes, int chunkWidth, int numofOnes){
    int res = 0;
    for (int i = 0; i < doubleLoopTimes; i+=2){
        int pattern = 0;
        for (int j = 0; j < numOfOnes){
            int base = 1;
            base <<= j;
            pattern |= base;
        }
        res <<= chunkWidth;
        res |= pattern;
    }
    return ^res;
}


4) level_4 declares a local variable that is stored on the stack at 0x8(%rsp).
What is the type/size of this variable? Explain how can you discern its type
from following along in the assembly, even though there: is no explicit type
information in the assembly instructions. Within level_4 there is no instruction 
that writes to this variable. Explain how the variable is initialized (what value
it is set to and when/where does that happen?).
--------------------------------------------------------------------------------
0x8($rsp) is of type char **, size is 8 bytes and was initialized in strtol.

this value was passed to function solve as first argument ($rdi)
then in solve $rdi was saved to $r12
then $r12 was passed to strtol as 2nd argument

strotol 2nd argument is of type char**.
After strtol, 2nd argmuent will be updated to point to another char*, whose value 
is set by strtol to the next charater in str after the numerical value.

To be specific, 0x8($sp) was initialized at following instruction
401e2a:   e8 81 f2 ff ff       callq  4010b0 <strtol@plt>





5) level_5 calls a function named count. What does does this function count? 
What are the parameters to this function? How is count used within level_5?
--------------------------------------------------------------------------------
count(int arg1, int *start, int n, *(void *)fn)
arg1 is a number from sorted array.
start is pointer to an element in unsorted array
n is time of loop
fn is the function to decide weather first digit of two numbers are the same

count will proceed if arg1 and *(int *)unsorted share same first digit.
count will loop from start for n times.
find number that first digit is same as arg1's first digit
return the count of such number.


