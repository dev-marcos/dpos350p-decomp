/* Functions of nonmatching/libm/ (one file per function, named after it).
 * Addresses are of build 2025-06-19. Names are DERIVED unless the C file says otherwise. */
#ifndef LIBM_H
#define LIBM_H

#include "rt/bits.h"

double sqrt(double x);                                                                           /* 0x4003DF74 */
double pow(double x, double y);                                                                  /* 0x4005D4A4 */
double log10(double x);                                                                          /* 0x4005DAD0 */
double tanh(double x);                                                                           /* 0x4005F91C */
double __sincos_core(double x, int q);                                                           /* 0x4006613C */
double cos(double x);                                                                            /* 0x40066530 */
double sin(double x);                                                                            /* 0x40066534 */
double exp(double x);                                                                            /* 0x40068F08 */
double frexp(double x, int *e);                                                                  /* 0x40069F54 */
double __pow_core(double x, double y, int k);                                                    /* 0x40069FD0 */
double log(double x);                                                                            /* 0x4006A31C */
double ldexp(double x, int n);                                                                   /* 0x4006B86C */

#endif
