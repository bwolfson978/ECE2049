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


void main(void)
{
    // Define some local variables
    float flt = 190.68;
    int  tst = 0x0400, i = 0;   // In C prefix 0x means the number that follows is in hex
    long unsigned X = 200022;   // No prefix so number is assumed to be in decimal
    char myGrade='A';
    char initial='J';
    char name[8] = "Barrett\0"; // What happens when you change the array length?
    									// What should it be?
    unsigned int somethingFun = 0x2121;

    // Variable to record button state for later
    CAP_BUTTON keypressed_state;

	// Stop WDT
    WDTCTL = WDTPW | WDTHOLD;		// Stop watchdog timer

    //Perform initializations (see peripherals.c)
    configTouchPadLEDs();
    configDisplay();
    configCapButtons();

    // Intro Screen -- Write to the display screen
    GrStringDrawCentered(&g_sContext, "Welcome to", AUTO_STRING_LENGTH, 50, 16, TRANSPARENT_TEXT);
    GrStringDrawCentered(&g_sContext, "ECE2049", AUTO_STRING_LENGTH, 51, 32, TRANSPARENT_TEXT);
    GrStringDrawCentered(&g_sContext, "C-term 2015", AUTO_STRING_LENGTH, 51, 48, TRANSPARENT_TEXT);
    GrStringDrawCentered(&g_sContext, name, AUTO_STRING_LENGTH, 51, 58, TRANSPARENT_TEXT);


    // Refresh the display now that we have finished writing to it
    GrFlush(&g_sContext);

    // Some utterly useless instructions -- Step through them
    // What size does the Code Composer MSP430 Compiler use for the
    // following variable types? A float, an int, a long integer and a char?
    flt = flt*tst;
    X = tst+X;
    tst = tst-myGrade;  // A number minus a letter?? What's actually going on here?
                        // What value stored in myGrade (i.e. what's the ASCII code for "A")?
    				    // Thus, what is the new value of tst? Explain?

    P1SEL = P1SEL & ~BIT0;          // Select P1.0 for digital IO
    P1DIR |= BIT0;					// Set P1.0 to output direction


	/* Monitor Capacitive Touch Pads in endless "forever" loop */
    while(1)
    {
		P1OUT |= (BIT1 + BIT2 + BIT3 + BIT4 + BIT5);   // turn on all 5 LEDs

		// Check cap touch keys
		// This function returns 0 if no buttons were pressed, or sets a bit in
		// buttonState for each button that was pressed
		keypressed_state = CapButtonRead();

	    if (keypressed_state) // If any key was pressed
	    {
	    	// turn off LED of pressed key
	    	P1OUT &= ~(keypressed_state << 1);

	    	// If ceramic buzzer is not attached (P7.5 and GND)
	    	// This does nothing useful
            if (keypressed_state == BIT0 || keypressed_state == BIT4)
            	BuzzerOn();

	    	// Wait for a while so we can see the change
	    	swDelay(1);
	    	BuzzerOff(); // Does nothing useful without buzzer attached
	    }

	    //adding the blinking LED0, these lines configure and toggle the LED
		P1OUT = P1OUT ^ BIT0;		// Toggle P1.0 using exclusive-OR
		swDelay(1);

	    /*
	     * This example does not draw anything to the display after starting up.
	     *
	     * However, if we made any updates to the LCD during operation, we would
	     * need to refresh the display as shown in the next line (commented out).
	     *
	     * Since this operation is relatively slow, it is best to do this only
	     * sparsely and not with every screen write.
	     */
	    //GrFlush(&g_sContext);

    }
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



