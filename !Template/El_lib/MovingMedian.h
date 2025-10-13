/*
 * MovingMedian.h
 *
 *  Created on: Apr 21, 2025
 *      Author: Elessar, Eldalim
 */

#ifndef MOVINGMEDIAN_H_
#define MOVINGMEDIAN_H_

#include "kl_lib.h"

class MovingMedian_t {
private:
    int32_t *PBuff;
    uint8_t i = 0, N;
public:
	void Reset() {
		for(uint8_t i=0; i<N; i++)
			PBuff[i] = 0;
	}
	int32_t Update(int32_t NewValue) {
		PBuff[i] = NewValue;
		if (i < N-1) i ++;
		else i = 0;
		int32_t TmpBuff[N];
		for(uint8_t i=0; i<N; i++)
			TmpBuff[i] = PBuff[i];
	    return FindMediana(&TmpBuff[0], N);
	}

	MovingMedian_t(int32_t *APBuff, uint8_t AN) : PBuff(APBuff), N(AN) {}
};
/*
Example:
#define CURVE_MOV_MEDIAN_DEPTH  9
int32_t MovMedianBuff[CURVE_MOV_MEDIAN_DEPTH];
MovingMedian_t dSpeedMovMedian{&MovMedianBuff[0], CURVE_MOV_MEDIAN_DEPTH};
 */

class MovingMedian3_t {
private:
    int32_t Buff[3] = {0};
    uint8_t i = 0;
public:
	void Reset() {
		for(auto &tmp : Buff) tmp = 0;
	}
	int32_t Update(int32_t NewValue) {
		Buff[i] = NewValue;
		if (i < 2) i ++;
		else i = 0;
	    return (MAX_(Buff[0], Buff[1]) == MAX_(Buff[1], Buff[2])) ? MAX_(Buff[0], Buff[2]) : MAX_(Buff[1], MIN_(Buff[0], Buff[2]));
	}
};


#endif /* MOVINGMEDIAN_H_ */
