/*Controlador de Motor a Pasos Unipolar, modelo 28BYJ-48.
 * 5 Hilos, con código de colores. Conexión con el MSP430G2553
 * a través del puerto 2. Pines:
 * P2.0 -> Azul
 * P2.1 -> Rosa
 * P2.2 -> Amarillo
 * P2.3 -> Naranja
 * Rojo a 5V o 3.3 de la placa Launchpad
 * Conexión UART  9600 baudios, 8bits,sin paridad,1 stop bit
 * */

#include <msp430.h>
unsigned int iStep=0;	//Índice de paso
unsigned char _cR='\0';	//Caracter leído
unsigned char stepsH[8]={0x01,0x03,0x02,0x06,0x04,0x0c,0x08,0x09};	// Pasos sentido horario
unsigned char stepsA[8]={0x09,0x08,0x0c,0x04,0x06,0x02,0x03,0x01}; // Pasos sentido antihorario
//unsigned char stepsH[4]={0x03,0x06,0x0c,0x09};	// Pasos sentido horario
//unsigned char stepsA[4]={0x09,0x0c,0x06,0x03}; // Pasos sentido antihorario
unsigned char *steps;	//Apuntador al arreglo de pasos deseado
//const unsigned int vel[10]={1000,2000,3000,4000,5000,6000,7000,8000,9000,900}; //10 Velocidades
const unsigned int vel[4]={1000,1428,2500,10000}; //10 Velocidades
char uart_getc();	//	Obtener un caracter
void ejecutarComando(unsigned char comando[]);

int main(void)
{
  WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT
  if (CALBC1_1MHZ==0xFF){					// If calibration constant erased
    while(1);                               // do not load, trap CPU!!
  }
  unsigned char comando[2]="\0\0";		//Primer Caracter:Sentido de giro; Segundo Caracter: velocidad; Ejemplo: H2
  int recibido=0;		//Cuántos chars han sido recibidos
  TACTL = TASSEL_2+MC_1;	//Reloj interno, modo de cuenta a TACCR0, Interrupción
  P2DIR = 0xFF;			//Todo el P2 Como Salida
  P2OUT = 0;			//Saco un 0 por el puerto 2
  DCOCTL = 0;                               // Select lowest DCOx and MODx settings
  BCSCTL1 = CALBC1_1MHZ;                    // Set DCO
  DCOCTL = CALDCO_1MHZ;						// Set DCO
  P1SEL = BIT1 + BIT2 ;                     // P1.1 = RXD, P1.2=TX
  P1SEL2 = BIT1 + BIT2 ;                    // P1.1 = RXD, P1.2=TXD
  UCA0CTL1 |= UCSSEL_2;                     // SMCLK
  UCA0BR0 = 104;                             // 1MHz 9600
  UCA0BR1 = 0;                              // 1MHz 9600
  UCA0MCTL = UCBRS0;                        // Modulation UCBRSx = 1
  UCA0CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
  __bis_SR_register(GIE);      		 		// interrupts enabled
  while(1){
	  if(recibido<2){
	  comando[recibido++]=uart_getc();
	  }
	  else{
		  recibido=0;
		  ejecutarComando(comando);
	  }
  }
}

char uart_getc(void){
	_cR='\0';
	IE2|=UCA0RXIE;
	while(_cR=='\0');
	return _cR;

}

void ejecutarComando(unsigned char _comando[]){
	switch(_comando[0]){//Comando para el sentido del motor
	case 'A':	//Sentido Antihorario
		steps=stepsA;
		TACTL|=TAIE;		//Encendemos la interrupción de timer, limpiamos el TA0R
		break;
	case 'H':	//Sentido Horario
		steps=stepsH;
		TACTL|=TAIE;		//Encendemos la interrupción de timer, limpiamos el TA0R
		break;
	case 'S':	//Stop, detener el motor
		TACTL&=~TAIE;		//Limpiamos la interrupción del timer
		TACTL&=~TAIFG;	//Se limpia el flag de interrupción de timer
		iStep=0;
		break;
	}
	if(_comando[0]!='S'){
		if(_comando[1]>47 && _comando[1]<52){	//Si el char corresponde a un número
			TACCR0=vel[_comando[1]-48];		//Le damos a TACCR0 la velocidad de ese número en el arreglo
		}
	}
}


#pragma vector=USCIAB0RX_VECTOR
__interrupt void recep_isr(void){
	_cR=UCA0RXBUF;				// Echo back RXed character, confirm TX buffer is ready first
	while (!(IFG2&UCA0TXIFG));	// USCI_A0 TX buffer ready?
	  UCA0TXBUF = _cR;			// TX -> RXed character
	IE2&=~UCA0RXIE;
	IFG2&=~UCA0RXIFG;
}

#pragma vector=TIMER0_A1_VECTOR
__interrupt void timer_isr (void){	//ISR de timer
	iStep=(iStep<8)?iStep:0;		//Si el índice de pasos es menor a 8, no se altera, si no, se vuelve a 0
	P2OUT=steps[iStep++];			//Se saca el paso en el puerto 2
	TACTL &= ~ TAIFG;				//Se limpia el flag de interrupción de timer
}
