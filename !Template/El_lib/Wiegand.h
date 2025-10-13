/*
 * Wiegand.h
 *
 *  Created on: Oct 14, 2024
 *      Author: Eless
 */

#ifndef WIEGAND_H_
#define WIEGAND_H_

#include "kl_lib.h"

#define WG_TX_ENABLED   TRUE
#define WG_RX_ENABLED   TRUE


#if WG_TX_ENABLED // ========================== WG RX ==========================
void WgRxIrqHandler(); // void *p

class WiegandRx_t { // : public IrqHandler_t {
private:
    const PinIrq_t IClkIrq;
    GPIO_TypeDef *Gpio;
    const uint16_t DataPin;
//    uint64_t Buff = 0, DataOut = 0;
//	  uint8_t BitCounter = 0, Mode = 0;
	virtual_timer_t RstTmr;

public:
    void Init();
    void GetData64(uint64_t *PData, uint8_t *PMode);
    void GetData(uint32_t *PData, uint8_t *PMode);
    void GetData(uint32_t *PData);
    // internal use:
    void IRxParse();
    void IIrqHandler();

    WiegandRx_t(
    		GPIO_TypeDef *AGpio, uint16_t AClkPin,
            uint16_t ADataPin):
            	IClkIrq(AGpio, AClkPin, pudNone, WgRxIrqHandler), // this
				Gpio(AGpio), DataPin(ADataPin) {}
};
extern WiegandRx_t WiegandRx;
#endif


#if WG_RX_ENABLED // ========================== WG TX ==========================
class WiegandTx_t {
private:
    PinOutputPWM_t SamplingPWM;
    GPIO_TypeDef *Gpio;
    Timer_t SamplingTmr;
    TIM_TypeDef *ClkTmr;
    const uint16_t DataPin;
    bool IsBusy = false;
    uint64_t BitPosition = 0;
    uint64_t Buff = 0;
    virtual_timer_t SpaseTmr;

public:
    void Init();
    void IIrqHandler();
    bool SendData(uint64_t AData, uint8_t WG_mode);
    void IOnTxDone(); // internal use

    WiegandTx_t(
            GPIO_TypeDef *AGpio, uint16_t AClkPin, TIM_TypeDef *AClkTmr, uint16_t AChnl,
            uint16_t ADataPin):
        SamplingPWM(AGpio, AClkPin, AClkTmr, AChnl, invInverted, omPushPull, 1),
        Gpio(AGpio), SamplingTmr(AClkTmr), ClkTmr(AClkTmr), DataPin(ADataPin) {}
};
extern WiegandTx_t WiegandTx;
#endif

#endif /* WIEGAND_H_ */
