//                   programa teclado
#include    <16f84.h>                              //biblioteca do pic
#fuses      XT, NOWDT, NOPROTECT                    //condições de compilação relativas ao hardware (em relação ao osc; ao watchdog; ao travar o codigo)
#use        delay   (clock=4000000)
#byte       porta=5
#byte       portb=6



int varredura(int*num2)                   //subrotina de varredura das teclas; retorna o valor da tecla em hexa
 {
      int i, j;

      while(1){                                        //fará o controle das linhas e testará as colunas.
         for (i=0; i<=3; i++) {                       //o valor da linha a ser zerada será definido pelo i
            switch (i){                               //e transporta o valor para portb
               case (0):
               portb=0x7e;                            //eu faria apenas uma função direta tipo portb=0x7f-pow(2,i)
               break;                                 //usando a <math.h>, mas tava dando uma zica tremenda.
               case (1):
               portb=0x7d;
               break;
               case (2):
               portb=0x7b;
               break;
               case (3):
               portb=0x77;
               break;
            }
               if (!bit_test(portb,4)) {
                  *num2=(3*i)+1;                     //caso seja apertado um botão da primeira coluna, temos que o numero pode ser 
                  portb=0x7f;                        //1, 4, 7 ou *. ou seja, 3*i+1 
                  return(0);
               }
               if (!bit_test(portb,5)) {              //o mesmo para as outras colunas, com +2 e +3
                  *num2=(3*i)+2;
                  if (*num2==0x0b)*num2=0;
                  portb=0x7f;
                  return(0);
               }
               if (!bit_test(portb,6)){
                  *num2=(3*i)+3;
                  portb=0x7f;
                  return(0);
               }
            }
         }
 }
   void main ()
   {

      int ok=0,a,tenta=0,numero=15,senha[4],senha2[4];



      set_tris_b (0x70);                                  //define entradas e saídas
      set_tris_a (0x00);                                 //B0 a B3-> out - B4 a B6-> in
      port_b_pullups (true);                            //seta o pull up
      porta=0x0a;
      portb=0x7f;

//começa a rotina
         for (a=1; a<6; a++){
            varredura (&numero);                //primeiro será digitada a senha a ser gravada. quando qualquer
            bit_set(portb,7);                   //tecla é apertada, o led pisca. isso é importante para saber
            delay_ms(30);                       //se a tecla realmente foi pressionada ao tentar digitar a senha
            bit_clear(portb,7);
            switch (numero){
               case(0x0a):                      //caso a tecla corrige seja pressionada, o programa apaga o numero
                  if (a>2) porta=senha[a-3];    //que foi digitado anteriormente.
                  else porta=0x0a;
                  a=a-2;                        //se essa tecla for a primeira a ser apertada, a senha é colocada 
                  if (a>0xfd) a=0;              //na primeira posição.
                  break;
               case(0x0c):
                  if(a==5) porta=0x0c;             //coloca "C" no display, se certo ou "E" se errado.
                  else {
                     porta=0x0e;
                     a=0;
                  }                   //ao confirmar a senha com menos de 4 digitos, ele
           			   delay_ms(200);                 //informa erro e começa denovo
                     porta=0x0a;
                  break;
               default:
                  if(a!=5){
                     porta=numero;
                     senha[a-1]=numero;
                  }
                  else (a=a-1);
            }
            portb=0x00;
     		   while(portb!=0x70)
            delay_ms(10);
         }

         while(1)
               {
         for (a=1; a<6; a++){
            varredura (&numero);                //teste da senha
            bit_set(portb,7);                   
            delay_ms(30);
            bit_clear(portb,7);                 //esta parte é parecida com a anterior. a diferença é que 
            switch (numero){                    //a senha digitada aqui é comparada com a senha gravada.
               case(0x0a):
                  porta=0x0a;
                  a=a-2;
                  if (a>0xfd) a=0;
                  break;
                  case(0x0c):
                     if(a==5 && ok>=3){
                        porta=0x0c;
                        tenta=0;
                     }
                     else{
                        porta=0x0e;
                        tenta=tenta+1;
                        a=0;
                     }
                     delay_ms(200);
                     ok=0;
                     a=0;                                           //ao confirmar a senha com menos de 4 digitos, ele
                     break;
               default:
                  if(a!=5){
                     porta=0x0a;
                     senha2[a-1]=numero;
                  }
                  else (a=a-1);

                  if (senha2[a-1]==senha[a-1])ok=ok+1;         //nessa parte a senha é testada.
                  else ok=ok-1;
            }//switch
            porta=0x0a;                                        //no final, o display continua exibindo A, e as saídas de portb são
            portb=0x00;                                        //todas colocadas em zero, até que nenhum botão seja pressionado.
     		   while(portb!=0x70)                                 
               delay_ms(10);
            if (tenta==3){                                     //se houver 3 tentativas com erro, o programa trava por um tempo
               bit_set(portb,7);                               //e depois reinicia a contagem de tentativas.
               delay_ms(2500);
               bit_clear(portb,7);
               tenta=0;
            }
         }//fim for
      }//fim while de repetição
   }//fim geral
