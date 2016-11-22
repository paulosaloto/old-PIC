//Programa Servo Motor Controlado - Sistemas Embarcados

/*O programa abaixo atende três tarefas principais:

1 - Obter o ângulo_pedido pelo operador, e então, através dos parâmtetros do servo, o valor_ref (long int). [Interrupção RB4-RB7]
2 - Obter, por conversão AD, o valor da tensão no potênciômetro, valor_pot (long int). [Interrupção Timer0]
3 - Ajustar a posição do servo. [Programa Principal]

*/

#include <16F877a.h>             //Inclui a biblioteca 16F877a.h, disponível em
#device icd='true'                                // c:\Arquivos de programas\PICC\Devices
#device adc=10 //10bits
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

#include <lcdkit1.c> 
//Variáveis globais//
int1 sentido_ant=1;
unsigned int angulo_pedido;
unsigned int angulo_pedido_anterior;
long int valor_pot, erro, integral; 
long int controle;
//Variáveis responsáveis pela transformação ângulo_pedido => valor_ref
long int valor_ref;
double valor_aux;
//Variáveis DescobreNumero, Interrupt RB4-RB7
unsigned int num_digitos, interrupcao=0,int_anterior, tecla, i;
long int numero;

//Interrupção do teclado
#int_RB
int RB_isr()
{
   int_anterior=interrupcao;
   interrupcao=portb;
   if(interrupcao>0xF0)interrupcao=int_anterior;
   return(0);
}

//Interrupção do timer0
#int_timer0
interrupt_timer0(){
valor_pot = read_adc();
set_timer0 (246);
}

int DescobreNumero ()
{
   //lcd_init();
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
   switch (tecla)
   {
      case(0x0a)://caso a tecla * seja pressionada, o programa apaga o numero e faz com que o número requisitado seja
         numero = angulo_pedido;//igual ao valor do angulo pedido anterior.
         num_digitos=0;
         printf(lcd_putc,"\fValor %ld\nDescartado!", numero);
         break;
      case(0x0c)://quando a tecla # é pressionada, o valor inserido é aplicado no angulo pedido e a quantidade de dígitos é resetada.
         if (angulo_pedido==numero) break;
         angulo_pedido = numero;
         num_digitos=0;
         printf(lcd_putc,"\fValor %u\nInserido!", angulo_pedido);
         break;
      case(0x0b)://quando a tecla 0 é pressionada, o valor 0x0b é corrigido para zero.
         tecla = 0;
      default://para as demais teclas, o valor inserido é o próprio valor correspondente
      if (num_digitos==0)//caso seja o primeiro dígito, o número inserido é o próprio valor da tecla digitado
      {
         numero=tecla;
         if(numero>0)
            num_digitos=1;
         printf(lcd_putc,"\f%d",tecla);
      }
      else//se não, o valor anterior é jogado para os próximos dígitos e a quantidade de digitos é aumentada em 1
      {
         numero=(numero*10+tecla);
         num_digitos=num_digitos+1;
         printf(lcd_putc,"\f%ld",numero);
      }
      if (numero>180||num_digitos>3)//caso o número inserido seja maior que 180º, o valor inserido é ignorado pelo programa
         {//e todos os dígitos são zerados.
            printf(lcd_putc,"\fValor %ld fora\ndo range!", numero);
            numero=angulo_pedido;
            num_digitos=0;
         }
   }
   tecla=0;
   interrupcao=0;//no fim da função, o valor da tecla é zerado para que o valor obtido só seja computado uma vez.
}

void Define_referencia(int&faixa)
{

   //Adotamos três faixas de valores para definir valor_ref

   switch(faixa)
   {
      //Uma para ângulos entre 0 e 60º = 6,8395x + 45,889
      case(1):
      valor_aux = 6.8395*angulo_pedido + 45.889; //Esse é double, então o número é "quebrado"
      break;
      
      //Outra para ângulos entre 60º e 100º = 0,0026x3 - 0,627x2 + 51,567x - 935,34
      case(2):
      valor_aux = 0.0026*angulo_pedido*angulo_pedido*angulo_pedido - 0.627*angulo_pedido*angulo_pedido + 51.567*angulo_pedido - 935.34; //Esse é double, então o número é "quebrado"
      break;
      
      //Outra para ângulos entre 100º e 180º
      case(3):
      valor_aux = (-0.0457)*angulo_pedido*angulo_pedido + 18.496*angulo_pedido - 894.78; //Esse é double, então o número é "quebrado"
      break;
      
   }  
   valor_ref = valor_aux; //Como esse é long int, o número aqui é inteiro 
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
   
   if(sentido_ant!=sentido)//quando o ponteiro passa do ponto pedido, a ação integral zera(tem que ver se ta certo)
      integral=0;
   else 
      if (erro<255) integral=(100*integral+erro)*0.01;//a ação integral só ativa para um erro menor, afim de que ela não estoure.
   
   controle = (erro*4+integral);//o controle é feito pela ação proporcional com Kp=4 mais a ação integral 
   if (controle>1023)controle=1023;
  
   if(sentido==1)// dependendo do 'sentido' ele faz o controle para que o ponteiro vá para o lado certo
   {
      bit_clear(portc,1);
      set_pwm1_duty(controle); //"0" e "0" aqui é parado
   }
   else
   {
      bit_set(portc,1);
      set_pwm1_duty(1023-controle);
   }
   sentido_ant=sentido;
}


void main()
{  
   // Configuração de vias das portas do PIC como saída ou como entrada.
   // Se o bit 0 do registrador especial TRISA é 0, a via 0 da portA do PIC é saída, caso contrário é entrada.
   // Assim são configuradas todas as vias das portas do PIC, de qualquer porta.
   // Exemplo: TRISA = | 0 | 1 | 0 | 1 | 0 | 1 | -> PORTA = | S | E | S | E | S | E | , no 16F877A a portA tem 6 vias.
   
   set_tris_a(0x02); // Define que somente 1 via do PIC será entrada.
   set_tris_b(0xF0); //
   set_tris_c(0x00); //
   
   //Configuração inicial do teclado
   portb=0;
   port_b_pullups (true);
   
   //Configuração inicial dos leds
   valor_ref=0;
   
   bit_set(porta,4); // Ativa os leds, saturando o transistor chave no catodo comum dos leds. Vide página 13 do manual.
   portd=0;
   
   //Isso faz o motor ir inicialmente para o ângulo que vc quiser. O padrão é 0, mas pode jogar aí qlqr valor pra testar.
   angulo_pedido=0;
   angulo_pedido_anterior=10;
  
   //Configuração do Conversor AD (Potenciômetro do Servo)
   setup_adc_ports(RA0_RA1_RA3_ANALOG);
   setup_adc(ADC_CLOCK_DIV_8);
   set_adc_channel(1);
   delay_us(100);
   
   //Configuração do PWM, que controla a velocidade do servo
   
   setup_ccp1(ccp_pwm); //Configura CCP1 como PWM
   
   //Clock interno : (1/clock)*4    //4 clocks internos = 1 clock externo
   //O tempo de clock será: clock_interno*Div_TM2*(Periodo+1);
   setup_timer_2(T2_DIV_BY_1,255, 1);
   
   setup_timer_0 (RTCC_INTERNAL|RTCC_DIV_128);   //Configuração do Timer0 para clock interno = 1E6 dividido por 256
   
  
   delay_us(100);
  
   enable_interrupts(GLOBAL);
   enable_interrupts(INT_rb);
   enable_interrupts(INT_timer0);
   EXT_INT_EDGE(L_TO_H);
   
   lcd_init();
   delay_ms(10);
   printf(lcd_putc,"\f Programa \n Servo motor");
   interrupcao=0;     
   
   
   while(1){
   //Varredura do teclado: Testa se não há botões pressionados. se não houver, realiza a varredura das colunas.
      if (bit_test(portb,7) && bit_test(portb,6) && bit_test(portb,5) && bit_test(portb,4))
      {
         if(!bit_test(portb,2)) {//se o portb tiver os bits menos significativos 011, passará a ter 110
            bit_clear(portb,0);
            bit_set(portb,2);
         }
         else 
            if(!bit_test(portb,1)) {//de mesmo modo, se tiver 101, passará a ter 011
               bit_clear(portb,2);
               bit_set(portb,1);
            }
            else
               if(!bit_test(portb,0)) {//e se tier 110, passará a ter 101
                  bit_clear(portb,1);
                  bit_set(portb,0);
               }
      }
   //Interrupção: quando há uma interrupção, esta parte do programa entra em ação para trabalhar o valor obtido
      if (interrupcao != 0) //
      {
      delay_ms(200);
         if(interrupcao<0xF0) Descobrenumero();//se for uma interrupção onde o botão é pressionado, ele entra na função para obter o número correspondente.
         else interrupcao=0;//se for quando o botao é solto, não faz nada.
      }
      
      
      //Definição do angulo pedido: o angulo pedido(graus) será a referência em que o valor de tensão da entrada do conversor AD deve atingir(10bits);
      if(angulo_pedido!=angulo_pedido_anterior)//caso seja diferente, define o valor do angulo de referencia
      {  
         if ((angulo_pedido>=0)&&(angulo_pedido<=60)) i=1;
         if ((angulo_pedido>60)&&(angulo_pedido<100)) i=2;
         if ((angulo_pedido>=100)&&(angulo_pedido<=180)) i=3;
         Define_referencia(i);//O código só precisa entrar aqui qnd angulo_pedido é alterado.
      }
      //valor_pot = read_adc(); //ISSO ESTÁ AQUI SÓ PARA TESTES! ISSO FICA DENTRO DA INTERRUPT TIMER0.
      
      //quando o valor de tensão no potenciômetro é diferente do valor de referencia, o pwm é ativado.
      if (valor_pot!=valor_ref) 
      {
         funcao_pwm();
      }
   
   }
}
   
