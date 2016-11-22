//programa comanda led

#include	<16f84.h>					       	//biblioteca do pic
#fuses	XT, NOWDT, NOPROTECT			   	//condições de compilação relativas ao hardware (em relação ao osc; ao watchdog; ao travar o codigo)
#use delay	(clock=4000000)
#byte porta=5
#byte portb=6	




void main()
{

	set_tris_b (0x3f);					   	//define entradas e saídas
	set_tris_a (0x00);
	port_b_pullups (true);				   	//seta o pull up
	bit_clear (portb,1);				     		//coloca o estado inicial do led como apagado

	while (1)
	{
		if (bit_test(portb,0))					//testa a chave (ativo em zero)
			bit_clear(portb,1);		   		//se tiver aberta, apaga
		else						               //se não, acende
			bit_set(portb,1);
	}
}
