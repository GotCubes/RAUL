//// ece 362 mini-project peripheral initializations

// Initialize RTI // done
CRGINT = 0x80; // turn on rti
RTICTL = 0x27; // interrupts every 2.048ms

// Initialize TIM // done			 		  			 		  		
TSCR1 = 0x80; // enable TIM
TSCR2 = 0x0C; //reset counter on CH7 OC, prescaler = 16
TIOS = 0x80; // set CH7 for OC
TIE = 0x00; // initially disable CH7 interrupts
TC7 = 15000; // interrupt every 10ms (100 Hz)

// Initialize LCD // done     
PTT_PTT2 = 1; // pull LCDCLK high (idle)
PTT_PTT1 = 0; // pull R/W' low (write state)
send_i(LCDON); // turn on LCD (LCDON instruction)
send_i(TWOLINE); // enable two-line mode (TWOLINE instruction)
send_i(LCDCLR); // clear LCD (LCDCLR instruction)
lcdwait(); // wait for 2ms so that the LCD can wake up

// Initialize ATD // done
DDRAD = 0x00; // AN0-2 are inputs
ATDDIEN = 0x06; // AN1 and AN2 are digital
ATDCTL2 = 0x80; // turn on ATD
ATDCTL3 = 0x08; // conversion length = 1
ATDCTL4 = 0x85; // 8-bit, nominal sample time

// Initialize SPI // done
SPICR1 = 0x50; // no interrupts, on, master, active high clk, data sampling at odd edge, no slave select, data msb first
SPICR2 = 0x00; // not bi-directional
SPIBR = 0x01; // for baud rate of 6Mbs

// Initialize Port M // done
DDRM = 0x01; // PM0 is an output to the RESET button LED

// Initialize Port T // done
DDRT = 0xF7; // PT0-2 and PT4-7 are outputs

// Initialize PWM // need to find which frequencies to use, set prescaler and scaler
PWME = 0x08; // enable PWM3
PWMPOL = 0x00; // active low
PWMCTL = 0x00; // separate inputs
PWMCLK = 0x08; // use clk SB
PWMPRCLK = 0x30; // prescaler for B is 8
PWMCAE = 0x00; // left-aligned
PWMSCLB = 0x3B; // scaler = 59 x2, 99.7Hz
PWMPER3 = 0x00; // period = 0, off
PWMDTY3 = 0x00; // duty = 0, off
MODRR = 0x08; // PTT3 = PWM3

// Initialize Port E // done
DDRE = 0x03; // PE0 and PE1 are outputs to button LEDs