/*
 * LPF_int.h
 *
 *  Created on: Okt 3, 2025
 *      Author: Elessar
 */

#ifndef HPF_INT_H_
#define HPF_INT_H_

#include "kl_lib.h"

class HPF_t {
private:
    int32_t x1=0, y1=0;
    uint8_t k;
public:
    void Reset() {
        x1=0;
        y1=0;
    }
    int32_t Update(int32_t x0) {
        int32_t y0 = ((y1 + x0 - x1)*10)/k;
        y1 = y0;
        x1 = x0;
        return y0;
    }
    void SetCoeff(uint8_t Ak) {
        k = Ak;
        Reset();
    }
    HPF_t(uint8_t Ak) : k(Ak) {}
};
/*
NOTE:
k - is a fixed-point number and must be at least 10!

Example: HPF_t HpfSpeed {15};
 */

#endif /* HPF_INT_H_ */
