/*
 * ArithMean.h
 *
 *  Created on: Sep 12, 2025
 *      Author: Elessar, Eldalim
 */

#ifndef ARITHMEAN_H_
#define ARITHMEAN_H_

#include "kl_lib.h"


class ArithMean_t {
private:
    int32_t Summ = 0;
    uint32_t Cnt = 0;
public:
	void Reset() {
		Summ = 0;
		Cnt = 0;
	}
	void Add(int32_t NextValue) {
		Summ += NextValue;
		Cnt ++;
	}
	int32_t Get() {
		int32_t Out=0;
		if (Cnt>0)
			Out = Summ/Cnt;
        Reset();
        return Out;
    }
};


#endif /* ARITHMEAN_H_ */
