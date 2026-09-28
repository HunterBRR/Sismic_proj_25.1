// Código base para ADC: lê um canal e salva o resultado

#include <msp430.h>
#include <stdint.h>
#define BUZZER_PIN BIT2  // P1.2 (TA0.1)

// LCD_Base
// Rotinas básicas para usar o LCD

// P3.0 ==> SDA
// P3.1 ==> SCL

#include <msp430.h>
#define LED1 BIT7
#define LED2 BIT4
#define LED3 BIT6
#define LED4 BIT3

#define S1 BIT0

#define TRUE    1
#define FALSE   0

// Definição do endereço do PCF_8574
#define PCF_ADR1 0X27
#define PCF_ADR2 0X3F
#define PCF_ADR  PCF_ADR1

#define BR_100K    11  //SMCLK/100K = 11
#define BR_50K     21  //SMCLK/50K  = 21
#define BR_10K    105  //SMCLK/10K  = 105

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
void lcdWriteNibble(char nibble, int isChar);
void lcdWriteString(char* string);

volatile uint16_t adcResult = 0;  // Resultado da conversão
int iniciar=1;
int chave=1;
int chave_mante=0;
void initADC(uint8_t channel);
uint16_t readADC(void);
volatile int BPM=60;
void initPWM() {
    // Configura o pino do buzzer
    P1DIR |= BUZZER_PIN;
    P1SEL |= BUZZER_PIN;  // Ativa função alternativa (PWM)
    
    // Configura o Timer0_A para PWM
    TA0CCR0 = 1000-1;      // Valor do período (ajustável)
    TA0CCTL1 = OUTMOD_7;   // Modo reset/set
    TA0CCR1 = 500;         // 50% duty cycle
    TA0CTL = TASSEL_2 + MC_1; // SMCLK (DCO não calibrado), modo up
}

void setTone(unsigned int freq) {
    if(freq == 0) {
        TA0CTL = 0;        // Desliga PWM
        P1OUT &= ~BUZZER_PIN;
        return;
    }
      // Calcula o período baseado no clock não calibrado (~1MHz)
    TA0CCR0 = (1000000/freq) - 1;
    TA0CCR1 = TA0CCR0/2;   // 50% duty cycle
    TA0CTL = TASSEL_2 + MC_1; // Reinicia timer com SMCLK
}
void main(void) {
    WDTCTL = WDTPW | WDTHOLD;  // Parar watchdog
    P2DIR |=LED1;
    P2DIR |=LED2;
    P1DIR |=LED3;
    P2DIR |=LED4;

    
    // Escolher canal
    uint8_t channel = 1;

    // Configurar ADC
    initADC(channel);

    uint16_t vetorLeituras[100];
    
    //__enable_interrupt();       // Habilitar interrupções globais

    // Esse loop faz 100 leituras e as armazena em "vetorLeituras"
    gpio_config();
    i2c_config();

    if (pcf_teste(PCF_ADR)==FALSE){
        led_VM();           //Indicar que não houve ACK
        while(TRUE);        //Travar
    }
    else    led_VD();       //Houve ACK, tudo certo

    lcd_inic();     //Inicializar LCD
    pcf_write(0x08);   //Acender Back Light
    P1DIR |=BIT0;
    P2DIR &= ~S1;            // P2.1 é uma entrada
    P2REN |= S1;             // Habilitar resistor
    P2OUT |= S1;             // Usar resistor de pull-up
    P2IES |= (S1 );  // Interrupção na borda de descida
    P2IFG &= ~(S1); // Limpa flags de interrupção
    P2IE |= (S1);            // Habilita interrupção para P2.0
    __enable_interrupt(); // Habilita interrupções globais
    int centenas_num;
    int dezenas_num;
    int unidades_num;
    int valor=0;
    char str_valor[4];
    unsigned char a;
    int prim;
    int sec;
    int ter;
    int valor1=0;
    int valor_final=0;
    // Loop infinito
    int asci=0x30;
    int tempo_final=0;
   

    initPWM();
    for(;;){
      
      int indice;
      if (iniciar==1){
        for (indice = 0; indice < 100; indice++) {
            vetorLeituras[indice] = readADC();
            __delay_cycles(1000);  // Atraso entre leituras
        }
        valor = readADC();
         P2OUT&=~LED2;
         P1OUT&=~LED3;
         P2OUT&=~LED1;
         P2OUT&=~LED4;
        setTone(0);
        // Variáveis temporárias para armazenar os dígitos numéricos
        

    // --- Extraindo os dígitos ---
        int valor_final= (((valor/40))+60);
        BPM = valor_final;
        if (valor_final>=160){
          valor_final=160;}
        unidades_num = valor_final % 10;             // Pega o último dígito (unidades)
        dezenas_num = (valor_final / 10) % 10;       // Pega o dígito do meio (dezenas)
        int centenas_num = valor_final / 100;
        
        TA1CCTL0 &= ~CCIE;
        chave=1;
        if ((valor_final>=100)&&(abs(valor1 - valor) >= 25)){
          
          valor1=valor;
          str_valor[0] = centenas_num ; // Converte e armazena o dígito das centenas
          str_valor[1] = dezenas_num ;  // Converte e armazena o dígito das dezenas
          str_valor[2] = unidades_num ; // Converte e armazena o dígito das unidades
          int prim = asci + centenas_num;
          int sec = asci + dezenas_num;
          int ter = asci + unidades_num;
          lcdWriteNibble(0x1, 0);
          __delay_cycles(5000);
          lcdWriteNibble(prim, 1);
          __delay_cycles(1000);
          lcdWriteNibble(sec, 1);
          __delay_cycles(1000);
          lcdWriteNibble(ter, 1);
          __delay_cycles(1000);
          lcdWriteNibble('B', 1);
          __delay_cycles(1000);
          lcdWriteNibble('P', 1);
          __delay_cycles(1000);
          lcdWriteNibble('M', 1);
          __delay_cycles(1000);
        __delay_cycles(100000);
        }
        else if ((valor_final<100)&&(abs(valor1 - valor) >= 23)){
          
          valor1=valor;
          //str_valor[0] = centenas_num ; // Converte e armazena o dígito das centenas
          str_valor[1] = dezenas_num ;  // Converte e armazena o dígito das dezenas
          str_valor[2] = unidades_num ; // Converte e armazena o dígito das unidades
          int prim = asci + centenas_num;
          int sec = asci + dezenas_num;
          int ter = asci + unidades_num;
          lcdWriteNibble(0x1, 0);
          __delay_cycles(5000);
          //lcdWriteNibble(prim, 1);
         //__delay_cycles(1000);
          lcdWriteNibble(sec, 1);
          __delay_cycles(1000);
          lcdWriteNibble(ter, 1);
          __delay_cycles(1000);
          lcdWriteNibble('B', 1);
          __delay_cycles(1000);
          lcdWriteNibble('P', 1);
          __delay_cycles(1000);
          lcdWriteNibble('M', 1);
          __delay_cycles(1000);
        __delay_cycles(100000);
        
        }
    }
    else{
      int batidas = ((32768 * 60)/BPM);
      TA1CCR0 =  batidas; // Calcula o intervalo do Timer (em ciclos de clock)
      TA1CTL = TASSEL_1 + MC_1;  // SMCLK (1 MHz), modo up, sem divisão
      TA1CCTL0 = CCIE;                 // Habilita interrupção no TA1CCR0
      while (iniciar==0){
        if (chave==chave_mante){
          setTone(0);}
        else if (chave!=chave_mante){
        
          if (chave==1){
            setTone(2000);      // Som grave (~200Hz)
            __delay_cycles(35000);
            setTone(0);
            chave_mante=chave;
            P2OUT&=~LED2;
            P1OUT&=~LED3;
            P2OUT|=LED1;
            P2OUT&=~LED4;
          }
          else if (chave==2){
            setTone(200);      // Som grave (~200Hz)
            __delay_cycles(35000);
            setTone(0);
            chave_mante=chave;
            P2OUT|=LED2;
            
          }
          else if (chave==3){
            setTone(200);      // Som grave (~200Hz)
            __delay_cycles(35000);
            setTone(0);
            chave_mante=chave;
           
            P1OUT|=LED3;
            

          }
          else if (chave==4){
            setTone(200);      // Som grave (~200Hz)
            __delay_cycles(35000);
            setTone(0);
            chave_mante=chave;
            
            P2OUT|=LED4;
        }
        //setTone(200);      // Som grave (~200Hz)
        //__delay_cycles(400000); // 0.4s
        //setTone(0);        // Silêncio
        //__delay_cycles(1000000); // 1.0s

        //setTone(2000);      // Som grave (~2000Hz)
        //__delay_cycles(400000); // 0.4s
        //setTone(0);        // Silêncio
        //__delay_cycles(1000000); // 1.0s
        }
      }
    }
        
     
      }

  
     
    }
    

// Rotina de interrupção do Timer_A1
#pragma vector=TIMER1_A0_VECTOR
__interrupt void TIMER1_A0_ISR(void) {
    P1OUT ^= BIT0;             // Alterna o LED (ou pode gerar um beep por hardware)
    chave++;
    if (chave>4){
      chave=1;
    }
    
}

void initADC(uint8_t channel) {
    // Selecionar função alternativa de P6.x 
    P6SEL |= (1 << channel);


    // Configurar ADC12
    ADC12CTL0 &= ~ADC12ENC;             // Desabilitar para iniciar configuração
    ADC12CTL0 = ADC12SHT0_2 | ADC12ON;  // Usar 16 ciclos de clock por conversão (SHT0=2), ligar ADC
    
    ADC12MCTL0 = channel;               // Escolher o canal certo, o valor obtido será armazenado em ADC12MEM0

    // "pulse mode" (SHP=1)
    // Usar ADC12SC como gatilho (SHS=0)
    // Selecionar SMCLK como fonte de clock (SSEL=2)
    // Modo "single-channel, single-conversion" (CONSEQ=0)
    ADC12CTL1 =  ADC12SHP | ADC12SHS_0 | ADC12SSEL_2 | ADC12CONSEQ_0;

    // ADC12IE = BIT0;                  // Habilitar interrupção de ADC12MCTL0
    
    ADC12CTL2 = ADC12RES_2;             // Resolução de 12 bits (RES_0 = 8 bits, RES_1 = 10 bits, RES_2 = 12 bits). O valor lido ficará entre 0 e (2^n)-1, n = número de bits
    
    ADC12CTL0 |= ADC12ENC;              // Habilitar conversão
}


uint16_t readADC() {
    ADC12CTL0 |= ADC12SC;               // Iniciar conversão
    while (!(ADC12IFG & ADC12IFG0));    // Esperar a conversão terminar

    // Em algmas configurações, será preciso zerar o bit ADC12SC manualmente após cada conversão
    // ADC12CTL0 &= ~ADC12SC;

    return ADC12MEM0;                   // Retornar valor
}

// Interrupção do ADC12 (não usada nesse código)
#pragma vector=ADC12_VECTOR
__interrupt void ADC12_ISR(void) {
    switch(ADC12IV){
        // interrupção do ADC12MCTL0
        case ADC12IV_ADC12IFG0:
            break;

        default:
            break;
    }
}
#pragma vector=PORT2_VECTOR
__interrupt void PORT1_ISR(void) {
    if (P2IFG & S1) {     
        debounce();  
        P2OUT ^=LED1;
        P1OUT ^=LED2;
        if (iniciar==1){
          iniciar=0;
        }
        else if (iniciar==0){
          iniciar=1;
        }
        P2IFG &= ~S1;     
                     
    }
        
    }

// Vetor de interrupção para a porta P2

// Incializar LCD modo 4 bits
void lcd_inic(void){

    // Preparar I2C para operar
    UCB0I2CSA = PCF_ADR;    //Endereço Escravo
    UCB0CTL1 |= UCTR    |   //Mestre TX
                UCTXSTT;    //Gerar START
    while ( (UCB0IFG & UCTXIFG) == 0);          //Esperar TXIFG=1
    UCB0TXBUF = 0;                              //Saída PCF = 0;
    while ( (UCB0CTL1 & UCTXSTT) == UCTXSTT);   //Esperar STT=0
    if ( (UCB0IFG & UCNACKIFG) == UCNACKIFG)    //NACK?
                while(1);

    // Começar inicialização
    lcd_aux(0);     //RS=RW=0, BL=1
    delay(20000);
    lcd_aux(3);     //3
    delay(10000);
    lcd_aux(3);     //3
    delay(10000);
    lcd_aux(3);     //3
    delay(10000);
    lcd_aux(2);     //2

    // Entrou em modo 4 bits
    lcd_aux(2);     lcd_aux(8);     //0x28
    lcd_aux(0);     lcd_aux(8);     //0x08
    lcd_aux(0);     lcd_aux(1);     //0x01
    lcd_aux(0);     lcd_aux(6);     //0x06
    lcd_aux(0);     lcd_aux(0xF);   //0x0F

    while ( (UCB0IFG & UCTXIFG) == 0)   ;          //Esperar TXIFG=1
    UCB0CTL1 |= UCTXSTP;                           //Gerar STOP
    while ( (UCB0CTL1 & UCTXSTP) == UCTXSTP)   ;   //Esperar STOP
    delay(50);
}

// Auxiliar inicialização do LCD (RS=RW=0)
// * Só serve para a inicialização *
void lcd_aux(char dado){
    while ( (UCB0IFG & UCTXIFG) == 0);              //Esperar TXIFG=1
    UCB0TXBUF = ((dado<<4)&0XF0) | BIT3;            //PCF7:4 = dado;
    delay(50);
    while ( (UCB0IFG & UCTXIFG) == 0);              //Esperar TXIFG=1
    UCB0TXBUF = ((dado<<4)&0XF0) | BIT3 | BIT2;     //E=1
    delay(50);
    while ( (UCB0IFG & UCTXIFG) == 0);              //Esperar TXIFG=1
    UCB0TXBUF = ((dado<<4)&0XF0) | BIT3;            //E=0;
}

// Ler a porta do PCF
int pcf_read(void){
    int dado;
    UCB0I2CSA = PCF_ADR;                //Endereço Escravo
    UCB0CTL1 &= ~UCTR;                  //Mestre RX
    UCB0CTL1 |= UCTXSTT;                //Gerar START
    while ( (UCB0CTL1 & UCTXSTT) == UCTXSTT);
    UCB0CTL1 |= UCTXSTP;                //Gerar STOP + NACK
    while ( (UCB0CTL1 & UCTXSTP) == UCTXSTP)   ;   //Esperar STOP
    while ( (UCB0IFG & UCRXIFG) == 0);  //Esperar RX
    dado=UCB0RXBUF;
    return dado;
}

// Escrever dado na porta
void pcf_write(char dado){
    UCB0I2CSA = PCF_ADR;        //Endereço Escravo
    UCB0CTL1 |= UCTR    |       //Mestre TX
                UCTXSTT;        //Gerar START
    while ( (UCB0IFG & UCTXIFG) == 0)   ;          //Esperar TXIFG=1
    UCB0TXBUF = dado;                              //Escrever dado
    while ( (UCB0CTL1 & UCTXSTT) == UCTXSTT)   ;   //Esperar STT=0
    if ( (UCB0IFG & UCNACKIFG) == UCNACKIFG)       //NACK?
                while(1);                          //Escravo gerou NACK
    UCB0CTL1 |= UCTXSTP;                        //Gerar STOP
    while ( (UCB0CTL1 & UCTXSTP) == UCTXSTP)   ;   //Esperar STOP
}

// Testar endereço I2C
// TRUE se recebeu ACK
int pcf_teste(char adr){
    UCB0I2CSA = adr;                            //Endereço do PCF
    UCB0CTL1 |= UCTR | UCTXSTT;                 //Gerar START, Mestre transmissor
    while ( (UCB0IFG & UCTXIFG) == 0);          //Esperar pelo START
    UCB0CTL1 |= UCTXSTP;                        //Gerar STOP
    while ( (UCB0CTL1 & UCTXSTP) == UCTXSTP);   //Esperar pelo STOP
    if ((UCB0IFG & UCNACKIFG) == 0)     return TRUE;
    else                                return FALSE;
}

// Configurar UCSB0 e Pinos I2C
// P3.0 = SDA e P3.1=SCL
void i2c_config(void){
    UCB0CTL1 |= UCSWRST;    // UCSI B0 em ressete
    UCB0CTL0 = UCSYNC |     //Síncrono
               UCMODE_3 |   //Modo I2C
               UCMST;       //Mestre
    UCB0BRW = BR_100K;      //100 kbps
    P3SEL |=  BIT1 | BIT0;  // Use dedicated module
    UCB0CTL1 = UCSSEL_2;    //SMCLK e remove ressete
}

void led_vd(void)   {P4OUT &= ~BIT7;}   //Apagar verde
void led_VD(void)   {P4OUT |=  BIT7;}   //Acender verde
void led_vm(void)   {P1OUT &= ~BIT0;}   //Apagar vermelho
void led_VM(void)   {P1OUT |=  BIT0;}   //Acender vermelho

// Configurar leds
void gpio_config(void){
    P1DIR |=  BIT0;      //Led vermelho
    P1OUT &= ~BIT0;      //Vermelho Apagado
    P4DIR |=  BIT7;      //Led verde
    P4OUT &= ~BIT7;      //Verde Apagado
}

void delay(long limite){
    volatile long cont=0;
    while (cont++ < limite) ;
}

void lcdWriteString(char* string) {
    int i=0;
    for (i=0; i<=sizeof(string)+1; i++) {
        lcdWriteNibble(string[i], 1);
        __delay_cycles(100000);
    }
}

void lcdWriteNibble(char nibble, int isChar) {
    //char number;
    //Escrever um nibble na tela
    if (isChar) {
        // Parte alta do nibble (4 bits mais significativos)
        pcf_write((nibble & 0xF0) | 0x09);  // Envia a parte alta com 0x09
        pcf_write((nibble & 0xF0) | 0x0D);  // Envia a parte alta com 0x0D
        pcf_write((nibble & 0xF0) | 0x09);  // Envia a parte alta com 0x09

        // Parte baixa do nibble (4 bits menos significativos)
        pcf_write((nibble << 4) | 0x09);    // Envia a parte baixa com 0x09
        pcf_write((nibble << 4) | 0x0D);    // Envia a parte baixa com 0x0D
        pcf_write((nibble << 4) | 0x09);    // Envia a parte baixa com 0x09
    } else {
        // Parte alta do nibble (4 bits mais significativos)
        pcf_write((nibble & 0xF0) | 0x08);  // Envia a parte alta com 0x08
        pcf_write((nibble & 0xF0) | 0x0C);  // Envia a parte alta com 0x0C
        pcf_write((nibble & 0xF0) | 0x08);  // Envia a parte alta com 0x08

        // Parte baixa do nibble (4 bits menos significativos)
        pcf_write((nibble << 4) | 0x08);    // Envia a parte baixa com 0x08
        pcf_write((nibble << 4) | 0x0C);    // Envia a parte baixa com 0x0C
        pcf_write((nibble << 4) | 0x08);    // Envia a parte baixa com 0x08
    }
}

void debounce(){
    // for(contador1 = 20000; contador1 >= 0; contador1--); 
    __delay_cycles(250000);
}
