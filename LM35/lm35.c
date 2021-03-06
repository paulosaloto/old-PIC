//Programa Servo Motor Controlado - Sistemas Embarcados

/*O programa abaixo atende tr�s tarefas principais:

1 - Obter o �ngulo_pedido pelo operador, e ent�o, atrav�s dos par�mtetros do servo, o valor_ref (long int). [Interrup��o RB4-RB7]
2 - Obter, por convers�o AD, o valor da tens�o no pot�nci�metro, valor_pot (long int). [Interrup��o Timer0]
3 - Ajustar a posi��o do servo. [Programa Principal]

*/

#include <16F877a.h>             //Inclui a biblioteca 16F877a.h, dispon�vel em
                                // c:\Arquivos de programas\PICC\Devices
#device adc=10 //10bits


#fuses HS,NOWDT,NOPROTECT,PUT    //Define para o compilador: a faixa de frequ�ncia do clock (pode ser LS, XT, HS),
                                 //n�o uso do watchdog timer,
                                 //n�o uso de prote��o de c�digo,
                                 //uso do Power-up-timer, temporizador utilizado para inicializa��o do PIC.
#use delay(clock=8000000)        //Define a frequ�ncia de clock.
/* Estas diretivas definem que o controle E/S das portas ser� definido pelo programador */

#use fast_io(a)
#use fast_io(b)
#use fast_io(c)
#use fast_io(d)
#use fast_io(e)

/* Defini��o de ponteiros para as portas */
/* Esta defini��o � utilizada pois a biblioteca 16F877A.h n�o associa o r�tulo portX com o endere�o da referida porta */
/* � uma prefer�ncia minha de uso, n�o obrigat�ria, pode ser utilizada a refer�ncia a pinos das portas conforme 16F877A.h */

#INCLUDE <stdlib.h>

#include <mod_lcd.c> 
#byte porta = 5
#byte portb = 6
#byte portc = 7
#byte portd = 8
#byte porte = 9

//Vari�veis globais//

   long int tensao;
   unsigned int temp;

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
   set_tris_c(0x00); //
   set_tris_d(0x00); //
   set_tris_e(0x00); //
   //Configura��o inicial do teclado
   portb=0;
   port_b_pullups (true);

   portd=0;
  
   //Configura��o do Conversor AD (Potenci�metro do Servo)
   setup_adc_ports(RA0_RA1_RA3_ANALOG);
   setup_adc(ADC_CLOCK_DIV_8);
   set_adc_channel(1);
 
   delay_ms(100);

   lcd_ini();
   delay_ms(100);

   printf(lcd_escreve,"\fQUADRADO VIADO");
   delay_ms(1000);

////////////////////////////////////////////////////////////////////////////////
////                                LOOPING                                 ////
////////////////////////////////////////////////////////////////////////////////         
   while(1)
   {
      tensao=(read_adc());
      temp=tensao/2.046;
      printf(lcd_escreve,"\f");
      printf(lcd_escreve,"V:%04lu\nT:%02d", tensao, temp);
      bit_set(portc,3);
      delay_ms(200);
      bit_clear(portc,3);
      delay_ms(200);
   }
}

