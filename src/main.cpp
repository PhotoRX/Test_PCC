#include <Arduino.h>
/*
Version 20240802

Communications control with STM chips:	STM32F401CC

Board versions:
Arduino					V1.8.19
STM32 MCU Based Boards	V2.7.1			Arduino core for STM32 MCUs (STMicroelectronics)
Preferences:			https://raw.githubusercontent.com/stm32duino/BoardManagerFiles/main/package_stmicroelectronics_index.json


********************************************************************************************************
********************************************************************************************************
Connections:					STM32F401CC and STM32F103Cx
Only BlackPill:
KEY pin (on board):				PA0					where the button (internal) is connected.

BluePill and BlackPill:
LED pin (on board):				PC13				where the blue LED (internal) is connected.
Oscilloscope pin:				PB5					oscilloscope output.
Oscilloscope pin:				PA1					oscilloscope synchronization.

BluePill and BlackPill:
USART_1 Tx Pin:					PA9					Serial, or Serial1 if USB is enabled as Serial.
USART_1 Rx Pin:					PA10				Serial, or Serial1 if USB is enabled as Serial.

BluePill and BlackPill:
USART_2 Tx Pin:					PA2					Serial1, or Serial2 if USB is enabled as Serial.
USART_2 Rx Pin:					PA3					Serial1, or Serial2 if USB is enabled as Serial.

Only BluePill:
USART_3 Tx Pin:					PB10				Serial2, or Serial3 if USB is enabled as Serial.
USART_3 Rx Pin:					PB11				Serial2, or Serial3 if USB is enabled as Serial.

Only BlackPill:
USART_6 Tx Pin:					PA11				Serial6; USB is disabled as Serial.
USART_6 Rx Pin:					PA12				Serial6; USB is disabled as Serial.

********************************************************************************************************
********************************************************************************************************
PC I57400, Board STM32F401CC (FLASH=262.144, RAM=65.536), V_Ard 1.8.19; Arduino core for STM32 MCUs (STMicroelectronics) V2.7.1:
	  Flash Usage	RAM Usage (Bytes)	Remaining
	 30.440 (11%)	    5.852	 (8%)	   59.684	V1.0	20240804	CtrRem  	(STMicroelectronics) V2.7.1 with USB, UART1 and UART2

PC I57400, Board STM32F401CC (FLASH=262.144, RAM=65.536), V_Ard 1.8.19; Arduino core for STM32 MCUs (STMicroelectronics) V2.7.1:
	  Flash Usage	RAM Usage (Bytes)	Remaining
	 29.116 (11%)	    6.116	 (9%)	   59.420	V1.0	20240804	CtrRem  	(STMicroelectronics) V2.7.1 without USB; with UART1, UART2 and UART6

PC I57400, Board STM32F103Cx (FLASH=131.072, RAM=20.480), V_Ard 1.8.19; Arduino core for STM32 MCUs (STMicroelectronics) V2.7.1:
	  Flash Usage	RAM Usage (Bytes)	Remaining
	 30.456 (23%)	    5.608	(27%)	   14.872	V1.0	20240804	CtrRem  	(STMicroelectronics) V2.7.1 with USB, UART1, UART2 and UART3

*/

// **********************************************
// **** Constant declaration ********************
// **********************************************
#define VERSION						"V1.0"

//**** Uncomment the line below, if using BluePill.****:
#define CON_BLUE_PILL								//comment if it is not an STM32F103Cx

#ifndef CON_BLUE_PILL								//it is not a BluePill board.
	//The USB and UART6 in the STM32F4x1 share the same pins, so they cannot work at the same time
	//**** Uncomment the line below, if using BlackPill with UART6 and without USB.****:
	#define SIN_USB_CON_U6						 	//uncomment if you use a BlackPill without USB with Uart1,Uart2 and Uart6. 
#endif

#ifndef SIN_USB_CON_U6								//using BlackPill without USB with Uart1,Uart2 and Uart6. 
	#define PR_USB					Serial			//communications with the trace; USB.
	#define PR_ESP32				Serial1			//communications with USB stick.
#endif	

//Pin definitions:
#define P_LED						PC13			//pin where the blue LED (internal) is connected.
#ifndef CON_BLUE_PILL								//using BlackPill (STM32F4x1); BluePill has not KEY in the board.
	#define P_KEY					PA0				//pin where the internal button is connected.
#endif
#define P_OSCI						PB5				//oscilloscope output pin.
#define P_SINC						PA1				//oscilloscope synchronization pin.

//Low-level pin actions:
#define INV_LED			digitalWrite(P_LED, !digitalRead(P_LED))
#define LEE_LED			(GPIOC->IDR & 0x2000)		//returns true, if PC13 is HIGH B0010000000000000
#define LED_OFF			GPIOC->BSRR = (1U << 13)	//Set the 13th bit of port C -> internal led on.
#define LED_ON			GPIOC->BSRR = (1U << 29)	//Clear the 13th bit of port C -> internal led off.
#define LEE_KEY			!(GPIOA->IDR & 0x0001) 		//returns true, if PA0 is HIGH B0000000000000001
#define OSC_BN_H		GPIOB->BSRR = (1U << 5)		//Set the 5th bit of port B gpio_write_bit(GPIOB, 5, true);
#define OSC_BN_L		GPIOB->BSRR = (1U << 21)	//Clear the 5th bit of port B
#define SIN_BN_H		GPIOA->BSRR = (1U << 1)		//Set the 1 bit of port A
#define SIN_BN_L		GPIOA->BSRR = (1U << 17)	//Clear the 1 bit of port A
#define LEE_SIN			(GPIOA->IDR & 0x0002)		//returns true, if PA1 is HIGH

//Miscellaneous declarations:
#define TIMEOUT						2000			//timeout of 2 seconds.
#define L_TX_S						256				//output text length.
#define L_TX_I						128				//intermediate text length.
#define L_TX_E						128				//length of input text.
#define L_TX_R						126				//trimmed input text length.

// **********************************************
// **** Global variable declaration *************
// **********************************************
//General variables:
char							_tEnt[L_TX_E];		//default input text.
char							_tInt[L_TX_I];		//intermediate default text.
char							_tSal[L_TX_S]; 		//output default text.

//Information that appears when you press the command I:
const char T_MENU0[] =
	"\r\nUSB and UARTs Control.\r\n" \
	"Version for STM32F103Cx and STM32F4x1.\r\n\r\nCommands:\r\n"\
	"*x\tsend the text that is behind the *x to device.";

const char T_MENU1[] =
	"\tx=device [0=ESP32; 1=PCC(left); 2=PCC(right)].\r\n" \
	"I\tthis information.\r\n" \
	"IT\ttechnical information.\r\n" \
	"T\trun the speed test (oscilloscope in pin PB5)\r\n\r\n";

#if defined (SIN_USB_CON_U6)						//using BlackPill without USB with Uart1,Uart2 and Uart6. 
	const char T_INF_TEC0[] =
		"\r\n********** \r\nConnections and communications:\r\n\r\n" \
		"Commands and TRACE, through the USART1 (PR_ESP32 and PR_USB)\r\n";
#else	
	const char T_INF_TEC0[] =
		"\r\n********** \r\nConnections and communications:\r\n\r\n" \
		"Commands and TRACE, through the USB (PR_USB)\r\n";
#endif

const char T_INF_TEC1[] =
	"Communication with ESP32 (PR_ESP32):\r\n" \
	"Tx USART_1\t->PA9\t\tconnect to the Rx of the ESP32\r\n" \
	"Rx USART_1\t->PA10\t\tconnect to the Tx of the ESP32\r\n";

const char T_INF_TEC2[] =
	"Communication with PCC(left) (PR_PCC_L):\r\n" \
	"Tx USART_2\t->PA2\t\tconnect to the Rx of the PCC(left)\r\n"\
	"Rx USART_2\t->PA3\t\tconnect to the Tx of the PCC(left)\r\n";

#if defined (CON_BLUE_PILL)
	const char T_INF_TEC3[] =
		"Communication with PCC(right) (PR_PCC_R):\r\n" \
		"Tx USART_3\t->PB10\t\tconnect to the Rx of the PCC(right)\r\n" \
		"Rx USART_3\t->PB11\t\tconnect to the Tx of the PCC(right)\r\n";
#else	
	#if defined (SIN_USB_CON_U6)
		const char T_INF_TEC3[] =
			"Communication with PCC(right) (PR_PCC_R):\r\n" \
			"Tx USART_6\t->PA11\t\tconnect to the Rx of the PCC(right)\r\n" \
			"Rx USART_6\t->PA12\t\tconnect to the Tx of the PCC(right)\r\n";
	#else
		const char T_INF_TEC3[] =
			"Communication with PCC(right) (PR_PCC_R):\r\n" \
			"There is no possibility to connect with PCC(right).\r\n" \
			"Only two USARTs are available.\r\n";
	#endif
#endif

const char T_INF_TEC4[] =
	"Oscilloscope\t->A1\t\t(P_SINCP) loop speed\r\n" \
	"Oscilloscope\t->PB5\t\t(P_OSCI) PIN change speed comparison\r\n\r\n";
/*


const char T_INF_TEC1[] =
	"Communication with ESP32 (PR_ESP32):\r\n" \
	"Communication (Tx USART_1) \t->PA9\t\tconnect to the Rx of the ESP32\r\n" \
	"Communication (Rx USART_1) \t->PA10\t\tconnect to the Tx of the ESP32";

const char T_INF_TEC2[] =
	"Communication with PCC(left) (PR_PCC_L):\r\n" \
	"Communication (Tx USART_2) \t->PA2\t\tconnect to the Rx of the PCC(left)\r\n"\
	"Communication (Rx USART_2) \t->PA3\t\tconnect to the Tx of the PCC(left)";

#if defined (CON_BLUE_PILL)
	const char T_INF_TEC3[] =
		"Communication with PCC(right) (PR_PCC_R):\r\n" \
		"Communication (Tx USART_3) \t->PB10\t\tconnect to the Rx of the PCC(right)\r\n" \
		"Communication (Rx USART_3) \t->PB11\t\tconnect to the Tx of the PCC(right)";
#else	
	#if defined (SIN_USB_CON_U6)
		const char T_INF_TEC3[] =
			"Communication with PCC(right) (PR_PCC_R):\r\n" \
			"Communication (Tx USART_6) \t->PA11\t\tconnect to the Rx of the PCC(right)\r\n" \
			"Communication (Rx USART_6) \t->PA12\t\tconnect to the Tx of the PCC(right)";
	#else
		const char T_INF_TEC3[] =
			"Communication with PCC(right) (PR_PCC_R):\r\n" \
			"There is no possibility to connect with PCC(right).\r\n" \
			"Only two USARTs are available.";
	#endif
#endif

const char T_INF_TEC4[] =
	"Oscilloscope\t\t\t->A1\t\t(P_SINCP) loop speed\r\n" \
	"Oscilloscope\t\t\t->PB5\t\t(P_OSCI) PIN change speed comparison\r\n\r\n";


*/


// **********************************************
// **** Function declaration ********************
// **********************************************

//Principal functions:
void	setup(void);
void 	loop(void);

//General functions:
bool	EjecutaComando(char*);
bool	EntradaUSB(char*);
void	Info(void);
void	InfoFW(char*);
void	InfoTec(void);
void	EjecutaTest(void);

// **********************************************
// **** Object declaration **********************
// **********************************************
//It is necessary to declare the two serials (2 and 3 or 2 and 6) here
//since the libraries that BLACKPILL and BluePill loads by default do not have them implemented:
#if defined (CON_BLUE_PILL)
	//USART_3 Tx Pin->PB10	USART_3 Rx Pin->PB11
	HardwareSerial PR_PCC_R(USART3);				//or 'HardwareSerial PR_PCC_R(PB11, PB10);
#else												//STM32F4x1->using BlackPill with two UARTs plus USB, or three UARTs without USB:
	#if defined (SIN_USB_CON_U6)					//using BlackPill without USB with Uart1, Uart2 and Uart6. 
		//USART_1 Tx Pin->PA9	USART_2 Rx Pin->PA10
		HardwareSerial PR_ESP32(USART1);			//or 'HardwareSerial PR_PCC_L(PA10, PA9);	
		#define PR_USB				PR_ESP32		//communications with the trace; with USB stick not USB.
		//USART_6 Tx Pin->PA11	USART_6 Rx Pin->PA12
		HardwareSerial PR_PCC_R(USART6);			//or 'HardwareSerial PR_PCC_R(PA12, PA11);
	#endif	
#endif
//USART_2 Tx Pin->PA2	USART_2 Rx Pin->PA3
HardwareSerial PR_PCC_L(USART2);					//or 'HardwareSerial PR_PCC_L(PA3, PA2);	


// **********************************************
// **** Library declaration *********************
// **********************************************
#include <stdint.h>

// **********************************************
// **** Principal functions *********************
// **********************************************
void setup() {
	//Configure pins:
	pinMode(P_LED, OUTPUT);							//PC13->pin where the green LED (internal) is connected.
#ifndef CON_BLUE_PILL								//using BlackPill (STM32F4x1); BluePill has not KEY in the board.
	pinMode(P_KEY, INPUT_PULLUP);					//PA0->pin where the internal button is connected.
#endif
	pinMode(P_OSCI, OUTPUT);						//B5->oscilloscope output pin.
	pinMode(P_SINC, OUTPUT);						//PA1->oscilloscope synchronization pin.
	
	//Initial state of the pins:
	LED_OFF;										//green led (internal) off.
	OSC_BN_L;										//B5->oscilloscope output pin.
	SIN_BN_L;										//PA1->oscilloscope synchronization pin.
	
	//Initialize communications:
	PR_ESP32.begin(9600);							//USART1(BluePill and BlackPill)->PR_ESP32 communications and TRACE if BlackPill without USB.
	PR_PCC_L.begin(9600);							//USART2(BluePill and BlackPill)->PCC(left) communications.

#if defined (CON_BLUE_PILL)							//using BluePill
	PR_USB.begin(115200);							//USB(BluePill)->communications with the trace.
	PR_PCC_R.begin(9600);							//USART3(BluePill)->PCC(right) communications.
#else												//using BlackPill
	#if defined (SIN_USB_CON_U6)					//using BlackPill without USB with Uart6. 
		PR_PCC_R.begin(9600);						//USART6(BlackPill)->PCC(right) communications.
	#else											//using BlackPill without three UARTs and with USB
		PR_USB.begin(115200);						//USB(BlackPill)->communications with the trace.
	#endif	
#endif
}

void loop() {
	if (LEE_SIN)	SIN_BN_L;	else	SIN_BN_H;	//inverts the state of the sync pin.

	//If USB data is available, process it:
	if (PR_USB.available()>0)		EntradaUSB(_tSal);
#ifndef SIN_USB_CON_U6								//using BluePill (STM32F103Cx) or BlackPill (STM32F4x1) whit USB and whitout USART6
	//If there is data in USART1, send it to USB:
	if (PR_ESP32.available()>0)		PR_USB.write(PR_ESP32.read());
#endif
	//If there is data in USART2, send it to USB:
	if (PR_PCC_L.available()>0)		PR_USB.write(PR_PCC_L.read());

#if defined (CON_BLUE_PILL) || defined (SIN_USB_CON_U6)		//using BluePill (STM32F103Cx) or BlackPill (STM32F4x1) whitout USB and whit USART6 
	//If there is data in USART3 (BluePill) or USART6 (BlackPill), send it to USB or USART1 respectively:
	if (PR_PCC_R.available()>0)		PR_USB.write(PR_PCC_R.read());
#endif

#ifndef CON_BLUE_PILL								//using BlackPill (STM32F4x1); BluePill has not KEY in the board.
	//Checks if the button has been pressed:
	if (LEE_KEY) {
		INV_LED;
		PR_USB.println(F("Push button pressed"));
		delay(100);
		while (LEE_KEY);
		delay(100);
	}
#endif
}

// **********************************************
// **** General functions ***********************
// **********************************************
bool EntradaUSB(char * tSal) {
	uint8_t b1 = 0;
	uint32_t ulTie = TIMEOUT + millis();			//reset the timeout.

	while (true) {
		if (PR_USB.available()>0) {
			ulTie = TIMEOUT + millis();				//reset the timeout.
			_tEnt[b1] = PR_USB.read();
			if (_tEnt[b1]=='\n') {
				_tEnt[b1] = (char)0;				//add the final text.
				EjecutaComando(_tEnt);
				return true;
			}
			if (_tEnt[b1]!='\r')	b1++;			//increment the counter if it is not '\r'.
			if (b1 >= L_TX_R)	break;				//output by text length.
		}
		if (ulTie < millis())	break;				//output by TimeOut.
	}
	_tEnt[b1+1] = (char)0;							//add the final text.
	sprintf_P(tSal, PSTR("Bad command(USB)\t[%s]"), _tEnt);
	return false;
}

bool EjecutaComando(char *tEnt) {
	uint16_t uResul;

	sprintf_P(_tSal, PSTR("Command received:\t[%s]"), tEnt);
	PR_USB.println(_tSal);
	switch (tEnt[0]) {
		case '*':
			switch (tEnt[1]) {
				case '1':					//send data to PCC left.
					PR_PCC_L.println(&tEnt[2]);
					return true;
				case '2':					//send data to PCC right.
#if defined (CON_BLUE_PILL) || defined (SIN_USB_CON_U6)	//BluePill (STM32F103Cx) or BlackPill (STM32F4x1) whitout USB
					PR_PCC_R.println(&tEnt[2]);
#else 
					PR_USB.println("Error, there is no output for the PCC_R, it is a BlackPill (STM32F4x1) with USB");
#endif
					return true;
				default:					//send data to ESP32.
					PR_ESP32.println(&tEnt[2]);
					return true;
			}
			return true;
		case 'i': case 'I':
			switch (tEnt[1]) {
				case 't': case 'T':					//technical information.
					InfoTec();
					return true;
				default:
					Info();
					return true;
			}
		case 't': case 'T':							//run the test.
			EjecutaTest();
			return true;
	}
	sprintf_P(_tSal, PSTR("Unknown command:\r\n%d[%s]"), strlen(tEnt), tEnt);
	PR_USB.println(_tSal);
	return false;
}

void Info() {
	PR_USB.println(T_MENU0);
	PR_USB.println(T_MENU1);
}

void InfoTec() {
	sprintf_P(_tSal, PSTR("\r\nVersion: %s\r\nTechnical information:"), VERSION);
	InfoFW(_tSal);
	PR_USB.println(_tSal);
	PR_USB.println(BOARD_NAME);
	sprintf_P(_tSal, PSTR("Clock cycles = %luMHz"), (F_CPU / 1000000));
	PR_USB.println(_tSal);
	PR_USB.println(T_INF_TEC0);
	PR_USB.println(T_INF_TEC1);
	PR_USB.println(T_INF_TEC2);
	PR_USB.println(T_INF_TEC3);
	PR_USB.println(T_INF_TEC4);
}

void InfoFW(char *mcTSal) {
	int b1, b2;

	sprintf_P(_tInt, PSTR("\r\nFW:\r\nF:%s"), __DATE__);
	strcat(mcTSal, _tInt);
	sprintf_P(_tInt, PSTR("\r\nV:%d_"), ARDUINO);
	strcat(mcTSal, _tInt);
	strcpy(_tInt, __FILE__);
	b1 = strlen(_tInt) - 4;
	_tInt[b1] = (char)0;
	//File name without ".ino" and without directory, where the Sketch is:
	b2 = 0;
	b1-= 2;
	if (b1<=0) {
		return;										//protection.
	}
	while (b1>0) {
		if (_tInt[b1]=='\\') {
			b2 = b1 + 1;
			break;
		}
		b1--;
	}
	strcat(mcTSal, &_tInt[b2]);
	strcat_P(mcTSal, PSTR("_"));
	strcat_P(mcTSal, PSTR(VERSION));
}

void EjecutaTest() {
	uint32_t uTieIni, uTieFin;
	
	PR_USB.println("Running the speed test (oscilloscope in pin PB5).");
	uTieIni = micros();
	OSC_BN_H;
	OSC_BN_L;
	OSC_BN_H;
	OSC_BN_L;
	OSC_BN_H;
	OSC_BN_L;
	OSC_BN_H;
	OSC_BN_L;
	delayMicroseconds(2);
	digitalWrite(P_OSCI, HIGH);
	digitalWrite(P_OSCI, LOW);
	digitalWrite(P_OSCI, HIGH);
	digitalWrite(P_OSCI, LOW);
	digitalWrite(P_OSCI, HIGH);
	digitalWrite(P_OSCI, LOW);
	digitalWrite(P_OSCI, HIGH);
	digitalWrite(P_OSCI, LOW);
	uTieFin = micros();	
	PR_USB.printf_P(PSTR("End test (%luµs)"), (uTieFin- uTieIni));
}