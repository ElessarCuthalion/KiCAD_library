/*
 * BitMacros.h
 *
 *  Created on: Mar 3, 2025
 *      Author: Elessar
 */

#ifndef BITMACROS_H_
#define BITMACROS_H_

#include "kl_lib.h"

// Bit manipulation
#ifndef SetBit
#define SetBit(w, bitNum)   (w) |= (1U << (bitNum)))
#endif

#ifndef ClrBit
#define ClrBit(w, bitNum)    ((w) &= (~(1U << (bitNum))))
#endif

#ifndef CheckBit
#define CheckBit (w, bitNum)    ((w) & (1U << (bitNum)))
#endif

#endif /* BITMACROS_H_ */
