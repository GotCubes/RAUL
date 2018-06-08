// ATD conversion
ATDCTL5 = 0x00; // left, unsigned, no scan, one ch, ch0
while(ATDSTAT0_SCF == 0) { } // wait for sequence to finish

// ATDDR0H has input