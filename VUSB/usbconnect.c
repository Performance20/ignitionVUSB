
#include "usbdrv.h"
#include "usbconnect.h" 
#include "usbdrv.h"
#include "definitions.h"
#include "tools.h"
#include "protocol.h"
#include "ringbf.h"


//static char bf[6];  		// itoa buffer;
//static uchar dataReceived, dataLength, dataSent; // for USB_DATA_IN/_OUT
uchar tbl, pos, remain;

PROGMEM const char usbHidReportDescriptor[22] = {    /* USB report descriptor */
	0x06, 0x00, 0xff,              // USAGE_PAGE (Generic Desktop)
	0x09, 0x01,                    // USAGE (Vendor Usage 1)
	0xa1, 0x01,                    // COLLECTION (Application)
	0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
	0x26, 0xff, 0x00,              //   LOGICAL_MAXIMUM (255)
	0x75, 0x08,                    //   REPORT_SIZE (8)
	0x95, DATA_SIZE_IN_BYTE,       //   REPORT_COUNT (1), I think is not needed, but anyway
	0x09, 0x00,                    //   USAGE (Undefined)
	0xb2, 0x02, 0x01,              //   FEATURE (Data,Var,Abs,Buf)
	0xc0                           // END_COLLECTION
};
/*
int usb_putchar( char c, FILE *stream )
{
    // if( c == '\n' )
    //    usb_putchar( '\r', stream );
	
	tx_write(c);    
	return 0;
}
*/
int usb_putchar( char c)
{
    /*if( c == '\n' )
        usb_putchar( '\r', stream );
	*/
	tx_write(c, true);    
	return 0;
}

uchar USBWriteStr(const char* strdata) 
{
	int i, len; 

	len = strlen(strdata);
	//i = tx_remaining();
	//if(len > i) 	// not enough space, but not needed because is checked by buffer if not in overwrite mode
    // 		len = i; 	// send an incomplete chunk 
    for(i = 0; i < len ; i++) tx_write(strdata[i], true); // copy the data to the buffer
    return len; // return real chunk size  
}



// wait a specified number of milliseconds (very roughly), refreshing in the background
void USBDelay_ms(unsigned int milli) { 
    unsigned int i;     
    for (i = 0; i<milli; i++) {
        _delay_ms (1);
	 	wdt_reset(); // keep the watchdog happy
		usbPoll();
  	}
}

// This gets called when data is sent from PC to the device
// not implemeneted yet
//USB_PUBLIC uchar usbFunctionWrite(uchar *data, uchar len) {
//	uchar i;
//	
//	for(i = 0; dataReceived < dataLength && i < len; i++, dataReceived++)
//	rsBuffer[dataReceived] = data[i];
//	
//	return (dataReceived == dataLength); // 1 if we received it all, 0 if not
//}


//USB MSG handling

usbMsgLen_t usbFunctionSetup(uchar data[8])
{
	usbRequest_t    *rq = (usbRequest_t*)((void *)data);
	
	int				j, i; 
	static uchar    dataBuffer[4];

	if((rq->bmRequestType & USBRQ_TYPE_MASK) == USBRQ_TYPE_VENDOR)
	{   
		switch(rq->bRequest) 
		{  	
		case REQ_LOGGING: 
					dataBuffer[0] = tx_read(); // return 0 if nothing to send
					/*
						if (llen > ppos) {
							usbMsgPtr = &dataBuffer[ppos];
							ppos++;
						}
						else {
							llen = ppos = 0;
							dataBuffer[ppos] = 0;  // send zero because no logging info received
							usbMsgPtr = &dataBuffer[ppos];
						}
					*/	
					usbMsgPtr = &dataBuffer[0]; 
					return 1; 

		case REQ_LOGGING_SET:
					if (rq->wValue.bytes[0] == VAL_STATE_ON) {
						LOG_state = VAL_STATE_ON;	
						LOGHINT;					
					}
					else {
					 if (rq->wValue.bytes[0] == VAL_STATE_OFF) { 
						LOG_state = VAL_STATE_OFF;
						LOGHINT;	 
					}
					 else 
						LOGERR2(rq->wValue.bytes[0]); 	
					}
					break;

		case REQ_LOGGING_GET:
					dataBuffer[0] = LOG_state;
					usbMsgPtr = &dataBuffer[0];
					LOGHINT;
					return 1;

		case REQ_ONBOARD_LED_SET: 
					if (rq->wValue.bytes[0] == VAL_STATE_ON) {
						SetLED_On();
						LOGHINT;
					}
					else {
					if (rq->wValue.bytes[0] == VAL_STATE_OFF) {
						SetLED_Off(); 
						LOGHINT;
					}
					else
					LOGERR2(rq->wValue.bytes[0]); 
					}
					break;
		
		case REQ_ONBOARD_LED_GET: 
					dataBuffer[0] = READ_PIN(LED_BUILTIN);
					usbMsgPtr = &dataBuffer[0];
					LOGHINT;
					return 1;
					
		case REQ_ignition_mode_SET:	
					switch(rq->wValue.bytes[0])
					{
						case VAL_ignition_mode_M0:
				     		ignition_mode = VAL_ignition_mode_M0;
							LOGHINT;
							break;
						case VAL_ignition_mode_M1:
							ignition_mode = VAL_ignition_mode_M1;
							LOGHINT;
							break;
						case VAL_ignition_mode_M2:
							ignition_mode = VAL_ignition_mode_M2;
							LOGHINT;
							break;
						case VAL_ignition_mode_M3:
							ignition_mode = VAL_ignition_mode_M3;
							LOGHINT;
							break;
						deafault:
							LOGERR2(rq->wValue.bytes[0]); 
					}
					break;

		case REQ_ignition_mode_GET:				
					dataBuffer[0] = ignition_mode;
					usbMsgPtr = dataBuffer;
					LOGHINT;
					return 1;

		case REQ_ignition_fix_startpoint_SET:				
					if ((rq->wValue.bytes[0] >= MAX_ignition_fix_startpoint) && (rq->wValue.bytes[0] <= MIN_ignition_fix_startpoint)) {
	                 	ignition_fix_startpoint = rq->wValue.bytes[0];
						LOGHINT2(ignition_fix_startpoint);
					}
					else {
						//itoa(rq->wValue.bytes[0],bf,10);
						LOGERR2(rq->wValue.bytes[0]); 
					}
					break;

		case REQ_ignition_fix_startpoint_GET:				
					dataBuffer[0] = ignition_fix_startpoint;
					usbMsgPtr = dataBuffer;
					return 1;

		case REQ_ip_tbl_SET:				
					if ((rq->wValue.bytes[0] >= VAL_ip_table_1) && (rq->wValue.bytes[0] <= VAL_ip_table_3)) {
	                 	active_ip_tbl = rq->wValue.bytes[0];
						LOGHINT2(active_ip_tbl);
					}
					else {
						//itoa(rq->wValue.bytes[0],bf,10);
						LOGERR2(rq->wValue.bytes[0]); 
					}
					break;

		case REQ_ip_tbl_GET:				
					dataBuffer[0] = active_ip_tbl;
					usbMsgPtr = dataBuffer;
					LOGHINT;
					return 1;

		case REQ_rpm_GET:
					for(j=0,i=sizeof(act_rpm)-1; i>=0; i--,j++){
						dataBuffer[j]= (act_rpm>>(i*8))&0xff;
					}				
					usbMsgPtr = dataBuffer;
					LOGHINT;
					return sizeof(act_rpm);

		case REQ_rps_GET:
					for(j=0,i=sizeof(act_rps)-1; i>=0; i--,j++){
						dataBuffer[j]= (act_rps>>(i*8))&0xff;
					}				
					usbMsgPtr = dataBuffer;
					LOGHINT;
					return sizeof(act_rps);

		case REQ_ip_tbl_entry_GET:
					tbl = rq->wValue.bytes[0];
					pos = rq->wValue.bytes[1];

					if (tbl  >= 1 && tbl  <= 3) {
					 if (pos >= 1 && pos <= ignition_point_tbl_SIZE) {
					   dataBuffer[0] = (ignition_point_tbls[tbl][pos].rpm>>8) & 0xff;
					   dataBuffer[1] = (ignition_point_tbls[tbl][pos].rpm) & 0xff;
					   dataBuffer[2] = ignition_point_tbls[tbl][pos].degree;
					   usbMsgPtr = dataBuffer;
					   LOGHINT3(tbl, pos); 		  
					   return 3;
					  }
					  else 
					   LOGERR2(pos);
					 }
					 else
					   LOGERR2(tbl);
					break;

		case REQ_ip_tbl_entry_SET:
					tbl = rq->wValue.bytes[0];
					pos = rq->wValue.bytes[1];
					 
					if (tbl >= 1 && tbl <= 3) 
					  if (pos >= 1 && pos <= ignition_point_tbl_SIZE) {
					   //ignition_point_tbls[tbl][pos].rpm = rq->wIndex.word;
					   //ignition_point_tbls[tbl][pos].degree = rq->wLength.bytes[0];
					   LOGHINT3(tbl, pos); 	
					}
					break;
					
/*
		case REQ_ip_table_GET:
					tbl = rq->wValue.bytes[0];
					if (tbl  >= 1 && tbl  <= 3) { 
					   usbMsgPtr = (uchar*) &ignition_point_tbls[tbl][0];
					   LOGHINT3(tbl, pos); 		  
					   return sizeof(ignition_point_tbl1);  //should be 30 Bytes
					// return sizeOf(ignition_point_t) * ignition_point_tbl_SIZE
					  }
					 else
					   LOGHINT2(tbl);
					break;

		case REQ_ip_table_SET:
					if (tbl >= 1 && tbl <= 3) {
					   LOGHINT2(tbl); 	
					   tbl = rq->wValue.bytes[0];
					   pos = 0;
					   remain = sizeof(ignition_point_tbl1); 
						//sizeOf(ignition_point_t) * ignition_point_tbl_SIZE  30
					   return USB_NO_MSG; // use usbFunctionWrite
					}
					 else
					   LOGERR2(tbl);
					break;
  */
		case REQ_firmware_version_GET:				
					dataBuffer[0] = firmwareMain;
					dataBuffer[1] = firmwareSub;
					usbMsgPtr = dataBuffer;
					LOGHINT;
					return 2;

		case REQ_operation_sec_GET:				
					for(j=0,i=sizeof(operationTime)-1; i>=0; i--,j++){
						dataBuffer[j]= (operationTime >>(i*8))&0xff;
					}				
					usbMsgPtr = dataBuffer;
					LOGHINT;
					return sizeof(operationTime);

		case REQ_operation_sec_SET:				
				   	operationTime = ((unsigned long) rq->wValue.word << 16) + rq->wIndex.word;
					LOGHINT;		
					break;

		default:  break;				
		};
	} 
	return 0;
}

uchar usbFunctionWrite(unsigned char *data, unsigned char len)
{
    uchar i;
    //uchar 	*table;    

    if(len > remain)                // if this is the last incomplete chunk
        len = remain;               // limit to the amount we can store
    remain = remain - len;
   // memcpy(&ignition_point_tbls[tbl][pos], data, len);
// table = (uchar*) &ignition_point_tbls[tbl][pos];
//  for(i = 0; i < len; i++)
//      table[pos++] = data[i];
    pos+= len; 
    return remain == 0;             // return 1 if we have all data
}

