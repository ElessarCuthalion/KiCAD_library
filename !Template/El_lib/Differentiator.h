/*
 * Diff.h
 *
 *  Created on: Okt 3, 2025
 *      Author: Elessar
 */

#ifndef DIFF_H_
#define DIFF_H_

#include "kl_lib.h"

class Diff_t {
private:
    float b1, x1=0, y1=0;
public:
    void Reset() {
        x1=0;
        y1=0;
    }
    float Update(float x0) {
        float  y0 = x0 - x1 + y1*b1;
        x1 = x0;
        y1 = y0;
        return y0;
    }
    void SetCoeff(float Ab1) {
        b1 = Ab1;
        Reset();
    }
    Diff_t(float Ab1) : b1(Ab1) {}
};

#endif /* DIFF_H_ */
