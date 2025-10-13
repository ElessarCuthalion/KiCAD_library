/*
 * Encoder.h
 *
 *  Created on: Feb 29, 2024
 *      Author: Elessar
 */

#ifndef ENCODER_H_
#define ENCODER_H_

#include "kl_lib.h"
#include "board.h"

struct EncoderSetup_t {
    GPIO_TypeDef *PGpio;
    uint8_t PinA, PinB;
    AlterFunc_t AlterFunc;
    TIM_TypeDef *PTimer;
    uint16_t TopVaule;
    EncoderSetup_t(GPIO_TypeDef *APGpio, uint8_t APinA, uint8_t APinB, AlterFunc_t AAlterFunc, TIM_TypeDef *APTimer, uint16_t ATopVaule) :
        PGpio(APGpio), PinA(APinA), PinB(APinB), AlterFunc(AAlterFunc), PTimer(APTimer), TopVaule(ATopVaule) {}
};

const EncoderSetup_t EncSetup {Encoder_AB_PIN};
static Timer_t EncTim {EncSetup.PTimer};

namespace Encoder {     // Supported TIM1, 2, 3
    void Init() {
        PinSetupAlterFunc(EncSetup.PGpio, EncSetup.PinA, omPushPull, pudNone, EncSetup.AlterFunc);
        PinSetupAlterFunc(EncSetup.PGpio, EncSetup.PinB, omPushPull, pudNone, EncSetup.AlterFunc);
        EncTim.Init();
        EncTim.SetTopValue(EncSetup.TopVaule*2);
        EncTim.SelectSlaveMode(smEncoder1);
        EncTim.SetTriggerInput(tiTI1FP1);
        EncTim.SetupInput1(0b01, EncTim.pscDiv1, rfFalling);
        EncTim.SetupInput2(0b01, EncTim.pscDiv1, rfFalling);
        EncTim.Enable();
    }
    void Set(uint32_t AValue) { EncTim.SetCounter(AValue*2); }
    uint32_t Get() { return EncTim.GetCounter()/2; }

    void Clear() { EncTim.SetCounter(0); }
    uint32_t IGetAndClear() {
        chSysLock();
        uint32_t temp = Get();
        Clear();
        chSysUnlock();
        return temp;
    }

};

#endif /* ENCODER_H_ */
