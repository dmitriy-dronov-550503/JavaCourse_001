#include <msp430.h>
#include <stdint.h>

//-----------------------------------------
//  Macros
//-----------------------------------------

#define SET_BIT(PORT, BIT) (PORT) |= (BIT)
#define CLEAR_BIT(PORT, BIT) (PORT) &= ~(BIT)
#define INVERT_BIT(PORT, BIT) (PORT) ^= (BIT)
#define IS_BIT_SET(PORT, BIT) ((PORT) & (BIT)) == 1

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
#define CPU_CLK 4000000UL
#define TIMER_DIV 4
#define TIMER_CLK (CPU_CLK / TIMER_DIV)
#define ms (TIMER_CLK / 1000)

//-----------------------------------------
//  Function declarations
//-----------------------------------------

void InitPorts();
void InitTimer2();

inline void Delay(uint32_t count);

void Animation(void);

//-----------------------------------------
//  Function definitions
//-----------------------------------------

void InitPorts()
{
    // Initialize all ports
    PBDIR = 0;
    PBOUT = 0;
    PCDIR = 0;
    PCOUT = 0;
    PDDIR = 0;
    PDOUT = 0;
    P1DIR = 0;
    P1OUT = 0;
    P2DIR = 0;
    P2OUT = 0;

    // Setup LED direction
    P1DIR |= BIT0; // P1.0 LED 1
    P8DIR |= BIT1; // P8.1 LED 2
    P8DIR |= BIT2; // P8.2 LED 3

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
    // TODO: Is it necessary?
    //TA2CCTL0 = CCIE;          // CCR0 interrupt enabled
    TA2CCR0 = 5 * ms;           // Timer threshold
    TA2CTL = TASSEL__SMCLK + ID__4 + TACLR + TAIE;  // SMCLK, div/4, clear TAR, enable interrupt
    CLEAR_BIT(TA2CTL, TAIFG);   // Clear interrupt flag
    SET_BIT(TA2CTL, MC_1);      // Start timer

}

inline void Delay(uint32_t count)
{
    static volatile uint32_t i;
    for(i = count; i; i--);
}

//-----------------------------------------
//  Shared variables
//-----------------------------------------

volatile uint32_t   timeLeft;
volatile uint8_t    buttonPressCount;
volatile uint8_t    isTimerRun;

//-----------------------------------------
//  Main code
//-----------------------------------------

int main(void)
{
    WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT

    isTimerRun = false;

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
    if (!isTimerRun)
    {
        // Initialize decrement counter after the first button press
        isTimerRun = true;
        timeLeft = 100;
        buttonPressCount = 1;
    }
    else
    {
        // While timer is working counting press times
        buttonPressCount++;
    }

    // Reset interrupt flag
    CLEAR_BIT(P2IFG, BIT2);
}

// Timer2 A0 interrupt service routine
#pragma vector=TIMER2_A0_VECTOR
__interrupt void TIMER2_A0_ISR(void)
{
    // States for every LED
    static bool isLed1Active = false;
    static bool isLed2Active = false;
    static bool isLed3Active = false;

    // Active during button timeout
    if (isTimerRun)
    {
        timeLeft--;

        if (timeLeft == 0)
        {
            isTimerRun = false;

            switch (buttonPressCount)
            {
            case 1:
                isLed1Active = !isLed1Active;
                break;
            case 2:
                isLed2Active = !isLed2Active;
                break;
            case 3:
                isLed3Active = !isLed3Active;
                break;
            default:
                break;
            }
        }
    }

    LED_BLINK(P1OUT, BIT0, 50, isLed1Active)
    LED_BLINK(P8OUT, BIT1, 100, isLed2Active)
    LED_BLINK(P8OUT, BIT2, 150, isLed3Active)
}

//-----------------------------------------
//  Miscellaneous
//-----------------------------------------

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
