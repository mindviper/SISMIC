#include <msp430f5529.h>

//
#define TARGET_INTERVAL 14051  // 1 count = 1us
#define DEBOUNCE_INTERVAL 90 // .1ms debounce interval to compensate for slight hardware deviations
#define high_interval 2360 //2.2ms interval between RISING edge means logic 1
#define low_interval 1237 //1.18ms interval between RISING edge means logic 0
#define smalldebounceinterval 50 //
#define TRUE 1
#define FALSE 0

unsigned int cont = 0;
unsigned int interval = 0;
unsigned int currentCapture = 0;
unsigned int lastCapture = 0; // Stores the last capture value
unsigned int nbits = 32; //number of bits to be captured
unsigned long packet = 0;
unsigned long red_ledON = 0x00FFA25D; //value for red led to turn on
unsigned long red_ledOFF = 0x00FF629D;
unsigned long red_ledTOGGLE = 0x00FFE21D;
unsigned long green_ledON = 0x00FF22DD;
unsigned long green_ledOFF = 0x00FF02FD;
unsigned long green_ledTOGGLE = 0x00FFC23D;
unsigned long mode_change = 0x00FF6897; //asterisk key
unsigned long pwm_inc = 0x00FF18E7;
unsigned long pwm_dec = 0x00FF4AB5;
volatile char mode= 0;
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



/**
 * IMPORTANTE
 *
 * Este projeto utiliza um controle ligeiramente diferente do proposto na proposta do visto 2
 * Como consequência, tive que fazer algumas alterações em sua funcionalidade.
 * Diferentemente do pedido na proposta, ao invés de utilizar as teclas numeradas para escolher
 * o ciclo de carga em incrementos de 10% por tecla, utilizei as setas para incrementar e decrementar o ciclo de carga
 * em incrementos de 10%. Ao pressionar a tecla da seta para cima, o ciclo de carga é aumentado em 10% e reduzido em 10%
 * quando pressionada a tecla da seta para baixo. Abaixo segue um resumo das funcionalidades das teclas:
 *
 * MODO 0:
 *      1: Liga led vermelho
 *      2: Desliga led vermelho
 *      3: Inverte led vermelho
 *      4: Liga led verde
 *      5: Desliga led verde
 *      6: Inverte led verde
 *      *: Altera Modo de operação para o MODO 1
 *
 *
 * MODO 1:
 *      1: Liga led vermelho
 *      2: Desliga led vermelho
 *      3: Inverte led vermelho
 *      4: 100% de ciclo de carga da PWM do led verde
 *      5: 0% de ciclo de carga da PWM do led verde
 *      6: Inverte ciclo de carga da PWM do led verde led verde
 *      UP: Incerementa em 10% o ciclo de carga da PWM do led verde (até  100%)
 *      DOWN: Recuz em 10% o ciclo de carga da PWM do led verde (até 0%)
 *      *: Altera Modo de operação para o MODO 0
 *
 */


// Initialize the system
void main(void) {
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

    setupINT();

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
        if (interval >= (high_interval - DEBOUNCE_INTERVAL) && interval <= (high_interval + DEBOUNCE_INTERVAL)){
            nbits--;
            writeone();
        }
        if (interval >= (low_interval - DEBOUNCE_INTERVAL) && interval <= (low_interval + DEBOUNCE_INTERVAL)){
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
    if(mode == 0){
        if (packet == red_ledON){
            P1OUT |= BIT0; // Turn on red LED
        }
        if (packet == red_ledOFF){
            P1OUT &= ~BIT0; // Turn off red LED
        }
        if (packet == red_ledTOGGLE){
            P1OUT ^= BIT0; // Toggle red LED
        }
        if (packet == green_ledON){
            P4OUT |= BIT7; // Turn on green LED
        }
        if (packet == green_ledOFF){
            P4OUT &= ~BIT7; // Turn off green LED
        }
        if (packet == green_ledTOGGLE){
            P4OUT ^= BIT7; // Toggle green LED
        }
        if (packet == mode_change){
            mode = 1;
            PWM_timer_setup();
        }
    }

    else { // PWM mode
        if (packet == red_ledON){
            P1OUT |= BIT0; // Turn on red LED
        }
        if (packet == red_ledOFF){
            P1OUT &= ~BIT0; // Turn off red LED
        }
        if (packet == red_ledTOGGLE){
            P1OUT ^= BIT0; // Toggle red LED
        }
        if (packet == green_ledON){
            dutycycle = 10000; // Set duty cycle to 100% (completely on)
            TBCCR1 = dutycycle;
        }
        if (packet == green_ledOFF){
            dutycycle = 0; // Set duty cycle to 0%  (completely off)
            TBCCR1 = dutycycle;
        }
        if (packet == green_ledTOGGLE){
            dutycycle = 10000 - dutycycle; // Toggle green LED
            TBCCR1 = dutycycle;
        }
        if (packet == pwm_inc){
            dutycycle += 1000;
            if (dutycycle >= 10000){
                dutycycle = 10000; // Prevent duty cycle from exceeding 100%
            }
            TBCCR1 = dutycycle;
        }
        if (packet == pwm_dec){
            dutycycle -= 1000;
            if (dutycycle <= 0){
                dutycycle = 0; // Prevent duty cycle from going below 0%
            }
            TBCCR1 = dutycycle;
        }
        if (packet == mode_change){
            mode = 0;
            PWM_timer_undo();
        }
    }
}

void PWM_timer_setup(){
    // Configure P4.7 for Timer B0 CCR1 output
    P4DIR |= BIT7;
    P4SEL |= BIT7;

    // Enable write access to port mapping registers
    PMAPKEYID = PMAPKEY;
    P4MAP7 = PM_TB0CCR1A; // Map TB0.1 to P4.7
    PMAPKEYID = 0; // Disable write access to port mapping registers

    // Set Timer B0 to use SMCLK, up mode
    TBCTL = TBSSEL__SMCLK | MC__UP | TBCLR;

    // Set the period of the PWM signal
    TBCCR0 = 10000;

    // Set the duty cycle
    TBCCR1 = dutycycle;

    // Set output mode to Reset/Set
    TBCCTL1 = OUTMOD_7;
}

void PWM_timer_undo(){
    // Enable write access to port mapping registers
    PMAPKEYID = PMAPKEY;
    P4MAP7 = PM_NONE; // Unmap TB0.1 from P4.7
    PMAPKEYID = 0; // Disable write access to port mapping registers

    // Reset Timer B0
    TBCTL = TBCLR;
    TBCCR0 = 0;
    TBCCR1 = 0;
    TBCCTL1 = 0;

    // Set P4.7 as GPIO output
    P4DIR |= BIT7;
    P4SEL &= ~BIT7; // Ensure P4.7 is not selected for peripheral function
    P4OUT &= ~BIT7;
}
