//                   programa teclado
#include    <16f877a.h>                              //biblioteca do pic
#device     ICD=TRUE
#fuses      XT, NOWDT, NOPROTECT                    //condições de compilação relativas ao hardware (em relação ao osc; ao watchdog; ao travar o codigo)
#use        delay   (clock=4000000)
#byte       porta=5
#byte       portb=6
#byte       portc=7
#byte       portd=8

/***************************** VARIÁVEIS GLOBAIS ******************************/
   unsigned int num_digitos, interrupcao=0,int_anterior, tecla, i, hora, horario;
   
/************************ INTERRUPÇÃO - RB (TECLADO) **************************/
#int_RB
int RB_isr()
{
   interrupcao=portb;
   return(0);
}

/************************ FUNÇÃO VARREDURA (TECLADO) **************************/
void varredura(){
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
      int_anterior=interrupcao;
         for(i=0;i<=4;i++)//Debounce: o valor adquirido na interrupção tem que ser o mesmo durante 75ms; 
         {
           // if(interrupcao==int_anterior) delay_ms(15);
            //else i=0;
         }
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
            
         //no fim da função, o valor da tecla é zerado para que o valor obtido só seja computado uma vez.
         }
         interrupcao=0;//se for quando o botao é solto, não faz nada.
         
      }
}
 
/*********************** FUNÇÃO - PEGANUMERO (TECLADO) ************************/ 
void peganumero ()
{
   int min_menor=0, min_maior=0;
   
   if (hora!=0x00)hora_maior=hora;
   switch(tecla){
      case(10):
         hora=0x00;
         return;
      case(11):
         break;
      case(12):
         hora=hora_maior;
         horario=1;
         break;
      default:
         hora_menor=tecla;
   }
   if((hora_maior>0x10 && hora_maior!=0x20)|| (hora_maior*16+hora_menor)>0x23)
      hora=hora_menor;
   else
      hora=hora_maior*16+hora_menor;
   tecla=0;
}
        



/*******************************ROTINA PRINCIPAL*******************************/
void main ()
{
  
   set_tris_a (0x00);
   set_tris_b (0xF0);
   set_tris_d (0x00);                                                        //define entradas e saídas
   set_tris_c (0x00);
   
   port_b_pullups (true);                                                    //seta o pull up
   porta=0x0e;
   portc=0x00;
   portb=0xf0;
   portd=0x00;
   enable_interrupts (GLOBAL);
   enable_interrupts (int_RB);
   interrupcao=0;
   while(horario==0)
   {
      varredura();
      if(tecla!=0) peganumero();
      portd=hora;
   }
}

