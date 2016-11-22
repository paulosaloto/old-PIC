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

//Vari�veis usadas pelas interrup��es//
int angulo_pedido;
int angulo_pedido_anterior;
long int valor_pot;

//Fim das v�riaveis usadas pelas interrup��es//

//Interrup��o do teclado
/*
interrupt_teclado()
{
angulo_pedido = ? //Aqui tem que entrar o seu valor entre 0 e 180�
}
*/

//Interrup��o do timer0
#int_timer0
interrupt_timer0(){
valor_pot = read_adc();
}

void main()
{ 
   //Vari�veis respons�veis pela transforma��o �ngulo_pedido => valor_ref
   long int valor_ref;
   double valor_aux;
   
   //Isso faz o motor ir inicialmente para o �ngulo que vc quiser. O padr�o � 0, mas pode jogar a� qlqr valor pra testar.
   angulo_pedido=170;
   angulo_pedido_anterior=10;
   
   // Configura��o de vias das portas do PIC como sa�da ou como entrada.
   // Se o bit 0 do registrador especial TRISA � 0, a via 0 da portA do PIC � sa�da, caso contr�rio � entrada.
   // Assim s�o configuradas todas as vias das portas do PIC, de qualquer porta.
   // Exemplo: TRISA = | 0 | 1 | 0 | 1 | 0 | 1 | -> PORTA = | S | E | S | E | S | E | , no 16F877A a portA tem 6 vias.
   
   set_tris_a(0x01); // Define que somente 1 via do PIC ser� entrada.
   set_tris_b(0x00); //
   set_tris_c(0x00); //
   set_tris_d(0x00); //
  
   //Configura��o do Conversor AD (Potenci�metro do Servo)
   setup_adc_ports(ALL_ANALOG);
   setup_adc(ADC_CLOCK_DIV_8);
   set_adc_channel(0);
   delay_us(100);
   
   //Configura��o do PWM, que controla a velocidade do servo
   
   setup_ccp1(ccp_pwm); //Configura CCP1 como PWM
   
   //Clock interno : (1/clock)*4    //4 clocks internos = 1 clock externo
   //O tempo de clock ser�: clock_interno*Div_TM2*(Periodo+1);
   setup_timer_2(T2_DIV_BY_1,255, 1);
   
   //Declara��o Par�metros Timer0
   setup_timer_0 (T0_INTERNAL | T1_DIV_BY_1);   //Configura��o do Timer0 para clock interno = 1E6 dividido por 256
   set_timer0 (247);
  
   delay_us(100);
  
   enable_interrupts(GLOBAL);
   enable_interrupts(INT_rb);
   enable_interrupts(INT_timer0);
   EXT_INT_EDGE(L_TO_H);
        
   while(1){
   
   
   if(angulo_pedido!=angulo_pedido_anterior){ //O c�digo s� precisa entrar aqui qnd angulo_pedido � alterado.
   //Adotamos tr�s faixas de valores para definir valor_ref
   //Uma para �ngulos entre 0 e 60�
   if ((angulo_pedido>=0)&&(angulo_pedido<=60)){
   valor_aux = 6.8858*angulo_pedido + 51.515; //Esse � double, ent�o o n�mero � "quebrado"
   valor_ref = valor_aux; //Como esse � long int, o n�mero aqui � inteiro
   //Aproxima��o
   if( (valor_aux - valor_ref) >= 0.5)
      valor_ref = valor_ref+1;
   }
   
   //Outra para �ngulos entre 60� e 100�
   if ((angulo_pedido>60)&&(angulo_pedido<100)){
   valor_aux = 0.0019*angulo_pedido*angulo_pedido*angulo_pedido - 0.4852*angulo_pedido*angulo_pedido + 41.224*angulo_pedido - 681.49; //Esse � double, ent�o o n�mero � "quebrado"
   valor_ref = valor_aux; //Como esse � long int, o n�mero aqui � inteiro
   //Aproxima��o
   if( (valor_aux - valor_ref) >= 0.5)
      valor_ref = valor_ref+1;
   }
   
   //Outra para �ngulos entre 100� e 180�
   if ((angulo_pedido>=100)&&(angulo_pedido<=180)){
   valor_aux = (-0.0372)*angulo_pedido*angulo_pedido + 16.419*angulo_pedido - 755.86; //Esse � double, ent�o o n�mero � "quebrado"
   valor_ref = valor_aux; //Como esse � long int, o n�mero aqui � inteiro
   //Aproxima��o
   if( (valor_aux - valor_ref) >= 0.5)
      valor_ref = valor_ref+1;
   }
   angulo_pedido_anterior = angulo_pedido;
   }
   
   //valor_pot = read_adc(); //ISSO EST� AQUI S� PARA TESTES! ISSO FICA DENTRO DA INTERRUPT TIMER0.
   
   //erro = valor_pot-valor_ref; //s� para facilitar o racioc�nio da parte abaixo.
   
   if (valor_pot>=valor_ref){ //Ele deve diminuir a leitura do potenci�metro!
   bit_clear(portc,1);
   set_pwm1_duty(valor_pot-valor_ref); //"0" e "0" aqui � parado
   }
    
   if (valor_pot<valor_ref){ //Ele deve aumentar a leitura do potenci�metro!
   bit_set(portc,1);
   set_pwm1_duty(1023-(valor_ref-valor_pot)); //"1" e "1" aqui � parado
   }
   
  }
  
}
