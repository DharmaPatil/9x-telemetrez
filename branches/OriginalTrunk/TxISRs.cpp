#include "telemetrEZ.h"
#include "externalVariables.h"

// ************************************************
// *** Transmit to 9x ***
// ************************************************
#define TxIDLE 0
#define sendSwitchpacket 1
#define sendFrskypacket 2
ISR(USART1__UDRE_vect) {
    static uint8_t SwitchBuf_count;
    static uint8_t U1TXstate = TxIDLE;

    switch(U1TXstate) {
        case TxIDLE:
            if(flags.switchto9x) { // if switch packet is ready to send
                SwitchBuf_count = 0;
                UDR1 = SwitchBuf[SwitchBuf_count++];
                U1TXstate = sendSwitchpacket;
#ifdef BLUETOOTH
                pin16DDR &= ~(1<<IO16); // disable output to bluetooth
#endif
                break;
            }
            if(NinexTx_RB.empty()) { // if the buffer is empty
                    UCSR1B &= ~(1<<UDRIE1); // disable interrupt
                    break;
            }
#ifdef BLUETOOTH
            pin16DDR |= (1<<IO16); // enable output to bluetooth
#endif
            UDR1 = NinexTx_RB.front(); // load next byte from buffer
            NinexTx_RB.pop(); // remove the byte from the buffer
            U1TXstate = sendFrskypacket;
            break;
        case sendSwitchpacket:
            UDR1 = SwitchBuf[SwitchBuf_count++];
#ifdef ROTARYENCODER
	    if(SwitchBuf_count == 5) {
#else
            if(SwitchBuf_count == 3) {
#endif
                U1TXstate = TxIDLE; // done sending switch packet
                flags.switchto9x = 0;
            }
            break;
        case sendFrskypacket:
            if(NinexTx_RB.front() == 0x7e) {
                U1TXstate = TxIDLE;
            }
            UDR1 = NinexTx_RB.front(); // load next byte from buffer
            NinexTx_RB.pop(); // remove the byte from the buffer
            break;
        default:
            U1TXstate = TxIDLE;
            break;
    } // switch
}

// ************************************************
// *** Transmit to Frsky ***
// ************************************************
ISR(USART0__UDRE_vect) {
    static uint8_t U0TXstate = TxIDLE;

    switch(U0TXstate) {
        case TxIDLE:
            if(FrskyTx_RB.empty()) { // if the buffer is empty
                    UCSR0B &= ~(1<<UDRIE0); // disable interrupt
                    break;
            }
            UDR0 = FrskyTx_RB.front(); // load next byte from buffer
            FrskyTx_RB.pop(); // remove the byte from the buffer
            U0TXstate = sendFrskypacket;
            break;
        case sendFrskypacket:
            if(FrskyTx_RB.front() == 0x7e) {
                U0TXstate = TxIDLE;
            }
            UDR0 = FrskyTx_RB.front(); // load next byte from buffer
            FrskyTx_RB.pop(); // remove the byte from the buffer
            break;
    } // switch
}

