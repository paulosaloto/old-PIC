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
#use rs232(baud=9600, xmit=PIN_C6,rcv=PIN_C7,BITS =8) //LEMBRAR DE ACIONAR AP�S #use delay(clock=4000000)

#use fast_io(a)
#use fast_io(b)
#use fast_io(c)
#use fast_io(d)
#use fast_io(e)

/* Defini��o de ponteiros para as portas */
/* Esta defini��o � utilizada pois a biblioteca 16F877A.h n�o associa o r�tulo portX com o endere�o da referida porta */
/* � uma prefer�ncia minha de uso, n�o obrigat�ria, pode ser utilizada a refer�ncia a pinos das portas conforme 16F877A.h */

//#include <i2c.c>               //biblioteca para escrita e leitura da mem�ria 24lc256 e RTC PFC8583 atrav�s do i2c

#byte porta = 5
#byte portb = 6
#byte portc = 7
#byte portd = 8
#byte porte = 9

#include <lcdkit1.c> 
//Vari�veis globais//
int1 sentido_ant=1;
unsigned int angulo_pedido=0, angulo_pedido2=0, angulo_pedido_anterior, stop;
long int  valor_pot, erro, integral, controle, valor_ref=0;
double valor_aux;
//Vari�veis DescobreNumero, Interrupt RB4-RB7
unsigned int interrupcao=0, i;
//vari�veis RTC e mem�ria
unsigned long int aux_timer=0;

int kp;
int checksum_rx, flag_rs,num_recebido_rs,angulo_rs;
int16 buffer_rs232[6]; //Checar se angulo_pedido ta como int16, pensar se isso muda alguma coisa.

////////////////////////////////////////////////////////////////////////////////


int16 trata_ascii(int aux_buffer) //Pega um valor decimal referente da tabela ascii, e interpreta
{
   if((aux_buffer>=48)&&(aux_buffer<=57)) //Se o valor ascii estiver na parte num�rica da tabela.
   {      
         return(aux_buffer-48);      
   }
   else
   {
          if(aux_buffer==67){return(10);}//C //Returns usados na fun��o trata_buffer
          if(aux_buffer==70){return(11);}//F
   }
   return(12);
}
#INT_RDA
void receber()
{   
      if(flag_rs!=1)
      {        
         buffer_rs232[num_recebido_rs]=trata_ascii(getchar());        
         if(num_recebido_rs>=5)
         {              
            flag_rs=1; //Trata um n�mero de cada vez
         }                  
      }
      num_recebido_rs=num_recebido_rs+1;
      if ((num_recebido_rs==1)&&(buffer_rs232[0]!=10))
         num_recebido_rs=0;
      return;
      //printf(lcd_putc,"Recmebeu %d",num_recebido_rs);
}

void trata_buffer()   //Formato do pacote:C1111F
{   
   if((buffer_rs232[0]==10)&&(buffer_rs232[5]==11))//Verifica in�cio e fim do pacote
   {
         angulo_rs=100*buffer_rs232[1]+10*buffer_rs232[2]+buffer_rs232[3];
         if(angulo_rs<=180)
         {
            checksum_rx=buffer_rs232[1]+buffer_rs232[2]+buffer_rs232[3];
            if(checksum_rx > 9)
               checksum_rx=checksum_rx/10 + checksum_rx%10;
            if (checksum_rx==buffer_rs232[4]){
               stop=angulo_rs; //angulo_pedido j� foi
               if (stop < 179)               
                  {
                     bit_clear(portc,1);
                     set_pwm1_duty(0); //"0" e "0" aqui � parado
                     angulo_pedido=valor_pot/5.68;
                  }
               else angulo_pedido=angulo_pedido2;
               
               printf(lcd_putc,"\n       %03u",angulo_pedido);
            }
            else{
               printf(lcd_putc,"\n      Erro");
            }
         printf("%i",checksum_rx);
         }
   }
   
   flag_rs=0; //Agora ja pode receber outro valor na Serial
   num_recebido_rs=0; //Zera o numero para a proxima chamada. S� chama trata_buffer qnd tem o numero todo.
}

void Define_referencia(int faixa)
{

   //Adotamos tr�s faixas de valores para definir valor_ref

   switch(faixa)
   {
      //Uma para �ngulos entre 0 e 60� = 6,8395x + 45,889
      case(1):
      valor_aux = 6.8395*angulo_pedido + 45.889; //Esse � double, ent�o o n�mero � "quebrado"
      kp=4;
      break;
      
      //Outra para �ngulos entre 60� e 100� = 0,0026x3 - 0,627x2 + 51,567x - 935,34
      case(2):
      valor_aux = 0.0026*angulo_pedido*angulo_pedido*angulo_pedido - 0.627*angulo_pedido*angulo_pedido + 51.567*angulo_pedido - 935.34; //Esse � double, ent�o o n�mero � "quebrado"
      kp=10;
      break;
      
      //Outra para �ngulos entre 100� e 180�
      case(3):
      valor_aux =(-0.0356)*angulo_pedido*angulo_pedido + 16.022*angulo_pedido - 731.71;// (-0.0457)*angulo_pedido*angulo_pedido + 18.496*angulo_pedido - 894.78; //Esse � double, ent�o o n�mero � "quebrado"
      kp=5;
      break;
      
   }  
   valor_ref = valor_aux; //Como esse � long int, o n�mero aqui � inteiro 
   if( (valor_aux - valor_ref) >= 0.5)
      valor_ref = valor_ref+1;
   
   angulo_pedido_anterior = angulo_pedido;
}

void funcao_pwm()
{
   int1 sentido;
   
   if (valor_pot>valor_ref){//define para que lado deve rodar
      sentido=1;
      erro=valor_pot-valor_ref;
   }
   else {
      sentido=0;
      erro=valor_ref-valor_pot;
   }
   
   if(sentido_ant!=sentido)//quando o ponteiro passa do ponto pedido, a a��o integral zera(tem que ver se ta certo)
      integral=1;
   else 
      if (erro<1023/kp) integral=(integral+erro);
      if (integral>=30000) integral=30000;
      ;//a a��o integral s� ativa para um erro menor, afim de que ela n�o estoure.
   
   controle = kp*(erro+integral/200);//o controle � feito pela a��o proporcional com Kp=4 mais a a��o integral 
   if (controle>1023)controle=1023;
  
   if(sentido==1)// dependendo do 'sentido' ele faz o controle para que o ponteiro v� para o lado certo
   {
      bit_clear(portc,1);
      set_pwm1_duty(controle); //"0" e "0" aqui � parado
   }
   else
   {
      bit_set(portc,1);
      set_pwm1_duty(1023-controle);
   }
   sentido_ant=sentido;
}

//Interrup��o do timer0
#int_timer0
interrupt_timer0()
{
   valor_pot = read_adc();
   if (valor_pot<40) valor_pot=0;
   if (valor_pot>1000)valor_pot=1023;
   set_timer0 (243);
   aux_timer=aux_timer+1;
   if(aux_timer>100)
   {
      aux_timer=0;
      if(stop>178)
      {
         switch(angulo_pedido2)
         {
            case(0):
               angulo_pedido=150;
               angulo_pedido2=150;
               break;
               
            case(150):
               angulo_pedido=30;
               angulo_pedido2=30;
               break;
               
            case(30):
               angulo_pedido=120;
               angulo_pedido2=120;
               break;
               
            case(120):
               angulo_pedido=60;
               angulo_pedido2=60;
               break;
               
            case(60):
               angulo_pedido=0;
               angulo_pedido2=0;
               break;
         }
         
      }
   }
////////////////////////////////////////////////////////////////////////////////
////                        A��O DE CONTROLE DO PWM                         ////
////////////////////////////////////////////////////////////////////////////////
//Defini��o do angulo pedido: o angulo pedido(graus) ser� a refer�ncia em que o valor de tens�o da entrada do conversor AD deve atingir(10bits);
   printf(lcd_putc,"\n  %03u   %04lu",angulo_pedido, aux_timer);
   
   
   
   if(angulo_pedido!=angulo_pedido_anterior)//caso seja diferente, define o valor do angulo de referencia
   {  
      if ((angulo_pedido>=0)&&(angulo_pedido<=60)) Define_referencia(1);
      if ((angulo_pedido>60)&&(angulo_pedido<100)) Define_referencia(2);
      if ((angulo_pedido>=100)&&(angulo_pedido<=180)) Define_referencia(3);
   //O c�digo s� precisa entrar aqui qnd angulo_pedido � alterado.
   }
   //quando o valor de tens�o no potenci�metro � diferente do valor de referencia, o pwm � ativado.
   if (valor_pot!=valor_ref) 
   {
      funcao_pwm();   
   }
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
   set_tris_c(0x80); // Define que apenas a via RC7 como entrada (RX).
   
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
   enable_interrupts(INT_RDA); //Lembrar de habilitar aqui
   enable_interrupts(INT_timer0);
   //init_ext_eeprom();
   lcd_init();
   delay_ms(10);
   printf(lcd_putc,"\f    Programa \nPaulo e  Mariana");
   
   interrupcao=0;
   i=0;
   delay_ms(1000);
   printf(lcd_putc,"\f  Angulo Pedido");
   printf(lcd_putc,"\n       %03u",angulo_pedido);
   flag_rs=0;
   num_recebido_rs=0;
   //o looping da fun��o main se divide em 2 partes: inser��o dos angulos e hor�rios e a��o de controle do motor.

   while(1)
   {
////////////////////////////////////////////////////////////////////////////////
////               INSER��O DAS DATAS E  �NGULOS PELO USU�RIO               ////
////////////////////////////////////////////////////////////////////////////////         

      if(flag_rs==1)
      {
      trata_buffer();    
      }
}
}

