                                    // PROGRAMA - SEQU�NCIA DE LEDS USANDO INTERRUP��O EXTERNA (RB0) - 16/02/2012

#include <16F877a.h>                //Inclui a biblioteca do pic 16F877A
#fuses XT,NOWDT,NOPROTECT,PUT       //Define para o compilador: a faixa de frequ�ncia do clock (pode ser LS, XT, HS),
                                    //n�o uso do watchdog timer,
                                    //n�o uso de prote��o de c�digo,
                                    //uso do Power-up-timer, temporizador utilizado para inicializa��o do PIC.

#use delay(clock=4000000)           //Define a frequ�ncia de clock.

#use fast_io(a)                     //Estas diretivas definem que o controle E/S das portas ser� definido pelo programador
#use fast_io(b)
#use fast_io(c)
#use fast_io(d)
#use fast_io(e)

#byte porta = 5                     // Defini��o de ponteiros para as portas
#byte portb = 6

int dir=0;                          //insere a vari�vel direcional que determina para que lado os bits ser�o rotacionados,
                                    //que come�a com valor=0 para que seja necess�rio pressionar o bot�o para a sequencia inicializar.
#int_EXT                            //diretiva para a interrup��o;
void EXT_isr(void)                  //in�cio da fun��o de interrup��o, que basicamente realiza o bounce da botoeira
{                                   //e inverte o sentido de rota��o.
   do {                             //o programa n�o faz nada enquanto o bot�o continuar pressionado.
      delay_ms(150);
   }while (!bit_test(portb,0));
   if (dir==0) dir=1;
   else dir=0;
}

main()                              //fun��o principal:
{
   int work;                        //defini��o da vari�vel auxiliar;
   enable_interrupts(global);       //habilita todas interrup��es;
   enable_interrupts(int_ext);      //habilita as interrup��es externas;
   ext_int_edge (H_TO_L);           //define como in�cio da interrup��o quando RB0 sai de n�vel l�gico alto para baixo;
   set_tris_a(0x00);                //define que todas as vias da portA do PIC ser�o sa�das;
   set_tris_d(0x00);                //define que todas as vias da portD do PIC ser�o sa�das;
   set_tris_b(0x01);                //define RB0 como entrada;
   port_b_pullups(TRUE);            //habilita os pull-ups;
   portb = (0x00);                  //zera o portb;
   portd = (0xFF);                  //define todos os leds acesos inicialmente;
   work = (0xFF);                   //define o mesmo valor de portd para work;
   while (1)                        //in�cio do looping principal;
   {
      if(dir!=0 && portd>0x00)      //para a dir=1 (sentido direito):
      {                             //se portd for maior que zero, ele roda os bits para a direita;
         rotate_right(&work,2);     //se for menor que zero, significa que j� chegou ao final, n�o precisa executar;
         bit_clear (work,7);        //a rota��o. depois de rotacionar, limpa o bit mais significativo, caso haja
      }                             //algum ru�do vindo do carry.

      if(dir==0 && work<0xFF)       //para dir==0, repete o mesmo procedimento anterior, s� que vai rotacionando os bits
      {                             //para a esquerda e os acendendo;
         rotate_left(&work,2);
         bit_set (work,0);
      }
      portd=work;                   //aplica o valor rotacionado em portd;
      delay_ms(200);                //d� o tempo necess�rio para perceber o 'movimento' das luzes.

   }
}


