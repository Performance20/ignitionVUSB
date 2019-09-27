/*
 * VUSB.c
 *
 * Created: 20.09.2019 19:45:32
 * Author : helge
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>

#include <util/delay.h>

#include "usbdrv.h"
#include "oddebug.h"        /* This is also an example for using debug macros */

#if CALIBRATE_OSCCAL
#include "osccal.c"
#endif

#include "tools.h"

#define USB_LED_OFF 0
#define USB_LED_ON  1
#define USB_DATA_OUT 2
#define USB_DATA_WRITE 3
#define USB_DATA_IN 4

#define LED_BUILTIN   B,1

static uchar replyBuf[50] = "Hello, hier ist Helge";
static uchar dataReceived = 0, dataLength = 0; // for USB_DATA_IN

// this gets called when custom control message is received
USB_PUBLIC uchar usbFunctionSetup(uchar data[8]) {
	usbRequest_t *rq = (void *)data; // cast data to correct type
	
	switch(rq->bRequest) { // custom command is in the bRequest field
		case USB_LED_ON:
			 HIGH_SET(LED_BUILTIN); // turn LED on
			 return 0;
		case USB_LED_OFF:
			LOW_SET(LED_BUILTIN);  // turn LED off
			return 0;
		case USB_DATA_OUT: // send data to PC
			usbMsgPtr = replyBuf;
			return sizeof(replyBuf);
		case USB_DATA_WRITE: // modify reply buffer
			replyBuf[7] = rq->wValue.bytes[0];
			replyBuf[8] = rq->wValue.bytes[1];
			replyBuf[9] = rq->wIndex.bytes[0];
			replyBuf[10] = rq->wIndex.bytes[1];
			return 0;
		case USB_DATA_IN: // receive data from PC
			dataLength  = (uchar)rq->wLength.word;
			dataReceived = 0;		
			if(dataLength  > sizeof(replyBuf)) // limit to buffer size
			dataLength  = sizeof(replyBuf);		
			return USB_NO_MSG; // usbFunctionWrite will be called now
	};

	return 0; // should not get here
}

// This gets called when data is sent from PC to the device
USB_PUBLIC uchar usbFunctionWrite(uchar *data, uchar len) {
	uchar i;
	
	for(i = 0; dataReceived < dataLength && i < len; i++, dataReceived++)
	replyBuf[dataReceived] = data[i];
	
	return (dataReceived == dataLength); // 1 if we received it all, 0 if not

}


int main(void)
{
 uchar i;
	 
	OUTPUT_SET(LED_BUILTIN);  //LED PB1 as output 	
 	wdt_enable(WDTO_1S); // enable 1s watchdog timer

 	usbInit();
 	
 	usbDeviceDisconnect(); // enforce re-enumeration
 	for(i = 0; i<125; i++) { // wait 500 ms
	 	wdt_reset(); // keep the watchdog happy
	 	_delay_ms(2);
 	}
 	usbDeviceConnect();
 	sei(); // Enable interrupts after re-enumeration

 	while(1==1) {
	 	wdt_reset(); // keep the watchdog happy
	 	usbPoll();
 	}
}

