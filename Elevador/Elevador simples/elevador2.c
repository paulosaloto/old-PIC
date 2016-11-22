//programa ELEVADOR

#include	<16f84.h>

#fuses	XT, NOWDT, NOPROTECT			   	                                      //condições de compilação relativas ao hardware
#use delay	(clock=4000000)                                                     //(em relação ao osc; ao watchdog; ao travar o codigo)
#byte porta=5
#byte portb=6	                                       				              //para o pic identificar a porta (portb=6 e porta=5)



   void main ()
   {

      int erro,aux,andar=4,chama=0;                                               //é possivel realizar até duas chamadas ao mesmo tempo


      set_tris_b (0x3F);                                                        //define entradas e saídas
      set_tris_a (0x00);
      port_b_pullups (true);                                                    //seta o pull up
      porta=0x0e;
      portb=0;


      while (andar==4)                                                          //para iniciar o programa, deve-se mostrar
      {                                                                         //onde está o elevador inicialmente.
         if (!bit_test(portb,3))
            andar=1;
         if (!bit_test(portb,4))
            andar=2;
         if (!bit_test(portb,5))
            andar=3;
         }

      porta=andar;                                                              //mostra o andar inicial no display.
                                                                                //caso esteja no terreo, temos andar=1; e o display mostra
      while (1)                                                                 //o numero 1
      {

         if (!bit_test(portb,0)) chama=1;                                        //aqui são testadas as chamadas.

         if (!bit_test(portb,1)) chama=2;

         if (!bit_test(portb,2)) chama=3;

         if (!bit_test(portb,3)) erro=erro+1;                                    //é verificado se há mais de um fim de curso pressionado
         if (!bit_test(portb,4)) erro=erro+1;                                    //ao mesmo tempo. caso seja, aparece E no display
         if (!bit_test(portb,5)) erro=erro+1;
        
         while (erro>1) porta=0x0e;
         erro=0;

      while (bit_test(portb,5) && bit_test(portb,4) && bit_test(portb,3) && chama==0) porta=0x0e;//é testado se o elevador está fora de um fim
                                                                                 //de curso sem que uma chamada tenha sido feita
         if(chama>andar){                                                        
            aux=1;                                                               //se for feita uma chamada para um andar acima, o led de 
            bit_set(portb,6);                                                    //subida acende e a variável auxiliar fica =1
         }
         else if (chama!=0 && chama<andar){                                      //se for a um andar abaixo, é verificada se houve uma chamda
               aux=2;                                                            //a variavel aux fica =2 e o led de descida setado
               bit_set(portb,7);
            }
            else aux=0;                                                          //se a chamada for igual ao andar, aux=0.


         if (chama!='\0')                                                        //caso haja a chamada, terão os 5 casos seguintes:
         switch (aux){

         case (0):
            bit_clear(portb,7);                                                  //se for chamado para o mesmo andar, os leds apagam.
            bit_clear(portb,6);
            chama=0;


         case (1):                                                               //se for chamado para um andar acima, pode estar chamando
            switch (andar){                                                      //a partir do 2º, o que faz ser finalizado apenas
               case(2):                                                          //quando o elevador chega no 3º
               while(andar==2){
                  if(chama==0)break;
                  if (bit_test(portb,4)){
                     if (!bit_test(portb,5)){                                    //testando aqui se ele ja saiu do 2º e chegou no 3º. se sim o led apaga
                        andar=3;
                        bit_clear(portb,6);
                     }
                     while (!bit_test(portb,3)) porta=0x0e;                      //se for pressionada a porta de direção contrária ou
                  }                                                              //as outras portas enquanto ele ainda está no 2º andar, sinaliza erro
                  else while(!bit_test(portb,3) || !bit_test(portb,5)) porta=0x0e;

               }
               break;

               case(1):                                                          //neste caso, partindo do 1º andar, é realizado procedimento
               while(andar==1){                                                  //parecido com o anterior, a diferença é que quando ele chega no 2º
                  if(chama==0)break;                                             //andar ele pergunta se foi aquele o andar desejado. se não, o led continua
                  if (bit_test(portb,3)){                                        //aceso. se sim, o led apaga e volta o processo todo.
                     if (!bit_test(portb,4)){
                        andar=2;
                        if(chama==andar){
                           chama=0;
                           bit_clear(portb,6);
                        }
                     }
                     while(!bit_test(portb,5)) porta=0x0e;
                  }
                  else while(!bit_test(portb,5) || !bit_test(portb,4)) porta=0x0e;
               }
               break;


            }
         break;

         case(2):                                                                //o mesmo procedimento de subida pode ser aplicado analogamente
               switch (andar){                                                   // a descida, mas invertendo as variáveis. ou seja, quando está 
                  case(3):                                                       // no 2º pergunta se chegou no 1º, e quando está no 3º pergunta se
                  while(andar==3){                                               // chegou no 2º e se era este o andar desejado.
                     if(chama==0)break;                                          //quando já está no 1º não tem sentido haver descida.
                     if (bit_test(portb,5)){
                        if (!bit_test(portb,4)){
                           andar=2;
                           if(chama==andar){
                              chama=0;
                              bit_clear(portb,7);
                           }
                        }
                        while(!bit_test(portb,3)) porta=0x0e;
                     }
                     else while(!bit_test(portb,3) || !bit_test(portb,4)) porta=0x0e;

                  }
                  break;

                  case(2):
                  while(andar==2){
                     if(chama==0)break;
                     if (bit_test(portb,4)){
                        if (!bit_test(portb,3)){
                           andar=1;
                           bit_clear(portb,7);
                        }
                        while (!bit_test(portb,5)) porta=0x0e;
                     }
                     else while(!bit_test(portb,3) || !bit_test(portb,5)) porta=0x0e;
                  }

                  break;
               }
            break;
         }

      porta=andar;                                                               //no final do processo, os switchs abertos são fechados, e 
      }                                                                          //o andar no qual o elevador se encontra é mostrado no display
   }                                                                             //através do PORTA
