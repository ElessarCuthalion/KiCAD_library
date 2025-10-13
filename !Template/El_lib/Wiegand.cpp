/*
 * Wiegand.cpp
 *
 *  Created on: Oct 14, 2024
 *      Author: Eless
 */

#include "Wiegand.h"
#include "uart.h"

#define WG_SPASE_TIMEOUT_MS 40

#if WG_TX_ENABLED // ========================== WG RX ==========================
void RstTmrCallback(void *p) {
    chSysLockFromISR();
    ((WiegandRx_t*)p)->IRxParse();
    chSysUnlockFromISR();
}
#if 1 // ==== IRQ =====
uint8_t BitCounter = 0, Mode = 0;
uint64_t Buff = 0, DataOut = 0;
void WgRxIrqHandler() { // void *p
//  ((WiegandRx_t*)p)->IIrqHandler();
    WiegandRx.IIrqHandler();
}
void WiegandRx_t::IIrqHandler() {
    chVTSetI(&RstTmr, TIME_MS2I(WG_SPASE_TIMEOUT_MS), RstTmrCallback, this);
    BitCounter ++;
    Buff = Buff<<1 | PinIsHi(Gpio, DataPin);
//    PrintfI("WgRx: bit[%u]=%u\r", BitCounter, PinIsHi(Gpio, DataPin));
}
#endif
void WiegandRx_t::IRxParse() {
//    PrintfI("WgRx=%u Bits: %u\r", Buff, BitCounter);
    if (ANY_OF_3(BitCounter, 26, 34, 42)) { // in Parity control
        bool Parity = 1;
        uint64_t BitPosition = 0b1;
        uint8_t i=1; // skip Lo-parity bit
        for(; i<BitCounter/2; i++) { // calc Lo-parity
            BitPosition <<= 1;
            Parity ^= (bool)(Buff & BitPosition);
//            PrintfI("WgRx_Lo: bit[%u]=%u Parity=%u\r", i, Buff&BitPosition, Parity);
        }
//        PrintfI("Parity_Lo=%u(%u)\r", Parity, Buff&0b1);
        if (Parity != (Buff&0b1)) goto ParseFail;
        Parity = 0;
        for(; i<BitCounter-1; i++) { // calc Hi-parity
            BitPosition <<= 1;
            Parity ^= (bool)(Buff & BitPosition);
//            PrintfI("WgRx_Hi: bit[%u]=%u Parity=%u\r", i, Buff&BitPosition, Parity);
        }
        BitPosition <<= 1;
//        PrintfI("Parity_Hi=%u(%u)\r", Parity, Buff&BitPosition);
        if (Parity != (bool)(Buff & BitPosition)) goto ParseFail;
        DataOut = (Buff&(~BitPosition))>>1; // clear Parity bits
        Mode = BitCounter;
//        PrintfI("WgRx: %u (%u)\r", (uint32_t)DataOut, Mode);
    } else if (ANY_OF_4(BitCounter, 4, 8, 24, 32)) { // no Parity control
        DataOut = Buff;
        Mode = BitCounter;
    } else goto ParseFail;
    BitCounter = 0;
    Buff = 0;
    return;

    ParseFail:
    BitCounter = 0;
    Buff = 0;
    PrintfI("Wg: parse fail\r");
}

void WiegandRx_t::Init() {
    IClkIrq.Init(ttRising);
    IClkIrq.EnableIrq(IRQ_PRIO_LOW);
    PinSetupInput(Gpio, DataPin, pudPullDown);
}
void WiegandRx_t::GetData64(uint64_t *PData, uint8_t *PMode) {
    *PData = DataOut;
    *PMode = Mode;
    DataOut = 0;
    Mode = 0;
}
void WiegandRx_t::GetData(uint32_t *PData, uint8_t *PMode) {
    *PData = (uint32_t)DataOut;
    *PMode = Mode;
    DataOut = 0;
    Mode = 0;
}
void WiegandRx_t::GetData(uint32_t *PData) {
    *PData = (uint32_t)DataOut;
    DataOut = 0;
    Mode = 0;
}
#endif



#if WG_RX_ENABLED // ========================== WG TX ==========================
#define WG_DATA_PULCES_US	40  // 20-200uS
#define WG_DATA_SPACE_US	400 // 300-3000uS

void SpaseTmrCallback(void *p) {
    chSysLockFromISR();
    ((WiegandTx_t*)p)->IOnTxDone();
    chSysUnlockFromISR();
}
#if 1 // ==== IRQ =====
extern "C" {
void WG_SAMPLING_TMR_IRQ_HNDLR(void) {
    CH_IRQ_PROLOGUE();
    chSysLockFromISR();
    WiegandTx.IIrqHandler();
//    PrintfI("WG RX IRQ\r\n");
    chSysUnlockFromISR();
    CH_IRQ_EPILOGUE();
}
}
void WiegandTx_t::IIrqHandler() {
    SamplingTmr.ClearUpdateIrqPendingBit();
    if (BitPosition >= 1)
        SamplingTmr.Enable();
     else
        chVTSetI(&SpaseTmr, TIME_MS2I(WG_SPASE_TIMEOUT_MS), SpaseTmrCallback, this);
    if (Buff & BitPosition)
        PinSetHi(Gpio, DataPin);
    else
        PinSetLo(Gpio, DataPin);
//    bool CurrBit = Buff&BitPosition;
//    PrintfI("WgTx: bit[%u]=%u\r", BitPosition, CurrBit);
    BitPosition >>= 1;
}
#endif

void WiegandTx_t::Init() {
    SamplingPWM.Init(); // init Timer and Channel
    SamplingTmr.Disable();
    // ==== Timer ====
//	  SamplingTmr.Init();
//    SamplingTmr.EnableCCOutput2();
//    SamplingTmr.SetupOutput2(0b0111);   // PWM mode 2 - Inverted (PWM mode 1 is 0b0110)
    ClkTmr->CR1 |=  TIM_CR1_OPM;    // One-pulse mode
    ClkTmr->BDTR = 0xC000;          // Main output Enable
    // Setup timings
    SamplingTmr.SetupPrescaler(1000000); // Input Freq: 1 MHz => one tick = 1 uS
    SamplingTmr.SetTopValue(WG_DATA_PULCES_US+WG_DATA_SPACE_US);
    SamplingTmr.SetCCR2(WG_DATA_SPACE_US);
	// Setup IRQ
	SamplingTmr.EnableIrqOnUpdate();
	SamplingTmr.EnableIrq(WG_SAMPLING_TMR_IRQ, IRQ_PRIO_MEDIUM);
	// Setup pin
//	PinSetupAlterFunc(Gpio, ClkPin, omPushPull, pudNone, AF1);
    PinSetupOut(Gpio, DataPin, omPushPull);
}

bool WiegandTx_t::SendData(uint64_t AData, uint8_t WG_mode) {
    if (!IsBusy) {
        Buff = AData;
        BitPosition = 1U<<(WG_mode-1); // sending MSb first
        if (ANY_OF_3(WG_mode, 26, 34, 42)) { // in Parity control
            bool Parity = 1;
            uint8_t i=0;
            for(; i<WG_mode/2-1; i++) { // calc Lo-parity
                Parity ^= AData & 0b1;
//                PrintfI("WgTx_Lo: bit[%u]=%u Parity=%u\r", i+1, AData&0b1, Parity);
                AData >>= 1;
            }
//            PrintfI("Parity_Lo=%u\r", Parity);
            Buff = Buff<<1 | Parity;
            Parity = 0;
            for(; i<WG_mode-2; i++) { // calc Hi-parity
                Parity ^= AData & 0b1;
//                PrintfI("WgTx_Hi: bit[%u]=%u Parity=%u\r", i+1, AData&0b1, Parity);
                AData >>= 1;
            }
//            PrintfI("Parity_Hi=%u\r", Parity);
            Buff |= (uint64_t)Parity << (WG_mode-1);
        }
        else if (!ANY_OF_4(WG_mode, 4, 8, 24, 32)) { // no Parity control
                BitPosition = 0;
                Buff = 0;
                return false;
            }
        SamplingTmr.GenerateUpdateEvt();
//        PrintfI("WgTx=%u Bits: %u\r", Buff, WG_mode);
        return true;
    } else
        return false;
}

void WiegandTx_t::IOnTxDone() {
    IsBusy = false;
//    PrintfI("WgTx Done\r");
}

#endif
