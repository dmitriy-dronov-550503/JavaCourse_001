#include "Timers.h"

//-----------------------------------------
//  Macros
//-----------------------------------------

#define nsec_to_hz(ns) 1000000/ns

//-----------------------------------------
//  Functions definitions
//-----------------------------------------

void Timer_Init(void)
{
    // Button press count interval
    //TA2CCTL0 = CCIE;                // CCR0 interrupt enabled
    TA2CCR0 = nsec_to_hz(10000);    // Timer COMP0 interval 10 ms

    // LED control interval
    TA2CCTL1 = CCIE;                // CCR1 interrupt enabled
    TA2CCR1 = nsec_to_hz(1000);     // Timer COMP1 interval 1 ms

    // Setup and start timer
    TA2CTL = TASSEL__SMCLK + ID__4 + TACLR + MC__CONTINOUS;
}

