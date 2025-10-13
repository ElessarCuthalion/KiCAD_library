/*
 * Mathematics.h
 *
 *  Created on: Okt 3, 2025
 *      Author: Elessar
 */

#ifndef MATHEMATICS_H_
#define MATHEMATICS_H_

#include "kl_lib.h"

uint32_t isqrt(uint32_t x) {
    uint32_t m, y, b;
    m = 0x40000000;
    y = 0;
    while (m != 0) {
        b = y | m;
        y >>= 1;
        if (x >= b) {
            x -= b;
            y |= m;
        }
        m >>= 2;
    }
    return y;
}

#endif /* MATHEMATICS_H_ */
