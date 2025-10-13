/*
 * LPF_int.h
 *
 *  Created on: Apr 18, 2025
 *      Author: Elessar
 */

#ifndef LPF_INT_H_
#define LPF_INT_H_

#include "kl_lib.h"


#if 1 // ================================ Class ================================
class LPFs_t {
private:
    int32_t Summ = 0, Out = 0;
    uint8_t k;
public:
	void Reset() {
		Summ = 0;
		Out = 0;
	}
	int32_t Update(int32_t NewValue) {
		Summ += NewValue - Out;
	    Out = Summ/k;
	    return Out;
	}
    void SetCoeff(uint8_t Ak) {
        k = Ak;
        Reset();
    }

    LPFs_t(uint8_t Ak) : k(Ak) {}
    // higher value k -> stronger smoothing
};
#endif

#if 0 // =========================== Simple function ===========================
#define LPF_k 5
static int32_t LowPassFilt(const int32_t NewValue) {
   static int32_t Summ = 0, Out = 0;
   Summ += NewValue - Out;
   Out = Summ/LPF_k;
   return Out;
}
#endif

#endif /* LPF_INT_H_ */
