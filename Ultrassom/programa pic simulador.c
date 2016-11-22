//Programa Servo Motor Controlado - Sistemas Embarcados

/*O programa abaixo atende três tarefas principais:

1 - Obter o ângulo_pedido pelo operador, e então, através dos parâmtetros do servo, o valor_ref (long int). [Interrupção RB4-RB7]
2 - Obter, por conversão AD, o valor da tensão no potênciômetro, valor_pot (long int). [Interrupção Timer0]
3 - Ajustar a posição do servo. [Programa Principal]

*/

#include <16F877a.h>             //Inclui a biblioteca 16F877a.h, disponível em
                                // c:\Arquivos de programas\PICC\Devices

#fuses XT,NOWDT,NOPROTECT,PUT    //Define para o compilador: a faixa de frequência do clock (pode ser LS, XT, HS),
                                 //não uso do watchdog timer,
                                 //não uso de proteção de código,
                                 //uso do Power-up-timer, temporizador utilizado para inicialização do PIC.
#use delay(clock=4000000)        //Define a frequência de clock.
/* Estas diretivas definem que o controle E/S das portas será definido pelo programador */
#use fast_io(a)
#use fast_io(b)
#use fast_io(c)
#use fast_io(d)
#use fast_io(e)
/* Definição de ponteiros para as portas */
/* Esta definição é utilizada pois a biblioteca 16F877A.h não associa o rótulo portX com o endereço da referida porta */
/* É uma preferência minha de uso, não obrigatória, pode ser utilizada a referência a pinos das portas conforme 16F877A.h */

#byte porta = 5
#byte portb = 6
#byte portc = 7
#byte portd = 8
#byte porte = 9

void main()
{  
////////////////////////////////////////////////////////////////////////////////
////                       CONFIGURAÇÕES DO PIC                            /////
////////////////////////////////////////////////////////////////////////////////
   // Configuração de vias das portas do PIC como saída ou como entrada.
   // Se o bit 0 do registrador especial TRISA é 0, a via 0 da portA do PIC é saída, caso contrário é entrada.
   // Assim são configuradas todas as vias das portas do PIC, de qualquer porta.
   // Exemplo: TRISA = | 0 | 1 | 0 | 1 | 0 | 1 | -> PORTA = | S | E | S | E | S | E | , no 16F877A a portA tem 6 vias.
   
   set_tris_a(0x01); // Define que somente 1 via do PIC será entrada.
   delay_us(100);
   
   int recebeu=0, i=0;
   long int x=1000;
   
    while(1)
   {
      //Primeiro está funcionando como entrada
      if(bit_test(porta,0)==1){
         recebeu++;
         set_tris_a(0x00); //Configura como saída
         bit_set(porta,0); //Sobe
         delay_us(7000);
         bit_clear(porta,0); //Desce
         set_tris_a(0x01); //Configura como entrada
         delay_us(100);
          x=x-2;
          if (x<1000) x=5000;
          
            
      }
   }
}




