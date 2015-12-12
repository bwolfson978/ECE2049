#include <msp430.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
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
#include "alien.h"


// Define global variables
long stringWidth = 0;

// Function prototypes for this file
void swDelay(char numLoops);
void displayWelcomeScreen();
void countDown();;
void displayRow();
void readPads();
int gameOver();
int boardEmpty();
void nextLevelScreen();
void addRow();
void moveAliensDown(int speed);
void displayScreen();
void gameOverScreen();
void killColumnOne();
void killColumnTwo();
void killColumnThree();
void killColumnFour();
void killColumnFive();


alien col1[10];
alien col2[10];
alien col3[10];
alien col4[10];
alien col5[10];

void main(void)
{

    // Variable to record button state for later
    CAP_BUTTON keypressed_state;

	// Stop WDT
    WDTCTL = WDTPW | WDTHOLD;

    //Perform initializations (see peripherals.c)
    configTouchPadLEDs();
    configDisplay();
    configCapButtons();

    // Refresh the display now that we have finished writing to it
    GrFlush(&g_sContext);

    //starting state of the program
    int state,level,waves;

    state = 0;
    level = 1;
    waves = 2;

	/* Monitor Capacitive Touch Pads in endless "forever" loop */
    while(1)
    {
    	P1OUT |= (LED4 + LED5 + LED6 + LED7 + LED8);
		//P1OUT |= (BIT0 + BIT1 + BIT2 + BIT3 + BIT4);   // turn on all 5 LEDs
		keypressed_state = CapButtonRead();

		switch(state)
		{
			case 0 : //Display Welcome Screen
				displayWelcomeScreen();
				if(keypressed_state == BIT0)
					state++;
				break;

			case 1 : //Start count down to game
				countDown();
				GrClearDisplay(&g_sContext);
				state++;
				break;

			case 2: //play the game
				addRow(); //first row of aliens
				displayScreen();
				while(waves > 0)
				{
					moveAliensDown(level); //move the aliens down to give allusion of falling
					displayScreen();
					readPads(); //read in touch pads and react
					displayScreen();
					if(boardEmpty()){ //if aliens of current wave have all been killed
						addRow(); //add a new wave of aliens to the screen
						waves--;
					}
					if(gameOver()){
						state = 3;
						break;
					}
					else{
						level++;
						waves = level;
						nextLevelScreen();
						swDelay(5);
						countDown();
					}
				}
				break;

			case 3: //game over, player lost
				gameOverScreen();
				swDelay(5);
				GrClearDisplay(&g_sContext);
				state = 0;
				break;
		}
    }
}



void displayWelcomeScreen()
{
    // Welcome Screen -- Write to the display screen
    GrStringDrawCentered(&g_sContext, "Space Invaders", AUTO_STRING_LENGTH, 51, 16, TRANSPARENT_TEXT);
    GrStringDrawCentered(&g_sContext, "Press the X", AUTO_STRING_LENGTH, 51, 30, TRANSPARENT_TEXT);
    GrStringDrawCentered(&g_sContext, "key to begin...", AUTO_STRING_LENGTH, 51, 45, TRANSPARENT_TEXT);
    GrFlush(&g_sContext);
}


void countDown()
{
	GrClearDisplay(&g_sContext);
    GrStringDrawCentered(&g_sContext, "3", AUTO_STRING_LENGTH, 51, 30, TRANSPARENT_TEXT);
    GrFlush(&g_sContext);
    swDelay(2);
    GrClearDisplay(&g_sContext);
    GrStringDrawCentered(&g_sContext, "2", AUTO_STRING_LENGTH, 51, 30, TRANSPARENT_TEXT);
    GrFlush(&g_sContext);
    swDelay(2);
    GrClearDisplay(&g_sContext);
    GrStringDrawCentered(&g_sContext, "1", AUTO_STRING_LENGTH, 51, 30, TRANSPARENT_TEXT);
    GrFlush(&g_sContext);
}

void readPads(){
	CAP_BUTTON keypressed_state = CapButtonRead();
	switch(keypressed_state){
		case BIT0:
			killColumnOne();
			break;
		case BIT1:
			killColumnTwo();
			break;
		case BIT2:
			killColumnThree();
			break;
		case BIT3:
			killColumnFour();
			break;
		case BIT4:
			killColumnFive();
			break;
	}
}

//adds a row of aliens to the game
void addRow(){
	col1[0] = constructAlien('X', 10, 10);
	col2[0] = constructAlien('X', 30, 10);
	col3[0] = constructAlien('X', 50, 10);
	col4[0] = constructAlien('X', 70, 10);
	col5[0] = constructAlien('X', 90, 10);

	/*
	int a = rand() % 2; //generate random 0 or 1
	srand(time(NULL)); //change seed to get different randoms next time
	if(a == 1){
		col1[0] = constructAlien("X", 10, 10);
	}
	else
		col1[0] = constructAlien(" ", 10, 10);

	int b = rand() % 2; //generate random 0 or 1
	srand(time(NULL)); //change seed to get different randoms next time
	if(b == 1){
		col2[0] = constructAlien("X", 30, 10);
	}
	else
		col2[0] = constructAlien(" ", 30, 10);

	int c = rand() % 2; //generate random 0 or 1
	srand(time(NULL)); //change seed to get different randoms next time
	if(c == 1){
		col3[0] = constructAlien("X", 50, 10);
	}
	else
		col3[0] = constructAlien(" ", 50, 10);

	int d = rand() % 2; //generate random 0 or 1
	srand(time(NULL)); //change seed to get different randoms next time
	if(d == 1){
		col4[0] = constructAlien("X", 70, 10);
	}
	else
		col4[0] = constructAlien(" ", 70, 10);

	int e = rand() % 2; //generate random 0 or 1
	srand(time(NULL)); //change seed to get different randoms next time
	if(e == 1){
		col5[0] = constructAlien("X", 90, 10);
	}
	else
		col5[0] = constructAlien(" ", 90, 10);
	*/
}


void moveAliensDown(int speed){
	int i,j,k,l,m;
	for(i = 0; i < 10; i++){
		col1[i].ypos += speed;
	}
	for(j = 0; j < 10; j++){
		col2[j].ypos += speed;
	}
	for(k = 0; k < 10; k++){
		col3[k].ypos += speed;
	}
	for(l = 0; l < 10; l++){
		col4[l].ypos += speed;
	}
	for(m = 0; m < 10; m++){
		col5[m].ypos += speed;
	}
}

void displayScreen(){
	GrClearDisplay(&g_sContext);
	int i,j,k,l,m;
	for(i = 0; i < 10; i++){
		alien a = col1[i];
		char print[1] = {a.symbol};
		GrStringDrawCentered(&g_sContext, print, AUTO_STRING_LENGTH, a.xpos, a.ypos, TRANSPARENT_TEXT);
	}
	for(j = 0; j < 10; j++){
		alien a = col2[j];
		char print[1] = {a.symbol};
		GrStringDrawCentered(&g_sContext, print, AUTO_STRING_LENGTH, a.xpos, a.ypos, TRANSPARENT_TEXT);
	}
	for(k = 0; k < 10; k++){
		alien a = col3[k];
		char print[1] = {a.symbol};
		GrStringDrawCentered(&g_sContext, print, AUTO_STRING_LENGTH, a.xpos, a.ypos, TRANSPARENT_TEXT);
	}
	for(l = 0; l < 10; l++){
		alien a = col4[l];
		char print[1] = {a.symbol};
		GrStringDrawCentered(&g_sContext, print, AUTO_STRING_LENGTH, a.xpos, a.ypos, TRANSPARENT_TEXT);
	}
	for(m = 0; m < 10; m++){
		alien a = col5[m];
		char print[1] = {a.symbol};
		GrStringDrawCentered(&g_sContext, print, AUTO_STRING_LENGTH, a.xpos, a.ypos, TRANSPARENT_TEXT);
	}
	GrFlush(&g_sContext); //dump drawings stored in buffer onto the display
}

void killColumnOne(){
	int i, index_lowest; //index of the lowest alien in this column
	index_lowest = 0;
	for(i = 0; i < 10; i++){
		if(col1[i].ypos < 150 && col1[i].ypos > 0){ //make sure legitimate value, on screen
			if(col1[index_lowest].ypos < col1[i].ypos){
				index_lowest = i;
			}
		}
	}
	col1[index_lowest].symbol = ' ';
}

void killColumnTwo(){
	int i, index_lowest; //index of the lowest alien in this column
	index_lowest = 0;
	for(i = 0; i < 10; i++){
		if(col2[i].ypos < 150 && col2[i].ypos > 0){ //make sure legitimate value, on screen
			if(col2[index_lowest].ypos < col2[i].ypos){
				index_lowest = i;
			}
		}
	}
	col2[index_lowest].symbol = ' ';
}

void killColumnThree(){
	int i, index_lowest; //index of the lowest alien in this column
	index_lowest = 0;
	for(i = 0; i < 10; i++){
		if(col3[i].ypos < 150 && col3[i].ypos > 0){ //make sure legitimate value, on screen
			if(col3[index_lowest].ypos < col3[i].ypos){
				index_lowest = i;
			}
		}
	}
	col3[index_lowest].symbol = ' ';
}

void killColumnFour(){
	int i, index_lowest; //index of the lowest alien in this column
	index_lowest = 0;
	for(i = 0; i < 10; i++){
		if(col4[i].ypos < 150 && col4[i].ypos > 0){ //make sure legitimate value, on screen
			if(col4[index_lowest].ypos < col4[i].ypos){
				index_lowest = i;
			}
		}
	}
	col4[index_lowest].symbol = ' ';
}

void killColumnFive(){
	int i, index_lowest; //index of the lowest alien in this column
	index_lowest = 0;
	for(i = 0; i < 10; i++){
		if(col5[i].ypos < 150 && col5[i].ypos > 0){ //make sure legitimate value, on screen
			if(col5[index_lowest].ypos < col5[i].ypos){
				index_lowest = i;
			}
		}
	}
	col5[index_lowest].symbol = ' ';
}

//check if the player has lost(an alien has reached the bottom)
//returns 1 if all aliens are dead (screen is blank)
//returns 0 if aliens still alive on the board
int gameOver(){
	int gameOver;
	gameOver = 0; //0 if still playing, 1 if the game is over
	int i;
	for(i = 0; i < 10; i++){
		if((col1[i].ypos) > 62 && (col1[i].ypos < 100)) //alien has hit the bottom of the screen
			gameOver = 1;
	}
	for(i = 0; i < 10; i++){
		if((col2[i].ypos) > 62 && (col2[i].ypos < 100)) //alien has hit the bottom of the screen
			gameOver = 1;
	}
	for(i = 0; i < 10; i++){
		if((col3[i].ypos) > 62 && (col3[i].ypos < 100)) //alien has hit the bottom of the screen
			gameOver = 1;
	}
	for(i = 0; i < 10; i++){
		if((col3[i].ypos) > 62 && (col3[i].ypos < 100)) //alien has hit the bottom of the screen
			gameOver = 1;
	}
	for(i = 0; i < 10; i++){
		if((col3[i].ypos) > 62 && (col3[i].ypos < 100)) //alien has hit the bottom of the screen
			gameOver = 1;
	}

	return gameOver;
}

//returns 1 if all aliens are dead (screen is blank)
//returns 0 if aliens still alive on the board
int boardEmpty(){
	int empty; //0 if all aliens dead, 1 if still alive
	empty = 1;
	int i;
	for(i = 0; i < 10; i++){
		if((col1[i].ypos) < 62 && (col1[i].ypos > 0) && \
				(col1[i].xpos < 102) && (col1[i].xpos > 0) && (col1[i].symbol != ' ')){ //this alien is still on the board
					empty = 0;
		}
	}
	for(i = 0; i < 10; i++){
		if((col2[i].ypos < 62) && (col2[i].ypos > 0) && \
				(col2[i].xpos < 102) && (col2[i].xpos > 0) && (col2[i].symbol != ' ')){ //this alien is still on the board
			empty = 0;
		}
	}
	for(i = 0; i < 10; i++){
		if((col3[i].ypos < 62) && (col3[i].ypos) > 0 && \
				(col3[i].xpos < 102) && (col3[i].xpos > 0) && (col3[i].symbol != ' ')){ //this alien is still on the board
			empty = 0;
		}
	}
	for(i = 0; i < 10; i++){
		if((col4[i].ypos < 62) && (col4[i].ypos > 0) && \
				(col4[i].xpos < 102) && (col4[i].xpos > 0) && (col4[i].symbol != ' ')){ //this alien is still on the board
			empty = 0;
		}
	}
	for(i = 0; i < 10; i++){
		if((col5[i].ypos < 62) && (col5[i].ypos > 0) && \
				(col5[i].xpos < 102) && (col5[i].xpos > 0) && (col5[i].symbol != ' ')){ //this alien is still on the board
			empty = 0;
		}
	}
	return empty;
}

//display the next level screen
void nextLevelScreen(){
	// Next Level Screen
	GrStringDrawCentered(&g_sContext, "Next Level...", AUTO_STRING_LENGTH, 51, 16, TRANSPARENT_TEXT);
	GrFlush(&g_sContext);
}

//display the next game over screen
void gameOverScreen(){
	//game over screen
	GrClearDisplay(&g_sContext);
	GrStringDrawCentered(&g_sContext, "GAME OVER", AUTO_STRING_LENGTH, 51, 16, TRANSPARENT_TEXT);
	GrFlush(&g_sContext);
}

//delay
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





