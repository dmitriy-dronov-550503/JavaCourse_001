#include "Buttons.h"

//-----------------------------------------
//  Functions definitions
//-----------------------------------------

void Button_Init()
{
    // Setup button S2 and interrupt
    CLEAR_BIT(P2DIR, BIT2);     // Set P2.2 as input
    SET_BIT(P2REN, BIT2);       // Enable pull resistor
    SET_BIT(P2OUT, BIT2);       // Select pull-up
    SET_BIT(P2IES, BIT2);       // Interrupt edge: high -> low
    CLEAR_BIT(P2IFG, BIT2);     // Clear interrupt flag
    SET_BIT(P2IE, BIT2);        // Enable interrupt
}
