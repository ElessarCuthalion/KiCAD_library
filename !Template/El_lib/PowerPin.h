/*
 * PowerPin.h
 *
 *  Created on: Jan 20, 2025
 *      Author: Elessar
 */

#ifndef POWERPIN_H_
#define POWERPIN_H_

#include "board.h"
#include "kl_lib.h"

#define RESET_TIMEOUT_MS    500
enum ActivLevel_t {alActivLo=1, alActivHi=0};

#if CH_VERSION_YEAR == 19
static void RstPinTmrCallback(void *p);
#else
static void RstPinTmrCallback(virtual_timer_t *vtp, void *p);
#endif

class PowerPin_t {
private:
    const PinOutput_t SwPin;
    const bool ActivLo = false;
    bool SwOn = false;
    virtual_timer_t RstPinTmr;
public:
    PowerPin_t(
            GPIO_TypeDef *APGPIO, uint16_t APin, PinOutMode_t AOutputType, const ActivLevel_t AActivLo) :
                SwPin(APGPIO, APin, AOutputType), ActivLo((bool)AActivLo) {}
    PowerPin_t(
            GPIO_TypeDef *APGPIO, uint16_t APin, PinOutMode_t AOutputType) :
                SwPin(APGPIO, APin, AOutputType) {}
    void InitAndOff(){
        if (ActivLo) SwPin.InitAndSetHi();
        else SwPin.Init();
    }
    void InitAndOn(){
        if (ActivLo) SwPin.Init();
        else SwPin.InitAndSetHi();
    }
    void ON() {
        if (ActivLo) SwPin.SetLo();
        else SwPin.SetHi();
        SwOn = true;
    }
    void OFF() {
        if (ActivLo) SwPin.SetHi();
        else SwPin.SetLo();
        SwOn = false;
    }
    bool IsON() { return SwOn; }
    void IReset() {
        chSysLock();
        OFF();
        chVTSetI(&RstPinTmr, TIME_MS2I(RESET_TIMEOUT_MS), RstPinTmrCallback, this);
        chSysUnlock();
    }
};
#if CH_VERSION_YEAR == 19
void RstPinTmrCallback(void *p) {
#else
void RstPinTmrCallback(virtual_timer_t *vtp, void *p) {
#endif
    chSysLockFromISR();
    ((PowerPin_t*)p)->ON();
    chSysUnlockFromISR();
}
#endif /* POWERPIN_H_ */
