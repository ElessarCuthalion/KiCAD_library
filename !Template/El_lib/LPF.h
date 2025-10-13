/*
 * LPF.h
 *
 *  Created on: Apr 18, 2025
 *      Author: Elessar
 */

#ifndef LPF_H_
#define LPF_H_

#include "kl_lib.h"

#if 0 // ============================= High order ==============================
class LPF_t {
private:
    float a0, a1, a2, b1, b2, x1=0, x2=0, y1=0, y2=0;
public:
    int32_t Update(float x0) {
        float y0 = a0 * (float)x0 + a1 * x1 + a2 * x2 - b1 * y1 - b2 * y2;
        x2 = x1;
        x1 = x0;
        y2 = y1;
        y1 = y0;
        return (int32_t)y0;
    }
    LPF_t(float a0, float a1, float a2, float b1, float b2) : a0(a0),a1(a1),a2(a2),b1(b1),b2(b2) {}
};
// example: LPF_t LPF {0.020083, 0.040167, 0.020083, -1.561018, 0.641352};
#endif

#if 1 // ================================ Class ================================
class LPFs_t {
private:
    float k, Summ = 0, Out = 0;
public:
	void Reset() {
		Summ = 0;
		Out = 0;
	}
	float Update(float NewValue) {
		Summ += NewValue - Out;
	    Out = Summ*k;
	    return Out;
	}
    void SetCoeff(float Ak) {
        k = Ak;
        Reset();
    }
    LPFs_t(float Ak) : k(Ak) {}
};
#endif

#if 0 // =========================== Simple function ===========================
#define LPF_k 0.8F
static float LowPassFilt(const float NewValue) {
   static float Summ = 0, Out = 0;
   Summ += NewValue - Out;
   Out = Summ*LPF_k;
   return Out;
}
#endif

#endif /* LPF_H_ */
