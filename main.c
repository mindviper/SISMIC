#include <msp430.h>
#include <stdint.h>

/**
 * Pinout
 *
 * RC522 | MSP430F5529
         |
 * SDA   | P3.3
 * SCK   | P3.2
 * MOSI  | P3.0
 * MISO  | P3.1
 * RQ    | Not Connected
 * GND   | GND
 * RST   | P2.0
 * 3.3V  | 3.3V
 *
 *
 * LCD    | MSP430F5529
 *        |
 *  GND   | GND
 * VCC    | 5V
 * SDA    | P4.1
 * SCL    | P4.2
 *
 *
 */

#define PCF_ADR1 0x3F
#define PCF_ADR2 0x27
#define PCF_ADR  PCF_ADR1

#define LED_PIN BIT0
#define BUTTON_PIN1 BIT1
#define BUTTON_PIN2 BIT1
#define BUTTON_PIN8 BIT1


#define TRUE 1
#define FALSE 0
#define ABERTA 1 //Chave aberta
#define FECHADA 0 //Chave fechada
#define DBC 1000 //Sugestão para o debounce


#define BR_100K    11  //SMCLK/100K = 11

unsigned char ms9;
unsigned char msD;
unsigned char ls9;
unsigned char lsD;

void init_all();
int mon_s1(void);
int mon_s2(void);
int mon_button(void);
void lcd_inic(void);
void lcd_aux(char dado);
int pcf_read(void);
void pcf_write(char dado);
int pcf_teste(char adr);
void led_vd(void);
void led_VD(void);
void led_vm(void);
void led_VM(void);
void i2c_config(void);
void gpio_config(void);
void delay(long limite);
int string_length(const char* str);
int get_ascii_value(char c);
int ms_byte_9(char c);
int ms_byte_D(char c);
int ls_byte_9(char c);
int ls_byte_D(char c);
void char_build(char c);
void char_print(void);
void lcd_send_command(char command);
void lcd_clear(void);
void lcd_set_cursor(int row, int col);
void print_item(unsigned int index);
void reset_timer(void);
void start_timer(void);
void stop_timer(void);


unsigned int seconds = 0;
unsigned int thirtysecs = 0;
int wait = 0;


//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

#define TRUE    1
#define FALSE   0

#define CS_PIN BIT3
#define CS_PORT P3OUT
#define CS_DIR P3DIR

// RC522 Commands
#define PCD_IDLE 0x00
#define PCD_AUTHENT 0x0E
#define PCD_RECEIVE 0x08
#define PCD_TRANSMIT 0x04
#define PCD_TRANSCEIVE 0x0C
#define PCD_RESETPHASE 0x0F
#define PCD_CALCCRC 0x03

// Mifare_One card command word
#define PICC_REQIDL 0x26
#define PICC_REQALL 0x52
#define PICC_ANTICOLL 0x93
#define PICC_SElECTTAG 0x93
#define PICC_AUTHENT1A 0x60
#define PICC_AUTHENT1B 0x61
#define PICC_READ 0x30
#define PICC_WRITE 0xA0
#define PICC_DECREMENT 0xC0
#define PICC_INCREMENT 0xC1
#define PICC_RESTORE 0xC2
#define PICC_TRANSFER 0xB0
#define PICC_HALT 0x50

#define MI_OK 0
#define MI_NOTAGERR 1
#define MI_ERR 2


const uint8_t brake_pads[5] = {0x33, 0x34, 0x07, 0x35, 0x35};
const uint8_t ignition_coils[5] = {0x83, 0x80, 0x87, 0x34, 0xB0};
const uint8_t oil_filter[5] = {0x43, 0x65, 0x88, 0x34, 0x9A};
const uint8_t component_box1[5] = {0xF3, 0x18, 0x95, 0x34, 0x4A};
const uint8_t fueltech_ecu[5] = {0x03, 0xA7, 0x3B, 0x08, 0x97};
const uint8_t drexler_diff[5] = {0x63, 0x08, 0x7B, 0x34, 0x24};
const uint8_t engine_blocks[5] = {0x55, 0x9C, 0xF9, 0x2A, 0x1A};
const uint8_t modo_consulta[5] = {0x03, 0xAC, 0x0F, 0x14, 0xB4};

char itemNames[8][16] = {"Brake Pads", "Ignition Coils", "Oil Filters", "Component Box 1", "FuelTech600", "Drexler", "Fuel Injectors", "Scan Item Tag"};

#pragma PERSISTENT(itemQuantities)
volatile char itemQuantities[8] = {4, 4, 1, 0, 3, 1, 3, 0};


#define MAX_LEN 16

void SPI_Init(void);
void GPIO_Init(void);
void RC522_Init(void);
uint8_t SPI_Transfer(uint8_t data);
void RC522_Select(void);
void RC522_Deselect(void);
void RC522_Reset(void);
void RC522_WriteRegister(uint8_t addr, uint8_t val);
uint8_t RC522_ReadRegister(uint8_t addr);
void RC522_SetBitMask(uint8_t reg, uint8_t mask);
void RC522_ClearBitMask(uint8_t reg, uint8_t mask);
void RC522_AntennaOn(void);
uint8_t RC522_Request(uint8_t reqMode, uint8_t *TagType);
uint8_t RC522_Anticoll(uint8_t *serNum);
uint8_t RC522_ToCard(uint8_t command, uint8_t *sendData, uint8_t sendLen, uint8_t *backData, uint8_t *backLen);
uint8_t compareArrays(const uint8_t *array1, const uint8_t *array2, uint8_t length);

uint8_t cardID[5];


int reset();

int main(void) {
    WDTCTL = WDTPW | WDTHOLD;

    init_all(); // Contains all the initializations required

    uint8_t status;
    uint8_t str[MAX_LEN];
    uint8_t i = 0;

    if (pcf_teste(PCF_ADR) == FALSE){
        led_VM();           // Means there was no ACK
        while(TRUE);        // stop
    } else {
        led_VD();           // ACK received, all good
        P4OUT &= ~BIT7;
    }

    lcd_inic();     // LCD initialization
    pcf_write(8);   // Turn on backlight

    lcd_clear();

    int home_screen = 0;
    while (1) {

        if (home_screen == 0){
            home_screen = 1;
            print_item(7);
            start_timer();
        }
        wait = 1;  // Variable for quantity control
        status = RC522_Request(PICC_REQIDL, str);
        if (status == MI_OK) {
            status = RC522_Anticoll(str);
            if (status == MI_OK) {
                stop_timer();
                home_screen = 0;
                pcf_write(8); //turn on backlight again
                // Copy the card ID
                for (i = 0; i < 5; i++) {
                    cardID[i] = str[i];

                }
                if (compareArrays(cardID, brake_pads, 5)){

                    lcd_clear();
                    print_item(0);
                    seconds = 0;

                    while (wait){


                        if (mon_button() == TRUE){
                            P1OUT ^= BIT0;
                            P4OUT ^= BIT7;

                            wait = 0;
                            thirtysecs = 0;
                            lcd_clear();
                        }
                        if (mon_s1() == TRUE){
                            itemQuantities[0] = itemQuantities[0] - 1;
                            print_item(0);
                            thirtysecs = 0;
                        }
                        if (mon_s2() == TRUE){
                            itemQuantities[0] = itemQuantities[0] + 1;
                            print_item(0);
                            thirtysecs = 0;
                        }

                    }

                }
                if (compareArrays(cardID, ignition_coils, 5)){
                    P4OUT ^= BIT7;
                    lcd_clear();
                    print_item(1);
                    seconds = 0;

                    //WDT_kick();

                    while (wait){

                        //WDT_kick();

                        if (mon_button() == TRUE){
                            P1OUT ^= BIT0;
                            P4OUT ^= BIT7;

                            wait = 0;

                            lcd_clear();
                            //                        __delay_cycles(1000000);
                        }
                        if (mon_s1() == TRUE){
                            itemQuantities[1] = itemQuantities[1] - 1;
                            print_item(1);
                        }
                        if (mon_s2() == TRUE){
                            itemQuantities[1] = itemQuantities[1] + 1;
                            print_item(1);
                        }

                    }

                }
                if (compareArrays(cardID, oil_filter, 5)){
                    P4OUT ^= BIT7;
                    lcd_clear();
                    print_item(2);
                    seconds = 0;

                    //WDT_kick();

                    while (wait){

                        //WDT_kick();

                        if (mon_button() == TRUE){
                            P1OUT ^= BIT0;
                            P4OUT ^= BIT7;

                            wait = 0;

                            lcd_clear();
                            //                        __delay_cycles(1000000);
                        }
                        if (mon_s1() == TRUE){
                            itemQuantities[2] = itemQuantities[2] - 1;
                            print_item(2);
                        }
                        if (mon_s2() == TRUE){
                            itemQuantities[2] = itemQuantities[2] + 1;
                            print_item(2);
                        }

                    }

                }

                if (compareArrays(cardID, component_box1, 5)){
                    P4OUT ^= BIT7;
                    lcd_clear();
                    print_item(3);
                    seconds = 0;

                    //WDT_kick();

                    while (wait){

                        //WDT_kick();

                        if (mon_button() == TRUE){
                            P1OUT ^= BIT0;
                            P4OUT ^= BIT7;

                            wait = 0;

                            lcd_clear();
                            //                        __delay_cycles(1000000);
                        }
                        if (mon_s1() == TRUE){
                            itemQuantities[3] = itemQuantities[3] - 1;
                            print_item(3);
                        }
                        if (mon_s2() == TRUE){
                            itemQuantities[3] = itemQuantities[3] + 1;
                            print_item(3);
                        }

                    }

                }
                if (compareArrays(cardID, fueltech_ecu, 5)){
                    P4OUT ^= BIT7;
                    lcd_clear();
                    print_item(4);
                    seconds = 0;

                    //WDT_kick();

                    while (wait){

                        //WDT_kick();

                        if (mon_button() == TRUE){
                            P1OUT ^= BIT0;
                            P4OUT ^= BIT7;

                            wait = 0;


                            lcd_clear();
                            //                        __delay_cycles(1000000);
                        }
                        if (mon_s1() == TRUE){
                            itemQuantities[4] = itemQuantities[4] - 1;
                            print_item(4);
                        }
                        if (mon_s2() == TRUE){
                            itemQuantities[4] = itemQuantities[4] + 1;
                            print_item(4);
                        }

                    }

                }
                if (compareArrays(cardID, drexler_diff, 5)){
                    P4OUT ^= BIT7;
                    lcd_clear();
                    print_item(5);
                    seconds = 0;

                    //WDT_kick();

                    while (wait){

                        //WDT_kick();

                        if (mon_button() == TRUE){
                            P1OUT ^= BIT0;
                            P4OUT ^= BIT7;

                            wait = 0;

                            lcd_clear();
                            //                        __delay_cycles(1000000);
                        }
                        if (mon_s1() == TRUE){
                            itemQuantities[5] = itemQuantities[5] - 1;
                            print_item(5);
                        }
                        if (mon_s2() == TRUE){
                            itemQuantities[5] = itemQuantities[5] + 1;
                            print_item(5);
                        }

                    }

                }
                if (compareArrays(cardID, engine_blocks, 5)){
                    P4OUT ^= BIT7;
                    lcd_clear();
                    print_item(6);
                    seconds = 0;


                    while (wait){

                        //WDT_kick();

                        if (mon_button() == TRUE){
                            P1OUT ^= BIT0;
                            P4OUT ^= BIT7;

                            wait = 0;

                            lcd_clear();
                            //                        __delay_cycles(1000000);
                        }
                        if (mon_s1() == TRUE){
                            itemQuantities[6] = itemQuantities[6] - 1;
                            print_item(6);
                        }
                        if (mon_s2() == TRUE){
                            itemQuantities[6] = itemQuantities[6] + 1;
                            print_item(6);
                        }

                    }

                }
                if (compareArrays(cardID, modo_consulta, 5)){
                    P4OUT ^= BIT7;
                    lcd_clear();
                    //WDT_kick();
                    seconds = 0;

                    int index = 0;

                    print_item(0);

                    while (wait){

                        //WDT_kick();

                        if (mon_button() == TRUE){
                            P1OUT ^= BIT0;
                            P4OUT ^= BIT7;

                            wait = 0;

                            lcd_clear();
                            //                        __delay_cycles(1000000);
                        }
                        if (mon_s1() == TRUE){
                            index = index - 1;
                            if (index < 0){
                                index = 6;
                            }
                            lcd_clear();
                            print_item(index);
                        }
                        if (mon_s2() == TRUE){
                            index = index + 1;
                            if (index > 6){
                                index = 0;
                            }
                            lcd_clear();
                            print_item(index);
                        }

                    }

                }
            }
        }
    }
}


void init_all(void){

    P1REN |= BUTTON_PIN1;
    P1OUT |= BUTTON_PIN1; // Enable pull-up resistor for button pin 1
    P1DIR &= ~BUTTON_PIN1;
    P2DIR &= ~BUTTON_PIN2;
    P2REN |= BUTTON_PIN2;
    P2OUT |= BUTTON_PIN2; // Enable pull-up resistor for button pin 2
    P8OUT |= BUTTON_PIN8;
    P8REN |= BUTTON_PIN8;
    P8DIR &= ~BUTTON_PIN8;


    P4DIR |= BIT7;
    P4DIR &= ~BIT0;

    P1DIR |= BIT0;
    P1OUT &= ~BIT0; // Initialize P1.0 to low (LED off)

    //WDT_kick();

    gpio_config();  //LCD INIT
    i2c_config();

    //WDT_kick();

    GPIO_Init(); // RFID RC522 INIT
    SPI_Init();
    RC522_Init();

    //WDT_kick();

}


void GPIO_Init(void) {
    // Configure CS_PIN
    CS_DIR |= CS_PIN;
    CS_PORT |= CS_PIN;  // Set CS high
}

void SPI_Init(void) {
    UCB0CTL1 |= UCSWRST;                      // Put USCI in reset mode
    UCB0CTL0 = UCMST | UCSYNC | UCCKPL | UCMSB; // 3-pin, 8-bit SPI master, Clock polarity high, MSB first
    UCB0CTL1 = UCSSEL_2 | UCSWRST;            // Use SMCLK, keep USCI in reset mode
    UCB0BR0 = 0x02;                           // SPI clock divider
    UCB0BR1 = 0;
    P3SEL |= BIT0 | BIT1 | BIT2;              // P3.0, P3.1, P3.2 option select
    UCB0CTL1 &= ~UCSWRST;                     // Initialize USCI state machine
}

void RC522_Init(void) {
    RC522_Deselect();
    RC522_Reset();
    RC522_WriteRegister(0x2A, 0x8D); // Timer: TPrescaler*TreloadVal/6.78MHz = 24ms
    RC522_WriteRegister(0x2B, 0x3E); // Timer: TPrescaler*TreloadVal/6.78MHz = 24ms
    RC522_WriteRegister(0x2D, 30);   // Timer: TPrescaler*TreloadVal/6.78MHz = 24ms
    RC522_WriteRegister(0x2C, 0);
    RC522_WriteRegister(0x15, 0x40); // 100% ASK
    RC522_WriteRegister(0x11, 0x3D); // CRC initial value 0x6363
    RC522_AntennaOn();
}

uint8_t SPI_Transfer(uint8_t data) {
    UCB0TXBUF = data;                         // Transmit data
    while (!(UCB0IFG & UCRXIFG));             // Wait for RX buffer (full)
    return UCB0RXBUF;                         // Return received data
}

void RC522_Select(void) {
    CS_PORT &= ~CS_PIN;                       // Set CS low
}

void RC522_Deselect(void) {
    CS_PORT |= CS_PIN;                        // Set CS high
}

void RC522_Reset(void) {
    RC522_WriteRegister(0x01, PCD_RESETPHASE);
}

void RC522_WriteRegister(uint8_t addr, uint8_t val) {
    RC522_Select();
    SPI_Transfer((addr << 1) & 0x7E);
    SPI_Transfer(val);
    RC522_Deselect();
}

uint8_t RC522_ReadRegister(uint8_t addr) {
    uint8_t val;
    RC522_Select();
    SPI_Transfer(((addr << 1) & 0x7E) | 0x80);
    val = SPI_Transfer(0x00);
    RC522_Deselect();
    return val;
}

void RC522_SetBitMask(uint8_t reg, uint8_t mask) {
    RC522_WriteRegister(reg, RC522_ReadRegister(reg) | mask);
}

void RC522_ClearBitMask(uint8_t reg, uint8_t mask) {
    RC522_WriteRegister(reg, RC522_ReadRegister(reg) & (~mask));
}

void RC522_AntennaOn(void) {
    uint8_t temp = RC522_ReadRegister(0x14);
    if (!(temp & 0x03)) {
        RC522_SetBitMask(0x14, 0x03);
    }
}

uint8_t RC522_Request(uint8_t reqMode, uint8_t *TagType) {
    uint8_t status;
    uint8_t backBits;

    RC522_WriteRegister(0x0D, 0x07);
    TagType[0] = reqMode;
    status = RC522_ToCard(PCD_TRANSCEIVE, TagType, 1, TagType, &backBits);

    if ((status != MI_OK) || (backBits != 0x10)) {
        status = MI_ERR;
    }

    return status;
}

uint8_t RC522_Anticoll(uint8_t *serNum) {
    uint8_t status;
    uint8_t i;
    uint8_t serNumCheck = 0;
    uint8_t unLen;

    RC522_WriteRegister(0x0D, 0x00);
    serNum[0] = PICC_ANTICOLL;
    serNum[1] = 0x20;
    status = RC522_ToCard(PCD_TRANSCEIVE, serNum, 2, serNum, &unLen);

    if (status == MI_OK) {
        for (i = 0; i < 4; i++) {
            serNumCheck ^= serNum[i];
        }
        if (serNumCheck != serNum[i]) {
            status = MI_ERR;
        }
    }

    return status;
}

uint8_t RC522_ToCard(uint8_t command, uint8_t *sendData, uint8_t sendLen, uint8_t *backData, uint8_t *backLen) {
    uint8_t status = MI_ERR;
    uint8_t irqEn = 0x00;
    uint8_t waitIRq = 0x00;
    uint8_t lastBits;
    uint8_t n;
    uint8_t i;

    switch (command) {
    case PCD_AUTHENT:
        irqEn = 0x12;
        waitIRq = 0x10;
        break;
    case PCD_TRANSCEIVE:
        irqEn = 0x77;
        waitIRq = 0x30;
        break;
    default:
        break;
    }

    RC522_WriteRegister(0x02, irqEn | 0x80);
    RC522_ClearBitMask(0x04, 0x80);
    RC522_SetBitMask(0x0A, 0x80);
    RC522_WriteRegister(0x01, PCD_IDLE);

    for (i = 0; i < sendLen; i++) {
        RC522_WriteRegister(0x09, sendData[i]);
    }
    RC522_WriteRegister(0x01, command);

    if (command == PCD_TRANSCEIVE) {
        RC522_SetBitMask(0x0D, 0x80);
    }

    i = 2000;
    do {
        n = RC522_ReadRegister(0x04);
        i--;
    } while ((i != 0) && !(n & 0x01) && !(n & waitIRq));

    RC522_ClearBitMask(0x0D, 0x80);

    if (i != 0) {
        if (!(RC522_ReadRegister(0x06) & 0x1B)) {
            status = MI_OK;
            if (n & irqEn & 0x01) {
                status = MI_NOTAGERR;
            }
            if (command == PCD_TRANSCEIVE) {
                n = RC522_ReadRegister(0x0A);
                lastBits = RC522_ReadRegister(0x0C) & 0x07;
                if (lastBits) {
                    *backLen = (n - 1) * 8 + lastBits;
                } else {
                    *backLen = n * 8;
                }
                if (n == 0) {
                    n = 1;
                }
                if (n > MAX_LEN) {
                    n = MAX_LEN;
                }
                for (i = 0; i < n; i++) {
                    backData[i] = RC522_ReadRegister(0x09);
                }
            }
        } else {
            status = MI_ERR;
        }
    }

    return status;
}

uint8_t compareArrays(const uint8_t *array1, const uint8_t *array2, uint8_t length) {

    uint8_t i = 0;

    for (i = 0; i < length; i++) {
        if (array1[i] != array2[i]) {
            return 0; // Arrays are not equal
        }
    }
    return 1; // Arrays are equal
}


///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////



void lcd_inic(void){
    // Prepare I2C to operate
    UCB1I2CSA = PCF_ADR;    //Slave Address
    UCB1CTL1 |= UCTR | UCTXSTT;    //Master TX
    while ((UCB1IFG & UCTXIFG) == 0);          //Wait for TXIFG=1
    UCB1TXBUF = 0;                              //PCF Output = 0;
    while ((UCB1CTL1 & UCTXSTT) == UCTXSTT);   //Wait for STT=0
    if ((UCB1IFG & UCNACKIFG) == UCNACKIFG)    //NACK?
        while(1);

    // Start initialization
    lcd_aux(0);     //RS=RW=0, BL=1
    delay(20000);
    lcd_aux(3);     //3
    delay(10000);
    lcd_aux(3);     //3
    delay(10000);
    lcd_aux(3);     //3
    delay(10000);
    lcd_aux(2);     //2

    // Enter 4-bit mode
    lcd_aux(2); lcd_aux(8);     //0x28
    lcd_aux(0); lcd_aux(8);     //0x08
    lcd_aux(0); lcd_aux(1);     //0x01
    lcd_aux(0); lcd_aux(6);     //0x06
    lcd_aux(0); lcd_aux(0xF);   //0x0F

    while ((UCB1IFG & UCTXIFG) == 0);          //Wait for TXIFG=1
    UCB1CTL1 |= UCTXSTP;                           //Generate STOP
    while ((UCB1CTL1 & UCTXSTP) == UCTXSTP);   //Wait for STOP
    delay(50);
}

// Auxiliary LCD initialization function (RS=RW=0)
void lcd_aux(char dado){
    while ((UCB1IFG & UCTXIFG) == 0);              //Wait for TXIFG=1
    UCB1TXBUF = ((dado << 4) & 0xF0) | BIT3;            //PCF7:4 = dado;
    delay(50);
    while ((UCB1IFG & UCTXIFG) == 0);              //Wait for TXIFG=1
    UCB1TXBUF = ((dado << 4) & 0xF0) | BIT3 | BIT2;     //E=1
    delay(50);
    while ((UCB1IFG & UCTXIFG) == 0);              //Wait for TXIFG=1
    UCB1TXBUF = ((dado << 4) & 0xF0) | BIT3;            //E=0;
}

// Function to read from the PCF port
int pcf_read(void){
    int dado;
    UCB1I2CSA = PCF_ADR;                //Slave Address
    UCB1CTL1 &= ~UCTR;                  //Master RX
    UCB1CTL1 |= UCTXSTT;                //Generate START
    while ((UCB1CTL1 & UCTXSTT) == UCTXSTT);
    UCB1CTL1 |= UCTXSTP;                //Generate STOP + NACK
    while ((UCB1CTL1 & UCTXSTP) == UCTXSTP);   //Wait for STOP
    while ((UCB1IFG & UCRXIFG) == 0);  //Wait for RX
    dado = UCB1RXBUF;
    return dado;
}

// Function to write data to the PCF port
void pcf_write(char dado){
    UCB1I2CSA = PCF_ADR;        //Slave Address
    UCB1CTL1 |= UCTR | UCTXSTT;        //Master TX
    while ((UCB1IFG & UCTXIFG) == 0);          //Wait for TXIFG=1
    UCB1TXBUF = dado;                              //Write data
    while ((UCB1CTL1 & UCTXSTT) == UCTXSTT);   //Wait for STT=0
    if ((UCB1IFG & UCNACKIFG) == UCNACKIFG)       //NACK?
        while(1);                          //Slave generated NACK
    UCB1CTL1 |= UCTXSTP;                        //Generate STOP
    while ((UCB1CTL1 & UCTXSTP) == UCTXSTP);   //Wait for STOP
}

// Function to test I2C address, returns TRUE if ACK received
int pcf_teste(char adr){
    UCB1I2CSA = adr;                            //PCF Address
    UCB1CTL1 |= UCTR | UCTXSTT;                 //Generate START, Master transmitter
    while ((UCB1IFG & UCTXIFG) == 0);          //Wait for START
    UCB1CTL1 |= UCTXSTP;                        //Generate STOP
    while ((UCB1CTL1 & UCTXSTP) == UCTXSTP);   //Wait for STOP
    if ((UCB1IFG & UCNACKIFG) == 0) return TRUE;
    else return FALSE;
}

// Configure UCSB1 and I2C pins
void i2c_config(void){
    UCB1CTL1 |= UCSWRST;    // UCSI B1 in reset
    UCB1CTL0 = UCSYNC |     //Synchronous
            UCMODE_3 |   //I2C Mode
            UCMST;       //Master
    UCB1BRW = BR_100K;      //100 kbps
    P4SEL |= BIT1 | BIT2;  // Use dedicated module
    UCB1CTL1 = UCSSEL_2;    //SMCLK and remove reset
}

void led_vd(void)   {P4OUT &= ~BIT7;}   //Turn off green LED
void led_VD(void)   {P4OUT |= BIT7;}    //Turn on green LED
void led_vm(void)   {P1OUT &= ~BIT0;}   //Turn off red LED
void led_VM(void)   {P1OUT |= BIT0;}    //Turn on red LED

// Configure LEDs
void gpio_config(void){
    P1DIR |= BIT0;      //Red LED
    P1OUT &= ~BIT0;     //Red LED off
    P4DIR |= BIT7;      //Green LED
    P4OUT &= ~BIT7;     //Green LED off
}

void delay(long limite){
    volatile long cont = 0;
    while (cont++ < limite);
}

// Function to calculate the length of a string
int string_length(const char* str) {
    int length = 0;
    while (str[length] != '\0') {
        length++;
    }
    return length;
}

int get_ascii_value(char c) {
    return (int)c;
}

int ms_byte_9(char c) {
    return (c & 0xF0) | 0x09;
}

// Function to calculate MS byte with D (0x4D)
int ms_byte_D(char c) {
    return (c & 0xF0) | 0x0D;
}

int ls_byte_9(char c) {
    return ((c & 0x0F) << 4) | 0x09;
}

// Function to calculate LS byte with D (0x5D)
int ls_byte_D(char c) {
    return ((c & 0x0F) << 4) | 0x0D;
}

void char_build(char c){              // Sets up for printing on lcd display
    ms9 = ms_byte_9(c);
    msD = ms_byte_D(c);
    ls9 = ls_byte_9(c);
    lsD = ls_byte_D(c);
}

void char_print(void){
    pcf_write(ms9);
    pcf_write(msD);
    pcf_write(ms9);
    pcf_write(ls9);
    pcf_write(lsD);
    pcf_write(ls9);
}

void lcd_send_command(char command) {
    char high_nibble = command & 0xF0;
    char low_nibble = (command << 4) & 0xF0;

    // Send high nibble
    pcf_write(high_nibble | 0x08); // RS=0 for command, RW=0, E=0
    pcf_write(high_nibble | 0x0C); // E=1
    pcf_write(high_nibble | 0x08); // E=0

    // Send low nibble
    pcf_write(low_nibble | 0x08); // RS=0 for command, RW=0, E=0
    pcf_write(low_nibble | 0x0C); // E=1
    pcf_write(low_nibble | 0x08); // E=0
}

void lcd_clear(void) {
    lcd_send_command(0x01); // Clear display command
    delay(2000); // Delay to allow command to complete
}

void lcd_set_cursor(int row, int col) {
    char command;
    switch(row) {
    case 0:
        command = 0x80 + col;
        break;
    case 1:
        command = 0xC0 + col;
        break;
    default:
        return; // Invalid row, do nothing
    }
    lcd_send_command(command);
}

// Function to print a specific item and its quantity on the LCD
void print_item(unsigned int index) {
    lcd_clear();

    // Print the item name in the top row
    lcd_set_cursor(0, 0);
    unsigned int i = 0;
    while (itemNames[index][i] != '\0') {
        char_build(itemNames[index][i]);
        char_print();
        i++;
    }

    // Print " Quant: {number}" in the bottom row
    lcd_set_cursor(1, 0);
    const char *quant_label = "Quant: ";
    i = 0;
    while (quant_label[i] != '\0') {
        char_build(quant_label[i]);
        char_print();
        i++;
    }
    char_build(itemQuantities[index] + '0'); // Convert int to char
    char_print();
}



int mon_s1(void){
    static int ps1=ABERTA; //Guardar passado de S1
    thirtysecs = 0; // Resets 30 second timer if a button is pressed
    if ( (P2IN&BIT1) == 0)
    { //Qual estado atual de S1?
        if (ps1==ABERTA)
        { //Qual o passado de S1?
            __delay_cycles(1000);
            ps1=FECHADA;
            return TRUE;
        }
    }
    else
    {
        if (ps1==FECHADA)
        { //Qual o passado de S1?
            __delay_cycles(1000);
            ps1=ABERTA;
            return FALSE;
        }
    }
    return FALSE;
}

int mon_s2(void){
    static int ps2=ABERTA; //Guardar passado de S2
    thirtysecs = 0;
    if ( (P1IN&BIT1) == 0){ //Qual estado atual de S2?
        if (ps2==ABERTA){ //Qual o passado de S2?
            __delay_cycles(1000);
            ps2=FECHADA;
            return TRUE;
        }
    }
    else{
        if (ps2==FECHADA){ //Qual o passado de S2?
            __delay_cycles(1000);
            ps2=ABERTA;
            return FALSE;
        }
    }
    return FALSE;
}

int mon_button(void){
    static int ps8=ABERTA; //Guardar passado de S2
    thirtysecs = 0;
    if ( (P8IN&BIT1) == 0){ //Qual estado atual de S2?
        if (ps8==ABERTA){ //Qual o passado de S2?
            __delay_cycles(1000);
            ps8=FECHADA;
            return TRUE;
        }
    }
    else{
        if (ps8==FECHADA){ //Qual o passado de S2?
            __delay_cycles(1000);
            ps8=ABERTA;
            return FALSE;
        }
    }
    return FALSE;
}

void start_timer(void) {
    // Configure Timer_A
    TA0CTL = TACLR;                    // Clears timer
    TA0CCTL0 = CCIE;                   // Enable Timer A interrupt
    TA0CCR0 = 32767;                   // Set the timer count (assuming 32768 Hz ACLK, 1-second intervals)
    TA0CTL = TASSEL_1 + MC_1 + TACLR;  // ACLK, up mode, clear TAR

    __bis_SR_register(GIE);            // Enable global interrupts
}

void stop_timer(void) {
    seconds = 0;
    TA0CTL = MC_0;  // Stop the timer by setting the mode control bits to 0
}


void lcd_backlight_off(void) {
    pcf_write(0);  // Send command to turn off the backlight
}

#pragma vector = TIMER0_A0_VECTOR
__interrupt void Timer_A(void) {

    seconds++;
    thirtysecs++;
    if (seconds >= 10) {
        lcd_clear();
        lcd_backlight_off();
        TA0CCTL0 &= ~CCIE;  // Disable Timer A interrupt
    }

    if (thirtysecs >= 30) {
        lcd_clear();
        lcd_backlight_off();
        TA0CCTL0 &= ~CCIE;  // Disable Timer A interrupt
    }
}







