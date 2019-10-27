#include <msp430.h> 
//#include "vector.h"
#include "Common/List.h"

List bluetoothDataRaw;
List bluetoothDataParsed;

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

/*
    int i;
    int counter = 0;
    while(true){
    	bluetoothDataRaw.init();
    	for (i = 0; i < 10; ++i) {
    		bluetoothDataRaw.append(i);
		}

    	bluetoothDataRaw.clear();
    	i=0;
    	counter++;
    }
*/
	return 0;
}

// Test Data
// FF FF 01 00 32 FF FF


static int conditionBitCounter = 0;
//  Echo back RXed character, confirm TX buffer is ready first
#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void)
{
  while (!(IFG2&UCA0TXIFG));                // USCI_A0 TX buffer ready?
  int result = UCA0RXBUF;                   // TX -> RXed character

  // count start and stop bits
  if(result == 0xFF){
	  conditionBitCounter++;

	  // Stop bits received => reset counter + parse bt-data + enable other interrupts
	  if(conditionBitCounter == 4)
	  {
	    conditionBitCounter = 0;

	    int i;
	    for (i = 0; i < bluetoothDataRaw.getSize(); i++) {

		}

	    bluetoothDataRaw.clear();
	  }

	  return;
  }

  // Start bits received => collect data
  if(conditionBitCounter == 2 ){
	  bluetoothDataRaw.append(result);
  }




}
