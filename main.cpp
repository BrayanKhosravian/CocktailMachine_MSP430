#include <msp430.h> 

#if MSP430G2553
#include <msp430g2553.h>
#endif


#if MSP430f2274
#include <msp430f2274.h>
#endif


#define position0 BIT0 // grundpos
#define position1 BIT1 // Port1
#define position2 BIT2
#define position3 BIT3
#define position4 BIT4
#define position5 BIT5
#define position6 BIT6

#define engineForward BIT0 //engine goes away from position0 // PORT4
#define engineBackward BIT1 //engine goes to position0		 // PORT 4

// Engine
enum class EngineStatus{Stop, Forward, Backward};
void engineControl(EngineStatus engineDirection);
int getPortBottlePosition();

// setup
void setup();


// bluetooth
bool isRawBtDataValid();
void parseBtData();
void clearBtState();

// TODO: scale
void scaleTurnOn(bool state);
// void scaleMeasure(); // <= use maybe interrupt

static volatile bool _btDataReceived = false;
static volatile int _btParsedData[12];
static volatile bool _engineFinished;

static volatile int _routine = 0;
volatile int _bottlePosition = 0;

// example
// static int test[5] = {4,500,6,250,2,233};

int main(void) {
    setup();

    while(true){
    	// data received
		if(_btDataReceived){
			for (_routine = 0; _routine <= sizeof(_btParsedData)/sizeof(int); ) {
				// TODO: NOTAUS

				// IMMER getr�nk position
				if(_routine % 2 == 0){
				   _bottlePosition = _btParsedData[_routine];
				   engineControl(EngineStatus::Forward);
				}
				// IMMER milliliter
				else if(_routine % 2 == 1){

				}

			}

			reset();



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


void setup(){

	WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT
	if (CALBC1_1MHZ==0xFF)					// If calibration constant erased
	{
		while(1);                               // do not load, trap CPU!!
	}
    DCOCTL = 0;                               // Select lowest DCOx and MODx settings
	// TODO: This configurations are used for the msp430g2553 and not for the msp430f2274 => add preprocessor condition + configure for f2274
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


    //p2.6 input oscillator, p2.7 oscillator out

    // TODO: check if port interrupt for bottle positon is configuret correctly
    P1DIR &= 0x00; // set Port 1 in Input direction
    P1IE  |= BIT0 + BIT1 + BIT2 + BIT3 + BIT4 + BIT5 + BIT6 + BIT7; // P1.1-P1.7 Interrupt enable // => 0x7F
    P1IFG |= 0x7F; // declares Port 1 Input (except P1.0) as Interrupt Flags
    P1IES |= 0x00; // P1.1-P1.6 low to high Edge

    P4DIR |= 0x03; // P4.0 and P4.1 output => used for platform (engine)

    // TODO: Hatzold waage
    P2DIR &= 0x03; //set P2.0-P2.1 in Input direction
    P2IE  |= 0x03; // P2.0-P2.1 Interrupt enable
    P2IFG |= 0x03; //declares P2.0-P2.1 as Interrupt Flags
    P2IES |= 0x00; //P2.0-P2.1 low to high Edge

#if MSP430F2274


#endif

    __enable_interrupt();
}

void reset(){
	clearBtState();

	// TODO: linearantrieb runterfahren

	_bottlePosition = 0;
	engineControl(EngineDirection::Backward);

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

	if(_btRawData[22] != 0x00) // last byte is only a dummy and it has to have alawys a value of "0x00"
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

//			pos = _btRawData[i];	// bottle index
//			_btParsedData[pos] = _btRawData[i+1] + _btRawData[i+2];

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

// TODO: Add delay for transitors
// mosfet delay = 1.16 us * 3 = 3.6 uS
// bipolar fet delay = 28nS * 3 = 90 ns
void engineControl(EngineStatus engineDirection){
	if(engineDirection == EngineStatus::Stop){
		P4OUT &= ~engineForward;
		P4OUT &= ~engineBackward;
	}
	else if(engineDirection == EngineStatus::Backward){
		P4OUT &= ~engineForward;
		P4OUT |= engineBackward;
	}
	else if(engineDirection == EngineStatus::Forward){
		P4OUT &= ~engineBackward;
		P4OUT |= engineForward;
	}

}


#pragma vector=PORT1_VECTOR
__interrupt  void Port_1(void)
{
	int result = getPortBottlePosition();

	if(result == -1){
		reset(); // Fahre zur grundposition
	}
	else if(result == _bottlePosition){
		engineControl(EngineStatus::Stop);
		_routine++;
	}
	else if(result > _bottlePosition) // engine driove too far
		reset();
	else if(result >= 6)
		engineControl(EngineStatus::Stop);	// Stop because position 6 is the last available position.

	P1IFG &= ~0x04;	// P1.2 IFG cleared

}

int getPortBottlePosition(){

	// 0000 0001 => grundposition => 0x01
	// 0000 0010 => pos 1 => 0x02
	// 0000 0100 => pos 2 => 0x04
	// 0000 1000 => pos 3 => 0x08
	// 0001 0000 => pos 4 => 0x10
	// 0010 0000 => pos 5 => 0x20
	// 0100 0000 => pos 6 => 0x40

	// _bottlePosition => 0, 1, 2 ,3, 4, 5, 6

	int port = P1IN;

	if(port == 0x01)
		return 0;
	else if(port == 0x02)
		return 1;
	else if(port == 0x04)
		return 2;
	else if(port == 0x08)
		return 3;
	else if(port == 0x10)
		return 4;
	else if(port == 0x08)
		return 3;
	else if(port == 0x10)
		return 4;
	else if(port == 0x20)
		return 5;
	else if(port == 0x40)
		return 6;
	else if(port == 0x00)
		return -2;	// nothing

	else return -1;  // error => fahre zu grund position

}

//////////////////////////////////////////////////// ENGINE-END ///////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////// SCALE-START ///////////////////////////////////////////////////////////////////////////////////////////


// TODO: Scale interrupt

void scaleTurnOn(bool state){

}


//////////////////////////////////////////////////// SCALE-END ///////////////////////////////////////////////////////////////////////////////////////////


