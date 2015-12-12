#include <msp430.h>
#include "HAL_Cma3000.h"
#include <stdint.h>
#include "inc\hw_memmap.h"
#include "driverlibHeaders.h"
#include "CTS_Layer.h"
#include "grlib.h"
#include "LcdDriver/Dogs102x64_UC1701.h"
#include "peripherals.h"

// CONSTANTS
//#define MCLK                    25000000
//#define TICKSPERUS              (MCLK / 1000000)

// PORT DEFINITIONS for ACCELEROMTER
#define ACCEL_INT_IN            P2IN
#define ACCEL_INT_OUT           P2OUT
#define ACCEL_INT_DIR           P2DIR
#define ACCEL_SCK_SEL           P2SEL
#define ACCEL_INT_IE            P2IE
#define ACCEL_INT_IES           P2IES
#define ACCEL_INT_IFG           P2IFG
#define ACCEL_INT_VECTOR        PORT2_VECTOR
#define ACCEL_OUT               P3OUT
#define ACCEL_DIR               P3DIR
#define ACCEL_SEL               P3SEL

// PIN DEFINITIONS for ACCELEROMTER
#define ACCEL_INT               BIT5
#define ACCEL_CS                BIT5
#define ACCEL_SIMO              BIT3
#define ACCEL_SOMI              BIT4
#define ACCEL_SCK               BIT7
#define ACCEL_PWR               BIT6

// CMA3000 ACCELEROMETER REGISTER DEFINITIONS
#define REVID                   0x01
#define CTRL                    0x02
#define MODE_400                0x04        // Measurement mode 400 Hz ODR
#define DOUTX                   0x06
#define DOUTY                   0x07
#define DOUTZ                   0x08
#define G_RANGE_2               0x80        // 2g range
#define I2C_DIS                 0x10        // I2C disabled


// Define global variables used by CMA3000 accelerometer
int8_t accelData;
int8_t RevID;
int8_t Cma3000_xAccel;
int8_t Cma3000_yAccel;
int8_t Cma3000_zAccel;

// Stores x-Offset
int8_t Cma3000_xAccel_offset;

// Stores y-Offset
int8_t Cma3000_yAccel_offset;

// Stores z-Offset
int8_t Cma3000_zAccel_offset;

int xpos; //x position on board of the 'O'
int ypos; //y position on board of the 'O'

// Function prototypes for this file
void swDelay(char numLoops);

void set_position(int8_t xAccel, int8_t yAccel, int8_t zAccel);

void play_sound(int xpos, int ypos);

void BuzzerOnVar(char period);

void main(void)
{

    // Variable to record button state for later
    CAP_BUTTON keypressed_state;

	// Stop WDT
    WDTCTL = WDTPW | WDTHOLD;		// Stop watchdog timer

    //Perform initializations (see peripherals.c)
    configTouchPadLEDs();
    configDisplay();
    configCapButtons();


    while(1)
    {
    	Cma3000_init(); //initialize the accelerometer
    	Cma3000_readAccel(); //read the accelerations
    	Cma3000_setAccel_offset(-4,-3, 0); //set the offsets
    	Cma3000_readAccel_offset(); //read accelerations with appropriate offsets

        GrStringDrawCentered(&g_sContext, " ", AUTO_STRING_LENGTH, xpos, ypos, OPAQUE_TEXT);
        //GrFlush(&g_sContext);
        set_position(Cma3000_xAccel, Cma3000_yAccel, Cma3000_zAccel);
        play_sound(xpos,ypos);
        GrStringDrawCentered(&g_sContext, "O", AUTO_STRING_LENGTH, xpos, ypos, OPAQUE_TEXT);
        // Refresh the display now that we have finished writing to it
        GrFlush(&g_sContext);

    }
}

void set_position(int8_t xAccel, int8_t yAccel, int8_t zAccel){
	xpos = -xAccel + 51;
	ypos = -yAccel + 32;
}

void play_sound(int xpos, int ypos){
	int pitch;
	pitch = (xpos + ypos)/2;
	BuzzerOnVar(pitch);
}

// From peripherals, this buzzes at different frequencies
void BuzzerOnVar(char period)
{
	// Initialize PWM output on P7.5, which corresponds to TB0.3
	P7SEL |= BIT5; // Select peripheral output mode for P7.5
	P7DIR |= BIT5;
	// aclk = 32768
	TB0CTL = (TBSSEL__ACLK | ID__1 | MC__UP); // Configure Timer B0 to use ACLK, divide by 1, up mode
	TB0CTL &= ~TBIE; 		// Explicitly Disable timer interrupts for safety

	// Now configure the timer period, which controls the PWM period
	// Doing this with a hard coded values is NOT the best method
	// I do it here only as an example. You will fix this in Lab 2.
	TB0CCR0 = period; 					// Set the PWM period in ACLK ticks
	TB0CCTL0 &= ~CCIE;					// Disable timer interrupts

	// Configure CC register 3, which is connected to our PWM pin TB0.3
	TB0CCTL3 = OUTMOD_7;					// Set/reset mode for PWM
	TB0CCTL3 &= ~CCIE;					// Disable capture/compare interrupts
	TB0CCR3 = TB0CCR0 / 2; 					// Configure a 50% duty cycle
}

/***************************************************************************//**
 * @brief  Configures the CMA3000-D01 3-Axis Ultra Low Power Accelerometer
 * @param  none
 * @return none
 ******************************************************************************/

void Cma3000_init(void)
{
        // Need to provide power CMA3000 accelerometer to turn it on
    	ACCEL_SEL &= ~ACCEL_PWR;   // select P3.6 for digital io
        ACCEL_DIR |= ACCEL_PWR;    // set P3.6 as an output
        ACCEL_OUT |= ACCEL_PWR;	   // set P3.6 high (i.e. to 3.3V)

        // Need to set the pins for SIMO and SOMI (P3.3,4) in function mode
        ACCEL_SEL |= ACCEL_SIMO + ACCEL_SOMI; // P3SEL pins 3&4 = 1

        // Also need to set pin for SCLK (P2.7) to function mode
        ACCEL_SCK_SEL |= ACCEL_SCK;   // P3SEL pins 3&4 = 1

        // Setup P3.5 as digital IO to serve as the chip select, CS
        ACCEL_SEL &= ~ACCEL_CS;   // select P3.5 for digital io
        ACCEL_DIR |= ACCEL_CS;   // set P3.5 as an output
        ACCEL_OUT |= ACCEL_CS;   // set P3.5 high (CS inactive)

        // -------NOTE: What follows is specific to the CMA3000 and is not part of
        // part of general SPI configuration ---------------------
        // The CMA3000 is capable of generating interrupts. Its external
        // interrupt line is connected to P2.5 because Port 2 can generate
        // interrupts to the CPU. The interrupt signal from the CMA3000
        // is digital io input to the MSP430
        ACCEL_SCK_SEL &= ~ACCEL_INT;   // Select P2.5 for digital io
        ACCEL_INT_DIR &= ~ACCEL_INT;   // Set P2.5 as an input

        // Generate interrupt on Lo to Hi edge
        ACCEL_INT_IES &= ~ACCEL_INT;
        // Clear interrupt flag
        ACCEL_INT_IFG &= ~ACCEL_INT;
        // ------ End of CMA3000-specific interrupt configuration -------


        // ** Set UCSI A0 Reset=1 to configure control registers **
        UCA0CTL1 |= UCSWRST;
        // 3-pin (SCLK, SIMO, SOMI), 8-bit, this MSP430 is SPI master,
        // Clock polarity high, send data MSB first
        UCA0CTL0 = UCMST + UCSYNC + UCCKPH + UCMSB;
        // Use SMCLK as clock source, keep RESET = 1
        UCA0CTL1 = UCSWRST + UCSSEL_2;

        UCA0BR0 = 2;   // SCLK = SMCLK/2 = 524288Hz
        UCA0BR1 = 0;

        UCA0MCTL = 0;   // write MCTL as 0

        // Enable UCSI A0
        UCA0CTL1 &= ~UCSWRST;

    	//Wait until CMA3000 interrupt line goes high to show sensor is working
    	while (!(ACCEL_INT_IN & ACCEL_INT))
        {
          // Read REVID register
          RevID = Cma3000_readRegister(REVID);
          __delay_cycles(100);

          // Activate measurement mode: 2g range, I2C disabled, 400Hz sampling rate
          accelData = Cma3000_writeRegister(CTRL, G_RANGE_2 | I2C_DIS | MODE_400);

          // Settling time per datasheet ~ 10ms
          __delay_cycles(10485);   // remember by default MCLK = 1,048,576 Hz

          // INT pin interrupt disabled
          ACCEL_INT_IE  &= ~ACCEL_INT;

        }
}

/***************************************************************************//**
 * @brief  Disables the CMA3000-D01 3-Axis Ultra Low Power Accelerometer
 * @param  none
 * @return none
 ******************************************************************************/

void Cma3000_disable(void)
{
    // Set P3.6 to output direction low
    ACCEL_OUT &= ~ACCEL_PWR;

    // Disable P3.3,4 option select
    ACCEL_SEL &= ~(ACCEL_SIMO + ACCEL_SOMI);

    // Disable P2.7 option select
    ACCEL_SCK_SEL &= ~ACCEL_SCK;

    // Set CS inactive
    ACCEL_OUT |= ACCEL_CS;

    // INT pin interrupt disabled
    ACCEL_INT_IE  &= ~ACCEL_INT;

    // Assert Reset=1 to disable UCSI A0
    UCA0CTL1 |= UCSWRST;
}

/***************************************************************************//**
 * @brief  Reads data from the accelerometer
 * @param  None
 * @return None
 ******************************************************************************/

void Cma3000_readAccel(void)
{
    // Read DOUTX register
    Cma3000_xAccel = Cma3000_readRegister(DOUTX);
    __delay_cycles(50);

    // Read DOUTY register
    Cma3000_yAccel = Cma3000_readRegister(DOUTY);
    __delay_cycles(50);

    // Read DOUTZ register
    Cma3000_zAccel = Cma3000_readRegister(DOUTZ);
}

/***************************************************************************//**
 * @brief  Sets accelerometer offset.
 * @param  xAccel_offset  x-axis offset
 * @param  yAccel_offset  y-axis offset
 * @param  zAccel_offset  z-axis offset
 * @return None
 ******************************************************************************/

void Cma3000_setAccel_offset(int8_t xAccel_offset,
                             int8_t yAccel_offset,
                             int8_t zAccel_offset)
{
    // Store x-Offset
    Cma3000_xAccel_offset = xAccel_offset;

    // Store y-Offset
    Cma3000_yAccel_offset = yAccel_offset;

    // Store z-Offset
    Cma3000_zAccel_offset = zAccel_offset;
}

/***************************************************************************//**
 * @brief  Reads data from the accelerometer with removed offset
 * @param  None
 * @return None
 ******************************************************************************/

void Cma3000_readAccel_offset(void)
{
    // Read current accelerometer value
    Cma3000_readAccel();

    // remove offset
    Cma3000_xAccel -= Cma3000_xAccel_offset;

    // remove offset
    Cma3000_yAccel -= Cma3000_yAccel_offset;

    // remove offset
    Cma3000_zAccel -= Cma3000_zAccel_offset;
}

/***************************************************************************//**
 *
 * @brief  Reads data from the accelerometer
 * @param  Address  Address of register
 * @return Register contents
 ******************************************************************************/

int8_t Cma3000_readRegister(uint8_t Address)
{
    uint8_t Result;

    // Address to be shifted left by 2 and RW bit to be reset
    Address <<= 2;

    // Select CMA3000 accelerometer
    ACCEL_OUT &= ~ACCEL_CS;

    // Read RX buffer just to clear UCRXIFG
    Result = UCA0RXBUF;

    // Write address of the CMA3000 register to read to TX buffer
    // This write clears the UCTXIFG
    UCA0TXBUF = Address;

    // Wait until xmit is done
    while (!(UCA0IFG & UCTXIFG))
      __no_operation();

    // Write dummy data to TX buffer
    UCA0TXBUF = 0;
    // Wait until xmit is done
    while (!(UCA0IFG & UCTXIFG))
      __no_operation();

    // Wait until new data was written into RX buffer
    while (!(UCA0IFG & UCRXIFG))
     __no_operation();

    // Read RX buffer
    Result = UCA0RXBUF;

    // Deselect accelerometer
    ACCEL_OUT |= ACCEL_CS;

    // Return new data from RX buffer
    return Result;
}

/***************************************************************************//**
 * @brief  Writes data to the accelerometer
 * @param  Address     Address of CMA3000 register
 * @param  accelData     Data to be written to the accelerometer
 * @return  Received data (acknowledge from CMA3000)
 ******************************************************************************/

int8_t Cma3000_writeRegister(uint8_t Address, int8_t accelData)
{
    uint8_t Result;

    // Address to be shifted left by 2
    Address <<= 2;

    // RW bit to be set
    Address |= 2;

    // Select acceleration sensor
    ACCEL_OUT &= ~ACCEL_CS;

    // Read RX buffer just to clear UCRXIFG
    Result = UCA0RXBUF;

    // Write address to TX buffer
    UCA0TXBUF = Address;

    // Wait until ready to write
    while (!(UCA0IFG & UCTXIFG))
    	__no_operation();

    // Wait until new data was written into RX buffer
    // This is the acknowledge from the CMA3000
    while (!(UCA0IFG & UCRXIFG))
    	__no_operation();

    // Read RX buffer
    Result = UCA0RXBUF;

    // Write data to TX buffer
    UCA0TXBUF = accelData;

    // Wait until xmit is done
    while (!(UCA0IFG & UCTXIFG))
    	__no_operation();

     // Deselect accelerometer
    ACCEL_OUT |= ACCEL_CS;

    return Result;
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



