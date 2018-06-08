/***************************************************************************************************
ECE 362 - Mini-Project C Source File - Fall 2017
****************************************************************************************************	 	   			 		  			 		  		
Team ID: 32
Project Name: Remote Accumulation for University Locomotion
Team Members:
  -Team/Doc Leader:   Cameron Hudson  Signature: Cameron Hudson 
  -Software Leader:   Kyle Schmidt    Signature: Kyle Schmidt
  -Interface Leader:  Spencer Deak    Signature: Spencer Deak
  -Peripheral Leader: Megan Thieme    Signature: Megan Thieme
Academic Honesty Statement:
  In signing above, we hereby certify that we are the individuals who created this HC(S)12 source 
  file and that we have not copied the work of any other student (past or present) while completing 
  it. We understand that if we fail to honor this agreement, we will receive a grade of ZERO and be 
  subject to possible disciplinary action.
****************************************************************************************************
The objective of this Mini-Project is to design a box to be used as a capacity counter for a bus or
room that needs to keep track of the number of people inside.
****************************************************************************************************
List of project-specific success criteria (functionality that will be demonstrated):
 1. After the reset button is pushed, you should be able to set the time and capacity with the 
    potentiometer and green button 
 2. The 2nd line of the LCD should switch every 5 second between the correct time and until full
 3. The 1st line of the LCD should display the correct amount of people on the bus, with the LED bar
    Displaying the percentage of the capacity that is occupied, the second line should display the
    correct amount of people until full 
 4. 3 different sounds should be produced, a positive sound for when the green button is pushed and 
    the LED bar only has green LEDs lit, a warning sound for when the green button is pushed and the
    LED bar has both green and yellow LEDs lit, a negative sound for when the green button is 
    pressed and the LED bar has all LEDs lit
 5. The green LED button should be on as long as someone can enter, the red LED should be on as long
    as someone can leave, the reset LED should be on for 1 second after reset
****************************************************************************************************
Date code started: < 11/18/17 >
Update history (add an entry every time a significant change is made):
  Date: 11/18/17   Name: Kyle            Update: creation of skeleton file
  Date: 11/26/17   Name: Kyle            Update: wrote print drivers for LCD, created reset function, started main loop, created tdisp and pdisp
  Date: 12/ 1/17   Name: Kyle & Megan    Update: wrote more functions (ledbar, sounds) added in inializations
  Date: 12/ 2/17   Name: Kyle            Update: ATD code was added, everything but PWM seems complete
  Date: 12/ 3/17   Name: Kyle            Update: PWM code Finalized
***************************************************************************************************/
#include <hidef.h>       
#include "derivative.h"	 
#include <mc9s12c32.h>

// Function Declarations
void shiftout(char);
void lcdwait(void);
void wait(int x);
void send_byte(char);
void send_i(char);
void chgline(char);
void print_c(char);
void pmsglcd(char[]);
void start_code(void);
void tdisp(void);
void pdisp(void);
void fdisp(void);
void ledbar(void);
void sounds(void);
void people_check();
void green_blink();

// Globale Variable Declarations 	   			 		  			 		       
char greenpb  = 0; // green pushbutton flag
char prevgrpb = 0; // previous state for green pb
char redpb	  = 0; // red pushbutton flag
char prevrdpb = 0; // previous state for red pb
char runstp	  = 0; // run/stop flag
char onesec   = 0; // one second flag
char fivesec	= 0; // five second flag
int  pulscnt  = 0; // pulse count (read from PA every second)
int  onecnt   = 0; // count until one second
int  fivecnt  = 0; // count until five second
int  hr   = 0; // hour
int  min  = 0; // minute
int  sec  = 0; // seconds
int  hr1  = 0; // hour 1st digit
int  hr2  = 0; // hour 2nd digit
int  min1 = 0; // minute 1st digit
int  min2 = 0; // minute 2nd digit
int  sec1 = 0; // second 1st digit 
int  sec2 = 0; // second 2nd digit
int  cap1 = 0; // capacity 1st digit
int  cap2 = 0; // capacity 2nd digit
char ap = 'a'; // am or pm
int people    = 0; // count of people of bus
char disp      = 0; // switch between tdisp and fdisp;
int capacity  = 0; // current capacity of the bus

// LCD Communication Bit Masks
#define RS     0x10	// RS pin mask (PTT[0])
#define RW     0x20	// R/W pin mask (PTT[1])
#define LCDCLK 0x40	// LCD EN/CLK pin mask (PTT[2])

// LCD Instruction Characters
#define LCDON   0x0F // LCD initialization command
#define LCDCLR  0x01 // LCD clear display command
#define TWOLINE 0x38 // LCD 2-line enable command
#define CURMOV  0xFE // LCD cursor move instruction
#define LINE1   0x80 // LCD line 1 cursor position
#define LINE2   0xC0 // LCD line 2 cursor position

/***************************************************************************************************
Status: done
Function: initializations 
Inputs: void
Return: void
Description:
  initialize peripherals
***************************************************************************************************/
void  initializations(void) {
	// Set the PLL speed (bus clock = 24 MHz)
	CLKSEL = CLKSEL & 0x80; // disengage PLL from system
	PLLCTL = PLLCTL | 0x40; // turn on PLL
	SYNR = 0x02;            // set PLL multiplier
	REFDV = 0;              // set PLL divider
	while (!(CRGFLG & 0x08)){  }
	CLKSEL = CLKSEL | 0x80; // engage PLL

	// Disable watchdog timer (COPCTL register)
	COPCTL = 0x40; // COP off; RTI and COP stopped in BDM-mode

	// Initialize asynchronous serial port (SCI) for 9600 baud, interrupts off initially 
	SCIBDH =  0x00; //set baud rate to 9600
	SCIBDL =  0x9C; //24,000,000 / 16 / 156 = 9600 (approx)  
	SCICR1 =  0x00; //$9C = 156
	SCICR2 =  0x0C; //initialize SCI for program-driven operation
	DDRB   =  0x10; //set PB4 for output mode
	PORTB  =  0x10; //assert DTR pin on COM port
  	
	// Initialize TIM			 		  			 		  		
	TSCR1 = 0x80;  // enable TIM
	TSCR2 = 0x0C;  // reset counter on CH7 OC, prescaler = 16
	TIOS  = 0x80;  // set CH7 for OC
	TIE   = 0x00;  // initially disable CH7 interrupts
	TC7   = 15000; // interrupt every 10ms (100 Hz)
	
	// Initialize PWM // need to find which frequencies to use, set prescaler and scaler
	PWME     = 0x08; // enable PWM3
	PWMPOL   = 0x00; // active low
	PWMCTL   = 0x00; // separate inputs
	PWMCLK   = 0x08; // use clk SB
	PWMPRCLK = 0x70; // prescaler for B is 128
	PWMCAE   = 0x00; // left-aligned
	PWMSCLB  = 0x03; // scaler = 3 x2
	PWMPER3  = 0x00; // period = 0, off
	PWMDTY3  = 0x00; // duty = 0, off
	MODRR    = 0x08; // PTT3 = PWM3
		
	// Initialize ATD
	DDRAD   = 0xE0; // AN0-2 are inputs, AN7-AN5 is an output
	ATDDIEN = 0xE6; // AN1 and AN2 and AN7-AN5 are digital
	ATDCTL2 = 0x80; // turn on ATD
	ATDCTL3 = 0x08; // conversion length = 1
	ATDCTL4 = 0x85; // 8-bit, nominal sample time

  // Initialize RTI
	CRGINT = 0x80; // turn on rti
	RTICTL = 0x27; // interrupts every 2.048ms
	
	// Initialize SPI
	SPICR1 = 0x50; // no interrupts, on, master, active high clk, data sampling at odd edge, no slave select, data msb first
	SPICR2 = 0x00; // not bi-directional
	SPIBR  = 0x01; // for baud rate of 6Mbs

	// Initialize Port T
	DDRT = 0xF7; // PT0-2 and PT4-7 are outputs
	
  // Initialize LCD   
	PTT_PTT2 = 1;    // pull LCDCLK high (idle)
	PTT_PTT1 = 0;    // pull R/W' low (write state)
	send_i(LCDON);   // turn on LCD (LCDON instruction)
	send_i(TWOLINE); // enable two-line mode (TWOLINE instruction)
	send_i(LCDCLR);  // clear LCD (LCDCLR instruction)
	lcdwait();       // wait for 2ms so that the LCD can wake up
}

/***************************************************************************************************
Status: done
Function: main
Inputs: void
Return: void
Description: 
	main loop
***************************************************************************************************/
void main(void) {
  DisableInterrupts;
	initializations(); 		  			 		  		
  
  // enable interrupts
  asm {
    cli 
  }
  TIE = 0x80; // enable TIM interrupts

	start_code();	// code to run on reset section
	
	for(;;) {	  
		// if 5 seconds passes switch line 2
		if(fivesec == 1) {
			fivesec = 0;
			if(disp == 0) {
				disp = 1;
			} else {
				disp = 0;
			}
		}
  	if(disp == 0) {
  	  tdisp();  
  	} else {
  	  fdisp();
  	}	
		// if green button is pushed
		if(greenpb == 1) {
			greenpb = 0;
			if(people < capacity) {
			  people++;
		  }
			people_check();
			
			if(people == capacity) {
				PTAD_PTAD6 = 0;
				chgline(LINE2);
				pmsglcd("BUS FULL         ");
				sounds(); // sounds to play when button is pressed
				wait(250); 
			} else {
				PTAD_PTAD6 = 1;
				sounds(); // sounds to play when button is pressed  
			}
		}

		// if red button is pushed 
		if(redpb == 1) {
			redpb = 0;
			if(people > 0) {
			  people--;
			}
			people_check();
			if(people == 0) {
				PTAD_PTAD5 = 0;
				chgline(LINE2);
				pmsglcd("BUS EMPTY         ");
				wait(250); 
			} else {
				PTAD_PTAD5 = 1;	  
			}
		}
		
		if(people < capacity) {
		  PTAD_PTAD6 = 1;  
		}
		if(people > 0) {
		  PTAD_PTAD5 = 1;  
		}
		
		// if one second passes
		if(onesec == 1) {
			onesec = 0;
		  sec++;
		  if(sec == 60) {
			  sec = 0;
				min++;
		  }
		  if(min == 60) {
				min = 0;
				hr++;       
		  }
		  if(hr == 12 && min == 0 && sec == 0) {
				if(ap == 'a') {
			  		ap = 'p'; 
				} else {
			  		ap = 'a';  
				} 
		  }
			if(hr == 13) {
			  hr = 1;  
		  }		  
		}

		lcdwait();
	}
}
   
/***************************************************************************************************
Status: done
Function: start_code
Inputs: void
Return: void
Description:
	-code that happens on reset
***************************************************************************************************/
void start_code(void) {
	// flash the RESET LED for 1 second
	PTAD_PTAD7 = 1;
	wait(500);
	PTAD_PTAD7 = 0;
		
	send_i(LCDCLR);
	pmsglcd("Set Hours:       ");
	while(greenpb == 0) {
		green_blink();
		chgline(LINE2);
	  pmsglcd("Time: ");
		// use pot to set hr
		ATDCTL5 = 0x00; // left, unsigned, no scan, one ch, ch0
		while(ATDSTAT0_SCF == 0) { } // wait for sequence to finish
		if(ATDDR0H < 22) {
			hr = 1; 			
		} else if(ATDDR0H < 43) {
			hr = 2; 
		} else if(ATDDR0H < 64) {
			hr = 3; 
		} else if(ATDDR0H < 85) {
			hr = 4; 
		} else if(ATDDR0H < 106) {
			hr = 5; 
		} else if(ATDDR0H < 127) {
			hr = 6; 
		} else if(ATDDR0H < 148) {
			hr = 7; 
		} else if(ATDDR0H < 169) {
			hr = 8; 
		} else if(ATDDR0H < 190) {
			hr = 9; 
		} else if(ATDDR0H < 211) {
			hr = 10; 		
		} else if(ATDDR0H < 232) {
			hr = 11; 
		} else if(ATDDR0H < 256) {
			hr = 12; 
		}		
		
		// calculate and print
		hr1 = ((hr/10)%10)+0x30;
		hr2 = (hr%10)+0x30;
		print_c(hr1);
		print_c(hr2);
		print_c(':');
		print_c('0');
    print_c('0');
    print_c(' ');
    print_c('a');
    print_c('m');
	  pmsglcd("          ");
	  
	  lcdwait();	
	}
	greenpb = 0;
	PTAD_PTAD6 = 1;
	wait(250);
	PTAD_PTAD6 = 0;

  send_i(LCDCLR);
	pmsglcd("Set Min (tens):     ");
	while(greenpb == 0) {
		green_blink();
		chgline(LINE2);
		pmsglcd("Time: ");
		
		// use pot to set min (tens)
		ATDCTL5 = 0x00; // left, unsigned, no scan, one ch, ch0
		while(ATDSTAT0_SCF == 0) { } // wait for sequence to finish
		if(ATDDR0H < 43) {
			min1 = 0; 			
		} else if(ATDDR0H < 85) {
			min1 = 1; 
		} else if(ATDDR0H < 127) {
			min1 = 2; 
		} else if(ATDDR0H < 169) {
			min1 = 3; 
		} else if(ATDDR0H < 211) {
			min1 = 4; 		
		} else if(ATDDR0H < 256) {
			min1 = 5; 
		}		

		min = 10 * min1;
		
		// calculate and print
		min1 = min1 + 0x30;	
		
		hr1 = ((hr/10)%10)+0x30;
		hr2 = (hr%10)+0x30;
		print_c(hr1);
		print_c(hr2);
		print_c(':');
		print_c(min1);
    print_c('0');
    print_c(' ');
    print_c('a');
    print_c('m');
    pmsglcd("          ");
		
	  lcdwait();
	}
	greenpb = 0;
	PTAD_PTAD6 = 1;
	wait(250);
	PTAD_PTAD6 = 0;

  send_i(LCDCLR);
	pmsglcd("Set Min (ones):     ");
	while(greenpb == 0) {
		green_blink();
		chgline(LINE2);
		pmsglcd("Time: ");
		
		// use pot to set min (ones)
		ATDCTL5 = 0x00; // left, unsigned, no scan, one ch, ch0
		while(ATDSTAT0_SCF == 0) { } // wait for sequence to finish
		if(ATDDR0H < 26) {
			min2 = 0; 			
		} else if(ATDDR0H < 51) {
			min2 = 1; 
		} else if(ATDDR0H < 76) {
			min2 = 2; 
		} else if(ATDDR0H < 101) {
			min2 = 3; 
		} else if(ATDDR0H < 126) {
			min2 = 4; 		
		} else if(ATDDR0H < 151) {
			min2 = 5; 
		} else if(ATDDR0H < 176) {
			min2 = 6;		
		} else if(ATDDR0H < 201) {
			min2 = 7;
		} else if(ATDDR0H < 226) {
			min2 = 8;
		} else if(ATDDR0H < 256) {
			min2 = 9;	
		}

		// calculate and print
		min2 = min2 + 0x30;	
		
		hr1 = ((hr/10)%10)+0x30;
		hr2 = (hr%10)+0x30;
		print_c(hr1);
		print_c(hr2);
		print_c(':');
		print_c(min1);
    print_c(min2);
    print_c(' ');
    print_c('a');
    print_c('m');
    pmsglcd("          ");
		
	  lcdwait();
	}
	min = min + min2 - 0x30;
	greenpb = 0;
	PTAD_PTAD6 = 1;
	wait(250);
	PTAD_PTAD6 = 0;
	
	send_i(LCDCLR);
	pmsglcd("am or pm:        ");
	while(greenpb == 0) {
		green_blink();
		chgline(LINE2);
		pmsglcd("Time: ");
		
		// use pot to set am/pm
		ATDCTL5 = 0x00; // left, unsigned, no scan, one ch, ch0
		while(ATDSTAT0_SCF == 0) { } // wait for sequence to finish
		if(ATDDR0H < 128) {
			ap = 'a'; 			
		} else if(ATDDR0H < 256) {
			ap = 'p'; 
		}
		
		// print	
	  hr1 = ((hr/10)%10)+0x30;
		hr2 = (hr%10)+0x30;
		print_c(hr1);
		print_c(hr2);
		print_c(':');
		print_c(min1);
    print_c(min2);
    print_c(' ');
    print_c(ap);
    print_c('m');
    pmsglcd("          ");
	  
	  lcdwait();
	}
	greenpb = 0;
	PTAD_PTAD6 = 1;
	wait(250);
	PTAD_PTAD6 = 0;

	send_i(LCDCLR);
	pmsglcd("Set Capacity:    ");
	while(greenpb == 0) {
		green_blink();
		chgline(LINE2);
		pmsglcd("  ");
		
		// use pot to set capacity
		ATDCTL5 = 0x00; // left, unsigned, no scan, one ch, ch0
		while(ATDSTAT0_SCF == 0) { } // wait for sequence to finish
		if(ATDDR0H < 64) {
			capacity = 10; 			
		} else if(ATDDR0H < 127) {
			capacity = 40; 
		} else if(ATDDR0H < 190) {
			capacity = 60; 
		} else if(ATDDR0H < 256) {
			capacity = 80;
		}
				
		// calculate and print
		cap1 = ((capacity/10)%10) + 0x30;	
		cap2 = (capacity % 10) + 0x30;
				
		print_c(cap1);
		print_c(cap2);
		pmsglcd("                 ");
		
	  lcdwait();
	}
	greenpb = 0;
	PTAD_PTAD6 = 1;
  wait(250);
	PTAD_PTAD6 = 0;
	
	send_i(LCDCLR);
  pdisp();	
	fdisp();
}

/***************************************************************************************************
Status: done
Function: pdisp
Inputs: void
Return: void
Description:
	-lcd print screen for people 
***************************************************************************************************/
void pdisp(void) {
	int people1;
	int people2;
	
	chgline(LINE1);
	pmsglcd("People: ");
	people1 = ((people/10)%10)+0x30;
	people2 = (people%10)+0x30;
	if(people1 != 0x30) {
		print_c(people1);
	} 
	print_c(people2);
	pmsglcd("           ");
}

/***************************************************************************************************
Status: done
Function: tdisp
Inputs: void
Return: void
Description:
	-lcd print screen for time		
***************************************************************************************************/
void tdisp(void) {
	hr1 = ((hr/10)%10) + 0x30;
	
	chgline(LINE2);
	pmsglcd("Time: ");
	if(hr1 != 0) {
		print_c(hr1);
	} else {
		print_c(' ');
	}
	print_c(((hr)%10)+0x30);
	print_c(':');
	print_c(((min/10)%10)+0x30);
	print_c(((min)%10)+0x30);
	print_c(' ');
	print_c(ap);
	print_c('m');
	pmsglcd("         ");
}

/***************************************************************************************************
Status: done
Function: fdisp
Inputs: void
Return: void
Description:
	-lcd print screen for till full 
***************************************************************************************************/
void fdisp(void) {
	int full;
	int full1;
	int full2;
	
	full = 0;
	if(people < capacity) {	
		full = capacity - people;
	}
	chgline(LINE2);
	full1 = ((full/10)%10)+0x30;
	full2 = (full%10)+0x30;
	
  if(people == capacity) {
    pmsglcd("BUS FULL            ");  
  } else if(people == 0) {
    pmsglcd("BUS EMPTY            ");
  } else if(people < capacity) {
  pmsglcd("Until Full: ");
  if(full1 != 0x30) {	
    print_c(full1);
  } 
  	print_c(full2);
  	pmsglcd("          ");
	} else if(people == capacity) {
	  pmsglcd("BUS FULL            ");  
	} else if(people == 0) {
	  pmsglcd("BUS EMPTY            ");  
	}
}

/***************************************************************************************************
Status: done
Function: people_check
Inputs: void
Return: void
Description:
  -algorithm for what to do when buttons are pushed
***************************************************************************************************/
void people_check(void) {
	pdisp();
	ledbar();
	if(disp == 0) {
	  tdisp();  
	} else {
	  fdisp();
	}	
}

/***************************************************************************************************
Status: done
Function: green_blink
Inputs: void
Return: void
Description:
  -blink the green button LED
***************************************************************************************************/
void green_blink(void) {
  PTAD_PTAD6 = 1;
  wait(75);
  PTAD_PTAD6 = 0;
  wait(75);    
}

/***************************************************************************************************
Status: done
Function: ledbar
Inputs: void
Return: void
Description:
  -calculate which leds need to be turned on in the LED bar
***************************************************************************************************/
void ledbar(void) {
	if(people < (capacity/10)){	
		// less than 10%: turn all LEDs off
		PTT_PTT4 = 0;
		PTT_PTT5 = 0;
		PTT_PTT6 = 0;
		PTT_PTT7 = 0;
	} else if(people < (capacity*2/10)) {
		// less than 20%: 1G
		PTT_PTT4 = 1;
		PTT_PTT5 = 0;
		PTT_PTT6 = 0;
		PTT_PTT7 = 0;
	} else if(people < (capacity*3/10)) {
		// less than 30%: 2G
		PTT_PTT4 = 0;
		PTT_PTT5 = 1;
		PTT_PTT6 = 0;
		PTT_PTT7 = 0;
	} else if(people < (capacity*4/10)) {
		// less than 40%: 3G
		PTT_PTT4 = 1;
		PTT_PTT5 = 1;
		PTT_PTT6 = 0;
		PTT_PTT7 = 0;
	} else if(people < (capacity*5/10)) {
		// less than 50%: 4G
		PTT_PTT4 = 0;
		PTT_PTT5 = 0;
		PTT_PTT6 = 1;
		PTT_PTT7 = 0;
	} else if(people < (capacity*6/10)) {
		// less than 60%: 5G
		PTT_PTT4 = 1;
		PTT_PTT5 = 0;
		PTT_PTT6 = 1;
		PTT_PTT7 = 0;
	} else if(people < (capacity*7/10)) {
		// less than 70%: 6G
		PTT_PTT4 = 0;
		PTT_PTT5 = 1;
		PTT_PTT6 = 1;
		PTT_PTT7 = 0;
	} else if(people < (capacity*8/10)) {
		// less than 80%: 6G 1Y
		PTT_PTT4 = 1;
		PTT_PTT5 = 1;
		PTT_PTT6 = 1;
		PTT_PTT7 = 0;
	} else if(people < (capacity*9/10)) {
		// less than 90%: 6G 2Y
		PTT_PTT4 = 0;
		PTT_PTT5 = 0;
		PTT_PTT6 = 0;
		PTT_PTT7 = 1;
	} else if(people < capacity) {
		// less than 100%: 6G 3Y
		PTT_PTT4 = 1;
		PTT_PTT5 = 0;
		PTT_PTT6 = 0;
		PTT_PTT7 = 1;
	} else if(people == capacity) {
		// Equal to 100%: 6G 3Y 1R
		PTT_PTT4 = 0;
		PTT_PTT5 = 1;
		PTT_PTT6 = 0;
		PTT_PTT7 = 1;
	}		
}

/***************************************************************************************************
Status: done
Function: sounds
Inputs: void
Return: void
Description:
	-plays sounds depending on % of people
	-Happy Sound
		-When green button is pushed and 0-69%	
	-Warning Sound
		-When green button is pushed and 70-99%
	-Sad Sound
		-When green button is pushed and 100%
***************************************************************************************************/
void sounds(void) {
	if(people < (capacity*7/10)) {
		// less than 70%: happy sound 
		// first signal (lower than second signal)
		PWMPER3 = 80;
		PWMDTY3 = 40;
		wait(100);
		// second signal (higher than first signal) (higher than the first signal of the sad sound)
    PWMPER3 = 60;
    PWMDTY3 = 30;		
	  wait(100);
		PWMPER3 = 0;
	} else if(people < capacity) {
		// less than 100%: warning sound
		// first signal (same as the second signal) (low sound)
		PWMPER3 = 142;
		PWMDTY3 = 71;
		wait(100);
		// short pause ?
		PWMPER3 = 0;
		wait(10);	
		// second signal (same as the first signal) (low sound)
		PWMPER3 = 142;
		wait(100);
		PWMPER3 = 0;	
	} else if(people == capacity) {
		// equal to 100%: sad sound
		// first signal (higher than the second signal) (lower than the second signal of happy sound)
		PWMPER3 = 190;
    PWMDTY3 = 95;
	  wait(100);	
		// second signal (lower than the first signal)
		PWMPER3 = 239;
    PWMDTY3 = 120;
	  wait(100);	
		PWMPER3 = 0;
	}	
}

/***************************************************************************************************
Status: done
Function: RTI_ISR
Inputs: void
Return: void
Description:
  -RTI interrupt service routine: RTI_ISR
  -Initialized for 8.192 ms interrupt rate
  -Samples state of pushbuttons (PAD1 = green, PAD2 = red)
  -If change in state from "high" to "low" detected, set pushbutton flag
    -greenpb (for PAD1 H -> L), redpb (for PAD2 H -> L)
    -Recall that pushbuttons are momentary contact closures to ground
***************************************************************************************************/
interrupt 7 void RTI_ISR(void) {
	// clear RTI interrupt flag 
	CRGFLG = CRGFLG | 0x80; 
	if(prevgrpb == 1 && PORTAD0_PTAD1 == 0) {
		greenpb = 1;
	}
	prevgrpb = PORTAD0_PTAD1;
     
	if(prevrdpb == 1 && PORTAD0_PTAD2 == 0) {
		redpb = 1;	
	}
	prevrdpb = PORTAD0_PTAD2;
}

/***************************************************************************************************
Status: done
Function: TIM_ISR
Inputs: void
Return: void
Description:
  -TIM interrupt service routine
  -Initialized for 10.0 ms interrupt rate
  -Uses variable "fivecnt" to track if five seconds have accumulated and sets "fivesec" flag    ?????  ?
  -Uses variable "onecnt" to track if one second has accumulated and sets "onesec" flag		 		  			 		  		
***************************************************************************************************/
interrupt 15 void TIM_ISR(void) {
  // clear TIM CH 7 interrupt flag 
  TFLG1 = TFLG1 | 0x80;

	onecnt++;
	if(onecnt == 100) {
		onecnt = 0;
		onesec = 1;
	}

	fivecnt++;
	if(fivecnt == 500) {
		fivecnt = 0;
		fivesec = 1;
	}	
}

/***************************************************************************************************
Status: done
Function: wait
Inputs: x, 2ms*x=delay
Return: void
Description:
  Delay for 2 ms * a number			 		  		
***************************************************************************************************/
void wait(int x) {
  int i;
  for(i = 0; i < x; i++) {
    lcdwait();  
  }
}

/***************************************************************************************************
Status: done
Function: shiftout
Inputs: char x (character that you want to shift out)
Return: void
Description:
  Transmits the character x to external shift 
  register using the SPI.  It should shift MSB first.  
  MOSI = PM[4]
  SCK  = PM[5]  			 		  		
***************************************************************************************************/
void shiftout(char x) {
	int i;
	while(SPISR_SPTEF == 0) {}	// test the SPTEF bit: wait if 0, else continue
	if(SPISR_SPTEF == 1) {
		SPIDR = x;	// write data to SPI data register
	}
	for(i = 0; i < 4; i++) {}	// wait 30 cycles
}

/***************************************************************************************************
Status: done
Function: lcdwait
Inputs: void
Return: void
Description:
  Delay for approx 2 ms			 		  		
***************************************************************************************************/
void lcdwait(void) {
	int i = 0;
	for(i = 0; i < 8000; i++){}	// wait 48,000 cycles 
}

/***************************************************************************************************
Status: done
Function: send_byte
Inputs: char x (character to be written to LCD)
Return: void
Description:
  sends data to the LCD			 		  		
***************************************************************************************************/
void send_byte(char x) {
	shiftout(x);
	// pulse the LCD clock line (low->high->low)
	PTT_PTT2 = 0;
	PTT_PTT2 = 1;
	PTT_PTT2 = 0;
	lcdwait();
}

/***************************************************************************************************
Status: done
Function: send_i
Inputs: char x (instruction to be performed)
Return: void
Description:
  Sends instruction byte x to LCD 			 		  		
***************************************************************************************************/
void send_i(char x) {
	PTT_PTT0 = 0;	// set register select to instruction mode
	send_byte(x);
}

/***************************************************************************************************
Status: done
Function: chgline
Inputs: char x (line to move to)
Return: void
Description:
  Move LCD cursor to position x
  NOTE: Cursor positions are encoded in the LINE1/LINE2 variables 		  		
***************************************************************************************************/
void chgline(char x) {
	send_i(CURMOV);
	send_i(x);
}

/***************************************************************************************************
Status: done
Function: print_c
Inputs: char x (character to be printed on LCD)
Return: void
Description:
  Print (single) character x on LCD   		  		
***************************************************************************************************/
void print_c(char x) {
	PTT_PTT0 = 1;	// set register select to data mode
	send_byte(x);
}

/***************************************************************************************************
Status: done
Function: pmsglcd
Inputs: char str[] (string to be printed on LCD)
Return: void
Description:
  print character string str[] on LCD  		  		
***************************************************************************************************/
void pmsglcd(char str[]) {
	int i;
	i = 0;

	while(str[i] != 0) {
		print_c(str[i]);
		i++;
	}
}
