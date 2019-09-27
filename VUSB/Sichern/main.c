/*
 * VUSB.c
 *
 * Created: 20.09.2019 19:45:32
 * Author : helge
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <stdbool.h>
#include <util/delay.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>

#include "usbdrv.h"
//#include "oddebug.h"        /* This is also an example for using debug macros */
//#include "osccal.h"

#include "osccal.c"


#include "tools.h"

#define USB_LED_OFF 		0
#define USB_LED_ON  		(USB_LED_OFF + 1)
#define USB_LED_STATUS  	(USB_LED_ON + 1)
#define USB_DATA_OUT 		(USB_LED_STATUS + 1)
#define USB_DATA_WRITE 		(USB_DATA_OUT + 1)
#define USB_DATA_IN 		(USB_DATA_WRITE + 1)

#define LED_BUILTIN   B,1

#define MAX_BUFFER	20

static uchar rsBuffer[MAX_BUFFER] = "";
static uchar dataReceived = 0, dataLength = 0, dataSent = 0; // for USB_DATA_IN/_OUT
	   bool  LED_stateon = false;

// this gets called when custom control message is received
USB_PUBLIC uchar usbFunctionSetup(uchar data[8]) {
	usbRequest_t *rq = (void *)data; // cast data to correct type
    uchar cntData = 0;
	
	switch(rq->bRequest) { // custom command is in the bRequest field
		case USB_LED_ON:
			LED_stateon = true; 
			HIGH_SET(LED_BUILTIN); // turn LED on
			return 0;
		case USB_LED_OFF:
			LED_stateon = false;
			LOW_SET(LED_BUILTIN);  // turn LED off
			return 0;
		case USB_DATA_OUT: // send data to PC
			if (dataSent > 0) {
				cntData = dataSent;
				dataSent = 0;
				usbMsgPtr = rsBuffer;
				}
			return cntData;  
		case USB_DATA_WRITE: // modify reply buffer
			rsBuffer[0] = rq->wValue.bytes[0];
			rsBuffer[1] = rq->wValue.bytes[1];
			rsBuffer[2] = rq->wIndex.bytes[0];
			rsBuffer[3] = rq->wIndex.bytes[1];
			return 0;
		case USB_DATA_IN: // receive data from PC
			dataLength  = (uchar)rq->wLength.word;
			dataReceived = 0;		
			if(dataLength  > sizeof(rsBuffer)) // limit to buffer size
			dataLength  = sizeof(rsBuffer);		
			return USB_NO_MSG; // usbFunctionWrite will be called now
	};

	return 0; // should not get here
}

// This gets called when data is sent from PC to the device
USB_PUBLIC uchar usbFunctionWrite(uchar *data, uchar len) {
	uchar i;
	
	for(i = 0; dataReceived < dataLength && i < len; i++, dataReceived++)
	rsBuffer[dataReceived] = data[i];
	
	return (dataReceived == dataLength); // 1 if we received it all, 0 if not

}


uchar USBWriteStr(const char* data) {
  /*
   */
	uchar i; 
	uchar len; 

	len = strlen(data);
	if(len > MAX_BUFFER) 	// len is MAX_BUFFER chunk size 
     		len = MAX_BUFFER; // send an incomplete chunk 
     for(i = 0; i < len; i++) rsBuffer[i] = data[i]; // copy the data to the buffer
     dataSent = len; 
     return len; // return real chunk size } 
}


// wait a specified number of milliseconds (very roughly), refreshing in the background
void USBDelay(double milli) {
    unsigned int i;     
    for (i = 0; i<milli; i++) {
           _delay_ms (1);
	 	wdt_reset(); // keep the watchdog happy
		usbPoll();
  	}
}

inline void BlinkLED(void) {
   if (LED_stateon == true) {
	TOGGLE_SET(LED_BUILTIN);
	}
}


int main(void)
{
 uchar i;
	 
	OUTPUT_SET(LED_BUILTIN);  //LED PB1 as output 	
 	wdt_enable(WDTO_1S); // enable 1s watchdog timer
	//cli();
 	usbInit();
 	usbDeviceDisconnect(); // enforce re-enumeration, possible not needed
	i = 0;  
  	while(--i){           // fake USB disconnect for > 250 ms 
        _delay_ms(1);
	  wdt_reset(); 		// keep the watchdog happy 	
    	}
 	usbDeviceConnect();
 	sei(); // Enable interrupts after re-enumeration
      
 	for(;;){  
		USBWriteStr("Hello World!\n");
		BlinkLED();               
	 	USBDelay(1000);
 	}
}

