//programa ELEVADOR

#include	<16f877a.h>

#fuses	XT, NOWDT, NOPROTECT			   	                                      //condi��es de compila��o relativas ao hardware
#use delay	(clock=4000000)

#use fast_io(a)                     //Estas diretivas definem que o controle E/S das portas ser� definido pelo programador
#use fast_io(b)
#use fast_io(c)
#use fast_io(d)
#use fast_io(e)
                                                   //(em rela��o ao osc; ao watchdog; ao travar o codigo)
#byte porta=5
#byte portb=6
#byte portc=7
#byte portd=8
#byte status=3
	                                       				              //para o pic identificar a porta (portb=6 e porta=5)
byte v1,v2,v3,i,ordem,erro,andar=10,chama[8]={0,0,0,0,0,0,0,0};                                               //� possivel realizar at� duas chamadas ao mesmo tempo


#int_ext
int ext_isr()
{
   byte work;
   work=portb;
   do
   {
      delay_ms(30);
      if(work!=portb) work=portb;
   }while (work!=portb);

   for(i=0;i<8;i++)
      if(chama[i]==0){
         ordem=i;
         break;
      }
   if (!bit_test(portd,0)) {
      bit_set (chama[ordem],4);
      bit_set (portd,0);
   }
   delay_ms(70);
}

#int_rb
int RB_isr()
{
   byte work;
   work=portb;
   do
   {
      delay_ms(30);
      if(work!=portb) work=portb;
   }while (work!=portb);

   for(i=0;i<5;i++)
      if(chama[i]==0){
         ordem=i;
         break;
      }
   if (!bit_test(portb,4) && (!bit_test(portc,2) || !bit_test(portc,6))) {
      bit_set (chama[ordem],0);
      if(bit_test(portd,4)) bit_set (portc,2);
      if(bit_test(portd,5)) bit_set (portc,6);
   }
   if (!bit_test(portb,5) && !bit_test(portc,4)) {
      bit_set (chama[ordem],1);
      bit_set (portc,4);

   }
   if (!bit_test(portb,6) && !bit_test(portc,3)) {
      bit_set (chama[ordem],2);
      bit_set (portc,3);

   }
   if (!bit_test(portb,7) && (!bit_test(portc,5) || !bit_test(portc,7))) {
      bit_set (chama[ordem],3);
      if(bit_test(portd,6)) bit_set (portc,5);
      if(bit_test(portd,7)) bit_set (portc,7);
   }
   delay_ms (70);  //debounce

}

void PARAR (int &a, int &b, int&c)//a=posi��o do bit de chamada b=led correspondente ao andar
{
   bit_clear(portc,0);
   bit_clear(portc,1);
   for(i=0;i<=7;i++) bit_clear(chama[i],a);                                      //se for chamado para o mesmo andar, os leds apagam.
   bit_clear(portc,b);
   bit_clear(portc,c);
   if(andar==2 && bit_test(portd,0)) bit_clear(portd,0);
   delay_ms(1000);
}

#SEPARATE
void SUBIR(int&b, int &c)
{//b=andar acima c=andar sobrado
   bit_set(portc,1);
   bit_clear(portc,0);
   while (!bit_test(portb,andar))
      if(!bit_test(portb,b) || !bit_test(portb,c)) porta=0x0e;
      else porta=andar;
   while (bit_test(portb,b))
      if(!bit_test(portb,andar) || !bit_test(portb,c)) porta=0x0e;
   andar=andar+1;
   for(i=0;i<=7;i++)
   {
      if(chama[0]==8 && (chama[i]==4 || chama[i]==16)) //para o caso de sair do 1� para o 3� e ter q parar no 2�
      {
         bit_clear(chama[i],2);
         bit_clear(chama[i],4);
         bit_clear(portc,3);
         bit_clear(portd,0);
         bit_clear(portc,1);
         porta=2;
         delay_ms(1000);
      }
      if((chama[0]==2 || chama[0]==4) && (chama[i]==2 || chama[i]==4)) //para o caso de sair do 1� e o 2� subindo e o 2� descendo serem apertados simultaneamente
      {
         bit_clear(chama[i],1);
         bit_clear(chama[i],2);
         bit_clear(portc,3);
         bit_clear(portc,4);
         bit_clear(portc,1);
      }

   }
}

void DESCER(int&b, int&c)
{//b=andar abaixo c=andar sobrado
   bit_set(portc,0);
   bit_clear(portc,1);
   while (!bit_test(portb,andar))
      if(!bit_test(portb,b) || !bit_test(portb,c)) porta=0x0e;
      else porta=andar;
   while (bit_test(portb,b))
      if(!bit_test(portb,andar) || !bit_test(portb,c)) porta=0x0e;
   andar=andar-1;
   for(i=0;i<=7;i++)
   {
      if(chama[0]==1 && (chama[i]==2 || chama[i]==16)) //para o caso de sair do 3� para o 1� e ter q parar no 2�
      {
         bit_clear(chama[i],1);
         bit_clear(chama[i],4);
         bit_clear(portc,4);
         bit_clear(portd,0);
         bit_clear(portc,0);
         porta=2;
         delay_ms(1000);
      }
      if((chama[0]==2 || chama[0]==4) && (chama[i]==2 || chama[i]==4)) //para o caso de sair do 1� e o 2� subindo e o 2� descendo serem apertados simultaneamente
      {
         bit_clear(chama[i],1);
         bit_clear(chama[i],2);
         bit_clear(portc,3);
         bit_clear(portc,4);
         bit_clear(portc,0);
      }

   }
}











   void main ()
   {

      set_tris_a (0x00);
      set_tris_b (0xFF);
      set_tris_d (0xF0);                                                       //define entradas e sa�das
      set_tris_c (0x00);
      port_b_pullups (true);                                                    //seta o pull up
      porta=0x0e;
      portc=0x00;
      portb=0xff;
      portd=0x00;
      enable_interrupts (GLOBAL);
      enable_interrupts (int_RB);
      enable_interrupts (int_EXT);
      ext_int_edge(H_TO_L);
                                                                                                //para iniciar o programa, deve-se mostrar
      while(andar==10){                                                                         //onde est� o elevador inicialmente.
         if (!bit_test(portb,1))
            andar=1;
         if (!bit_test(portb,2))
            andar=2;
         if (!bit_test(portb,3))
            andar=3;
         }

      porta=andar;


   while (1)
   {
      if (!bit_test(portb,1))
      {
         andar=1;
         erro=erro+1;
      }                                    //� verificado se h� mais de um fim de curso pressionado
      if (!bit_test(portb,2))
      {
         andar=2;
         erro=erro+1;
      }
      if (!bit_test(portb,3))
      {
         andar=3;
         erro=erro+1;
      }
      if(erro>1) porta=0x0e;
      erro=0;
      while (bit_test(portb,1) && bit_test(portb,2) && bit_test(portb,3) && chama==0) porta=0x0e;
      porta=andar;
      switch(chama[0])
      {
         case(1):
         switch(andar)
         {
            case(1):
            v1=0;
            v2=2;
            v3=6;
            PARAR(v1,v2,v3);
            break;

            case(2):
            v1=1;
            v2=3;
            DESCER(v1,v2);//v1=andar abaixo v2=andar sobrado
            break;

            case(3):
            v1=2;
            v2=1;
            DESCER(v1,v2);
            break;
         }
         break;

         case(2):
         switch(andar)
         {
            case(1):
            v1=2;
            v2=3;
            SUBIR(v1,v2);
            break;

            case(2):
            v1=1;
            v2=4;
            v3=4;
            PARAR(v1,v2,v3);
            break;

            case(3):
            v1=2;
            v2=1;
            DESCER(v1,v2);
            break;
         }
         break;

         case(4):
         switch(andar)
         {
            case(1):
            v1=2;
            v2=3;
            SUBIR(v1,v2);
            break;

            case(2):
            v1=2;
            v2=3;
            v3=3;
            PARAR(v1,v2,v3);
            break;

            case(3):
            v1=2;
            v2=1;
            DESCER(v1,v2);
            break;
         }
         break;

         case(8):
         switch(andar)
         {
            case(1):
            v1=2;
            v2=3;
            SUBIR(v1,v2);
            break;

            case(2):
            v1=3;
            v2=1;
            SUBIR(v1,v2);
            break;

            case(3):
            v1=3;
            v2=5;
            v3=7;
            PARAR(v1,v2,v3);
            break;
         }
         break;

         case(16):
         switch(andar)
         {
            case(1):
            v1=2;
            v2=3;
            SUBIR(v1,v2);
            break;

            case(2):
            v1=4;
            v2=4;
            v3=3;
            PARAR(v1,v2,v3);
            break;

            case(3):
            v1=2;
            v2=1;
            DESCER(v1,v2);
            break;
         }
         default:
         
            if(chama[i]==0)for(i=0;i<=4;i++) if(chama[i+1]!=0)chama[i]=chama[i+1];
            if(chama[i+1]!=1 ||chama[i+1]!=2 || chama[i+1]!=4 || chama[i+1]!=8 || chama[i+1]!=16)
               chama[i+1]==0;
            if(chama[i]==6 && andar==2)chama[i]=chama[i]-6;
            break;
      }

   }
}
