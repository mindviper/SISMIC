#include <msp430.h>

// Define the target interval in timer counts (13.4ms)
#define TARGET_INTERVAL 14051  // Assuming 1 count = 1us
#define DEBOUNCE_INTERVAL 90 // .1ms debounce interval to ignore spurious edges
#define high_interval 2360 //2.2ms interval between RISING edge means logic 1
#define low_interval 1237 //1.18ms interval between RISING edge means logic 0
#define smalldebounceinterval 50 //
#define TRUE 1
#define FALSE 0

volatile unsigned int cont;
volatile unsigned int interval = 0;
volatile unsigned int currentCapture = 0;
volatile unsigned int lastCapture = 0; // Stores the last capture value
volatile unsigned int debounceTimer = 0; // Debounce timer to prevent spurious toggles
volatile unsigned int nbits = 32; //number of bits to be captured
volatile unsigned long packet = 0;
volatile unsigned long red_ledON = 0x00FFA25D; //value for red led to turn on
volatile unsigned long red_ledOFF = 0x00FF629D;
volatile unsigned long red_ledTOGGLE = 0x00FFE21D;
volatile unsigned long green_ledON = 0x00FF22DD;
volatile unsigned long green_ledOFF = 0x00FF02FD;
volatile unsigned long green_ledTOGGLE = 0x00FFC23D;
volatile unsigned long mode_change = 0x00FF6897; //asterisk key
volatile char mode= 0;

volatile char partok = FALSE;

//1010 0100

void setupINT();
//void configureTimer();
void setuppacketcapture();
void writezero();
void writeone();
void intrestore();
void effect();


// Initialize the system
void main(void) {
    // Stop watchdog timer
    WDTCTL = WDTPW | WDTHOLD;

    TA0CTL = TASSEL_2 | MC_2; // SMCLK, continuous mode
    //    TA0CCTL1 = CM_0; // No capture mode
    //    P1DIR |= BIT2; // P1.2 as output (optional)
    //    P1SEL |= BIT2; // Dedicated for TA0.1 (optional)

    // Set P1.0 as output (red LED)
    P1DIR |= BIT0;
    P1OUT &= ~BIT0; // Initialize P1.0 to low (LED off)


    //set P4.7 as output (green LED)
    P4DIR |= BIT7;
    P4OUT &= ~BIT0; //Initialize P4.7 to low

    // Set P2.0 as input
    P2DIR &= ~BIT0;
    P2REN |= BIT0; // Enable pull-up/pull-down resistor
    P2OUT |= BIT0; // Enable pull-up resistor

    setupINT();
    //    configureTimer();

    _enable_interrupt();
    while(1) {   // Infinite loop waiting for interrupt


    }
}

// Port 2 interrupt service routine
#pragma vector=PORT2_VECTOR
__interrupt void PORT2_ISR(void) {
    // Capture current timer value
    currentCapture = TA0R;
    interval = currentCapture - lastCapture;

    //    cont++;
    //    if (cont == 2)
    //        cont = 0;
    if (partok == FALSE){
        // Check if interval is within target range

        if (interval >= (TARGET_INTERVAL - DEBOUNCE_INTERVAL) && interval <= (TARGET_INTERVAL + DEBOUNCE_INTERVAL)) {
            setuppacketcapture();
            //            P1OUT ^= BIT0; //debugging led
            partok = TRUE;
            //
        }
    }
    else{
        if (interval >= (high_interval - DEBOUNCE_INTERVAL) && interval <= (high_interval + DEBOUNCE_INTERVAL)){
            nbits = nbits - 1; //decrements number of bits left
            writeone(); //writes one to the packet

        }

        if (interval >= (low_interval - DEBOUNCE_INTERVAL) && interval <= (low_interval + DEBOUNCE_INTERVAL)){
            nbits = nbits - 1; //decrements number of bits left
            writezero(); //writes zero to the packet

        }
    }


    if (nbits == 0){
        partok = FALSE;
        effect(); //decides what should happen after the packet is received
        intrestore(); //restores interruption parameters
    }
    // Update last capture value
    lastCapture = currentCapture;



    // Clear interrupt flag for P2.0
    P2IFG &= ~BIT0;
}

void setupINT() {
    // Configure interrupt on P2.0 for falling edge
    P2IE |= BIT0;   // Enable interrupt on P2.0
    P2IES |= BIT0;  // Set interrupt edge select to falling edge
    P2IFG &= ~BIT0; // Clear interrupt flag for P2.0
}

//void configureTimer(void) {
//    Configure Timer A0
//    TA0CTL = TASSEL_2 | MC_2; // SMCLK, continuous mode
//    TA0CCTL1 = CM_0; // No capture mode
//   P1DIR |= BIT2; // P1.2 as output (optional)
//    P1SEL |= BIT2; // Dedicated for TA0.1 (optional)
//}



void setuppacketcapture(){
    P2IES ^= BIT0; //sets up interrupt for RISING edge
    P2IFG &= ~BIT0; //clear interrupt flag
}

void writezero(){ //writes 0 to the packet
    packet = packet << 1; //shifts bits to the left
}

void writeone(){ //writes 1 to the packet
    packet = (packet << 1) | 1; //shifts bits to the left and does bitwose OR op
}


void intrestore(){ //restores the interrupt to initial configs
    nbits = 32;
    P2IES |= BIT0;

}


////////////// Use seitch case (better optimised) ////////////////

void effect(){ //decides what should happen after packet is fully received
//    if(mode == 0){
        if (packet == red_ledON){
            P1OUT |= BIT0; // Toggle LED
        }

        if (packet == red_ledOFF){
            P1OUT &= ~BIT0;
        }
        if (packet == red_ledTOGGLE){
            P1OUT ^= BIT0;
        }
        if (packet == green_ledON){
            P4OUT |= BIT7; // Toggle LED
        }

        if (packet == green_ledOFF){
            P4OUT &= ~BIT7;
        }
        if (packet == green_ledTOGGLE){
            P4OUT ^= BIT7;
        }
        if (packet == mode_change){
            mode = 1;
        }
//    }
    if (mode == 1){ //PWM mode

    }
}
