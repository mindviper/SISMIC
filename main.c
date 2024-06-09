#include <msp430f5529.h>

// Define an enum for button types
typedef enum {
    RED_LED_ON = 0x00FFA25D,
    RED_LED_OFF = 0x00FF629D,
    RED_LED_TOGGLE = 0x00FFE21D,
    GREEN_LED_ON = 0x00FF22DD,
    GREEN_LED_OFF = 0x00FF02FD,
    GREEN_LED_TOGGLE = 0x00FFC23D,
    MODE_CHANGE = 0x00FF6897,
    PWM_INC = 0x00FF18E7,
    PWM_DEC = 0x00FF4AB5
} ButtonType;

#define TARGET_INTERVAL 14051  // 1 count = 1us
#define DEBOUNCE_INTERVAL 90 // .1ms debounce interval to compensate for slight hardware deviations
#define HIGH_INTERVAL 2360 //2.2ms interval between RISING edge means logic 1
#define LOW_INTERVAL 1237 //1.18ms interval between RISING edge means logic 0
#define SMALL_DEBOUNCE_INTERVAL 50 //
#define TRUE 1
#define FALSE 0

unsigned int cont = 0;
unsigned int interval = 0;
unsigned int currentCapture = 0;
unsigned int lastCapture = 0; // Stores the last capture value
unsigned int nbits = 32; //number of bits to be captured
unsigned long packet = 0;
volatile char mode = 0;
int dutycycle = 5000;
volatile char partok = FALSE;

// Function prototypes
void setupINT();
void setuppacketcapture();
void writezero();
void writeone();
void intrestore();
void effect();
void PWM_timer_setup();
void PWM_timer_undo();
void Init();

void main(void) {

    Init(); // Initialize

    setupINT(); // Sets up interrupts

    _enable_interrupt();

    while(1) {
        // Infinite loop waiting for interrupt
    }
}

// Port 2 interrupt service routine
#pragma vector=PORT2_VECTOR
__interrupt void PORT2_ISR(void) {
    // Capture current timer value
    currentCapture = TA0R;
    interval = currentCapture - lastCapture;

    if (partok == FALSE){
        // Check if interval is within target range
        if (interval >= (TARGET_INTERVAL - DEBOUNCE_INTERVAL) && interval <= (TARGET_INTERVAL + DEBOUNCE_INTERVAL)) {
            setuppacketcapture();
            partok = TRUE;
        }
    } else {
        if (interval >= (HIGH_INTERVAL - DEBOUNCE_INTERVAL) && interval <= (HIGH_INTERVAL + DEBOUNCE_INTERVAL)){
            nbits--;
            writeone();
        }
        if (interval >= (LOW_INTERVAL - DEBOUNCE_INTERVAL) && interval <= (LOW_INTERVAL + DEBOUNCE_INTERVAL)){
            nbits--;
            writezero();
        }
    }

    if (nbits == 0){
        partok = FALSE;
        effect();
        intrestore();
    }

    // Update last capture value
    lastCapture = currentCapture;

    // Clear interrupt flag for P2.0
    P2IFG &= ~BIT0;
}

void Init(){

    // Stop watchdog timer
    WDTCTL = WDTPW | WDTHOLD;

    // Set Timer A0 to use SMCLK, continuous mode
    TA0CTL = TASSEL_2 | MC_2;

    // Set P1.0 as output (red LED)
    P1DIR |= BIT0;
    P1OUT &= ~BIT0; // Initialize P1.0 to low (LED off)

    // Set P4.7 as output (green LED)
    P4DIR |= BIT7;
    P4OUT &= ~BIT7; // Initialize P4.7 to low

    // Set P2.0 as input with pull-up resistor
    P2DIR &= ~BIT0;
    P2REN |= BIT0;
    P2OUT |= BIT0;

}

void setupINT() {
    // Configure interrupt on P2.0 for falling edge
    P2IE |= BIT0;   // Enable interrupt on P2.0
    P2IES |= BIT0;  // Set interrupt edge select to falling edge
    P2IFG &= ~BIT0; // Clear interrupt flag for P2.0
}

void setuppacketcapture(){
    P2IES ^= BIT0; // Set interrupt for RISING edge
    P2IFG &= ~BIT0; // Clear interrupt flag
}

void writezero(){
    packet = packet << 1; // Shift bits to the left
}

void writeone(){
    packet = (packet << 1) | 1; // Shift bits to the left and do bitwise OR operation
}

void intrestore(){
    nbits = 32;
    P2IES |= BIT0;
}

void effect(){
    switch (mode) {
        case 0:
            switch ((ButtonType)packet) {
                case RED_LED_ON:
                    P1OUT |= BIT0; // Turn on red LED
                    break;
                case RED_LED_OFF:
                    P1OUT &= ~BIT0; // Turn off red LED
                    break;
                case RED_LED_TOGGLE:
                    P1OUT ^= BIT0; // Toggle red LED
                    break;
                case GREEN_LED_ON:
                    P4OUT |= BIT7; // Turn on green LED
                    break;
                case GREEN_LED_OFF:
                    P4OUT &= ~BIT7; // Turn off green LED
                    break;
                case GREEN_LED_TOGGLE:
                    P4OUT ^= BIT7; // Toggle green LED
                    break;
                case MODE_CHANGE:
                    mode = 1;
                    PWM_timer_setup();
                    break;
                default:
                    break;
            }
            break;
        case 1: // PWM mode
            switch ((ButtonType)packet) {
                case RED_LED_ON:
                    P1OUT |= BIT0; // Turn on red LED
                    break;
                case RED_LED_OFF:
                    P1OUT &= ~BIT0; // Turn off red LED
                    break;
                case RED_LED_TOGGLE:
                    P1OUT ^= BIT0; // Toggle red LED
                    break;
                case GREEN_LED_ON:
                    dutycycle = 10000; // Set duty cycle to 100% (completely on)
                    TBCCR1 = dutycycle;
                    break;
                case GREEN_LED_OFF:
                    dutycycle = 0; // Set duty cycle to 0%  (completely off)
                    TBCCR1 = dutycycle;
                    break;
                case GREEN_LED_TOGGLE:
                    dutycycle = 10000 - dutycycle; // Toggle green LED
                    TBCCR1 = dutycycle;
                    break;
                case PWM_INC:
                    dutycycle += 1000;
                    if (dutycycle >= 10000){
                        dutycycle = 10000; // Prevent duty cycle from exceeding 100%
                    }
                    TBCCR1 = dutycycle;
                    break;
                case PWM_DEC:
                    dutycycle -= 1000;
                    if (dutycycle <= 0){
                        dutycycle = 0; // Prevent duty cycle from going below 0%
                    }
                    TBCCR1 = dutycycle;
                    break;
                case MODE_CHANGE:
                    mode = 0;
                    PWM_timer_undo(); // Turns off timer and unmaps TB0.1 from P4.7
                    break;
                default:
                    break;
            }
            break;
    }
}

void PWM_timer_setup(){
    // Configure P4.7 for Timer B0 CCR1 output
    P4DIR |= BIT7;
    P4SEL |= BIT7;

    PMAPKEYID = PMAPKEY;
    P4MAP7 = PM_TB0CCR1A; // Map TB0.1 to P4.7
    PMAPKEYID = 0;

    // Timer setup
    TBCTL = TBSSEL__SMCLK | MC__UP | TBCLR;
    TBCCR0 = 10000; // Aprox 10ms
    TBCCR1 = dutycycle;
    TBCCTL1 = OUTMOD_7;
}

void PWM_timer_undo(){
    PMAPKEYID = PMAPKEY;
    P4MAP7 = PM_NONE;
    PMAPKEYID = 0;

    // Reset Timer B0
    TBCTL = TBCLR;
    TBCCR0 = 0;
    TBCCR1 = 0;
    TBCCTL1 = 0;

    // Set P4.7 as output
    P4DIR |= BIT7;
    P4SEL &= ~BIT7;
    P4OUT &= ~BIT7;
}
