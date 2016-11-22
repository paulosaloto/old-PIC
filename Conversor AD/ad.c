//programa CONVERSOR A/D



#include	<16f877a.h>
#DEVICE ADC=8

#include <stdlib.h>


#fuses	XT, NOWDT, NOPROTECT,PUT			   	                                      //condi��es de compila��o relativas ao hardware
#use delay	(clock=4000000)
                                                                                      //(em rela��o ao osc; ao watchdog; ao travar o codigo)
#byte porta=5
#byte portb=6
#byte portc=7
#byte portd=8
	                                       				                                //para o pic identificar a porta (portb=6 e porta=5)


int main ()
{
   
   //tabela para mostrar a letra no display
   byte tabela[10] = {0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x67};
   
   
   //tabela para indicar qual display esta ativado
   long value, display;
   set_tris_a (0x1f);
   set_tris_d (0x00);
   set_tris_c (0x00);

   setup_adc_ports(ALL_ANALOG);
   setup_adc(ADC_CLOCK_DIV_8);
   set_adc_channel(0);
   delay_ms(10);

   while (1)
   {
      value=(read_adc());
      value=value*5;
      if (bit_test(portc,0))
      {
         display=(value/255);

         bit_set(portc,1);
         bit_clear(portc,0);
         portd=tabela[display];
         bit_set(portd,7);
      }
      else
      {
         value=value%255;
         value=value*10;
         display=(value/255);
         bit_set(portc,0);
         bit_clear(portc,1);  
         portd=tabela[display];
      }
      delay_ms(10);

   }

   
}
