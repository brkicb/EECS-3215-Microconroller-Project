/*
 * Copyright (c) 2017, NXP Semiconductor, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of NXP Semiconductor, Inc. nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
 
/**
 * @file    MKL43Z256xxx4__ElectronicLockSystem.c
 * @brief   Application entry point.
 */
#include <stdio.h>
#include "board.h"
#include "peripherals.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "MKL43Z4.h"
/* TODO: insert other include files here. */

/* TODO: insert other definitions and declarations here. */
#define FALSE 0
#define TRUE  1

int passwd[] = {1,2,3,4};
const int numButtons = 7;
int correct[] = {FALSE, FALSE, FALSE, FALSE};
int unlock = FALSE;
int attempt = 1;
int attemptDelay = 1;
int flash = 0;
int resetStart = TRUE;
volatile static int i = 0;
volatile static int j = 0;
volatile static int k = 0;
volatile static int start = TRUE;
volatile static int check = 0;

void delay(int d) {
	volatile uint32_t i = 0;
	volatile uint32_t j = 0;

	for (i=0; i < d; ++i) {
		for (j=0; j < 800000; ++j) {
			__asm("NOP");
		}
	}
}
void displayOne(void) {
	PTA->PSOR = 0b00000000000000000000000000000010;		// Segment A
	PTA->PSOR = 0b00000000000000000000000000010000;		// Segment D
	PTA->PSOR = 0b00000000000000000000000000100000;		// Segment E
	PTA->PSOR = 0b00000000000000000010000000000000;		// Segment F
	PTE->PSOR = 0b00000000000000000000000000000010;		// Segment F
}
void displayTwo(void) {
	PTA->PSOR = 0b00000000000000000001000000000000;		// Segment C
	PTA->PSOR = 0b00000000000000000010000000000000;		// Segment F
}
void displayThree(void) {
	PTA->PSOR = 0b00000000000000000000000000100000;		// Segment E
	PTA->PSOR = 0b00000000000000000010000000000000;		// Segment F
}
void displayFour(void) {
	PTA->PSOR = 0b00000000000000000000000000000010;		// Segment A
	PTA->PSOR = 0b00000000000000000000000000010000;		// Segment D
	PTA->PSOR = 0b00000000000000000000000000100000;		// Segment E
}
void displayFive(void) {
	PTA->PSOR = 0b00000000000000000000000000000100;		// Segment B
	PTA->PSOR = 0b00000000000000000000000000100000;		// Segment E
}
void setup(void) {
	// Light up LEDs. Start by setting all digit values to output zero.
	PTD->PCOR = 0b00000000000000000000000000111100;		// bits 5,4,3,2: Digits 4,3,2,1
	PTA->PCOR = 0b00000000000000000011000000110110;		// bits 13,12,5,4,2,1: Segments F,C,E,D,B,A 
	PTE->PCOR = 0b00000000000000000000000000000010;		// bit 1: Segment G
}
void digitOn(int digit) {
	setup();
	if (digit == 1) {
		// turn on Digit 1
		PTD->PSOR = 0b00000000000000000000000000000100;		// Digit 1 on. bits 2: Digits 1
	}
	else if (digit == 2) {
		// turn on digit 2
		PTD->PSOR = 0b00000000000000000000000000001000;		// Digit 2 on. bits 3: Digits 2
	}
	else if (digit == 3) {
		// turn on digit 3
		PTD->PSOR = 0b00000000000000000000000000010000;		// Digit 3 on. bits 4: Digits 3
	}
	else if (digit == 4) {
		// turn on digit 4
		PTD->PSOR = 0b00000000000000000000000000100000;		// Digit 4 on. bits 5: Digits 4
	}
}
void digitOff(int digit) {
	if (digit == 1) {
		// turn off digit 1
		PTD->PCOR = 0b00000000000000000000000000000100;		// Digit 1 off. Bits 2: Digits 1
	}
	else if (digit == 2) {
		// turn off digit 2
		PTD->PCOR = 0b00000000000000000000000000001000;		// Digit 2 off. Bits 3: Digits 2
	}
	else if (digit == 3) {
		// turn off digit 3
		PTD->PCOR = 0b00000000000000000000000000010000;		// Digit 3 off. Bits 4: Digits 3
	}
	else if (digit == 4) {
		// turn off digit 4
		PTD->PCOR = 0b00000000000000000000000000100000;		// Digit 4 off. Bits 5: Digits 4
	}
}
void redLED(int status) {
	if (status == 1) {
		PTD->PSOR |= 0b00000000000000000000000001000000;
	}
	else {
		PTD->PCOR |= 0b00000000000000000000000001000000;
	}
}
void greenLED(int status) {
	if (status == 1) {
		PTD->PSOR |= 0b00000000000000000000000010000000;
	}
	else {
		PTD->PCOR |= 0b00000000000000000000000010000000;
	}
}
void blueLED(int status) {
	if (status == 1) {
		PTB->PSOR |= 0b00000000000010000000000000000000;
	}
	else {
		PTB->PCOR |= 0b00000000000010000000000000000000;
	}
}
void ready(void) {
	if (start == TRUE) {
		blueLED(0);
	    redLED(1);
	    delay(1);
	    redLED(0);
	    delay(1);
	    greenLED(1);
	    delay(1);
	    greenLED(0);
	    delay(1);
	    blueLED(1);
	    delay(1);
	    blueLED(0);
	    delay(1);
	    blueLED(1);
	    start = FALSE;
	}
}
void unlocked(void) {
	blueLED(0);
	delay(1);
	greenLED(1);
	delay(1);
	greenLED(0);
	delay(1);
	greenLED(1);
	delay(1);
	greenLED(0);
	delay(1);
	greenLED(1);
	delay(1);
	greenLED(0);
	delay(1);
	blueLED(1);
	attemptDelay = 1;
	attempt = 1;
}
void wrongPasswd(void) {
	blueLED(0);
	for (check = 0; check < attemptDelay; ++check) {
	    for (flash = 0; flash < 3; ++flash) {
	    	redLED(1);
	    	delay(1);
	    	redLED(0);
	    	delay(1);
	    }
	}
	blueLED(1);
	if (attempt < 3) {
		attempt++;
	}
	else {
	    attemptDelay *= 2;
	}
}
void resetReady(void) {
	if (resetStart == TRUE) {
		blueLED(0);
		delay(1);
		blueLED(1);
		delay(1);
		blueLED(0);
		delay(1);
		greenLED(1);
		delay(1);
		greenLED(0);
		delay(1);
		redLED(1);
		delay(1);
		redLED(0);
		delay(1);
		blueLED(1);
		resetStart = FALSE;
	}
}
void resetButton(void) {
	if (unlock == TRUE) {
		while(1) {

			resetReady();

			if (k > 3) {
				break;
			}
			while(1) {

				i = PTB->PDIR;
				j = PTC->PDIR;

				if (k > 3) {
					break;
				}
				else {
					if ((i & 0x00010000) != 0) {
						// re-enter button
						digitOff(1);
						digitOff(2);
						digitOff(3);
						digitOff(4);
						k = 0;
						for (check = 0; check < 4; ++check) {
							correct[check] = FALSE;
						}
						break;
					}
					if ((i & 0x00020000) != 0) {
						// reset password button
						break;
					}
					if ((j & 0x0040) != 0) {
						// first button
						if (k == 0) {
							digitOn(1);
							displayOne();
							passwd[0] = 1;
							k++;
						}
						else if (k == 1) {
							digitOn(2);
							displayOne();
							passwd[1] = 1;
							k++;
						}
						else if (k == 2) {
							digitOn(3);
							displayOne();
							passwd[2] = 1;
							k++;
						}
						else if (k == 3) {
							digitOn(4);
							displayOne();
							passwd[3] = 1;
							k++;
						}
						break;
					}
					if ((j & 0x0001) != 0) {
						// second button
						if (k == 0) {
							digitOn(1);
							displayTwo();
							passwd[0] = 2;
							k++;
						}
						else if (k == 1) {
							digitOn(2);
							displayTwo();
							passwd[1] = 2;
							k++;
						}
						else if (k == 2) {
							digitOn(3);
							displayTwo();
							passwd[2] = 2;
							k++;
						}
						else if (k == 3) {
							digitOn(4);
							displayTwo();
							passwd[3] = 2;
							k++;
						}
						break;
					}
					if ((j & 0x0010) != 0) {
						// third button
						if (k == 0) {
							digitOn(1);
							displayThree();
							passwd[0] = 3;
							k++;
						}
						else if (k == 1) {
							digitOn(2);
							displayThree();
							passwd[1] = 3;
							k++;
						}
						else if (k == 2) {
							digitOn(3);
							displayThree();
							passwd[2] = 3;
							k++;
						}
						else if (k == 3) {
							digitOn(4);
							displayThree();
							passwd[3] = 3;
							k++;
						}
						break;
					}
					if ((j & 0x0080) != 0) {
						// fourth button
						if (k == 0) {
							digitOn(1);
							displayFour();
							passwd[0] = 4;
							k++;
						}
						else if (k == 1) {
							digitOn(2);
							displayFour();
							passwd[1] = 4;
							k++;
						}
						else if (k == 2) {
							digitOn(3);
							displayFour();
							passwd[2] = 4;
							k++;
						}
						else if (k == 3) {
							digitOn(4);
							displayFour();
							passwd[3] = 4;
							k++;
						}
						break;
					}
					if ((i & 0x00040000) != 0) {
						// fifth button
						if (k == 0) {
							digitOn(1);
							displayFive();
							passwd[0] = 5;
							k++;
						}
						else if (k == 1) {
							digitOn(2);
							displayFive();
							passwd[1] = 5;
							k++;
						}
						else if (k == 2) {
							digitOn(3);
							displayFive();
							passwd[2] = 5;
							k++;
						}
						else if (k == 3) {
							digitOn(4);
							displayFive();
							passwd[3] = 5;
							k++;
						}
						break;
					}
				}
			}
			delay(2);
		}
	}
}
/*
 * @brief   Application entry point.
 */
int main(void) {
  	/* Init board hardware. */
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitBootPeripherals();
  	/* Init FSL debug console. */
    BOARD_InitDebugConsole();

    // turn on the clocks for ports A, D, and E
    SIM->SCGC5 |= SIM_SCGC5_PORTA_MASK;
    SIM->SCGC5 |= SIM_SCGC5_PORTB_MASK;
    SIM->SCGC5 |= SIM_SCGC5_PORTC_MASK;
    SIM->SCGC5 |= SIM_SCGC5_PORTD_MASK;
    SIM->SCGC5 |= SIM_SCGC5_PORTE_MASK;

    // clock for ADC
    SIM->SCGC6 |= SIM_SCGC6_ADC0_MASK;

    // software triggering for ADC
    ADC0->CFG1 = 0x40 | 0x10 | 0x04 | 0x00;
    ADC0->SC2 &= ~0x40;

    // Set up mux's for each pin to GPIO (ALT1) ... 0x100
    // Ports for the hex display
    PORTA->PCR[1] = 0x100;		// PortA1: Cathode Pin 14 on 7 segment ("A")
    PORTA->PCR[2] = 0x100;		// PortA2: Cathode Pin 16 on 7 segment ("B")
    PORTA->PCR[4] = 0x100;		// PortA4: Cathode Pin 4 on 7 segment ("D")
    PORTA->PCR[5] = 0x100;		// PortA5: Cathode Pin 5 on 7 segment ("E")
    PORTA->PCR[12] = 0x100;		// PortA12: Cathode Pin 13 on 7 segment ("C")
    PORTA->PCR[13] = 0x100;		// PortA13: Cathode Pin 11 on 7 segment ("F")
    PORTD->PCR[2] = 0x100;		// PortD2: Common Anode, Digit 1
    PORTD->PCR[3] = 0x100;		// PortD3: Common Anode, Digit 2
    PORTD->PCR[4] = 0x100;		// PortD4: Common Anode, Digit 3
    PORTD->PCR[5] = 0x100;		// PortD5: Common Anode, Digit 4
    PORTE->PCR[1] = 0x100;		// PortE1: Cathode Pin 15 on 7 segment ("G")

    // Ports for the LEDs
    PORTD->PCR[6] = 0x100;		// red LED
    PORTD->PCR[7] = 0x100;		// green LED
    PORTB->PCR[19] = 0x100;		// blue LED

    // Ports for the Ultrasonic Sensor
    PORTC->PCR[1] = 0x000;
    PORTC->PCR[2] = 0x000;

    // Input ports for input buttons, password reset button, and re-enter button
    PORTB->PCR[16] = 0x103;
    PORTB->PCR[17] = 0x103;
    PORTB->PCR[18] = 0x103;
    PORTC->PCR[7] = 0x103;
    PORTC->PCR[6] = 0x103;
    PORTC->PCR[4] = 0x103;
    PORTC->PCR[0] = 0x103;

    // Set various pins as output (Port Direction Registers)
    PTD->PDDR |= 0b00000000000000000000000011111100;		// bits 5,4,3,2: Digits 4,3,2,1
    PTA->PDDR |= 0b00000000000000000011000000110110;		// bits 13,12,5,4,3,2: Segments F,C,E,D,
    PTE->PDDR |= 0b00000000000000000000000000000010;		// bit 0: Segment G
    PTB->PDDR |= 0b00000000000010000000000000000000;

    // Input Ports (Buttons)
    PTB->PDDR &= ~(0b00000000000001110000000000000000);
    PTC->PDDR &= ~(0b00000000000000000000000011110001);

    // Light up LEDs. Start by setting all digit values to output zero.
    PTD->PCOR |= 0b00000000000000000000000011111100;		// bits 5,4,3,2: Digits 4,3,2,1
    PTA->PCOR |= 0b00000000000000000011000000110110;		// bits 13,12,5,4,3,2: Segments F,C,E,D,
    PTB->PCOR |= 0b00000000000010000000000000000000;		// bit 0: Segment G

    /* Enter an infinite loop, just incrementing a counter. */
    while(1) {
    	// read channel 8 of ADC0
    	ADC0->SC1[0] = 0x08;

    	ready();

    	while(1) {

    		i = PTB->PDIR;
    		j = PTC->PDIR;

    		if (k > 3) {
    			unlock = TRUE;
    			for (check = 0; check < 4; ++check) {
    				if (correct[check] == FALSE) {
    					unlock = FALSE;
    					break;
    				}
    			}
    			if (unlock == TRUE) {
    				unlocked();
    			}
    			else if (unlock == FALSE) {
    				wrongPasswd();
    			}
    			while(1) {

    				i = PTB->PDIR;
    				j = PTC->PDIR;

    				if ((i & 0x00010000) != 0) {
    					// re-enter button
    					digitOff(4);
    					k = 0;
    					for (check = 0; check < 4; ++check) {
    						correct[check] = FALSE;
    					}
    					greenLED(0);
    					redLED(0);
    					blueLED(0);
    					start = TRUE;
    				    break;
    				}
    				if ((i & 0x00020000) != 0) {
    					// reset password button
    					k = 0;
    					resetStart = TRUE;
    					digitOff(4);
    					resetButton();
    					start = TRUE;
    					k = 0;
    					digitOff(4);
    					for (check = 0; check < 4; ++check) {
    					    correct[check] = FALSE;
    					}
    				    break;
    				}
    				delay(1);
    			}
    			break;
    		}
    		else {
    			if ((i & 0x00010000) != 0) {
    				// re-enter button
    				digitOff(1);
    				digitOff(2);
    				digitOff(3);
    				digitOff(4);
    				k = 0;
    				for (check = 0; check < 4; ++check) {
    				    correct[check] = FALSE;
    				}
    				break;
    			}
    			if ((i & 0x00020000) != 0) {
    				// reset password button
    				break;
    			}
    			if ((j & 0x0040) != 0) {
    				// first button
    				if (k == 0) {
    					digitOn(1);
    					displayOne();
    					if (passwd[0] == 1) {
    						correct[0] = TRUE;
    					}
    					k++;
    				}
    				else if (k == 1) {
    					digitOn(2);
    					displayOne();
    					if (passwd[1] == 1) {
    						correct[1] = TRUE;
    					}
    					k++;
    				}
    				else if (k == 2) {
    					digitOn(3);
    					displayOne();
    					if (passwd[2] == 1) {
    						correct[2] = TRUE;
    					}
    					k++;
    				}
					else if (k == 3) {
						digitOn(4);
						displayOne();
						if (passwd[3] == 1) {
							correct[3] = TRUE;
						}
						k++;
					}
					break;
				}
				if ((j & 0x0001) != 0) {
					// second button
					if (k == 0) {
						digitOn(1);
						displayTwo();
						if (passwd[0] == 2) {
							correct[0] = TRUE;
						}
						k++;
					}
					else if (k == 1) {
						digitOn(2);
						displayTwo();
						if (passwd[1] == 2) {
							correct[1] = TRUE;
						}
						k++;
					}
					else if (k == 2) {
						digitOn(3);
						displayTwo();
						if (passwd[2] == 2) {
							correct[2] = TRUE;
						}
						k++;
					}
					else if (k == 3) {
						digitOn(4);
						displayTwo();
						if (passwd[3] == 2) {
							correct[3] = TRUE;
						}
						k++;
					}
					break;
				}
				if ((j & 0x0010) != 0) {
					// third button
					if (k == 0) {
						digitOn(1);
						displayThree();
						if (passwd[0] == 3) {
							correct[0] = TRUE;
						}
						k++;
					}
					else if (k == 1) {
						digitOn(2);
						displayThree();
						if (passwd[1] == 3) {
							correct[1] = TRUE;
						}
						k++;
					}
					else if (k == 2) {
						digitOn(3);
						displayThree();
						if (passwd[2] == 3) {
							correct[2] = TRUE;
						}
						k++;
					}
					else if (k == 3) {
						digitOn(4);
						displayThree();
						if (passwd[3] == 3) {
							correct[3] = TRUE;
						}
						k++;
					}
					break;
				}
				if ((j & 0x0080) != 0) {
					// fourth button
					if (k == 0) {
						digitOn(1);
						displayFour();
						if (passwd[0] == 4) {
							correct[0] = TRUE;
						}
						k++;
					}
					else if (k == 1) {
						digitOn(2);
						displayFour();
						if (passwd[1] == 4) {
							correct[1] = TRUE;
						}
						k++;
					}
					else if (k == 2) {
						digitOn(3);
						displayFour();
						if (passwd[2] == 4) {
							correct[2] = TRUE;
						}
						k++;
					}
					else if (k == 3) {
						digitOn(4);
						displayFour();
						if (passwd[3] == 4) {
							correct[3] = TRUE;
						}
						k++;
					}
					break;
				}
				if ((i & 0x00040000) != 0) {
					// fifth button
					if (k == 0) {
						digitOn(1);
						displayFive();
						if (passwd[0] == 5) {
							correct[0] = TRUE;
						}
						k++;
					}
					else if (k == 1) {
						digitOn(2);
						displayFive();
						if (passwd[1] == 5) {
							correct[1] = TRUE;
						}
						k++;
					}
					else if (k == 2) {
						digitOn(3);
						displayFive();
						if (passwd[2] == 5) {
							correct[2] = TRUE;
						}
						k++;
					}
					else if (k == 3) {
						digitOn(4);
						displayFive();
						if (passwd[3] == 5) {
							correct[3] = TRUE;
						}
						k++;
					}
					break;
				}
			}
    	}
    	delay(2);
    }
    return 0 ;
}
