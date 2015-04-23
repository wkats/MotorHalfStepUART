#include <msp430.h>
unsigned int iStep;
const unsigned int steps[8]={0x01,0x03,0x02,0x06,0x04,0x0C,0x08,0x09};
const unsigned int vel[10]={1000,2000,3000,4000,5000,6000,7000,8000,9000,900};
void delay(void);
int main(void)
{
  WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT
  if (CALBC1_1MHZ==0xFF){					// If calibration constant erased
    while(1);                               // do not load, trap CPU!!
  }
	TACTL = TASSEL_2+MC_1+TAIE;	//Reloj interno, modo de cuenta a TACCR0, Interrupción
	TACCR0 = vel[0];			//Se pone la cuenta en 10,000 (decimal), frecuencia de 100Hz
	TACTL &= ~TAIFG;			//Se limpia el flag de interrupción de timer
	iStep=0;					//Se pone a a 0 el índice de pasos
	P2DIR = 0xFF;				//Todo el P2 Como Salida
	P2OUT = 0;					//Saco un 0 por el puerto 2
  DCOCTL = 0;                               // Select lowest DCOx and MODx settings
  BCSCTL1 = CALBC1_1MHZ;                    // Set DCO
  DCOCTL = CALDCO_1MHZ;
  P1SEL = BIT1 + BIT2 ;                     // P1.1 = RXD, P1.2=TX
  P1SEL2 = BIT1 + BIT2 ;                    // P1.1 = RXD, P1.2=TXD
  UCA0CTL1 |= UCSSEL_2;                     // SMCLK
  UCA0BR0 = 104;                            // 1MHz 9600
  UCA0BR1 = 0;                              // 1MHz 9600
  UCA0MCTL = UCBRS0;                        // Modulation UCBRSx = 1
  UCA0CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
  IE2 |= UCA0RXIE;                          // Enable USCI_A0 RX interrupt
  __bis_SR_register(GIE);       // interrupts enabled
}

void delay(void){
	volatile unsigned int i;//Delay
	for(i=0;i<8000;i++){;}
	return;
}		//Se cuenta hasta 8mil y se sale de la función

//  Echo back RXed character, confirm TX buffer is ready first
#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void)
{
  while (!(IFG2&UCA0TXIFG));                // USCI_A0 TX buffer ready?
  UCA0TXBUF = UCA0RXBUF;                    // TX -> RXed character
  switch(UCA0RXBUF){
  	  case'0':
  		  TACCR0=vel[0];
  		  break;
  	  case'1':
  		  TACCR0=vel[1];
  		  break;
  	  case'2':
  		  TACCR0=vel[2];
  		  break;
  	  case'3':
  		  TACCR0=vel[3];
  		  break;
  	  case'4':
  	  	  TACCR0=vel[4];
  	  	  break;
  	  case'5':
  		  TACCR0=vel[5];
  		  break;
  	  case'6':
  		  TACCR0=vel[6];
  		  break;
  	  case'7':
  		  TACCR0=vel[7];
  		  break;
  	  case'8':
  		  TACCR0=vel[8];
  		  break;
  	  case'9':
  	  	  TACCR0=vel[9];
  	  	  break;
  }
  delay();
}

#pragma vector=TIMER0_A1_VECTOR
__interrupt void timer_isr (void){	//ISR de timer
	iStep=(iStep<8)?iStep:0;		//Si el índice de pasos es menor a 4, no se altera, si no, se vuelve a 0
	P2OUT=steps[iStep++];			//Se saca el paso en el puerto 2
	TACTL &= ~ TAIFG;				//Se limpia el flag de interrupción de timer
}
