/*
 * fcyc.h - prototypes for the routines in fcyc.c that estimate the
 *     time in CPU cycles used by a test function f
 * 
 * Copyright (c) 2002, R. Bryant and D. O'Hallaron, All rights reserved.
 * May not be used, modified, or copied without permission.
 *
 */
#ifndef _CYC_H
#define _CYC_H

/* The test function takes a generic pointer as input */
typedef void (*test_funct)(void *);

/* Compute number of cycles used by test function f */
double fcyc(test_funct f, void* argp);

/* Compute number of seconds used by test function f */
double fsecs(test_funct f, void* argp);

#endif
