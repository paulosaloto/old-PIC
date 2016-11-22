//programa ELEVADOR

#include	<16f84.h>

#fuses	XT, NOWDT, NOPROTECT			   	                                      //condi��es de compila��o relativas ao hardware
#use delay	(clock=4000000)                                                     //(em rela��o ao osc; ao watchdog; ao travar o codigo)
#byte porta=5
#byte portb=6	                                       				              //para o pic identificar a porta (portb=6 e porta=5)



   void main ()
   {

      int erro,aux,andar=4,chama=0;                                               //� possivel realizar at� duas chamadas ao mesmo tempo


      set_tris_b (0x3F);                                                        //define entradas e sa�das
      set_tris_a (0x00);
      port_b_pullups (true);                                                    //seta o pull up
      porta=0x0e;
      portb=0;


      while (andar==4)                                                          //para iniciar o programa, deve-se mostrar
      {                                                                         //onde est� o elevador inicialmente.
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

         if (!bit_test(portb,0)) chama=1;                                        //aqui s�o testadas as chamadas.

         if (!bit_test(portb,1)) chama=2;

         if (!bit_test(portb,2)) chama=3;

         if (!bit_test(portb,3)) erro=erro+1;                                    //� verificado se h� mais de um fim de curso pressionado
         if (!bit_test(portb,4)) erro=erro+1;                                    //ao mesmo tempo. caso seja, aparece E no display
         if (!bit_test(portb,5)) erro=erro+1;
        
         while (erro>1) porta=0x0e;
         erro=0;

      while (bit_test(portb,5) && bit_test(portb,4) && bit_test(portb,3) && chama==0) porta=0x0e;//� testado se o elevador est� fora de um fim
                                                                                 //de curso sem que uma chamada tenha sido feita
         if(chama>andar){                                                        
            aux=1;                                                               //se for feita uma chamada para um andar acima, o led de 
            bit_set(portb,6);                                                    //subida acende e a vari�vel auxiliar fica =1
         }
         else if (chama!=0 && chama<andar){                                      //se for a um andar abaixo, � verificada se houve uma chamda
               aux=2;                                                            //a variavel aux fica =2 e o led de descida setado
               bit_set(portb,7);
            }
            else aux=0;                                                          //se a chamada for igual ao andar, aux=0.


         if (chama!='\0')                                                        //caso haja a chamada, ter�o os 5 casos seguintes:
         switch (aux){

         case (0):
            bit_clear(portb,7);                                                  //se for chamado para o mesmo andar, os leds apagam.
            bit_clear(portb,6);
            chama=0;


         case (1):                                                               //se for chamado para um andar acima, pode estar chamando
            switch (andar){                                                      //a partir do 2�, o que faz ser finalizado apenas
               case(2):                                                          //quando o elevador chega no 3�
               while(andar==2){
                  if(chama==0)break;
                  if (bit_test(portb,4)){
                     if (!bit_test(portb,5)){                                    //testando aqui se ele ja saiu do 2� e chegou no 3�. se sim o led apaga
                        andar=3;
                        bit_clear(portb,6);
                     }
                     while (!bit_test(portb,3)) porta=0x0e;                      //se for pressionada a porta de dire��o contr�ria ou
                  }                                                              //as outras portas enquanto ele ainda est� no 2� andar, sinaliza erro
                  else while(!bit_test(portb,3) || !bit_test(portb,5)) porta=0x0e;

               }
               break;

               case(1):                                                          //neste caso, partindo do 1� andar, � realizado procedimento
               while(andar==1){                                                  //parecido com o anterior, a diferen�a � que quando ele chega no 2�
                  if(chama==0)break;                                             //andar ele pergunta se foi aquele o andar desejado. se n�o, o led continua
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
               switch (andar){                                                   // a descida, mas invertendo as vari�veis. ou seja, quando est� 
                  case(3):                                                       // no 2� pergunta se chegou no 1�, e quando est� no 3� pergunta se
                  while(andar==3){                                               // chegou no 2� e se era este o andar desejado.
                     if(chama==0)break;                                          //quando j� est� no 1� n�o tem sentido haver descida.
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

      porta=andar;                                                               //no final do processo, os switchs abertos s�o fechados, e 
      }                                                                          //o andar no qual o elevador se encontra � mostrado no display
   }                                                                             //atrav�s do PORTA
