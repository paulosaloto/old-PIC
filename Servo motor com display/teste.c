//Programa Servo Motor Controlado - Sistemas Embarcados

/*O programa abaixo atende três tarefas principais:

1 - Obter o ângulo_pedido pelo operador, e então, através dos parâmtetros do servo, o valor_ref (long int). [Interrupção RB4-RB7]
2 - Obter, por conversão AD, o valor da tensão no potênciômetro, valor_pot (long int). [Interrupção Timer0]
3 - Ajustar a posição do servo. [Programa Principal]

*/

#include <16F877a.h>             //Inclui a biblioteca 16F877a.h, disponível em
                                // c:\Arquivos de programas\PICC\Devices
#device adc=10 //10bits


#define EEPROM_ADDRESS long int
#define EEPROM_SIZE   32768

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

#include <i2c.c>               //biblioteca para escrita e leitura da memória 24lc256 e RTC PFC8583 através do i2c

#byte porta = 5
#byte portb = 6
#byte portc = 7
#byte portd = 8
#byte porte = 9

#include <lcdkit1.c> 
//Variáveis globais//
int1 sentido_ant=1, pisca;
unsigned int angulo_pedido=0, angulo_pedido_anterior;
long int  valor_pot, erro, integral, controle, valor_ref=0;
double valor_aux;
//Variáveis DescobreNumero, Interrupt RB4-RB7
unsigned int num_digitos, interrupcao=0,int_anterior, tecla, i;
long int numero;
//variáveis RTC e memória
int hora=0, angulo=0, minutos=0, hora_anterior=0, minuto_anterior=0, segundos=0, dia =0, mes =0, ano =0, data =0, resto=0, horario=0, aux_timer=0, posicao=0, quantidade=0;
int igualdade[5]={0, 0, 0, 0, 0}, posicao_inicial=0;
int hora2=0, angulo2=0, minutos2=0, segundos2=0, dia2 =0, mes2 =0, ano2 =0, data2 =0, resto2=0, horario2=0, aux_timer2=0, posicao2=0, quantidade2=0;
//Interrupção do tecladoint 

#int_RB
int RB_isr()
{
   int_anterior=interrupcao;
   interrupcao=portb;
   if(interrupcao>0xF0)interrupcao=int_anterior;
   return(0);
}


void varredura(){
//Varredura do teclado: Testa se não há botões pressionados. se não houver, realiza a varredura das colunas.
   //Interrupção: quando há uma interrupção, esta parte do programa entra em ação para trabalhar o valor obtido
      if (interrupcao != 0) //
      {
      delay_ms(200);
      /*int_anterior=interrupcao;
         for(i=0;i<=4;i++)//Debounce: o valor adquirido na interrupção tem que ser o mesmo durante 75ms; 
         {
            if(interrupcao==int_anterior) delay_ms(20);
            else i=0;
         }*/
         if(interrupcao<0xF0)//se for uma interrupção onde o botão é pressionado, ele entra na função para obter o número correspondente.
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
         interrupcao=0;//se for quando o botao é solto, não faz nada.
         
      }
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

}

void def_hora (int destino/*0 para memoria e 1 para rtc*/)
{
   int hora_menor=0, hora_maior=0;
   
   if (hora!=0x00)hora_maior=hora;
   switch(tecla){
      case(10):
         hora=0x00;
         tecla=0;
         return;
      case(11):
         break;
      case(12):
         hora=hora_maior;
         horario=1;
         if(destino==1) write_RTC(4,hora);
         else write_ext_eeprom(3+6*posicao,hora);
         tecla=0;
         return;
      default:
         hora_menor=tecla;
   }
   if((hora_maior>0x10 && hora_maior!=0x20)|| ((hora_maior*16+hora_menor)>0x23))
      hora=hora_menor;
   else
      hora=hora_maior*16+hora_menor;
   tecla=0;
}
        
void def_min (int&min, int h/*3 para segundos e 2 para minutos*/, int destino/*0 para memoria e 1 para rtc*/)
{
   int min_menor=0, min_maior=0;
   
   if (min!=0x00)min_maior=min;
   switch(tecla){
      case(10):
         if(min!=0) min=0x00;
         else horario=horario-1;
         tecla=0;
         return;
      case(11):
         break;
      case(12):
         min=min_maior;
         horario=h;
         if(destino==1) write_RTC(5-h,min);
         else (write_ext_eeprom(h+2+6*posicao,min));
         tecla=0;
         return;
      default:
         min_menor=tecla;
   }
   if((min_maior>0x10 && min_maior!=0x20 && min_maior!=0x30 && min_maior!=0x40 && min_maior!=0x50 )|| ((min_maior*16+min_menor)>0x59))
      min=min_menor;
   else
      min=min_maior*16+min_menor;
   tecla=0;
}  

void def_dia (int destino)//Entra primeiro algarismo, entra #. Ou entra primeiro, entra segundo, entra #.
{
   int dia_menor=0, dia_maior=0;
   
   dia_maior=dia; //Manda para o algarismo da esquerda, já que sempre vai mandar, nunca vai ser 0x00.
   switch(tecla){
      case(10):
         dia=0x00;
         tecla=0;
         return;
      case(11):
         break;
      case(12): //Tem que funcionar para quando tem 1 só algarismo e para quando tem 2 algarismos 
         //dia=dia_maior;//Por causa do if superior
         if((dia_maior==0)&&(dia_menor==0)) dia = 1;
         data=3;
         if(destino==1) write_RTC(5,dia+resto);
         else (write_ext_eeprom(2+6*posicao,dia+resto));
         tecla=0;
         return;
      default:
         dia_menor=tecla;
   }
   //Após entrar o número, ele é avaliado, antes da próxima tecla.
   dia_maior = dia;
   if(dia_maior>0x03)
      dia=dia_menor;
   else {
      if ( ((mes==1)||(mes==3)||(mes==5)||(mes==7)||(mes==8)||(mes==10)||(mes==12)) && (dia_maior*16+dia_menor>0x31))
         dia = dia_menor;        
      if ( ((mes==4)||(mes==6)||(mes==9)||(mes==11)) && (dia_maior*16+dia_menor>0x30))
         dia = dia_menor;
      if ( (mes==2) && (bit_test(resto,7)==0) && (bit_test(resto,6)==0) && (dia_maior*16+dia_menor>0x29))
         dia = dia_menor;
      if ( (mes==2) && ((bit_test(resto,7)!=0)||(bit_test(resto,6)!=0)) && (dia_maior*16+dia_menor>0x28))
         dia = dia_menor;
      else
        dia=dia_maior*16+dia_menor; //Aqui q se transforma para hexa.
   }
   tecla=0;
}

void def_mes (int destino)//Entra primeiro algarismo, entra #. Ou entra primeiro, entra segundo, entra #.
{
   int mes_menor=0, mes_maior=0;
   
   mes_maior=mes; //Se já houver algum número, esse vai para o algarismo da esquerda
   switch(tecla){
      case(10):
         mes=0x00;
         tecla=0;
         return;
      case(11):
         break;
      case(12): //Tem que funcionar para quando tem 1 só algarismo e para quando tem 2 algarismos
         //mes=mes_maior; //Por causa do if superior
         if((mes_maior==0)&&(mes_menor==0)) mes = 1;
         data=2;
         if(destino==1) write_RTC(6,mes);
         else (write_ext_eeprom(1+6*posicao,dia+resto));
         write_RTC(6,mes);
         tecla=0;
         return;
      default:
         mes_menor=tecla;
   }
   //Após entrar o número, ele é avaliado, antes da próxima tecla.
   mes_maior = mes;
   if(mes_maior>0x01)
      mes=mes_menor;
   if((mes_maior*16+mes_menor)>0x12) //Avaliado pelo bcd, poderia ter sido pelo decimal tbm. 
      mes=mes_menor;
   else
      mes=mes_maior*16+mes_menor; //Aqui q se transforma para hexa.
   tecla=0;
}

void def_ano ()//Entra primeiro algarismo, entra #. Ou entra primeiro, entra segundo, entra #.
{
   int ano_menor=0, ano_maior=0;
   
   //ano_maior=ano; //Se já houver algum número, esse vai para o algarismo da esquerda
   switch(tecla){
      case(10):
         ano=0x00;
         tecla=0;
         return;
      case(11):
         break;
      case(12): //Tem que funcionar para quando tem 1 só algarismo e para quando tem 2 algarismos
         //ano=ano_maior; //Por causa do if superior
         //A variável global ano pode depois ser usada.
         //Ele não escreve no RTC aqui.
         resto=ano*128; //Facilita as coisas depois
         data=1;
         tecla=0;
         return;
      default:
         ano_menor=tecla;
   }
   //Após entrar o número, ele é avaliado, antes da próxima tecla.
   ano_maior=ano;
   if(ano_maior>0x09){
      ano=ano_menor;
   }
   else
      ano=ano_maior*16+ano_menor; //Aqui q se transforma para hexa.
   tecla=0;
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
         write_ext_eeprom(posicao*6,angulo);
         posicao=posicao+1;
         tecla=0;
         angulo=0;
         numero=5;
         return;
   }

   if (angulo==0 || (angulo*10+tecla)>180) angulo=tecla;
   else angulo=angulo*10+tecla;
   tecla=0;
}  

void write_horaminseg(praondescreve)//(0 para memoria e 1 para RTC)
{
   switch (horario)
   {
      case(0):
      printf(lcd_putc,"\n    %x:__:__",hora);
      if(tecla!=0) def_hora(praondescreve);
      break;
      
      case(1):
      printf(lcd_putc,"\b\n    %2x:%2x:__",hora,minutos);
      if(tecla!=0) def_min(minutos,2,praondescreve);
      break;
      
      case(2):
      printf(lcd_putc,"\b\n    %2x:%2x:%2x",hora,minutos,segundos);
      if(tecla!=0) def_min(segundos,3,praondescreve);
   }
}

void write_data(praondescreve)
{
   switch (data)
   {
      case(0): //Vai aumentando na função def_...
      printf(lcd_putc,"\n    __/__/%x",ano); //%2f
      if(tecla!=0) def_ano();
      break;
      
      case(1):
      printf(lcd_putc,"\b\n    __/%x/%2x",mes, ano);
      if(tecla!=0) def_mes(praondescreve);
      break;
      
      case(2):
      printf(lcd_putc,"\b\n    %x/%2x/%2x",dia, mes, ano);
      if(tecla!=0) def_dia(praondescreve);
      break;
   }
}

void Define_referencia(int faixa)
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
      if (erro<255) integral=(100*integral+erro)*0.04;//a ação integral só ativa para um erro menor, afim de que ela não estoure.
   
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

//Interrupção do timer0
#int_timer0
interrupt_timer0()
{
   valor_pot = read_adc();
     
   set_timer0 (243);
   aux_timer=aux_timer+2;
////////////////////////////////////////////////////////////////////////////////
////      AÇÃO DE LEITURA DO RTC E COMPARAÇÃO COM OS DADOS DA MEMÓRIA       ////
////////////////////////////////////////////////////////////////////////////////
//verifica se o horário mostrado no RTC é igual ao horário pedido nos ângulos
//a verificação para mudança de hora e mudança de minutos é feita separada para que
//não se perca muito tempo fazendo a comparação todas as vezes
   if(aux_timer==0)
   {
   
      segundos2=read_RTC(2);
      if(segundos2==0){
         minutos2=read_RTC(3);
         if(minutos2==0){
            hora2=read_RTC(4);
            if(hora2==0){
               dia2=read_RTC(5);
               if(dia2==1){
                  mes2=read_RTC(4);
               }
            }
         }
      }
              
      //if(hora2!=hora_anterior)
      //{
         for(posicao=posicao_inicial;posicao<quantidade;posicao++)
         {
            if(mes2==read_ext_eeprom(1+6*posicao))  
               if(dia2==read_ext_eeprom(2+6*posicao))
                  if(hora2==read_ext_eeprom(3+6*posicao))
                     if(minutos2==read_ext_eeprom(4+6*posicao))
                        if(segundos2==read_ext_eeprom(5+6*posicao))
                        {
                           angulo_pedido=read_ext_eeprom(6*posicao);
                           igualdade[posicao]=0;
                           if (posicao==posicao_inicial)posicao_inicial=posicao_inicial+1;
                        }
         }
         //hora_anterior=hora2;
         return(0);
      }
      else
      {
         if(minutos2!=minuto_anterior)
         {
            for(posicao=posicao_inicial;posicao<quantidade;posicao++)
            {
               if(minutos2==read_ext_eeprom(4+6*posicao))
                  if(segundos2==read_ext_eeprom(5+6*posicao))
                  {
                     angulo_pedido=read_ext_eeprom(6*posicao);
                     igualdade[posicao]=0;
                     if (posicao==posicao_inicial)posicao_inicial=posicao_inicial+1;
                  }
            }
            minuto_anterior=minutos2;
            return(0);
         }
         else
         {
            for(posicao=posicao_inicial;posicao<quantidade;posicao++)
            {
               if(segundos2==read_ext_eeprom(5+6*posicao))
               {
                  angulo_pedido=read_ext_eeprom(6*posicao);
                  igualdade[posicao]=0;
                  if (posicao==posicao_inicial)posicao_inicial=posicao_inicial+1;
               }
            }
         }
      }
   //}
////////////////////////////////////////////////////////////////////////////////
////                        AÇÃO DE CONTROLE DO PWM                         ////
////////////////////////////////////////////////////////////////////////////////
//Definição do angulo pedido: o angulo pedido(graus) será a referência em que o valor de tensão da entrada do conversor AD deve atingir(10bits);
   if(angulo_pedido!=angulo_pedido_anterior)//caso seja diferente, define o valor do angulo de referencia
   {  
      if ((angulo_pedido>=0)&&(angulo_pedido<=60)) Define_referencia(1);
      if ((angulo_pedido>60)&&(angulo_pedido<100)) Define_referencia(2);
      if ((angulo_pedido>=100)&&(angulo_pedido<=180)) Define_referencia(3);
   //O código só precisa entrar aqui qnd angulo_pedido é alterado.
   }
   //quando o valor de tensão no potenciômetro é diferente do valor de referencia, o pwm é ativado.
   if (valor_pot!=valor_ref) 
   {
      funcao_pwm();   
   }
}

void main()
{  
////////////////////////////////////////////////////////////////////////////////
////                       CONFIGURAÇÕES DO PIC                            /////
////////////////////////////////////////////////////////////////////////////////
   // Configuração de vias das portas do PIC como saída ou como entrada.
   // Se o bit 0 do registrador especial TRISA é 0, a via 0 da portA do PIC é saída, caso contrário é entrada.
   // Assim são configuradas todas as vias das portas do PIC, de qualquer porta.
   // Exemplo: TRISA = | 0 | 1 | 0 | 1 | 0 | 1 | -> PORTA = | S | E | S | E | S | E | , no 16F877A a portA tem 6 vias.
   
   set_tris_a(0x02); // Define que somente 1 via do PIC será entrada.
   set_tris_b(0xF8); //
   set_tris_c(0x00); //
   
   //Configuração inicial do teclado
   portb=0;
   port_b_pullups (true);
      
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
   //init_ext_eeprom();
   lcd_init();
   delay_ms(10);
   printf(lcd_putc,"\f    Programa \nPaulo e  Mariana");
   
   interrupcao=0;
   i=0;
   delay_ms(1000) ;
   //o looping da função main se divide em 2 partes: inserção dos angulos e horários e ação de controle do motor.
////////////////////////////////////////////////////////////////////////////////
////                CONFIGURAÇÃO DO RTC PELO USUÁRIO                        ////
////////////////////////////////////////////////////////////////////////////////
   printf(lcd_putc,"\fhora atual:");
   while(horario!=3){
      varredura();
      write_horaminseg(1);
   }
   printf(lcd_putc,"\fdata atual:");
   while(data!=3) //Também vai aumentando até ter ano, mes e dia.
   {
      varredura();
      write_data(1);   
   }     
////////////////////////////////////////////////////////////////////////////////
////             INSERÇÃO DA QUANTIDADE DE ÂNGULOS PELO USUÁRIO             ////
////////////////////////////////////////////////////////////////////////////////
   tecla=0;
   

   printf(lcd_putc,"\f# p/ novo angulo\n    %2x:%2x:%2x    ",hora2,minutos2,segundos2);
    while(1)
   {/*
   printf(lcd_putc,"\fInsira o Num.\nde Angulos:  %d",quantidade);
      while(quantidade==0)
      {
         varredura();
         if(tecla!=0 && tecla<10) 
         {
            quantidade=tecla;
            printf(lcd_putc,"\fInsira o Num.\nde Angulos:  %d",quantidade);
         }
      }      
      delay_ms(500);*/
////////////////////////////////////////////////////////////////////////////////
////               INSERÇÃO DAS DATAS E  ÂNGULOS PELO USUÁRIO               ////
////////////////////////////////////////////////////////////////////////////////         
      if(aux_timer==0)
      printf(lcd_putc,"\n%u'    %2x:%2x:%2x",angulo_pedido,hora2,minutos2,segundos2);
      
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
         data=0;
         printf(lcd_putc,"\fData Angulo %d:",quantidade);
         while(data!=3) //Também vai aumentando até ter ano, mes e dia.
         {
            varredura();
            write_data(0);
         }
                  printf(lcd_putc,"\fValor Angulo %d:",quantidade);
        for(numero=0;numero<=2;numero++)
         {
            varredura();
            if (tecla!=0){
               def_angulo();
               printf(lcd_putc,"\n   %u",angulo);
               delay_ms(10);
            }
            numero--;
         }
         numero=0;
         quantidade=quantidade+1;
         printf(lcd_putc,"\f# p/ novo angulo\n%u'    %2x:%2x:%2x",angulo_pedido,hora2,minutos2,segundos2);
  

      }
   }
}




