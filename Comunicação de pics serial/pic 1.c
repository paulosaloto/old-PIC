//Programa Servo Motor Controlado - Sistemas Embarcados

/*O programa abaixo atende tr�s tarefas principais:

1 - Obter o �ngulo_pedido pelo operador, e ent�o, atrav�s dos par�mtetros do servo, o valor_ref (long int). [Interrup��o RB4-RB7]
2 - Obter, por convers�o AD, o valor da tens�o no pot�nci�metro, valor_pot (long int). [Interrup��o Timer0]
3 - Ajustar a posi��o do servo. [Programa Principal]

*/

#include <16F877a.h>             //Inclui a biblioteca 16F877a.h, dispon�vel em
                                // c:\Arquivos de programas\PICC\Devices
#device adc=10 //10bits


#define EEPROM_ADDRESS long int
#define EEPROM_SIZE   32768

#fuses XT,NOWDT,NOPROTECT,PUT    //Define para o compilador: a faixa de frequ�ncia do clock (pode ser LS, XT, HS),
                                 //n�o uso do watchdog timer,
                                 //n�o uso de prote��o de c�digo,
                                 //uso do Power-up-timer, temporizador utilizado para inicializa��o do PIC.
#use delay(clock=4000000)        //Define a frequ�ncia de clock.

#use rs232(baud=9600,parity=N,xmit=PIN_C6,rcv=PIN_C7,bits=8)
/* Estas diretivas definem que o controle E/S das portas ser� definido pelo programador */
#use fast_io(a)
#use fast_io(b)
#use fast_io(c)
#use fast_io(d)
#use fast_io(e)

/* Defini��o de ponteiros para as portas */
/* Esta defini��o � utilizada pois a biblioteca 16F877A.h n�o associa o r�tulo portX com o endere�o da referida porta */
/* � uma prefer�ncia minha de uso, n�o obrigat�ria, pode ser utilizada a refer�ncia a pinos das portas conforme 16F877A.h */

#include <i2c.c>               //biblioteca para escrita e leitura da mem�ria 24lc256 e RTC PFC8583 atrav�s do i2c

#byte porta = 5
#byte portb = 6
#byte portc = 7
#byte portd = 8
#byte porte = 9

#include <lcdkit1.c> 
//Vari�veis globais//
int1 sentido_ant=1;
unsigned int angulo_pedido=0, angulo_pedido_anterior;
long int  valor_pot, erro, integral, controle, valor_ref=0;
double valor_aux;
//Vari�veis DescobreNumero, Interrupt RB4-RB7
unsigned int num_digitos, interrupcao=0,int_anterior, tecla, i;
long int numero;
//vari�veis RTC e mem�ria
int angulo=0, hora=0, minutos=0, segundos=0, dia =0, mes =0, ano =0, data =0, resto=0, horario=0, aux_timer=0, posicao=0, quantidade=0;
int igualdade[5]={0, 0, 0, 0, 0}, posicao_inicial=0;
int hora2=0, angulo_enviado[3], minutos2=0, segundos2=0, kp;
//Interrup��o do tecladoint 

#int_RB
int RB_isr()
{
   int_anterior=interrupcao;
   interrupcao=portb;
   if(interrupcao>0xF0)interrupcao=int_anterior;
   return(0);
}

void varredura(){
//Varredura do teclado: Testa se n�o h� bot�es pressionados. se n�o houver, realiza a varredura das colunas.
   //Interrup��o: quando h� uma interrup��o, esta parte do programa entra em a��o para trabalhar o valor obtido
      if (interrupcao != 0) //
      {
         for(i=0;i<=150;i++)
         {  
            if(bit_test(porte,2))
               bit_clear(porte,2);
            else bit_set(porte,2);
            delay_us(200);
         }
         delay_ms(170);
         
         if(interrupcao<0xF0)//se for uma interrup��o onde o bot�o � pressionado, ele entra na fun��o para obter o n�mero correspondente.
         {
            for (i=4; i<=7; i++){                     
               if (!bit_test (interrupcao,i)) {
                  tecla = i;
                  break;
               }
            }
            for (i=0;i<=2;i++){
               if (!bit_test (interrupcao,i)) {
                  tecla=(tecla-4)*3+i+1;
                  break;
               }
            }
         }
         interrupcao=0;//se for quando o botao � solto, n�o faz nada.
         
      }
   if (bit_test(portb,7) && bit_test(portb,6) && bit_test(portb,5) && bit_test(portb,4))
   {
      if(!bit_test(portb,2)) {//se o portb tiver os bits menos significativos 011, passar� a ter 110
         bit_clear(portb,0);
         bit_set(portb,2);
      }
      else 
         if(!bit_test(portb,1)) {//de mesmo modo, se tiver 101, passar� a ter 011
            bit_clear(portb,2);
            bit_set(portb,1);
         }
         else
            if(!bit_test(portb,0)) {//e se tier 110, passar� a ter 101
               bit_clear(portb,1);
               bit_set(portb,0);
            }
   }

}

void def_angulo ()
{
   switch(tecla){
      case(10):
         angulo=0;
         num_digitos=0;
         return;
      case(11):
         tecla=0;
         break;
      case(12):
         angulo_pedido=angulo;
         angulo_enviado[0]=angulo_pedido/100;
         angulo_enviado[1]=(angulo_pedido - angulo_enviado[0]*100)/10;
         angulo_enviado[2]=(angulo_pedido - angulo_enviado[0]*100 - angulo_enviado[1]*10)
         printf("C");
         for(i=0;i==2;i++)
            printf("%i",angulo_enviado[i]);
            printf("F");
         tecla=0;
         angulo=0;
         numero=5;
         return;
   }

   if (angulo==0 || (angulo*10+tecla)>180) angulo=tecla;
   else angulo=angulo*10+tecla;
   tecla=0;
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
   set_tris_c(0x00); //
   
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
   
   setup_timer_0 (RTCC_INTERNAL|RTCC_DIV_128);   //Configura��o do Timer0 para clock interno = 1E6 dividido por 256
  
   delay_us(100);
  
   enable_interrupts(GLOBAL);
   enable_interrupts(INT_rb);
   enable_interrupts(INT_timer0);
   //init_ext_eeprom();
   lcd_init();
   delay_ms(10);
   printf(lcd_putc,"\f    Programa \nPaulo e  Mariana");
   
   setup_psp(PSP_DISABLED);
   interrupcao=0;
   i=0;
   delay_ms(1000) ;
   //o looping da fun��o main se divide em 2 partes: inser��o dos angulos e hor�rios e a��o de controle do motor.
/*
////////////////////////////////////////////////////////////////////////////////
////                CONFIGURA��O DO RTC PELO USU�RIO                        ////
////////////////////////////////////////////////////////////////////////////////
   
   printf(lcd_putc,"\f Apagar memoria? \n Sim:#    Nao:*");
   while(tecla!=0x0c && tecla!=0x0a)
   {
      varredura();
      if(tecla==0x0c) quantidade=0;
      if(tecla==0x0a) quantidade=read_ext_eeprom(200);
   }
   printf(lcd_putc,"\f   Data Atual:");
   while(data!=3) //Tamb�m vai aumentando at� ter ano, mes e dia.
   {
      varredura();
      write_data();   
   }     

   printf(lcd_putc,"\f   Hora Atual:");
   while(horario!=3){
      varredura();
      write_horaminseg(1);
   }

////////////////////////////////////////////////////////////////////////////////
////             INSER��O DA QUANTIDADE DE �NGULOS PELO USU�RIO             ////
////////////////////////////////////////////////////////////////////////////////
   tecla=0;
   segundos2=read_RTC(2);
   minutos2=read_RTC(3);
   hora2=read_RTC(4);
   printf(lcd_putc,"\f# p/ Novo Angulo\n    %2x:%2x:%2x    ",hora2,minutos2,segundos2);
*/
   while(1)
   {
////////////////////////////////////////////////////////////////////////////////
////               INSER��O DAS DATAS E  �NGULOS PELO USU�RIO               ////
////////////////////////////////////////////////////////////////////////////////         
      if(aux_timer==0){
         printf(lcd_putc,"\n  %u'",angulo_pedido);
      }     
/*      
      varredura();
      if(tecla==0x0c)      
      {
         horario=0;
         printf(lcd_putc,"\fHorario Angulo %d:",quantidade);
         while(horario!=3)
         {
            varredura();
            write_horaminseg(0);
         }
*/
        printf(lcd_putc,"\f Valor Angulo %d:",quantidade);
        for(numero=0;numero<=2;numero++)
         {
            varredura();
            if (tecla!=0){
               def_angulo();
               printf(lcd_putc,"\n       %u",angulo);
               delay_ms(10);
            }
            numero--;
         }
         numero=0;
//         quantidade=quantidade+1;
//         write_ext_eeprom(200,quantidade);
         printf(lcd_putc,"\f# p/ Novo Angulo");
         
         
         //sonzinho para testar :P
         for(i=0;i<=175;i++)
         {  
            if(bit_test(porte,2))
               bit_clear(porte,2);
            else bit_set(porte,2);
            delay_us(191);
         }
         for(i=0;i<=219;i++)
         {  
            if(bit_test(porte,2))
               bit_clear(porte,2);
            else bit_set(porte,2);
            delay_us(152);
         }
         for(i=0;i<=254;i++)
         {  
            if(bit_test(porte,2))
               bit_clear(porte,2);
            else bit_set(porte,2);
            delay_us(128);
         }
         for(i=0;i<=254;i++)
         {  
            if(bit_test(porte,2))
               bit_clear(porte,2);
            else bit_set(porte,2);
            delay_us(128);
         }

      
   }
}

