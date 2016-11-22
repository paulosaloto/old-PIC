///////////////////////////////////////////////////////////////////////////
////   Biblioteca para memória serial EEPROM 24LC256 serial EEPROM     ////
////   e RTC PFC8583 para o PIC 16f877 com o kit.                      //// 
////   o RTC não precisa ser inicializado.                             ////
////                                                                   ////
////   init_ext_eeprom();    Call before the other functions are used  ////
////                                                                   ////
////   write_ext_eeprom(a, d);   Write the byte d to the address a     ////
////   write_RTC(a, d);          Write the byte d to the address a     ////
////                                                                   ////
////   d = read_ext_eeprom(a);   Read the byte d from the address a    ////
////   d = read_RTC(a);          Read the byte d from the address a    ////
////                                                                   //// 
////                                                                   ////
////   The main program may define eeprom_sda                          ////
////   and eeprom_scl to override the defaults below.                  ////
////                                                                   ////
///////////////////////////////////////////////////////////////////////////
////        (C) Copyright 1996,2003 Custom Computer Services           ////
//// This source code may only be used by licensed users of the CCS C  ////
//// compiler.  This source code may only be distributed to other      ////
//// licensed users of the CCS C compiler.  No other use, reproduction ////
//// or distribution is permitted without written permission.          ////
//// Derivative programs created using this software in object code    ////
//// form are not restricted in any way.                               ////
///////////////////////////////////////////////////////////////////////////


#ifndef EEPROM_SDA

#define EEPROM_SDA  PIN_C4
#define EEPROM_SCL  PIN_C3

#endif

#use i2c(master, slow, sda=EEPROM_SDA, scl=EEPROM_SCL)

#define EEPROM_ADDRESS long int
#define EEPROM_SIZE   32768

void init_ext_eeprom()
{
   output_float(EEPROM_SCL);
   output_float(EEPROM_SDA);

}


void write_ext_eeprom(long int address, BYTE data)
{
   short int status;
   i2c_start();
   i2c_write(0xaE);
   i2c_write(address>>8);
   i2c_write(address);
   i2c_write(data);
   i2c_stop();
   i2c_start();
   status=i2c_write(0xaE);
   while(status==1)
   {
   i2c_start();
   status=i2c_write(0xaE);
   }
}


BYTE read_ext_eeprom(long int address) {
   BYTE data;
   i2c_start();
   i2c_write(0xaE);
   i2c_write(address>>8);
   i2c_write(address);
   i2c_start();
   i2c_write(0xaF);
   data=i2c_read(0);
   i2c_stop();
   return(data);
}


void write_RTC(int address, BYTE data)
{
   short int status;
   i2c_start();
   i2c_write(0xa0);
   i2c_write(address);
   i2c_write(data);
   i2c_stop();
   i2c_start();
   status=i2c_write(0xa0);
   while(status==1)
   {
   i2c_start();
   status=i2c_write(0xa0);
   }
}

BYTE read_RTC(int address) {
   BYTE data;
   i2c_start();
   i2c_write(0xa0);
   i2c_write(address);
   i2c_start();
   i2c_write(0xa1);
   data=i2c_read(0);
   i2c_stop();
   return(data);
}
