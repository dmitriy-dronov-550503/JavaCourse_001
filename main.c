#include "System.h"
#include "Buttons.h"
#include "Timers.h"

//-----------------------------------------
//  Macros
//-----------------------------------------

#define LED_BLINK(PORT, BIT, BLINK_TIME, ACTIVE) static uint32_t count##PORT##BIT = 0; \
    if (ACTIVE) { \
        count##PORT##BIT++; \
        if (count##PORT##BIT == BLINK_TIME) \
        { \
            count##PORT##BIT = 0; \
            INVERT_BIT(PORT, BIT); \
        } \
    } \
    else { CLEAR_BIT(PORT, BIT); }

//-----------------------------------------
//  Shared variables
//-----------------------------------------

volatile uint32_t   timeLeft;           // timeout for button presses counter
volatile uint8_t    buttonPressCount;   // number of button presses

volatile bool isLed1Active;
volatile bool isLed2Active;
volatile bool isLed3Active;

//-----------------------------------------
//  Function declarations
//-----------------------------------------

void InitPorts();

//-----------------------------------------
//  Function definitions
//-----------------------------------------

void InitPorts()
{
    // Initialize all ports
    PADIR = 0;
    PAOUT = 0;
    PBDIR = 0;
    PBOUT = 0;
    PCDIR = 0;
    PCOUT = 0;
    PDDIR = 0;
    PDOUT = 0;

    // Setup LED direction
    SET_BIT(P1DIR, BIT0);   // P1.0 LED 1
    SET_BIT(P8DIR, BIT1);   // P8.1 LED 2
    SET_BIT(P8DIR, BIT2);   // P8.2 LED 3

    // Turn off LEDs
    CLEAR_BIT(P1OUT, BIT0);
    CLEAR_BIT(P8OUT, BIT1);
    CLEAR_BIT(P8OUT, BIT2);
}

int main(void)
{
    WDTCTL = WDTPW + WDTHOLD;   // stop watchdog timer

    InitPorts();
    Button_Init();
    Timer_Init();

    __bis_SR_register(LPM0_bits + GIE);       // Enter LPM0, enable interrupts
    __no_operation();                         // For debugger
}

//-----------------------------------------
//  Interrupts
//-----------------------------------------

// Button S2 interrupt service routine
#pragma vector=PORT2_VECTOR
__interrupt void PORT2_ISR(void)
{
    static uint16_t bounceGuardCount = BOUNCE_TIMEOUT;

    if (!IS_BIT_SET(P2IN, BIT2))
    {
        bounceGuard--;

        if (bounceGuardCount == 0)
        {
            if (!IS_BIT_SET(TA2CCTL0, CCIE))
            {
                // Initialize decrement counter after the first button press
                timeLeft = 1000;    // 1 second (1000 ms) timeout to press the button
                buttonPressCount = 1;
                SET_BIT(TA2CCTL0, CCIE);    // Start timer compare
            }
            else
            {
                // While timer is working counting press times
                buttonPressCount++;
            }
        }
    }
    else
    {
        // Restore bounce guard count
        bounceGuardCount = BOUNCE_TIMEOUT;
        // Reset interrupt flag
        CLEAR_BIT(P2IFG, BIT2);
    }
}

// Timer2 COMP0 interrupt service routine
#pragma vector=TIMER2_A0_VECTOR
__interrupt void TIMER2_A0_ISR(void)
{
    // Decrement each time in interrupt
    timeLeft--;

    if (timeLeft == 0)
    {
        switch (buttonPressCount)
        {
        case 1:
            isLed1Active != isLed1Active;
            break;
        case 2:
            isLed2Active != isLed2Active;
            break;
        case 3:
            isLed3Active != isLed3Active;
            break;
        default:
            break;
        }

        CLEAR_BIT(TA2CCTL0, CCIE); // Stop timer compare
    }

    // TODO: Control 250, 500, 1000 ms?
    LED_BLINK(P1OUT, BIT0, 250, isLed1Active)
    LED_BLINK(P8OUT, BIT1, 500, isLed2Active)
    LED_BLINK(P8OUT, BIT2, 1000, isLed3Active)
}
