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
#include "oddebug.h"        /* This is also an example for using debug macros */
#include "osccal.h"
#include "protocol.h"

//#include "osccal.c"


#include "tools.h"

#define USB_LED_OFF 		0
#define USB_LED_ON  		1
#define USB_LED_STATUS  	2
#define USB_DATA_OUT 		3
#define USB_DATA_WRITE 		4
#define USB_DATA_IN 		5

#define LED_BUILTIN   B,1

#define MAX_BUFFER	100
static uchar len = 0, pos = 0;

static uchar rsBuffer[MAX_BUFFER] = "";
static uchar valBuffer[1];
static uchar dataReceived = 0, dataLength = 0, dataSent = 0; // for USB_DATA_IN/_OUT
	   bool  LED_stateon = false;
	   
	   
PROGMEM const char usbHidReportDescriptor[22] = {    /* USB report descriptor */
	0x06, 0x00, 0xff,              // USAGE_PAGE (Generic Desktop)
	0x09, 0x01,                    // USAGE (Vendor Usage 1)
	0xa1, 0x01,                    // COLLECTION (Application)
	0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
	0x26, 0xff, 0x00,              //   LOGICAL_MAXIMUM (255)
	0x75, 0x08,                    //   REPORT_SIZE (8)
	0x95, 0x08,                    //   REPORT_COUNT (1)
	0x09, 0x00,                    //   USAGE (Undefined)
	0xb2, 0x02, 0x01,              //   FEATURE (Data,Var,Abs,Buf)
	0xc0                           // END_COLLECTION
};

inline void BlinkLED(void) {
  // if (LED_stateon == true) {
	TOGGLE_SET(LED_BUILTIN);
//	}
}

inline void SetLED_On(void) {
	HIGH_SET(LED_BUILTIN);
}

inline void SetLED_Off(void) {
	LOW_SET(LED_BUILTIN);
}


// this gets called when custom control message is received
/*
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
		default:
			if (len > pos) {
				usbMsgPtr = &rsBuffer[pos];
				pos++;
				return 1; // tell the driver to send 1 byte
				} else {
				// Drop through to return 0 (which will stall the request?)
				len = pos = 0; 
				return 0;
				}
	};

	return 0; // should not get here
}
*/

/*
usbMsgLen_t usbFunctionSetup(uchar data[8])
{
	usbRequest_t    *rq = (usbRequest_t*)((void *)data);

	if((rq->bmRequestType & USBRQ_TYPE_MASK) == USBRQ_TYPE_CLASS){    // HID class request 
		if(rq->bRequest == USBRQ_HID_GET_REPORT){  // wValue: ReportType (highbyte), ReportID (lowbyte) 
			// since we have only one report type, we can ignore the report-ID 
			//static uchar dataBuffer[2];  // buffer must stay valid when usbFunctionSetup returns 
			if (len > pos) {
				//usbMsgPtr = &rsBuffer[pos];
				int i;
				for (i=0; i<7; i++)
				{
					rsBuffer[i] = 'H';
				}
				rsBuffer[7]='\n';
				usbMsgPtr = &rsBuffer[0];
				//pos++;
				pos = pos + 8;
				return 7; // tell the driver to send 1 byte 
				} else {
				// Drop through to return 0 (which will stall the request?)
				len = pos = 0;
			  }
			}else if(rq->bRequest == USBRQ_HID_SET_REPORT){
			// since we have only one report type, we can ignore the report-ID 

			// TODO: Check race issues?
			//store_char(rq->wIndex.bytes[0], &rx_buffer);

		}
		}else{
		// ignore vendor type requests, we don't use any 
	}
	return 0;
}
*/

usbMsgLen_t usbFunctionSetup(uchar data[8])
{
	usbRequest_t    *rq = (usbRequest_t*)((void *)data);

	if((rq->bmRequestType & USBRQ_TYPE_MASK) == USBRQ_TYPE_VENDOR)
	{   
		switch(rq->bRequest) {  
			
		case REQ_LOGGING: if (len > pos) {
							usbMsgPtr = &rsBuffer[pos];
							pos++;
						 } 
						 else {
							len = pos = 0;
							rsBuffer[pos] = 0;  // send zero because no logging info received
							usbMsgPtr = &rsBuffer[pos];
						 }
						 return 1; // tell the driver to send 1 byte
						
		case REQ_ONBOARD_LED_ON: SetLED_On(); break;
		
		case REQ_ONBOARD_LED_OFF: SetLED_Off(); break;
		
		case REQ_ONBOARD_LED_STATUS: valBuffer[0] = READ_PIN(LED_BUILTIN);
									 usbMsgPtr = &valBuffer[0];
									 return 1;

		default:  break;				
			};
	} 
	return 0;
}


// This gets called when data is sent from PC to the device
USB_PUBLIC uchar usbFunctionWrite(uchar *data, uchar len) {
	uchar i;
	
	for(i = 0; dataReceived < dataLength && i < len; i++, dataReceived++)
	rsBuffer[dataReceived] = data[i];
	
	return (dataReceived == dataLength); // 1 if we received it all, 0 if not

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


uchar USBWriteStr(const char* data) {
  /*
   */
	int i; 

	//while (len > 0) USBDelay(10);
	pos = 0;
	len = strlen(data);
	if(len > MAX_BUFFER) 	// len is MAX_BUFFER chunk size 
     		len = MAX_BUFFER; // send an incomplete chunk 
     for(i = (len - 1); i >= 0; i--) rsBuffer[i] = data[i]; // copy the data to the buffer
     dataSent = len; 
     return len; // return real chunk size } 
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
        _delay_ms(10);
	  wdt_reset(); 		// keep the watchdog happy 	
    	}
 	usbDeviceConnect();
 	sei(); // Enable interrupts after re-enumeration
      
 	for(;;){  
		USBWriteStr("Hello World!\nUnd jetzt mit 50 Zeichen.\nUnd Noch mehr!!?!?!?\n");
		BlinkLED();               
	 	USBDelay(1000);
 	}
}

