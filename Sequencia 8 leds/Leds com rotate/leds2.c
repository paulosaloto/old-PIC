// Este programa realiza a contagem de 0 a 255 nos leds do Kit SD-1700.

#include <16F877a.h>             //Inclue a biblioteca 16F877a.h, dispon�vel em
                                 // c:\Arquivos de programas\PICC\Devices

#fuses XT,NOWDT,NOPROTECT,PUT    //Define para o compilador: a faixa de frequ�ncia do clock (pode ser LS, XT, HS),
                                 //n�o uso do watchdog timer,
                                 //n�o uso de prote��o de c�digo,
                                 //uso do Power-up-timer, temporizador utilizado para inicializa��o do PIC.

#use delay(clock=4000000)        //Define a frequ�ncia de clock.

/* Estas diretivas definem que o controle E/S das portas ser� definido pelo programador */
#use fast_io(a)
#use fast_io(b)
#use fast_io(c)
#use fast_io(d)
#use fast_io(e)

/* Defini��o de ponteiros para as portas */
/* Esta defini��o � utilizada pois a biblioteca 16F877A.h n�o associa o r�tulo portX com o endere�o da referida porta */
/* � uma prefer�ncia minha de uso, n�o obrigat�ria, pode ser utilizada a refer�ncia a pinos das portas conforme 16F877A.h */

#byte porta = 5
#byte portb = 6
#byte portc = 7
#byte portd = 8
#byte porte = 9
#byte status = 3
main()
{

   // Configura��o de vias das portas do PIC como sa�da ou como entrada.
   // Se o bit 0 do registrador especial TRISA � 0, a via 0 da portA do PIC � sa�da, caso contr�rio � entrada.
   // Assim s�o configuradas todas as vias das portas do PIC, de qualquer porta.
   // Exemplo: TRISA = | 0 | 1 | 0 | 1 | 0 | 1 | -> PORTA = | S | E | S | E | S | E | , no 16F877A a portA tem 6 vias.
   int work;
   int dir;
   set_tris_a(0x00); // Define que todas as vias da portA do PIC ser�o sa�das.
   set_tris_d(0x00); // Define que todas as vias da portD do PIC ser�o sa�das.
   set_tris_b(0xF0);
   bit_set(porta,4); // Ativa os leds, saturando o transistor chave no catodo comum dos leds. Vide p�gina 13 do manual.
   port_b_pullups(TRUE);
   bit_clear(portb,1);
   portd = (0xFF); // Define todos os leds acesos inicialmente, valor inicial da contagem FF (zero).
   work = (0xFF);
   dir = 1;
   while (1)
   {
      if (!bit_test(portb,5))
      {
         delay_ms(70);
         if (!bit_test(portb,5))
            if (dir==0) dir=1;
         else dir=0;
         do
         {
            delay_ms(100);
         }while(!bit_test(portb,5));
      }


      if(dir!=0)
      {
         if(portd>0x01)
         {
            rotate_right(&work,2);
            bit_clear (work,7);
            delay_ms(100);
         }
         else
         {
            work = 0;
            delay_ms(100);
         }
         portd = work;
      }
         if(dir==0 && work<0xFF)
      {
         rotate_left(&work,2);
         work=work+1;
         portd = work;
         delay_ms(100);
      }
   }
}


