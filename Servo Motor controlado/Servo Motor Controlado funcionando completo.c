//Programa Servo Motor Controlado - Sistemas Embarcados

/*O programa abaixo atende três tarefas principais:

1 - Obter o ângulo_pedido pelo operador, e então, através dos parâmtetros do servo, o valor_ref (long int). [Interrupção RB4-RB7]
2 - Obter, por conversão AD, o valor da tensão no potênciômetro, valor_pot (long int). [Interrupção Timer0]
3 - Ajustar a posição do servo. [Programa Principal]

*/

#include <16F877a.h>             //Inclui a biblioteca 16F877a.h, disponível em
                                 // c:\Arquivos de programas\PICC\Devices

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

//Variáveis globais//
int angulo_pedido;
int angulo_pedido_anterior;
long int valor_pot;
int digito, linha, tecla, i;
long int numero;
//Fim das váriaveis globais//

//Interrupção do teclado
#int_RB
int RB_isr()
{
   byte work;                       //
   work=portb;                      //
   do                               // 
   {                                // bounce
      delay_ms(30);                 //
      if(work!=portb) work=portb;   //
   }while (work!=portb);            //
      if (bit_test(portb,4) && bit_test(portb,5) && bit_test(portb,6) && bit_test(portb,7)) return(0);
      
      while(1){                                      //para obter o valor digitado, primeiro é testado qual das linhas
         for (i=4; i<=7; i++)                        //da matriz do teclado gerou a interrupção.
            if (!bit_test (portb,i)) linha = i;
         portb = 0xff;                               //depois de definida a linha, a coluna correspondente da tecla é encontrada
         for (i=0; i<=2; i++)                        //e é efetuada a operação para obter o valor da tecla pressionada.
         {
            bit_clear (portb,i);
            if (!bit_test(portb,linha)) tecla =((linha-4)*3+i+1);
            bit_set(portb,i);
         }
         portb = 0x08;
         
         if (bit_test(portb,4) && bit_test(portb,5) && bit_test(portb,6) && bit_test(portb,7)) return(0);
         else                             //
         {                                //
            work=portb;                   //
            do                            // debounce
            {                             //
               delay_ms(30);              //
               if(work!=portb) work=portb;//
            }while (work!=portb);         //
         }  
         return(0);
      }
}
 
//Interrupção do timer0
#int_timer0
interrupt_timer0(){
valor_pot = read_adc();
}

int peganumero ()
{
   switch (tecla)
   {
      case(0x0a)://caso a tecla * seja pressionada, o programa apaga o numero e faz com que o número requisitado seja
         numero = angulo_pedido;//igual ao valor do angulo pedido anterior.
         digito=0;
         break;
      case(0x0c)://quando a tecla # é pressionada, o valor inserido é aplicado no angulo pedido e a quantidade de dígitos é resetada.
         if (angulo_pedido==numero) break;
         angulo_pedido = numero;
         digito=0;
         break;
      case(0x0b)://quando a tecla 0 é pressionada, o valor 0x0b é corrigido para zero.
         tecla = 0;
      default://para as demais teclas, o valor inserido é o próprio valor correspondente
      if (digito==0)//caso seja o primeiro dígito, o número inserido é o próprio valor da tecla digitado
      {
         numero=tecla;
         digito=1;
      }
      else//se não, o valor anterior é jogado para os próximos dígitos e a quantidade de digitos é aumentada em 1
      {
         numero=(numero*10+tecla);
         digito=digito+1;
      }
      if (numero>180||digito>3)//caso o número inserido seja maior que 180º, o valor inserido é ignorado pelo programa
         {//e todos os dígitos são zerados.
            numero=angulo_pedido;
            digito=0;
         }
   }
      tecla=0;//no fim da função, o valor da tecla é zerado para que o valor obtido só seja computado uma vez.
}

void main()
{ 
   //Variáveis responsáveis pela transformação ângulo_pedido => valor_ref
   long int valor_ref;
   double valor_aux;
   
   //Isso faz o motor ir inicialmente para o ângulo que vc quiser. O padrão é 0, mas pode jogar aí qlqr valor pra testar.
   angulo_pedido=0;
   angulo_pedido_anterior=10;
   
   // Configuração de vias das portas do PIC como saída ou como entrada.
   // Se o bit 0 do registrador especial TRISA é 0, a via 0 da portA do PIC é saída, caso contrário é entrada.
   // Assim são configuradas todas as vias das portas do PIC, de qualquer porta.
   // Exemplo: TRISA = | 0 | 1 | 0 | 1 | 0 | 1 | -> PORTA = | S | E | S | E | S | E | , no 16F877A a portA tem 6 vias.
   
   set_tris_a(0x01); // Define que somente 1 via do PIC será entrada.
   set_tris_b(0xF0); //
   set_tris_c(0x00); //
   set_tris_d(0x00); //
   
   portb=0;
   port_b_pullups (true);
  
   //Configuração do Conversor AD (Potenciômetro do Servo)
   setup_adc_ports(ALL_ANALOG);
   setup_adc(ADC_CLOCK_DIV_8);
   set_adc_channel(0);
   delay_us(100);
   
   //Configuração do PWM, que controla a velocidade do servo
   
   setup_ccp1(ccp_pwm); //Configura CCP1 como PWM
   
   //Clock interno : (1/clock)*4    //4 clocks internos = 1 clock externo
   //O tempo de clock será: clock_interno*Div_TM2*(Periodo+1);
   setup_timer_2(T2_DIV_BY_1,255, 1);
   
   setup_timer_0 (T0_INTERNAL | T1_DIV_BY_1);   //Configuração do Timer0 para clock interno = 1E6 dividido por 256
   set_timer0 (247);
  
   delay_us(100);
  
   enable_interrupts(GLOBAL);
   enable_interrupts(INT_rb);
   enable_interrupts(INT_timer0);
   EXT_INT_EDGE(L_TO_H);
        
   while(1){
   
   if (tecla != 0)//
      peganumero();
   
   if(angulo_pedido!=angulo_pedido_anterior){ //O código só precisa entrar aqui qnd angulo_pedido é alterado.
   //Adotamos três faixas de valores para definir valor_ref
   //Uma para ângulos entre 0 e 60º
   if ((angulo_pedido>=0)&&(angulo_pedido<=60)){
   valor_aux = 6.8858*angulo_pedido + 51.515; //Esse é double, então o número é "quebrado"
   valor_ref = valor_aux; //Como esse é long int, o número aqui é inteiro
   //Aproximação
   if( (valor_aux - valor_ref) >= 0.5)
      valor_ref = valor_ref+1;
   }
   
   //Outra para ângulos entre 60º e 100º
   if ((angulo_pedido>60)&&(angulo_pedido<100)){
   valor_aux = 0.0019*angulo_pedido*angulo_pedido*angulo_pedido - 0.4852*angulo_pedido*angulo_pedido + 41.224*angulo_pedido - 681.49; //Esse é double, então o número é "quebrado"
   valor_ref = valor_aux; //Como esse é long int, o número aqui é inteiro
   //Aproximação
   if( (valor_aux - valor_ref) >= 0.5)
      valor_ref = valor_ref+1;
   }
   
   //Outra para ângulos entre 100º e 180º
   if ((angulo_pedido>=100)&&(angulo_pedido<=180)){
   valor_aux = (-0.0372)*angulo_pedido*angulo_pedido + 16.419*angulo_pedido - 755.86; //Esse é double, então o número é "quebrado"
   valor_ref = valor_aux; //Como esse é long int, o número aqui é inteiro
   //Aproximação
   if( (valor_aux - valor_ref) >= 0.5)
      valor_ref = valor_ref+1;
   }
   angulo_pedido_anterior = angulo_pedido;
   }
   
   //valor_pot = read_adc(); //ISSO ESTÁ AQUI SÓ PARA TESTES! ISSO FICA DENTRO DA INTERRUPT TIMER0.
   
   //erro = valor_pot-valor_ref; //só para facilitar o raciocínio da parte abaixo.
   
   if (valor_pot>=valor_ref){ //Ele deve diminuir a leitura do potenciômetro!
   bit_clear(portc,1);
   set_pwm1_duty(valor_pot-valor_ref); //"0" e "0" aqui é parado
   }
    
   if (valor_pot<valor_ref){ //Ele deve aumentar a leitura do potenciômetro!
   bit_set(portc,1);
   set_pwm1_duty(1023-(valor_ref-valor_pot)); //"1" e "1" aqui é parado
   }
   
  }
  
}
