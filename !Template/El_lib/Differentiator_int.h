/*
 * Diff_int.h
 *
 *  Created on: Okt 3, 2025
 *      Author: Elessar
 */

#ifndef DIFF_INT_H_
#define DIFF_INT_H_

#include "kl_lib.h"

class Diff_t {
private:
	int32_t x1=0, y1=0;
    uint8_t b1;
public:
	void Reset() {
		x1=0;
		y1=0;
	}
    int32_t Update(int32_t x0) {
    	int32_t y0 = x0 - x1 + (y1*10)/b1;
        x1 = x0;
        y1 = y0;
        return y0;
    }
    void SetCoeff(uint8_t Ab1) {
        b1 = Ab1;
        Reset();
    }
    Diff_t(uint8_t b1) : b1(b1) {}
};

/*
NOTE:
b1 - is a fixed-point number and must be at least 10!

Example: Diff_t DiffSpeed {15};
 */

#endif /* DIFF_INT_H_ */
