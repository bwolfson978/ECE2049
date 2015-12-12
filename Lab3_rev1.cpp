/* --COPYRIGHT--,BSD
 * Copyright (c) 2012, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * --/COPYRIGHT--*/

/**************************************************************************************/
/* This program demonstrates some of the features of the TI MSP EXP430F5529 Experimenter
 * Board that we will be using in lab this term. Specifically it demonstrates the use
 * of the chip-specific graphics library as well as the capacitive touch library.
 * This demo code is based off some of the TI-supplied example code hence the retention
 * of the legal header stuff above.
 *
 * This is the type of informative comment block that you should include at the
 * beginning of any program or function that you write.
 *
 * Inputs: None
 * Outputs None
 *
 * Written by : Susan Jarvis
 * 				ECE2049
 * 				25 Aug 2013
 *
 * Modified by : Nick DeMarinis, Jan 2014, Aug 2014, Jan 2015
 */

#include <msp430.h>
#include <stdint.h>
#include "inc\hw_memmap.h"
#include "driverlibHeaders.h"
#include "CTS_Layer.h"
#include <math.h>
#include "grlib.h"
#include "LcdDriver/Dogs102x64_UC1701.h"

/* Peripherals.c and .h are where the functions that implement
 * the LEDs and cap touch buttons are implemented. It is useful
 * to organize your code by putting like functions together in
 * files. You include the header associated with that file(s)
 * into the main file of your project. */
 #include "peripherals.h"

// Define global variables
long stringWidth = 0;

// Function prototypes for this file
void swDelay(char numLoops);

/********************LAB3: Making a Time and Temperature Display ****************/

//symbols for months of the year
char jan[3] = {'J','A','N'};
char feb[3] = {'F','E','B'};
char mar[3] = {'M','A','R'};
char apr[3] = {'A','P','R'};
char may[3] = {'M','A','Y'};
char jun[3] = {'J','U','N'};
char jul[3] = {'J','U','L'};
char aug[3] = {'A','U','G'};
char sep[3] = {'S','E','P'};
char oct[3] = {'O','C','T'};
char nov[3] = {'N','O','V'};
char dec[3] = {'D','E','C'};
char* months[12] = {jan,feb,mar,apr,may,jun,jul,aug,sep,oct,nov,dec};
int month_sizes[12] = {31,28,31,30,31,30,31,31,30,31,30,31}; //num days in each month

//functions to acquire data- ADC, time count, temperature etc.

//function to setup ADC
void setupADC();

//function to read data from temp sensor *check return type
int readTempSensor();

//function to read data from scroll wheel * check return type
int readScrollWheel();

//function to convert 8 bits read in from temp sensor into degrees C and F
struct temperature convertTempData(int data);

//function to convert time to month, day, hours, min, seconds etc.
void timeToDate();

//function to convert number into a string for drawing
void float_to_string(float n, char *res, int afterpoint);

//helper function for float_to_string()
int intToStr(int x, char str[], int d);

//helper function for intToStr()
void reverse(char *str, int len);

//configures and starts timer A2
void runtimerA2(void);

// Temperature Sensor Calibration = Reading at 30 degrees C is stored at addr 1A1Ah
// See end of datasheet for TLV table memory mapping
#define CALADC12_15V_30C  *((unsigned int *)0x1A1A)
// Temperature Sensor Calibration = Reading at 85 degrees C is stored at addr 1A1Ch                                            //See device datasheet for TLV table memory mapping
#define CALADC12_15V_85C  *((unsigned int *)0x1A1C)

unsigned int in_temp;
struct temperature temp;
volatile unsigned long timer = 1400202; //last seven digits of my student ID number

long times[60];
float tempC[60];

int seconds, minutes, hours, days;
unsigned long total_minutes, total_hours;
int month;
char date[6];
char time[8];

struct temperature{
	float tempC; //degrees celcius
	float tempF; //degrees fahrenheit
};

#pragma vector = TIMER2_A0_VECTOR
__interrupt void TimerA2_ISR (void){
	timer++;

	if(timer == (1400202 + 31536000)){
		times[timer%59] = timer;
		timer == 1400202; //reset timer if a year has gone by
	}

	timeToDate();

	ADC12CTL0 &= ~ADC12SC; //clear the start bit
	ADC12CTL0 |= ADC12SC;  // Sampling and conversion start

	// Poll busy bit waiting for conversion to complete
	while (ADC12CTL1 & ADC12BUSY)
		__no_operation();
	in_temp = readTempSensor();      // Read in results if conversion

	temp = convertTempData(in_temp);
	tempC[timer%59] = temp.tempC;

}

void main(void)
{
  WDTCTL = WDTPW + WDTHOLD;      // Stop WDT

  setupADC();

  configDisplay();
  //GrFlush(&g_sContext);

  //populate the tempC[] and times[] arrays with 0's to avoid null pointer exceptions
  int i;
  for(i = 0; i < 60; i++){
	  times[i] = 0;
	  tempC[i] = 0;
  }

  _BIS_SR(GIE); //Global Interrupt Enable

  while(1)
  {

	runtimerA2(); //start the timer

    char degCstr[6];
    char degFstr[6];
    float_to_string(temp.tempC, degCstr, 2); //convert the temperatures to strings for displaying
    float_to_string(temp.tempF, degFstr, 2);

    GrClearDisplay(&g_sContext);
    GrStringDrawCentered(&g_sContext, date, AUTO_STRING_LENGTH, 51, 15, TRANSPARENT_TEXT);
    GrStringDrawCentered(&g_sContext, time, AUTO_STRING_LENGTH, 51, 25, TRANSPARENT_TEXT);
    GrStringDrawCentered(&g_sContext, degCstr, AUTO_STRING_LENGTH, 51, 35, TRANSPARENT_TEXT);
    GrStringDrawCentered(&g_sContext, degFstr, AUTO_STRING_LENGTH, 51, 45, TRANSPARENT_TEXT);
    GrFlush(&g_sContext);
    //swDelay(5);

    //__no_operation();                       // SET BREAKPOINT HERE
  }
}

void setupADC(){
	//take data from two sources: the temp sensor and the scroll wheel
	//make single channel, single conversion readings for the temp sensor
	//select reference voltages that also work with reading the scroll wheel
	REFCTL0 &= ~REFMSTR;    // Reset REFMSTR to hand over control of
	                        // internal reference voltages to
		  	  	  	  	  	// ADC12_A control registers

	ADC12CTL0 = ADC12SHT0_9 | ADC12REFON | ADC12ON; // make reference voltage 1.5V available
	ADC12CTL1 = ADC12SHP; //SAMPCON signal sourced from sampling timer
	ADC12MCTL0 = ADC12SREF_1 + ADC12INCH_10; //use 1.5V as ref. voltage, read from analog input channel A10

	__delay_cycles(100); // delay to allow Ref to settle
	ADC12CTL0 |= ADC12ENC; // Enable conversion

}

int readTempSensor(){
	int in_temp;
	in_temp = ADC12MEM0;
	return in_temp;
}

//int readScrollWheel(){}

//take read temp bits from sensor and return struct with formed degrees C and F
struct temperature convertTempData(int in_temp){
	volatile float temperatureDegC;
	volatile float temperatureDegF;
	volatile float degC_per_bit;
	volatile unsigned int bits30, bits85;
	// Use calibration data stored in info memory
	bits30 = CALADC12_15V_30C;
	bits85 = CALADC12_15V_85C;
	degC_per_bit = ((float)(85.0 - 30.0))/((float)(bits85-bits30));

    // Temperature in Celsius. See the Device Descriptor Table section in the
    // System Resets, Interrupts, and Operating Modes, System Control Module
    // chapter in the device user's guide for background information on the
    // used formula.
    temperatureDegC = (float)((long)in_temp - CALADC12_15V_30C) * degC_per_bit +30.0;

    // Temperature in Fahrenheit Tf = (9/5)*Tc + 32
    temperatureDegF = temperatureDegC * 9.0/5.0 + 32.0;

    struct temperature t;
    t.tempC = temperatureDegC;
    t.tempF = temperatureDegF;
    return t;
}

// reverses a string 'str' of length 'len'
void reverse(char *str, int len)
{
    int i=0, j=len-1, temp;
    while (i<j)
    {
        temp = str[i];
        str[i] = str[j];
        str[j] = temp;
        i++; j--;
    }
}
// Converts a given integer x to string str[].  d is the number
// of digits required in output. If d is more than the number
// of digits in x, then 0s are added at the beginning.
int intToStr(int x, char str[], int d)
{
   int i = 0;
   while (x)
   {
       str[i++] = (x%10) + '0';
       x = x/10;
   }

   // If number of digits required is more, then
   // add 0s at the beginning
   while (i < d)
       str[i++] = '0';

   reverse(str, i);
   str[i] = '\0';
   return i;
}

// Converts a floating point number to string.
void float_to_string(float n, char *res, int afterpoint)
{
    // Extract integer part
    int ipart = (int)n;

    // Extract floating part
    float fpart = n - (float)ipart;

    // convert integer part to string
    int i = intToStr(ipart, res, 0);

    // check for display option after point
    if (afterpoint != 0)
    {
        res[i] = '.';  // add dot

        // Get the value of fraction part upto given no.
        // of points after dot. The third parameter is needed
        // to handle cases like 233.007
        fpart = fpart * pow(10, afterpoint);

        intToStr((int)fpart, res + i + 1, afterpoint);
    }
}

void runtimerA2(void){
	//timer counting 1 second at a time
	TA2CTL = TASSEL_1 + MC_1 + ID_0;
	TA2CCR0 = 32767; // 32767+1 = 32768 ACLK tics = 1 second
	TA2CCTL0 = CCIE; // TA2CCR0 interrupt enabled
}

void timeToDate(){
	seconds = timer % 60;
	total_minutes = timer / 60;
	minutes = total_minutes % 60;
	total_hours = total_minutes / 60;
	hours = total_hours % 24;
	days = total_hours / 24;
	int i = 0;
	while(1){
		if(days - month_sizes[i] < 0){
			days++;
			month = i;
			break;
		}
		days -= month_sizes[i];
		month = i;
		i++;
	}
	date[0] = months[month][0];
	date[1] = months[month][1];
	date[2] = months[month][2];
	date[3] = ' '; //space
	date[4] = (days < 10) ? '0' : '0' + days/10;
	date[5] = (days < 10) ? '0' + days : '0' + days % 10;

	time[0] = (hours < 10) ? '0' : '0' + hours/10;
	time[1] = (hours < 10) ? '0' + hours : '0' + hours%10;
	time[2] = ':';
	time[3] = (minutes < 10) ? '0' : '0' + minutes/10;
	time[4] = (minutes < 10) ? '0' + minutes : '0' + minutes%10;
	time[5] = ':';
	time[6] = (seconds < 10) ? '0' : '0' + seconds/10;
	time[7] = (seconds < 10) ? '0' + seconds : '0' + seconds%10;
}


void swDelay(char numLoops)
{
	// This function is a software delay. It performs
	// useless loops to waste a bit of time
	//
	// Input: numLoops = number of delay loops to execute
	// Output: none
	//
	// smj, ECE2049, 25 Aug 2013

	volatile unsigned int i,j;	// volatile to prevent optimization
			                            // by compiler

	for (j=0; j<numLoops; j++)
    {
    	i = 50000 ;					// SW Delay
   	    while (i > 0)				// could also have used while (i)
	       i--;
    }
}



