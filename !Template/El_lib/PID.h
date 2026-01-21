/*
 * PID.h
 *
 *  Created on: Mar 14, 2025
 *      Author: Elessar
 */

#ifndef PID_H_
#define PID_H_

#include "uart.h"

#define MAX_REG_VALUE   500000.0F
#define MIN_REG_VALUE   -20000.0F
#define MAX_ERR_VALUE   250.0F

#define FLT_MAX     3.40282347e+38F
#define FLT_MIN     1.17549435e-38F

//#define DEBUG_PID

class PID_t {
private:
    float OldErr = 0;   // Required for diff calculations
    float Summ = 0;
#ifdef DEBUG_PID
    systime_t Time = 0;
#endif
    // PID coeffs. "0" means "disabled".
    float Kp;
    float Ki;
    float Kd;
public:
    float TargetValue = 0;
    float Calculate(float NewValue, float dt = 1) {
        float Err = TargetValue - NewValue;
#if defined MAX_ERR_VALUE
        Limit2Bounds(Err, -MAX_ERR_VALUE, MAX_ERR_VALUE);
#endif
        float Rslt = 0;
        float prop, integ, dif;
        // Proportional
        if(Kp != 0) {
            prop = Kp*Err;
            Rslt += prop;
        }
        // Integral
        if(Ki != 0) {
            Summ += ((Err+OldErr)/2)*dt;
            Limit2Bounds(Summ, FLT_MIN/2, FLT_MAX/2);
            integ = Ki*Summ;
            Rslt += integ;
        }
        // Differential
        if(Kd != 0) {
            dif = (Kd*(Err-OldErr))/dt;
            Rslt += dif;
        }
//        PrintfI("Err=%.1f\t OldErr=%.1f\t dt=%.1f\t dif=%.1f\r\n", Err, OldErr, dt, dif);
        OldErr = Err;   // Save current Err
        // Output limitation
#if defined MAX_REG_VALUE
        LimitMaxValue(Rslt, MAX_REG_VALUE);
#endif
#if defined MIN_REG_VALUE
        LimitMinValue(Rslt, MIN_REG_VALUE);
#endif
#ifdef DEBUG_PID
//        PrintfI("in=%.1f\tErr=%.1f\tRslt=%.1f\tdt=%.1f\r\n", NewValue, Err, Rslt, dt);
        if (Time == 0) Time = chVTGetSystemTimeX();
        PrintfI("\r\n->%.1f;\t%.1f;\t%.1f", TIME_I2MS(chVTTimeElapsedSinceX(Time)), Err, Rslt);
        if(Kp != 0) PrintfI(";\t%.1f", prop);
        if(Ki != 0) PrintfI(";\t%.1f", integ);
        if(Kd != 0) PrintfI(";\t%.1f", dif);
        PrintfI("\r\n");
#endif
        return Rslt;
    }
    void Reset() {
//        TargetValue = 0;
        if(Ki != 0) Summ = 0;
        if(Kd != 0) OldErr = 0;
#ifdef DEBUG_PID
        Time = 0;
#endif
    }
    void SetCoeff(float AKp, float AKi, float AKd) {
        Kp = AKp;
        Ki = AKi;
        Kd = AKd;
        Reset();
    }

    PID_t(float AKp, float AKi, float AKd) :
        Kp(AKp), Ki(AKi), Kd(AKd) {}
};
#ifdef DEBUG_PID
#undef DEBUG_PID
#endif
#endif /* PID_H_ */
