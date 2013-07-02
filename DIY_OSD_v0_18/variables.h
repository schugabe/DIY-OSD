#ifndef __variables_H
#define __variables_H

#include "config.h"
#include <EEPROM.h>

unsigned char toptext[12] = {'S'-64,'P'-64,'E'-64,'E'-64,'D'-64,'L'-64,'O'-64,'S'-64,'A'-64,'L'-64,'T'-64,20};

// Linenumber
int line = 0;
int temp =0;

// Standard integers to use with for/while loops etc.
int a=0;
int i=0;
int k=0;

unsigned char ii=0;

//========================================
// Data for mAh
//========================================
// Measuring amps and mah's;
int loopcount=0;
int current=0;

unsigned char currentr[] = {
  3,3,3,3,3}; // Stores current characters (numbers) written to screen
unsigned char voltager[] = {
  3,3,3,3,3}; // Stores current characters (numbers) written to screen
int bat_volt =0;

long mah = 0;
long mahtemp=0;
unsigned char mahr[]={
  3,3,3,3,3,3}; // Stores mah characters (numbers) written to screen


//ADC (It's a 10 bit ADC, so we need to read from 2 registers and put them together. This is what ADSCtemp is used for
int ADCtemp=0;
int ADCtemp2=0;
int ADCreal=0;  // Just a stupid name for the complete analog-read
int ADCreal2=0; // Can be replaced with ADCreal

//========================================
// Buffers
//========================================
unsigned char buffer[20];

// Need an integer when reading large characeters (will exceed 256)
int buffer2[12]={
  12,12,12,12,12,12,12,12,12,12,12,12};
unsigned int buffer3[15];


//========================================
// Mixed
//========================================

unsigned int mahkm_buf[5];

unsigned char showcoordinates=1;

int rssi_reading=0;
unsigned char rssir[]={
  3,3,3,3};
unsigned char rssi_negative = 0;

// Numbers and letters
#endif
