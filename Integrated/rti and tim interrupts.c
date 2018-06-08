// rti
// make prevgrpb and prevrdpb !!!!
// clear RTI interrupt flag
CRGFLG = CRGFLG | 0x80; 

if(prevgrpb == 1 && PORTAD0_PTAD1 == 0)
{
    greenpb = 1;
}
     
prevgrpb = PORTAD0_PTAD1;
     
if(prevrdpb == 1 && PORTAD0_PTAD2 == 0)
{
    redpb = 1;
}

prevrdpb = PORTAD0_PTAD2;

// tim (every 10ms)
onecnt++;

if(onecnt == 100)
{
	onecnt = 0;
	onesec = 1;
}

tencnt++;

if(tencnt == 1000)
{
	tencnt = 0;
	tensec = 1;
}