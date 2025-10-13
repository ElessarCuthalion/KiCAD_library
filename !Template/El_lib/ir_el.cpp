/*
 * ir_el.cpp
 *
 *  Created on: 01.09.2023
 *      Author: Eless
 */

#include "ir_el.h"
#include "uart.h"

#if IR_TX_ENABLED // ========================== IR TX ==========================
//#define DBG_PINS
#ifdef DBG_PINS
#define DBG_GPIO1   GPIOC
#define DBG_PIN1    8
#define DBG_PIN_INIT()  PinSetupOut(DBG_GPIO1, DBG_PIN1, omPushPull)
#define DBG1_SET()  PinSetHi(DBG_GPIO1, DBG_PIN1)
#define DBG1_CLR()  PinSetLo(DBG_GPIO1, DBG_PIN1)
#else
#define DBG_PIN_INIT()
#define DBG1_SET()
#define DBG1_CLR()
#endif


namespace irLed {

#define IR_TX_DMA_MODE  \
    STM32_DMA_CR_CHSEL(IR_TX_DMA_CHNL) \
	| DMA_PRIORITY_HIGH \
    | STM32_DMA_CR_MSIZE_HWORD \
    | STM32_DMA_CR_PSIZE_HWORD \
    | STM32_DMA_CR_MINC \
    | STM32_DMA_CR_DIR_M2P \
    | STM32_DMA_CR_TCIE

#define IR_TX_BUF_SZ IR_BIT_CNT + 2

Timer_t CarrierTmr{TMR_CARRIER};
PinOutputPWM_t CarrierPWM(PWM_CARRIER);
Timer_t SamplingTmr{TMR_SAMPLING};
PinOutputPWM_t SamplingPWM(PWM_SAMPLING);
const stm32_dma_stream_t *PDmaTx = nullptr;
ftVoidVoid ICallbackI = nullptr;
static uint32_t TransactionSz;
uint16_t IR_TX_Buf[IR_TX_BUF_SZ];

uint8_t ConvertPercentToLevel(uint8_t Percent) {
    return IR_DAC_LVL_MIN+((int16_t)(IR_DAC_LVL_MAX-IR_DAC_LVL_MIN) * Percent)/100;
}


static inline void StartTx() {
	dmaStreamSetMemory0(PDmaTx, IR_TX_Buf);
	dmaStreamSetTransactionSize(PDmaTx, TransactionSz+1); // Last download is dummy (garbage)
	dmaStreamSetMode(PDmaTx, IR_TX_DMA_MODE);
    dmaStreamEnable(PDmaTx);
	CarrierPWM.Set(1);		// Enable Carrier frequency
    SamplingTmr.SetCounter(0);
    SamplingTmr.GenerateUpdateEvt();
    SamplingTmr.Enable();
}

void DmaTxEndIrqHandler(void *p, uint32_t flags) {
    chSysLockFromISR();
    SamplingTmr.Disable();
	CarrierPWM.Set(0);
    dmaStreamDisable(PDmaTx);
    if(ICallbackI) ICallbackI();
    chSysUnlockFromISR();
}

void Init() {
    // ==== GPIO ====
    // Once the DAC channel is enabled, the corresponding GPIO pin is automatically
    // connected to the DAC converter. In order to avoid parasitic consumption,
    // the GPIO pin should be configured in analog.
    PinSetupAnalog(IR_LEVEL);
    PinSetupAlterFunc(IR_OUT_PIN);
    // ==== DAC ====
    rccEnableDAC1();
    DAC->CR = DAC_CR_EN1;	// Enable Ch1

	// ==== Carrier timer ====
	CarrierPWM.Init();
	CarrierPWM.SetFrequencyHz(IR_CARRIER_HZ);
	CarrierPWM.Set(0);
	PinSetupAnalog(CARRIER_PIN);	// Disconnect from a pin

    // ==== Sampling timer ====
#if 1 // Implementation via PWM and Timer classes
	SamplingPWM.Init();
	SamplingPWM.Set(IR_SPACE_uS);
	SamplingTmr.SetupPrescaler(1000000); // Input Freq: 1 MHz => one tick = 1 uS
	SamplingTmr.DisableArrBuffering();
	SamplingTmr.EnableDmaOnUpdate();
	SamplingTmr.Disable();
	PinSetupAnalog(SAMPLING_PIN);	// Disconnect from a pin
#else // Only implementation via Timer class
	SamplingTmr.Init();
	SamplingTmr.Enable();
	SamplingTmr.SetupOutput1(0b0111); // PWM mode 2 (Inverted)
	SamplingTmr.EnableCCOutput1();
	TMR_SAMPLING->BDTR = 0xC000;   // Main output Enable
	SamplingTmr.SetCCR1(IR_SPACE_uS);
	SamplingTmr.SetupPrescaler(1000000); // Input Freq: 1 MHz => one tick = 1 uS
	SamplingTmr.DisableArrBuffering();
	SamplingTmr.EnableDmaOnUpdate();
	SamplingTmr.Disable();
//	PinSetupAlterFunc(SAMPLING_PIN, omPushPull, pudNone, AF5);
#endif
    // ==== DMA ====
	SYSCFG->CFGR1 |= SYSCFG_CFGR1_TIM16_DMA_RMP;	// Remap DMA request for TIM16_UP on DMA C4
    PDmaTx = dmaStreamAlloc(IR_TX_DMA, IRQ_PRIO_MEDIUM, DmaTxEndIrqHandler, nullptr);
    dmaStreamSetPeripheral(PDmaTx, &TMR_SAMPLING->ARR);
}

// Power is DAC value
void TransmitWord(uint32_t wData, uint8_t PowerPercent, ftVoidVoid CallbackI) {
    ICallbackI = CallbackI;
    // ==== Fill buffer depending on data ====
    uint16_t *p = IR_TX_Buf;
	// Put header
    *p++ = IR_SPACE_uS+IR_HEADER_uS;
    // Put data
    for(uint8_t i=0; i<IR_BIT_CNT; i++) {
        // Carrier
        if(wData & 0x1) *p++ = IR_SPACE_uS+IR_ONE_uS;
        else            *p++ = IR_SPACE_uS+IR_ZERO_uS;
        wData >>= 1; // Send LSB first
    }
    // Put pause
    *p++ = IR_SPACE_uS+IR_PAUSE_AFTER_uS;
    // ==== Start transmission ====
    DAC->DHR8R1 = ConvertPercentToLevel(PowerPercent);
    TransactionSz = p - IR_TX_Buf;
    StartTx();
}
void ReTransmit(uint8_t PowerPercent) {
	if (DAC->DHR8R1 != ConvertPercentToLevel(PowerPercent))
		DAC->DHR8R1 = ConvertPercentToLevel(PowerPercent);
    StartTx();
}

void ResetI() {
    dmaStreamDisable(PDmaTx);
    DAC->DHR8R1 = 0;
	CarrierPWM.Set(0);
	SamplingTmr.Disable();
	SamplingTmr.SetCounter(0);
}
} // namespace
#endif

#if IR_RX_ENABLED // ========================== IR RX ==========================
/* ==== Timer ====
______        ________
      |______|
      ^      ^
   TI2FP2   TI2FP1
   Trigger  Capture
   Reset    CCR1 => DMA req CCR1 => TIMx Ch1 (not Ch2!) request
*/
namespace irRcvr {

ftVoidUint32 ICallbackI;
Timer_t TmrRx{TMR_IR_RX};

void Init(ftVoidUint32 CallbackI) {
    ICallbackI = CallbackI;
    PinSetupAlterFunc(IR_RX_DATA_PIN);
    TmrRx.Init();
    TmrRx.SetTopValue(0xFFFF);        // Maximum
    TmrRx.SetupPrescaler(1000000);    // Input Freq: 1 MHz => one tick = 1 uS
    // Setup input capture mode for Channel2
    // Select TI2 as active input for CCR1
    TMR_IR_RX->CCMR1 = (0b10U << 0);
    // Select active polarity for TI2FP1 (capture CCR1) and TI2FP2 (trigger reset):
    // rising and falling edges, respectively (CC1P=0, CC2P=1). Look, TI2FP2 first, TI2FP1 second
    TMR_IR_RX->CCER = TIM_CCER_CC2P;
    // Select trigger input: TI2FP2 (TS = 110)
    TmrRx.SetTriggerInput(tiTI2FP2);
    // Configure slave mode controller in reset mode (SMS = 0100)
    TmrRx.SelectSlaveMode(smReset);
    // Enable the capture: CC1E = 1
    TMR_IR_RX->CCER |= TIM_CCER_CC1E;
    // IRQ
    TmrRx.EnableIrqOnCompare1();
    nvicEnableVector(TMR_IR_RX_IRQ, IRQ_PRIO_HIGH);
    // Start capture
    TmrRx.Enable();
}

// Parsing
static int8_t IBitCnt = -1; // Header not received
static uint32_t IRxData;
static systime_t RxStartTime = 0;

static inline void ProcessDurationI(uint32_t Dur) {
//    PrintfI("%d\r", Dur);
    if(IS_LIKE(Dur, IR_HEADER_uS, IR_DEVIATION_uS)) { // Header rcvd
        IBitCnt = 0;
        IRxData = 0;
        RxStartTime = chVTGetSystemTimeX();
    } else
    // Ignore received if error occured previously
	if(IBitCnt != -1) {
		if(chVTTimeElapsedSinceX(RxStartTime) < TIME_MS2I(IR_RX_PKT_TIMEOUT_MS)) {
			if     (IS_LIKE(Dur, IR_ONE_uS,  IR_DEVIATION_uS)) IRxData |= 1UL << IBitCnt;
            else
            	if(!IS_LIKE(Dur, IR_ZERO_uS, IR_DEVIATION_uS)) { IBitCnt = -1; return; } // Bad duration
            IBitCnt++;
            if(IBitCnt >= IR_BIT_CNT) { // Reception completed
                if(ICallbackI) ICallbackI(IRxData);
                IBitCnt = -1; // Wait header
            }
            else RxStartTime = chVTGetSystemTimeX(); // Restart timeout
//            PrintfI("Bit%i Dur%u\r\n", IBitCnt, Dur);
		}
        else IBitCnt = -1; // timeout occured
    }
}

} // namespace

// ==== IRQ ====
extern "C"
void TMR_IR_RX_IRQ_HNDLR() {
    CH_IRQ_PROLOGUE();
    chSysLockFromISR();
    irRcvr::ProcessDurationI(irRcvr::TmrRx.GetCCR1()); // Reading CCR1 clears IRQ flag, too
    chSysUnlockFromISR();
    CH_IRQ_EPILOGUE();
}
#endif
