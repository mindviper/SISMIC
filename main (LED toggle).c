#include <msp430.h>

#define LED_PIN BIT0
#define BUTTON_PIN1 BIT1
#define BUTTON_PIN2 BIT1


#define TRUE 1
#define FALSE 0
#define ABERTA 1 //Chave aberta
#define FECHADA 0 //Chave fechada
#define DBC 1000 //Sugestão para o debounce


int mon_s1(void);
int mon_s2(void);


int main(void){

     WDTCTL = WDTPW | WDTHOLD;


    // Configure the LED pin as output
    P1DIR |= LED_PIN;
    // Configure the button pins as inputs with pull-up resistors
    P1REN |= BUTTON_PIN1;
    P1OUT |= BUTTON_PIN1; // Enable pull-up resistor for button pin 1
    P2DIR &= ~BUTTON_PIN2;
    P2REN |= BUTTON_PIN2;
    P2OUT |= BUTTON_PIN2; // Enable pull-up resistor for button pin 2


    while(1) {
        // Check if either switch has been pressed
        if (mon_s1() == TRUE || mon_s2() == TRUE) {
            // Toggle the LED
            P1OUT ^= LED_PIN;
            // Wait for a short debounce period
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
               // __delay_cycles(5000);
                ps1=FECHADA;
                return TRUE;
            }
    }
    else
    {
        if (ps1==FECHADA)
        { //Qual o passado de S1?
           // __delay_cycles(5000);
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
           // __delay_cycles(5000);
            ps2=FECHADA;
            return TRUE;
        }
    }
    else{
        if (ps2==FECHADA){ //Qual o passado de S2?
           // __delay_cycles(5000);
            ps2=ABERTA;
            return FALSE;
        }
    }
    return FALSE;
}



