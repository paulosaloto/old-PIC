//Programa Servo Motor Controlado - Sistemas Embarcados

/*O programa abaixo atende tr�s tarefas principais:

1 - Obter o �ngulo_pedido pelo operador, e ent�o, atrav�s dos par�mtetros do servo, o valor_ref (long int). [Interrup��o RB4-RB7]
2 - Obter, por convers�o AD, o valor da tens�o no pot�nci�metro, valor_pot (long int). [Interrup��o Timer0]
3 - Ajustar a posi��o do servo. [Programa Principal]

*/

#include <16F877a.h>             //Inclui a biblioteca 16F877a.h, dispon�vel em
                                // c:\Arquivos de programas\PICC\Devices
#device adc=10 //10bits


#fuses XT,NOWDT,NOPROTECT,PUT    //Define para o compilador: a faixa de frequ�ncia do clock (pode ser LS, XT, HS),
                                 //n�o uso do watchdog timer,
                                 //n�o uso de prote��o de c�digo,
                                 //uso do Power-up-timer, temporizador utilizado para inicializa��o do PIC.
#use delay(clock=4000000)        //Define a frequ�ncia de clock.
/* Estas diretivas definem que o controle E/S das portas ser� definido pelo programador */

#use rs232(baud=9600, xmit=PIN_C6,rcv=PIN_C7,BITS =8)
#use fast_io(a)
#use fast_io(b)
#use fast_io(c)
#use fast_io(d)
#use fast_io(e)

/* Defini��o de ponteiros para as portas */
/* Esta defini��o � utilizada pois a biblioteca 16F877A.h n�o associa o r�tulo portX com o endere�o da referida porta */
/* � uma prefer�ncia minha de uso, n�o obrigat�ria, pode ser utilizada a refer�ncia a pinos das portas conforme 16F877A.h */

#include <i2c.c>               //biblioteca para escrita e leitura da mem�ria 24lc256 e RTC PFC8583 atrav�s do i2c
#INCLUDE <stdlib.h>

#include <lcdkit1.c> 
#byte porta = 5
#byte portb = 6
#byte portc = 7
#byte portd = 8
#byte porte = 9

//Vari�veis globais//

unsigned int angulo_pedido=0, angulo_pedido_anterior, distancia;
long int  angulo;
//Vari�veis DescobreNumero, Interrupt RB4-RB7
unsigned int num_digitos, interrupcao=0,int_anterior, tecla, i, flag, x, y, z;
long int numero;
//variaveis da comunica��o serial
int checksum_tx;
char angulo_dividido3_char[3];
int angulo_enviado[3];
unsigned long int sobe, desce;

//Interrup��o do tecladoint
void buzzer(int a, long int b)
{
   for(i=0;i<=a;i++)
   {  
      if(bit_test(porte,2))
         bit_clear(porte,2);
      else bit_set(porte,2);
      delay_us(b);
   }
   bit_clear(porte,2);
}

#int_CCP2
void descida() 
{
   if(flag==2)
   {
      sobe=CCP_1; 
      desce=CCP_2;   
      flag=1;
      z++;
   }
}

void QuebraAngulo(unsigned int angulo_qualquer){

   angulo_enviado[0]=angulo_pedido/100;
   angulo_enviado[1]=(angulo_pedido - angulo_enviado[0]*100)/10;
   angulo_enviado[2]=(angulo_pedido - angulo_enviado[0]*100 - angulo_enviado[1]*10);
   checksum_tx = angulo_enviado[0]+angulo_enviado[1]+angulo_enviado[2];
   if(checksum_tx > 9)
      checksum_tx=checksum_tx/10 + checksum_tx%10;
   sprintf(angulo_dividido3_char, "%ui", angulo_pedido);
   if(angulo_pedido<100){
      angulo_dividido3_char[2]=angulo_dividido3_char[1];
      angulo_dividido3_char[1]=angulo_dividido3_char[0];
      angulo_dividido3_char[0]='0';
   }
   if(angulo_pedido<10){
      angulo_dividido3_char[2]=angulo_dividido3_char[1];
      angulo_dividido3_char[1]=angulo_dividido3_char[0];
      angulo_dividido3_char[0]='0';
   }
   

}

void EnviaAngulo(){

   QuebraAngulo(angulo_pedido);
   printf("C");  
   printf("%c",angulo_dividido3_char[0]);
   printf("%c",angulo_dividido3_char[1]);
   printf("%c",angulo_dividido3_char[2]);
   printf("%i",checksum_tx);
   printf("F");
}

void main()
{  
////////////////////////////////////////////////////////////////////////////////
////                       CONFIGURA��ES DO PIC                            /////
////////////////////////////////////////////////////////////////////////////////
   // Configura��o de vias das portas do PIC como sa�da ou como entrada.
   // Se o bit 0 do registrador especial TRISA � 0, a via 0 da portA do PIC � sa�da, caso contr�rio � entrada.
   // Assim s�o configuradas todas as vias das portas do PIC, de qualquer porta.
   // Exemplo: TRISA = | 0 | 1 | 0 | 1 | 0 | 1 | -> PORTA = | S | E | S | E | S | E | , no 16F877A a portA tem 6 vias.
   
   set_tris_a(0x02); // Define que somente 1 via do PIC ser� entrada.
   set_tris_b(0xF8); //
   set_tris_c(0x80); //
   
   //Configura��o inicial do teclado
   portb=0;
   port_b_pullups (true);
      
   bit_set(porta,4); // Ativa os leds, saturando o transistor chave no catodo comum dos leds. Vide p�gina 13 do manual.
   portd=0;
   
   //Isso faz o motor ir inicialmente para o �ngulo que vc quiser. O padr�o � 0, mas pode jogar a� qlqr valor pra testar.
   angulo_pedido=0;
   angulo_pedido_anterior=10;
  
   //Configura��o do Conversor AD (Potenci�metro do Servo)
   setup_adc_ports(RA0_RA1_RA3_ANALOG);
   setup_adc(ADC_CLOCK_DIV_8);
   set_adc_channel(1);
   delay_us(100);
   
   //Configura��o do PWM, que controla a velocidade do servo
   
   setup_ccp1(ccp_pwm); //Configura CCP1 como PWM
   
   //Clock interno : (1/clock)*4    //4 clocks internos = 1 clock externo
   //O tempo de clock ser�: clock_interno*Div_TM2*(Periodo+1);
   setup_timer_2(T2_DIV_BY_1,255, 1);
   setup_timer_1(T1_INTERNAL|T1_DIV_BY_2); // Start timer 1 a 1us contando
   setup_timer_0 (RTCC_INTERNAL|RTCC_DIV_128);   //Configura��o do Timer0 para clock interno = 1E6 dividido por 256
  
   delay_us(100);
   lcd_init();  
   
   setup_ccp1(CCP_CAPTURE_RE); // Configure to capture rise
   setup_ccp2(CCP_CAPTURE_FE); // Configure CCP2 to capture fall
   
   enable_interrupts(INT_RDA);
   enable_interrupts(GLOBAL);
   enable_interrupts(INT_CCP2);
   
   delay_ms(10);
   interrupcao=0;
   i=0;
   printf(lcd_putc,"\f    ULTRASOM");
   delay_ms(1000);
   printf(lcd_putc,"\fdifer.: %lu \ndistancia: %ud", (desce=sobe), distancia);

   delay_ms(10);
   //delay_ms(1000) ;
   //o looping da fun��o main se divide em 2 partes: inser��o dos angulos e hor�rios e a��o de controle do motor.
////////////////////////////////////////////////////////////////////////////////
////               INSER��O DAS DATAS E  �NGULOS PELO USU�RIO               ////
////////////////////////////////////////////////////////////////////////////////         
   while(1)
   {
      // CASO O SINAL DO ULTRASOM SEJA RECEBIDO, ENTRA AQUI
      if (flag==1)
      {
         distancia=(desce-sobe)*0.03465;//verificar se o multiplicador est� certo
         if(distancia>180) distancia=180;
         if(distancia<10) distancia=10;
         printf(lcd_putc,"\timer1: %5lu \ndistancia: %3u",(desce-sobe),distancia);
         EnviaAngulo();
         flag=0;
         delay_us(5);
         y++;
      }
      // PARA VOLTAR A ENVIAR UM NOVO PULSO
         if(flag==0)
         {
            set_tris_a(0x00);
            
            bit_set(porta,1);
            delay_us(100);  
            bit_clear(porta,1);
            set_tris_a(0x02);
            //set_timer1(0);
            delay_us(750);
            flag=2;
            x++;
         }
   }
}

