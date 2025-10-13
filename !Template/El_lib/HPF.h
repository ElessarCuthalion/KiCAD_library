/*
 * LPF.h
 *
 *  Created on: Okt 3, 2025
 *      Author: Elessar
 */

#ifndef HPF_H_
#define HPF_H_

#include "kl_lib.h"

class HPF_t {
private:
    float k, x1=0, y1=0;
public:
    void Reset() {
        x1=0;
        y1=0;
    }
    float Update(float x0) {
        float y0 = (y1 + x0 - x1)*k;
        y1 = y0;
        x1 = x0;
        return y0;
    }
    void SetCoeff(float Ak) {
        k = Ak;
        Reset();
    }
    HPF_t(float Ak) : k(Ak) {}
};

#endif /* HPF_H_ */
