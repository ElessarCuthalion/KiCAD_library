/*
 * ir_el.h
 *
 *  Created on: 01.09.2023
 *      Author: Eless
 */

#pragma once

#include "kl_lib.h"
#include "board.h"

#define IR_TX_ENABLED   TRUE
#define IR_RX_ENABLED   TRUE

// Delays, uS
#define IR_HEADER_uS        1800UL//2400
#define IR_SPACE_uS         600UL
#define IR_ZERO_uS          600UL
#define IR_ONE_uS           1200UL
#define IR_PAUSE_AFTER_uS   1800UL//2400

#if IR_TX_ENABLED // ========================== IR TX ==========================
#define IR_MAX_PWR          255     // Top DAC value

namespace irLed {
    void Init();
    void TransmitWord(uint32_t wData, uint8_t PowerPercent, ftVoidVoid CallbackI);
    void ReTransmit(uint8_t PowerPercent);
    void ResetI();
} // namespace
#endif

#if IR_RX_ENABLED // ========================== IR RX ==========================
//#define IR_RX_POLLING_PERIOD_MS     90
#define IR_DEVIATION_uS             300//150
#define IR_RX_PKT_TIMEOUT_MS        4

namespace irRcvr {
    void Init(ftVoidUint32 CallbackI);
} // namespace
#endif
