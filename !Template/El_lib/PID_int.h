/*
 * PID_int.h
 *
 *  Created on: Mar 14, 2025
 *      Author: Elessar
 */

#ifndef PID_INT_H_
#define PID_INT_H_

#include "uart.h"

#define MAX_REG_VALUE   500000
#define MIN_REG_VALUE   -20000
#define MAX_ERR_VALUE   250

//#define DEBUG_PID

class PID_t {
private:
    int32_t OldErr = 0;   // Required for diff calculations
    int32_t Summ = 0;
#ifdef DEBUG_PID
    systime_t Time = 0;
#endif
    // PID coeffs. "0" means "disabled".
    int32_t Kp;
    int32_t Ki;
    int32_t Kd;
public:
    int32_t TargetValue = 0;
    int32_t Calculate(int32_t NewValue, int32_t dt = 1) {
        int32_t Err = TargetValue - NewValue;
#if MAX_ERR_VALUE
        Limit2Bounds(Err, -MAX_ERR_VALUE, MAX_ERR_VALUE);
#endif
        int32_t Rslt = 0;
        int32_t prop, integ, dif;
        // Proportional
        if(Kp != 0) {
            prop = Kp*Err;
            Rslt += prop;
        }
        // Integral
        if(Ki != 0) {
            Summ += ((Err+OldErr)/2)*dt;
            Limit2Bounds(Summ, INT32_MIN/2, INT32_MAX/2);
            integ = (Ki*Summ)/1000;
            Rslt += integ;
        }
        // Differential
        if(Kd != 0) {
            dif = (Kd*(Err-OldErr))/dt;
            Rslt += dif;
        }
//        PrintfI("Err=%i\t OldErr=%i\t dt=%i\t dif=%i\r\n", Err, OldErr, dt, dif);
        OldErr = Err;   // Save current Err
        // Output limitation
#if MAX_REG_VALUE
        LimitMaxValue(Rslt, MAX_REG_VALUE);
#endif
#if MIN_REG_VALUE
        LimitMinValue(Rslt, MIN_REG_VALUE);
#endif
#ifdef DEBUG_PID
//        PrintfI("in=%i\tErr=%i\tRslt=%i\tdt=%u\r\n", NewValue, Err, Rslt, dt);
        if (Time == 0) {
            Time = chVTGetSystemTimeX();
            PrintfI("Time\tErr\tRslt\tKp\tKi\tKd\r\n");
        }
        PrintfI("%u;\t%i;\t%i", TIME_I2MS(chVTTimeElapsedSinceX(Time)), Err, Rslt);
        if(Kp != 0) PrintfI(";\t%i", prop);
        if(Ki != 0) PrintfI(";\t%i", integ);
        if(Kd != 0) PrintfI(";\t%i", dif);
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
    void SetCoeff(uint32_t AKp, uint32_t AKi, uint32_t AKd) {
        Kp = AKp;
        Ki = AKi;
        Kd = AKd;
        Reset();
    }

    PID_t(uint32_t AKp, uint32_t AKi, uint32_t AKd) :
        Kp(AKp), Ki(AKi), Kd(AKd) {}
};
#ifdef DEBUG_PID
#undef DEBUG_PID
#endif
#endif /* PID_INT_H_ */
