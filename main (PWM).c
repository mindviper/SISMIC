#include <msp430.h>

#define TRUE 1
#define FALSE 0
#define ABERTA 1 //Chave aberta
#define FECHADA 0 //Chave fechada
#define DBC 1000 //Sugestão para o debounce

// Function prototypes

int mon_s1(void);
int mon_s2(void);

void configurePWM(void);

// Global variables to track duty cycle
volatile int dutyCycle = 10000; // initial duty cycle

int main(void) {
    WDTCTL = WDTPW | WDTHOLD; // Stop watchdog timer

    P1REN |= BIT1;
    P1OUT |= BIT1;
    P2OUT |= BIT1;
    P2REN |= BIT1;
    // Configure PWM
    configurePWM();

    // Main loop
    while(1) {
        // Monitor buttons and adjust duty cycle
        if (mon_s1() == TRUE)
        {
            if (dutyCycle > 0)
            {
                dutyCycle -= 1250;
                configurePWM();// decrease duty cycle by 1/8
                // Ensure duty cycle doesn't go negative
                if (dutyCycle < 0)
                    dutyCycle = 0;
                    configurePWM(); // Update PWM duty cycle
            }
        }
        if (mon_s2() == TRUE)
        {
            if (dutyCycle < 10485)
            { // Adjust the limit based on your requirement
                dutyCycle += 1250; // increase duty cycle by 1/8
                configurePWM();// Ensure duty cycle doesn't exceed maximum value
                if (dutyCycle > 10000) // Adjust the maximum value based on your requirement
                    dutyCycle = 10000;
                    configurePWM(); // Update PWM duty cycle
            }
        }
    }
    return 0;
}


int mon_s1(void){
    static int ps1=ABERTA; //Guardar passado de S1
    if ( (P2IN&BIT1) == 0)
    { //Qual estado atual de S1?
            if (ps1==ABERTA)
            { //Qual o passado de S1?
                __delay_cycles(5000);
                ps1=FECHADA;
                return TRUE;
            }
    }
    else
    {
        if (ps1==FECHADA)
        { //Qual o passado de S1?
            __delay_cycles(5000);
            ps1=ABERTA;
            return FALSE;
        }
    }
    return FALSE;
}

int mon_s2(void){
    static int ps2=ABERTA; //Guardar passado de S2
    if ( (P1IN&BIT1) == 0){ //Qual estado atual de S2?
        if (ps2==ABERTA){ //Qual o passado de S2?
            __delay_cycles(5000);
            ps2=FECHADA;
            return TRUE;
        }
    }
    else{
        if (ps2==FECHADA){ //Qual o passado de S2?
            __delay_cycles(5000);
            ps2=ABERTA;
            return FALSE;
        }
    }
    return FALSE;
}



void configurePWM(void) {

    //Configuração de TA0 e P1.2
    TA0CTL = TASSEL_2 | MC_1; //SMCLK e Modo 1
    TA0CCR0 = 10485; //Limite de contagem (10 ms -> 100 Hz)
    TA0CCTL1 = OUTMOD_6; //Modo da saída
    P1DIR |= BIT2; //P1.2 como saída
    P1SEL |= BIT2; //Dedicado para TA0.1
    TA0CCR1 = dutyCycle;
}


