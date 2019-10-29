#include <msp430.h> 

#if MSP430G2553
#include <msp430g2553.h>
#endif


#if MSP430f2274
#include <msp430f2274.h>
#endif

bool isRawBtDataValid();
void parseBtData();
void clearBtState();

typedef enum {start = 0, end = 1} _engineDirection;

// TODO: engine
void enginePowerOn(bool state);
void engineDirection(_engineDirection engineDirection);
void engineDriveToStart();						// <= use functions "enginePowerOn" "engineDirection" inside here
void engineDriveToBottle(int position);			// <= use functions "enginePowerOn" "engineDirection" inside here


// TODO: scale
void scaleTurnOn(bool state);
// void scaleMeasure(); // <= use maybe interrupt

static volatile bool _btDataReceived = false;
static volatile int _btParsedData[12];

int main(void) {
	WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT
	if (CALBC1_1MHZ==0xFF)					// If calibration constant erased
	{
		while(1);                               // do not load, trap CPU!!
	}
    DCOCTL = 0;                               // Select lowest DCOx and MODx settings
	BCSCTL1 = CALBC1_1MHZ;                    // Set DCO
    DCOCTL = CALDCO_1MHZ;
    P1SEL = BIT1 + BIT2 ;                     // P1.1 = RXD, P1.2=TXD
    P1SEL2 = BIT1 + BIT2 ;                    // P1.1 = RXD, P1.2=TXD
    UCA0CTL1 |= UCSSEL_2;                     // SMCLK
    UCA0BR0 = 104;                            // 1MHz 9600
    UCA0BR1 = 0;                              // 1MHz 9600
    UCA0MCTL = UCBRS0;                        // Modulation UCBRSx = 1
    UCA0CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
    IE2 |= UCA0RXIE;                          // Enable USCI_A0 RX interrupt

    __enable_interrupt();

    while(true){
    	// data received
    	if(_btDataReceived){
    		// 1.) disable bluetooth rx interrupt
    		// 2.) iterate over "_btParsedData"
    		//		=> every 1st index is drink position
    		//		=> every 2nd index is milliliter
    		// 3.) drive to first drink and disable engine
    		// 4.) enable scale interrupt
    		// 5.) add liquid to bottle and measure the liquid with the scale
    		// 6.) disable scale interrupt
    		// 7.) continue the iteration => drive to second bottle => add liquid => measure => continue till end
    		// 8.) at the end => enable bluetooth interrupt + call "clearBtState" and reset all states
    	}
    }

}


//////////////////////////////////////////////////// BLUETOOTH-START ///////////////////////////////////////////////////////////////////////////////////////////

// Test Data
// FF FF 01 00 32 FF FF

static volatile int _btRawData[23];
static int _stopBitCounter = 0;
static int _btInterruptCounter = 0;

#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void)
{
  while (!(IFG2&UCA0TXIFG));                // USCI_A0 TX buffer ready?
  int result = UCA0RXBUF;                   // TX -> RXed character

  _btRawData[_btInterruptCounter++] = result;

  if(result == 0xFF){
	  _stopBitCounter++;

	  if(_stopBitCounter == 4){				// finished => vildate + parse data
		  // 1.) validate raw data
		  if(!isRawBtDataValid()){			// <= breakpoint
			  clearBtState();
			  return;
		  }
		  // 2.) parse raw data
		  parseBtData();					// <= breakpoint
		  _btDataReceived = true;
		  return;
	  }
  }

}

bool isRawBtDataValid(){
	int stopBytesCounter = 0;
	int lastStopByteIndex = 0;

	// 1.) verify for start bytes
	if(_btRawData[0] != 0xFF && _btRawData[1] != 0xFF)
		return false;

	int i;
	for (i = 0; i <= sizeof(_btRawData)/sizeof(int); i++) {
		if(_btRawData[i] == 0xFF) stopBytesCounter++;
		if(stopBytesCounter == 4){
			lastStopByteIndex = i;
			break;
		}
	}

	// total count of 4 start/stop bytes are needed
	if(stopBytesCounter != 4)
		return false;

	// all other bytes after last stop byte have to be 0x00
	for (i = lastStopByteIndex + 1; i < sizeof(_btRawData)/sizeof(int); i++){
		if(_btRawData[i] != 0x00) return false;
	}

	// no more checks => everything is valid and raw data can be parsed
	return true;
}

void parseBtData(){

	int result = 0;

	int p = 1;
	int i;	// skip start bits
	for (i = 2; i <= sizeof(_btRawData)/sizeof(int); i+=3) {
		// parse first block
		if(i == 2) {
			_btParsedData[0] = _btRawData[2];
			result = _btRawData[3] + _btRawData[4];
			_btParsedData[1] = result;
			if(_btRawData[5] == 0xFF && _btRawData[6] == 0xFF) return;	// stop bytes => return
			p++;
		}
		else{	// parse other blocks

			_btParsedData[p] = _btRawData[i];
			result = _btRawData[i+1] + _btRawData[i+2];
			_btParsedData[++p] = result;
			if(i > 17 || _btRawData[p+1] == 0xFF && _btRawData[p+2] == 0xFF) return; // end reached or stop bytes => return
			p++;
		}
	}
}

void clearBtState(){
	_btDataReceived = false;
 	_stopBitCounter = 0;
 	_btInterruptCounter = 0;

	int i;
	for (i = 0; i < sizeof(_btRawData)/sizeof(int); i++) {
		_btRawData[i] = 0;
	}
	for (i = 0; i < sizeof(_btParsedData)/sizeof(int); i++) {
		_btParsedData[i] = 0;
	}
}


//////////////////////////////////////////////////// BLUETOOTH-END ///////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////// ENGINE-START ///////////////////////////////////////////////////////////////////////////////////////////


// TODO: Port-interrupt foreach bottle

void enginePowerOn(bool state){

}

void engineDirection(_engineDirection engineDirection){

}

void engineDriveToStart(){

}

void engineDriveToBottle(int position){

}


//////////////////////////////////////////////////// ENGINE-END ///////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////// SCALE-START ///////////////////////////////////////////////////////////////////////////////////////////


// TODO: Scale interrupt

void scaleTurnOn(bool state){

}


//////////////////////////////////////////////////// SCALE-END ///////////////////////////////////////////////////////////////////////////////////////////


