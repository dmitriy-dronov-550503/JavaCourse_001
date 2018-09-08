#include <msp430.h>
#include <stdint.h>

//-----------------------------------------
//  Macros
//-----------------------------------------

#define SET_BIT(PORT, BIT) (PORT) |= (BIT)
#define CLEAR_BIT(PORT, BIT) (PORT) &= ~(BIT)
#define INVERT_BIT(PORT, BIT) (PORT) ^= (BIT)
#define IS_BIT_SET(PORT, BIT) (((PORT) & (BIT)) == 1)

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
//  Type definitions
//-----------------------------------------

typedef uint8_t bool;

//-----------------------------------------
//  Constants
//-----------------------------------------

#define true 1
#define false 0
#define nsec_to_hz(ns) 1000000/ns

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
void InitTimer2();

void Delay(uint32_t count);
void Animation(void);

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

    // Setup button S2 and interrupt
    P2DIR &= ~BIT2;             // Set P2.2 as input
    SET_BIT(P2REN, BIT2);       // Enable pull resistor
    SET_BIT(P2OUT, BIT2);       // Select pull-up
    SET_BIT(P2IES, BIT2);       // Interrupt edge: high -> low
    CLEAR_BIT(P2IFG, BIT2);     // Clear interrupt flag
    SET_BIT(P2IE, BIT2);        // Enable interrupt
}

void InitTimer2()
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

int main(void)
{
    WDTCTL = WDTPW + WDTHOLD;   // stop watchdog timer

    InitPorts();

    Animation();

    InitTimer2();

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
    if (!IS_BIT_SET(TA2CCTL0, CCIE))
    {
        // Initialize decrement counter after the first button press
        timeLeft = 100; // 1 second timeout to press button
        buttonPressCount = 1;
        SET_BIT(TA2CCTL0, CCIE); // Start timer compare
    }
    else
    {
        // While timer is working counting press times
        buttonPressCount++;
    }

    // Reset interrupt flag
    CLEAR_BIT(P2IFG, BIT2);
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
}

// Timer2 COMP1 interrupt service routine
#pragma vector=TIMER2_A1_VECTOR
__interrupt void TIMER2_A1_ISR(void)
{
    // TODO: Control 250, 500, 1000 ms?
    LED_BLINK(P1OUT, BIT0, 250, isLed1Active)
    LED_BLINK(P8OUT, BIT1, 500, isLed2Active)
    LED_BLINK(P8OUT, BIT2, 1000, isLed3Active)
}

//-----------------------------------------
//  Miscellaneous
//-----------------------------------------

void Delay(uint32_t count)
{
    static volatile uint32_t i;
    for(i = count; i; i--);
}

void Animation(void)
{
    CLEAR_BIT(P1OUT, BIT0);
    CLEAR_BIT(P8OUT, BIT1);
    CLEAR_BIT(P8OUT, BIT2);
    Delay(10000);
    uint8_t i, j;
    for (i = 2; i; i--)
    {
        for (j = 2; j; j--)
        {
            INVERT_BIT(P1OUT, BIT0);
            Delay(5000);
            INVERT_BIT(P8OUT, BIT1);
            Delay(5000);
            INVERT_BIT(P8OUT, BIT2);
            Delay(5000);
        }
        Delay(5000);
        for (j = 2; j; j--)
        {
            INVERT_BIT(P8OUT, BIT2);
            Delay(5000);
            INVERT_BIT(P8OUT, BIT1);
            Delay(5000);
            INVERT_BIT(P1OUT, BIT0);
            Delay(5000);
        }
        Delay(5000);
    }
}
