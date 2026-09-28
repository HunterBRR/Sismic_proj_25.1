//------------------------------------------------------------------------------
#define FEEDBACK_PORT_DIR   P4DIR
#define FEEDBACK_PORT_OUT   P4OUT

#define LED_VERDE_PIN       BIT0  // P4.0
#define LED_VERMELHO_PIN    BIT1  // P4.1
#define BUZZER_PIN          BIT2  // P4.2
#ifndef KEYPAD_H
#define KEYPAD_H


#include <stdint.h>
#ifndef CONFIG_H
#define CONFIG_H

   //==============================================================================
   // CONFIGURAÇÃO DO TECLADO MATRICIAL 4x4
   //==============================================================================
   // Para simplificar o código, todos os 8 pinos do teclado estão no Port 2.
   //------------------------------------------------------------------------------
#define KEYPAD_PORT_DIR     P2DIR
#define KEYPAD_PORT_OUT     P2OUT
#define KEYPAD_PORT_REN     P2REN
#define KEYPAD_PORT_IN      P2IN

   // Mapeamento dos pinos. ATENÇÃO: COL2 está no Port 1.
   // Colunas (Configuradas como Saíd a)
#define KEYPAD_COL1_PIN     BIT0 // P2.0
#define KEYPAD_COL2_PIN     BIT3 // P1.3 <-- CORRIGIDO
#define KEYPAD_COL3_PIN     BIT2 // P2.2
#define KEYPAD_COL4_PIN     BIT3 // P2.3

   // Linhas (Configuradas como Entrada com Pull-up)
#define KEYPAD_ROW1_PIN     BIT4 // P2.4
#define KEYPAD_ROW2_PIN     BIT5 // P2.5
#define KEYPAD_ROW3_PIN     BIT6 // P2.6
#define KEYPAD_ROW4_PIN     BIT7 // P2.7
   //==============================================================================
   // CONFIGURAÇÃO DO SERVO MOTOR
   //==============================================================================
   // O pino do servo deve ser um que suporte saída de Timer/PWM.
   // Usamos P1.2, que é a saída TA0.1 no MSP430F5529.
   //------------------------------------------------------------------------------
#define SERVO_PIN           BIT2 // P1.2

   // Valores do Duty Cycle para o registrador TA0CCR1 do Timer.
   // Estes valores são calculados para um clock SMCLK de 1MHz.
   // Posição Travado (ex: 0 graus, pulso de 1.0ms): 1MHz * 0.001s = 1000
#define SERVO_POS_LOCKED    1000

   // Posição Destravado (ex: 90 graus, pulso de 1.5ms): 1MHz * 0.0015s = 1500
#define SERVO_POS_UNLOCKED  2500

   //==============================================================================
   // CONFIGURAÇÃO DE FEEDBACK (LEDs e Buzzer)
   //==============================================================================
   // Pinos de I/O de uso geral para os indicadores. Usamos o Port 4.
   //------------------------------------------------------------------------------
#define FEEDBACK_PORT_DIR   P4DIR
#define FEEDBACK_PORT_OUT   P4OUT

#define LED_VERDE_PIN       BIT0  // P4.0
#define LED_VERMELHO_PIN    BIT1  // P4.1
#define BUZZER_PIN          BIT2  // P4.2

   //==============================================================================
   // CONFIGURAÇÕES GERAIS DE SOFTWARE
   //==============================================================================
#define TAMANHO_SENHA       4
#define SENHA_PADRAO        "1234" // Senha pré-definida para o cofre

#endif /* CONFIG_H */
   //==============================================================================
   // CONFIGURAÇÕES GERAIS DE SOFTWARE
   //==============================================================================
#define TAMANHO_SENHA       4
#define SENHA_PADRAO        "1234" // Senha pré-definida para o cofre

#ifndef SERVO_H
#define SERVO_H

#ifndef STATUS_FEEDBACK_H
#define STATUS_FEEDBACK_H 
   /**
 ******************************************************************************
 * @file    keypad.c
 * @author  Bernardo Barros Blanco (Aluno)
 * @brief   Implementação do driver para um teclado matricial 4x4.
 *
 * @note    Este arquivo implementa a lógica de varredura para ler teclas
 * pressionadas no teclado. Ele depende das definições de pinos
 * configuradas no arquivo "config.h".
 ******************************************************************************
 */

#include <msp430.h>

#define NO_KEY_PRESSED '\0'



// Codigo Ronan


volatile uint16_t adcResult = 0;  // Resultado da conversão
#define S1 BIT0;
void initADC(uint8_t channel);
uint16_t readADC(void);
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


void i2c_config(void);
 
void delay1(long limite);
void lcdWriteNibble(char nibble, int isChar);
void lcdWriteString(char* string);




// Fim codigo Ronan


void servo_init(void);

/**
 * @brief Move o servo para a posição de "travado".
 * A posição exata (largura do pulso) é definida em config.h.
 * @param None
 * @return None
 */
void servo_lock(void);

/**
 * @brief Move o servo para a posição de "destravado".
 * A posição exata (largura do pulso) é definida em config.h.
 * @param None
 * @return None
 */
void servo_unlock(void);


#endif /* SERVO_H */

void keypad_init(void);

/**
 * @brief Realiza a varredura do teclado para detectar uma tecla pressionada.
 * @param None
 * @return O caractere ('0'-'9', 'A'-'D', '*', '#') correspondente à tecla
 * pressionada, ou NO_KEY_PRESSED se nenhuma tecla for detectada.
 */
char keypad_get_char(void);

#endif /* KEYPAD_H */

static const char keymap[4][4] = {
    {'1', '4', '7', 'A'},
    {'2', '5', '8', 'B'},
    {'3', '6', '9', 'C'},
    {'*', '0', '#', 'D'}
};


typedef enum {
    TRAVADO,
    RECEBENDO_SENHA,
    DESTRAVADO,
    ERRO
} EstadoCofre;

// --- Variáveis Globais ---
static EstadoCofre estado_atual;
static char senha_digitada[TAMANHO_SENHA + 1]; // +1 para o caractere nulo '\0'
static int indice_digito = 0;

/**
 * @brief  Configura o hardware inicial do sistema.
 */
static void setup(void) {
    // 1. Parar o Watchdog Timer - passo essencial para a maioria dos projetos MSP430.
    WDTCTL = WDTPW | WDTHOLD;

    // 2. Inicializar os módulos de hardware.
    feedback_init();
    keypad_init();
    servo_init();

    // 3. Definir estado inicial do cofre.
    estado_atual = TRAVADO;
    feedback_show_locked(); // LED vermelho aceso [cite: 25]
    servo_lock();           // Servo na posição de trava [cite: 25]
}

/**
 * @brief  Função principal
 */
void main(void) {
    // Executa as configurações iniciais uma vez.
    setup();

    //Codigo Ronan
    //Codigo Ronan

    WDTCTL = WDTPW | WDTHOLD;  // Parar watchdog
    

   
    // Escolher canal
    uint8_t channel = 1;
    
    i2c_config();

    if (pcf_teste(PCF_ADR)==FALSE){
                   //Indicar que não houve ACK
        while(TRUE);        //Travar
    }
    
    lcd_inic();     //Inicializar LCD
    pcf_write(0x08);   //Acender Back Light
    // Configurar ADC
    initADC(channel);
   
    uint16_t vetorLeituras[100];
    
    //__enable_interrupt();       // Habilitar interrupções globais

    // Esse loop faz 100 leituras e as armazena em "vetorLeituras"
    int indice;
    int valor=0;

    // Codigo Ronan

   



    // Loop principal infinito.
    while (1) {
        for (indice = 0; indice < 100; indice++) {
            vetorLeituras[indice] = readADC();
            __delay_cycles(1000);  // Atraso entre leituras
        }
        valor = readADC();

        if (valor <200) {
        // Máquina de Estados Finitos
          switch (estado_atual) {
              case TRAVADO:
              {
                  // No estado travado, o sistema aguarda a primeira tecla ser pressionada.
                  char tecla = keypad_get_char();
                  if (tecla != NO_KEY_PRESSED) {
                      feedback_keypress_confirm(); // Feedback sonoro/visual [cite: 26]
                      senha_digitada[indice_digito] = tecla;
                      indice_digito++;
                      estado_atual = RECEBENDO_SENHA; // Transita para o próximo estado
                  }
                  break;
              }

              case RECEBENDO_SENHA:
              {
                  // Continua recebendo as teclas até atingir o tamanho da senha.
                  char tecla = keypad_get_char();
                  if (tecla != NO_KEY_PRESSED) {
                      feedback_keypress_confirm();
                      if (indice_digito < TAMANHO_SENHA) {
                          senha_digitada[indice_digito] = tecla;
                          indice_digito++;
                      }
                  }

                  // Se a senha foi totalmente digitada, valida.
                  if (indice_digito >= TAMANHO_SENHA) {
                      senha_digitada[indice_digito] = '\0'; // Adiciona terminador nulo para usar strcmp

                      // Compara a senha digitada com a correta [cite: 27]
                      if (strcmp(senha_digitada, SENHA_PADRAO) == 0) {
                          estado_atual = DESTRAVADO; // Senha correta
                      } else {
                          estado_atual = ERRO; // Senha incorreta
                      }
                  }
                  break;
              }

              case DESTRAVADO:
              {
                  // Ações para o estado destravado.
                  feedback_show_unlocked(); // Acende LED verde [cite: 28]
                  servo_unlock();           // Gira o servo para a posição "Destravado" [cite: 28]

                  // Mantém o cofre aberto por 5 segundos.
                  // __delay_cycles usa o clock do sistema (MCLK). Se MCLK=1MHz, 5M de ciclos = 5s.
                  __delay_cycles(5000000);

                  // Prepara para a próxima operação, retornando ao estado travado.
                  indice_digito = 0;
                  memset(senha_digitada, 0, sizeof(senha_digitada)); // Limpa a senha anterior
                  estado_atual = TRAVADO;
                  feedback_show_locked(); // Retorna feedback visual para travado
                  servo_lock();           // Trava o servo novamente
                  break;
              }

              case ERRO:
              {
                  // Ações para o estado de erro de senha.
                  feedback_show_error(); // LED vermelho pisca e emite som [cite: 29]

                  // Limpa as variáveis e retorna ao estado travado para uma nova tentativa[cite: 29].
                  indice_digito = 0;
                  memset(senha_digitada, 0, sizeof(senha_digitada));
                  estado_atual = TRAVADO;
                  break;

              }
        }
        } // Fim do switch
    } // Fim do while(1)
} // Fim do main








void servo_init(void) {
    // 1. Configurar o pino P1.2 para sua função de periférico (saída do Timer TA0.1).
    //    Define a direção do pino como saída.
    P1DIR |= SERVO_PIN;
    //    Seleciona a função do pino (P1SEL.2 = 1) para ser controlada pelo Timer_A0.
    P1SEL |= SERVO_PIN;

    // 2. Definir o período do PWM em TA0CCR0.
    //    A frequência do PWM para servos é 50Hz, o que equivale a um período de 20ms.
    //    Cálculo: Período = Frequência_Clock * Tempo = 1,000,000Hz * 0.020s = 20000 ciclos.
    //    O timer conta de 0 até o valor em TA0CCR0, então o valor é 20000 - 1.
    TA0CCR0 = 20000 - 1;

    // 3. Configurar o modo de saída do PWM para o canal de comparação 1 (TA0.1).
    //    OUTMOD_7 (Modo Reset/Set): A saída do pino (P1.2) ficará em nível ALTO quando
    //    o timer zerar e ficará em nível BAIXO quando a contagem atingir o valor
    //    armazenado em TA0CCR1. Isso gera o pulso PWM desejado.
    TA0CCTL1 = OUTMOD_7;

    // 4. Configurar o registrador de controle do Timer_A0.
    //    TASSEL_2: Seleciona o SMCLK (Sub-System Master Clock) como fonte de clock.
    //    MC_1:     Inicia o timer em modo "Up". Ele conta de 0 até TA0CCR0 e reinicia.
    //    TACLR:    Limpa o timer, garantindo que ele comece do zero na primeira vez.
    TA0CTL = TASSEL_2 | MC_1 | TACLR;
}

/**
 * @brief Move o servo para a posição definida como "Travado".
 *
 * @note  Esta função apenas atualiza o valor do registrador de "duty cycle" (largura de pulso).
 * O valor SERVO_POS_LOCKED é definido em config.h.
 */
void servo_lock(void) {
    TA0CCR1 = SERVO_POS_LOCKED;
}

/**
 * @brief Move o servo para a posição definida como "Destravado".
 *
 * @note  Esta função apenas atualiza o valor do registrador de "duty cycle" (largura de pulso).
 * O valor SERVO_POS_UNLOCKED é definido em config.h.
 */
void servo_unlock(void) {
    TA0CCR1 = SERVO_POS_UNLOCKED;
}




/**
 * @brief Função de atraso simples (software delay).
 */
static void delay(volatile unsigned int cycles) {
    while (cycles--);
}

/**
 * @brief Inicializa os pinos de GPIO para o teclado matricial.
 */
void keypad_init(void) {
    // --- Configuração das Colunas (dividido por Port) ---
    // Configura as colunas que estão no Port 2 como SAÍDA
    P2DIR |= (KEYPAD_COL1_PIN | KEYPAD_COL3_PIN | KEYPAD_COL4_PIN);
    // Configura a coluna que está no Port 1 como SAÍDA
    P1DIR |= KEYPAD_COL2_PIN; // <-- ALTERAÇÃO IMPORTANTE

    // --- Configuração das Linhas (continuam todas no Port 2) ---
    // Configura os pinos das LINHAS como ENTRADA
    // Configura os pinos das LINHAS como ENTRADA
KEYPAD_PORT_DIR &= ~(KEYPAD_ROW1_PIN | KEYPAD_ROW2_PIN | KEYPAD_ROW3_PIN | KEYPAD_ROW4_PIN);

// Habilita resistores de PULL-UP para as linhas
KEYPAD_PORT_REN |= (KEYPAD_ROW1_PIN | KEYPAD_ROW2_PIN | KEYPAD_ROW3_PIN | KEYPAD_ROW4_PIN);
KEYPAD_PORT_OUT |= (KEYPAD_ROW1_PIN | KEYPAD_ROW2_PIN | KEYPAD_ROW3_PIN | KEYPAD_ROW4_PIN);
    // --- Estado Inicial ---
    // Garante que todas as colunas comecem em nível alto
    P2OUT |= (KEYPAD_COL1_PIN | KEYPAD_COL3_PIN | KEYPAD_COL4_PIN);
    P1OUT |= KEYPAD_COL2_PIN; // <-- ALTERAÇÃO IMPORTANTE
}
/**
 * @brief  Verifica o teclado e retorna o caractere da tecla pressionada.
 */
char keypad_get_char(void) {
    const unsigned char col_pins[] = {KEYPAD_COL1_PIN, KEYPAD_COL2_PIN, KEYPAD_COL3_PIN, KEYPAD_COL4_PIN};
    const unsigned char row_pins[] = {KEYPAD_ROW1_PIN, KEYPAD_ROW2_PIN, KEYPAD_ROW3_PIN, KEYPAD_ROW4_PIN};
    int col, row;

    for (col = 0; col < 4; col++) {
        // --- Ativa a coluna correta (coloca em LOW) ---
        if (col == 1) { // A coluna 2 (índice 1) é a que está no Port 1
            P1OUT &= ~col_pins[col];
        } else { // As outras colunas (0, 2, 3) estão no Port 2
            P2OUT &= ~col_pins[col];
        }

        delay(100);

        // A lógica de leitura das linhas não muda, pois todas estão no Port 2
        for (row = 0; row < 4; row++) {
            if (!(KEYPAD_PORT_IN & row_pins[row])) {
                while (!(KEYPAD_PORT_IN & row_pins[row]));

                // --- Desativa a coluna antes de retornar ---
                if (col == 1) { // Desativa a coluna do Port 1
                    P1OUT |= col_pins[col];
                } else { // Desativa a coluna do Port 2
                    P2OUT |= col_pins[col];
                }
                return keymap[row][col];
            }
        }

        // --- Desativa a coluna atual antes de passar para a próxima ---
        if (col == 1) { // Garante que a coluna do Port 1 volte para HIGH
            P1OUT |= col_pins[col];
        } else { // Garante que a coluna do Port 2 volte para HIGH
            P2OUT |= col_pins[col];
        }
    }

    return NO_KEY_PRESSED;
}

static void delay_ms(volatile unsigned int ms) {
    // Loop simples para gastar tempo. O valor 1000 foi estimado para 1MHz.
    while (ms--) {
        __delay_cycles(1000);
    }
}

/**
 * @brief Inicializa os pinos de GPIO para os LEDs e o Buzzer como saídas.
 */
void feedback_init(void) {
    // Configura os pinos dos LEDs e do Buzzer como saídas digitais.
    FEEDBACK_PORT_DIR |= (LED_VERDE_PIN | LED_VERMELHO_PIN | BUZZER_PIN);

    // Garante que todos os feedbacks comecem desligados.
    FEEDBACK_PORT_OUT &= ~(LED_VERDE_PIN | LED_VERMELHO_PIN | BUZZER_PIN);
}

/**
 * @brief Ativa o feedback visual para o estado "Travado".
 * Conforme o projeto, o LED vermelho deve ficar aceso.
 */
void feedback_show_locked(void) {
    // Apaga o LED verde e acende o LED vermelho.
    FEEDBACK_PORT_OUT &= ~LED_VERDE_PIN;
    FEEDBACK_PORT_OUT |= LED_VERMELHO_PIN;
}

/**
 * @brief Ativa o feedback visual para o estado "Destravado".
 * Conforme o projeto, o LED verde deve acender.
 */
void feedback_show_unlocked(void) {
    // Apaga o LED vermelho e acende o LED verde.
    FEEDBACK_PORT_OUT &= ~LED_VERMELHO_PIN;
    FEEDBACK_PORT_OUT |= LED_VERDE_PIN;
}

/**
 * @brief Ativa o feedback de erro (visual e sonoro).
 * Conforme o projeto, o LED vermelho pisca e um som de erro é emitido.
 */
void feedback_show_error(void) {
    int i;
    // Desliga o LED verde para garantir.
    FEEDBACK_PORT_OUT &= ~LED_VERDE_PIN;

    // Ativa o buzzer para um som de erro mais longo.
    FEEDBACK_PORT_OUT |= BUZZER_PIN;

    // Pisca o LED vermelho 3 vezes.
    for (i = 0; i < 3; i++) {
        FEEDBACK_PORT_OUT |= LED_VERMELHO_PIN;  // Acende
        delay_ms(150);
        FEEDBACK_PORT_OUT &= ~LED_VERMELHO_PIN; // Apaga
        delay_ms(150);
    }

    // Desliga o buzzer.
    FEEDBACK_PORT_OUT &= ~BUZZER_PIN;
}

/**
 * @brief Emite um 'beep' curto para confirmar o toque de uma tecla.
 * Conforme o projeto, um feedback sonoro é fornecido a cada toque.
 */
void feedback_keypress_confirm(void) {
    FEEDBACK_PORT_OUT |= BUZZER_PIN;    // Liga o buzzer
    delay_ms(30);                       // Por um curto período
    FEEDBACK_PORT_OUT &= ~BUZZER_PIN;   // Desliga o buzzer
}


void feedback_init(void);

/**
 * @brief Configura os LEDs para indicar o estado "Travado".
 * Geralmente, acende o LED vermelho.
 * @param None
 * @return None
 */
void feedback_show_locked(void);

/**
 * @brief Configura os LEDs para indicar o estado "Destravado".
 * Geralmente, acende o LED verde.
 * @param None
 * @return None
 */
void feedback_show_unlocked(void);

/**
 * @brief Ativa um feedback de erro visual e sonoro.
 * Geralmente, pisca o LED vermelho e emite um som no buzzer.
 * @param None
 * @return None
 */
void feedback_show_error(void);

/**
 * @brief Fornece um feedback curto (sonoro) para confirmar que uma
 * tecla foi pressionada.
 * @param None
 * @return None
 */
void feedback_keypress_confirm(void);

#endif /* STATUS_FEEDBACK_H */



// Codigo Ronan 


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
    
    ADC12CTL2 = ADC12RES_0;             // Resolução de 12 bits (RES_0 = 8 bits, RES_1 = 10 bits, RES_2 = 12 bits). O valor lido ficará entre 0 e (2^n)-1, n = número de bits
    
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
    delay1(20000);
    lcd_aux(3);     //3
    delay1(10000);
    lcd_aux(3);     //3
    delay1(10000);
    lcd_aux(3);     //3
    delay1(10000);
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
    delay1(50);
}

// Auxiliar inicialização do LCD (RS=RW=0)
// * Só serve para a inicialização *
void lcd_aux(char dado){
    while ( (UCB0IFG & UCTXIFG) == 0);              //Esperar TXIFG=1
    UCB0TXBUF = ((dado<<4)&0XF0) | BIT3;            //PCF7:4 = dado;
    delay1(50);
    while ( (UCB0IFG & UCTXIFG) == 0);              //Esperar TXIFG=1
    UCB0TXBUF = ((dado<<4)&0XF0) | BIT3 | BIT2;     //E=1
    delay1(50);
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



// Configurar leds

void delay1(long limite){
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

// FIM codigo Ronan
