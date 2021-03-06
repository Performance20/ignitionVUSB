
#include "usbdrv.h"
#include "usbconnect.h" 
#include "usbdrv.h"
#include "definitions.h"
#include "tools.h"
#include "protocol.h"
#include "ringbf.h"
#include "eeprom_circular_buffer.h"


//static char bf[6];  		// itoa buffer;
//static uchar dataReceived, dataLength, dataSent; // for USB_DATA_IN/_OUT
uchar tbl, pos, remain;
extern EE_PARAM parameter;
extern uint32_t operationTime;
extern  uint16_t act_rpm;	                  // revolutions per minute
extern  uint32_t act_rps;	                  // revolutions per second
extern  int8_t act_IP;	                      // current injection point in degree
extern  int16_t act_DWA;	                      // current dwell angle in degree
// time to next ignition in ms make it sense ?
extern  uint16_t next_ip_ms;	           // in ms
extern EE_PARAM EEMEM eeprom;
extern uint8_t  EEMEM eeprom_start;
extern ignition_point_t EEMEM eeignition_point_tbl1[ignition_point_tbl_SIZE];
extern ignition_point_t EEMEM eeignition_point_tbl2[ignition_point_tbl_SIZE];
extern ignition_point_t EEMEM eeignition_point_tbl3[ignition_point_tbl_SIZE];
extern ignition_point_t ignition_point_tbl[ignition_point_tbl_SIZE];
extern uint8_t restartflag;
extern struct ee_cb cb;


//static uint8_t  dataBuffer[DATA_STRING_SIZE_IN_BYTE];
static uint8_t  dataBuffer[DATA_TABLE_SIZE_IN_BYTE + 1];
static uint8_t	currentPosition, bytesRemaining;

PROGMEM const char usbHidReportDescriptor[22] = {    /* USB report descriptor */
	0x06, 0x00, 0xff,              // USAGE_PAGE (Generic Desktop)
	0x09, 0x01,                    // USAGE (Vendor Usage 1)
	0xa1, 0x01,                    // COLLECTION (Application)
	0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
	0x26, 0xff, 0x00,              //   LOGICAL_MAXIMUM (255)
	0x75, 0x08,                    //   REPORT_SIZE (8)
	0x95, DATA_STRING_SIZE_IN_BYTE,//   REPORT_COUNT (1), I think is not needed, but anyway
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

int usb_putchar( char c)
{
   //if( c == '\n' )
   //     usb_putchar( '\r', stream );
//	
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

*/

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
	int16_t				i; 
	int8_t          con8;
	int16_t         con16;
	int8_t			ee_update = false;
	
	if((rq->bmRequestType & USBRQ_TYPE_MASK) == USBRQ_TYPE_VENDOR)
	{   
		switch(rq->bRequest) 
		{  	
	/* LOGGING **********************************************************************/																	
		case REQ_LOGGING: 
					i = 0;
					while ((dataBuffer[i] = tx_read()))
					{
						i++;
						if (i == DATA_STRING_SIZE_IN_BYTE) break;
					}
					usbMsgPtr = dataBuffer; 
					return i; 
					
		case REQ_LOGGING_SET:
					if ((rq->wValue.bytes[0] >= MIN_LOG_state) && (rq->wValue.bytes[0] <= MAX_LOG_state))
					{
						parameter.LOG_state = rq->wValue.bytes[0];
						if (rq->wIndex.bytes[0] == 1) ee_update = true;
					}
					break;

		case REQ_LOGGING_GET:
					dataBuffer[0] = parameter.LOG_state;
					usbMsgPtr = dataBuffer;
					return 1;

	/* LED **********************************************************************/														
		case REQ_ONBOARD_LED_SET: 
					if ((rq->wValue.bytes[0] >= MIN_LED_state) && (rq->wValue.bytes[0] <= MAX_LED_state))
					{
						parameter.LED_state = rq->wValue.bytes[0];
						if (rq->wIndex.bytes[0] == 1) ee_update = true;
						if (parameter.LED_state == VAL_LED_STATE_ON) SetLED_On();
						else 
							if (parameter.LED_state == VAL_LED_STATE_OFF) SetLED_Off();
					}
					break;
		
		case REQ_ONBOARD_LED_GET: 
					dataBuffer[0] = parameter.LED_state;
					usbMsgPtr = dataBuffer;
					return 1;
					
					
	/* ithelper_startpoint **********************************************************************/									
		case REQ_ignition_mode_SET:		
					if ((rq->wValue.bytes[0] >= MIN_ignition_mode) && (rq->wValue.bytes[0] <= MAX_ignition_mode)) 
					{
						parameter.ignition_mode = rq->wValue.bytes[0];
						if (rq->wIndex.bytes[0] == 1) ee_update = true;
					}
					break;

		case REQ_ignition_mode_GET:				
					dataBuffer[0] = parameter.ignition_mode;
					usbMsgPtr = dataBuffer;
					return 1;
	
	/* ithelper_startpoint **********************************************************************/				
		case REQ_ITH_startpoint_SET: // ZZP Starten
					con8 = rq->wValue.bytes[0];		
					if ((con8 >= MIN_ithelper_startpoint) && (con8 <= MAX_ithelper_startpoint)) 
					{
						parameter.ithelper_startpoint = con8;
						if (rq->wIndex.bytes[0] == 1) ee_update = true;
					}
					break;

		case REQ_ITH_startpoint_GET:
					dataBuffer[0] = parameter.ithelper_startpoint;
					usbMsgPtr = dataBuffer;
					return 1;
					
	/* ithelper_RPM **********************************************************************/
		case REQ_starthelp_RPM_SET: // ZZP Starten
					if ((rq->wValue.word >= MIN_ithelper_RPM) && (rq->wValue.word <= MAX_ithelper_RPM))
					{
						parameter.ithelper_RPM = rq->wValue.word;
						if (rq->wIndex.bytes[0] == 1) ee_update = true;
					}
					break;

		case REQ_starthelp_RPM_GET:
					dataBuffer[0] = parameter.ithelper_RPM;
					dataBuffer[1] = parameter.ithelper_RPM >> 8;
					usbMsgPtr = dataBuffer;
					return 2;
					
/* ignition_fix_startpoint **********************************************************************/
		case REQ_ignition_fix_startpoint_SET:  				
					con8 = rq->wValue.bytes[0];				
					if ((con8 >= MIN_ignition_fix_startpoint) && (con8 <= MAX_ignition_fix_startpoint)) 
					{
	                 	parameter.ignition_fix_startpoint = con8;
						if (rq->wIndex.bytes[0] == 1) ee_update = true;
					}
					break;

		case REQ_ignition_fix_startpoint_GET:				
					dataBuffer[0] = parameter.ignition_fix_startpoint;
					usbMsgPtr = dataBuffer;
					return 1;

/* dwell_angle_fix **********************************************************************/
		case REQ_Dwell_Angle_SET:
					con16 = rq->wValue.word;
					if ( (con16 >= MIN_dwell_angle_fix) && (con16 <= MAX_dwell_angle_fix))
					{
						parameter.dwell_angle_fix = con16;
						if (rq->wIndex.bytes[0] == 1) ee_update = true;
					}
					break;

		case REQ_Dwell_Angle_GET:
					dataBuffer[0] = parameter.dwell_angle_fix;
					dataBuffer[1] = parameter.dwell_angle_fix >> 8;
					usbMsgPtr = dataBuffer;
					return 2;

/* IP Table set  **********************************************************************/
		case REQ_active_ip_tbl_SET:				
					if ((rq->wValue.bytes[0] >= MIN_active_ip_tbl) && (rq->wValue.bytes[0] <= MAX_active_ip_tbl)) {
	                 	parameter.active_ip_tbl = rq->wValue.bytes[0];
						if (rq->wIndex.bytes[0] == 1) ee_update = true;
					}
					break;

		case REQ_active_ip_tbl_GET:				
					dataBuffer[0] = parameter.active_ip_tbl;
					usbMsgPtr = dataBuffer;
					return 1;

/* real time values *******************************************************************/ 
		case REQ_rpm_GET:
					dataBuffer[0] = act_rpm;
					dataBuffer[1] = act_rpm >> 8;
					usbMsgPtr = dataBuffer;
					return 2;

		case REQ_rps_GET:
					dataBuffer[0] = act_rps;
					dataBuffer[1] = act_rps >> 8;
					dataBuffer[2] = act_rps >> 16;
					dataBuffer[3] = act_rps >> 24;
					usbMsgPtr = dataBuffer;
					return 4;

		case REQ_act_DWA_GET:
					dataBuffer[0] = act_DWA;
					dataBuffer[1] = act_DWA >> 8;
					usbMsgPtr = dataBuffer;
					return 2;
						
		case REQ_next_ip_time_in_ms_GET:
					dataBuffer[0] = next_ip_ms;
					dataBuffer[1] = next_ip_ms >> 8;
					usbMsgPtr = dataBuffer;
					return 2;
/*
		case REQ_ip_tbl_entry_GET:
					tbl = rq->wValue.bytes[0];
					pos = rq->wValue.bytes[1];

					if (tbl  >= 1 && tbl  <= 3) {
					 if (pos >= 1 && pos <= ignition_point_tbl_SIZE) {
					   dataBuffer[0] = (ignition_point_tbls[tbl][pos].rpm>>8) & 0xff;
					   dataBuffer[1] = (ignition_point_tbls[tbl][pos].rpm) & 0xff;
					   dataBuffer[2] = ignition_point_tbls[tbl][pos].degree;
					   usbMsgPtr = dataBuffer;
					   //LOGHINT3(tbl, pos); 		  
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
					   //LOGHINT3(tbl, pos); 	
					}
					break;
					
	
*/

		case REQ_ip_tbl_GET:
					if (rq->wValue.bytes[0] >= MIN_ip_tbl && rq->wValue.bytes[0] <= MAX_ip_tbl) 
					{
						switch (rq->wValue.bytes[0])
						{
							case VAL_ip_table_1:
								eeprom_read_block(&dataBuffer, &eeignition_point_tbl1, sizeof(ignition_point_tbl));						
								break;
	
							case VAL_ip_table_2:
								eeprom_read_block(&dataBuffer, &eeignition_point_tbl2, sizeof(ignition_point_tbl));						
								break;

							case VAL_ip_table_3:
								eeprom_read_block(&dataBuffer, &eeignition_point_tbl3, sizeof(ignition_point_tbl));						
						
						};
					   usbMsgPtr = dataBuffer;
					   return sizeof(ignition_point_tbl);  //should be 60 Bytes
					  }
					break;

		case REQ_ip_tbl_SET:
					if (rq->wValue.bytes[0] >= MIN_ip_tbl && rq->wValue.bytes[0] <= MAX_ip_tbl) 
					{
					   dataBuffer[0] = rq->wValue.bytes[0];  // store table number in byte 0
					   currentPosition = 1;
					   bytesRemaining = sizeof(ignition_point_tbl); 
					   return USB_NO_MSG; // use usbFunctionWrite
					}
					break;
 
		case REQ_firmware_version_GET:				
					dataBuffer[0] = firmwareMain;
					dataBuffer[1] = firmwareSub;
					usbMsgPtr = dataBuffer;
					return 2;

		case REQ_operation_sec_GET:		
					dataBuffer[0] = operationTime;
					dataBuffer[1] = operationTime >> 8;
					dataBuffer[2] = operationTime >> 16;
					dataBuffer[3] = operationTime >> 24;
					usbMsgPtr = dataBuffer;
					return 4;

		case REQ_operation_sec_SET:				
					operationTime = (((uint32_t) rq->wValue.word) << 16) + rq->wIndex.word;
					ee_cb_write(&cb, (uint8_t*) &operationTime);
					break;
					
		case REQ_eeprom_SET:	
		           switch (rq->wValue.bytes[0]) 
				   {					   
		           case REQ_eeprom_parameter_SET:
							eeprom_update_block(&parameter, &eeprom, sizeof(parameter));	
		           			break;
							   
					case REQ_eeprom_table1_SET:
							eeprom_update_block(&ignition_point_tbl, &eeignition_point_tbl1, sizeof(ignition_point_tbl));
							break;
							
					case REQ_eeprom_table2_SET:
							eeprom_update_block(&ignition_point_tbl, &eeignition_point_tbl2, sizeof(ignition_point_tbl));
							break;

					case REQ_eeprom_table3_SET:
							eeprom_update_block(&ignition_point_tbl, &eeignition_point_tbl3, sizeof(ignition_point_tbl));
							break;
							
					case REQ_eeprom_INIT_SET:
							eeprom_update_byte(&eeprom_start, 0xFF);
							restartflag = 2; 
							break;			
		           };	
				   break;
				   
		case REQ_modul_BOOT:
							restartflag = 2; 
							break;
		};
		
		if (ee_update == true)
		{
			// problem is (I don't know, i really a problem), each change in the struct lead to a complete write to the eeprom = 100 entries lead to 100 writes for each update)
			eeprom_update_block(&parameter, &eeprom, sizeof(eeprom));
		}
	} 
	return 0;
}

uint8_t usbFunctionWrite(uint8_t *data, uint8_t len)
{
    uint8_t i;

	if(len > bytesRemaining)                // if this is the last incomplete chunk
        len = bytesRemaining;               // limit to the amount we can store
	//LOGHINT2(bytesRemaining);
    bytesRemaining -= len;
	for(i = 0; i < len; i++)
	 dataBuffer[currentPosition++] = data[i];
	if (bytesRemaining == 0) // full table received
	{
		///LOGHINT2(dataBuffer[0]);
		switch (dataBuffer[0])
		{
			case VAL_ip_table_1:
		           eeprom_update_block(&dataBuffer[1], &eeignition_point_tbl1, sizeof(eeignition_point_tbl1));
				   break;
				           
			case VAL_ip_table_2:
		           eeprom_update_block(&dataBuffer[1], &eeignition_point_tbl2, sizeof(eeignition_point_tbl2));
				   break;

	        case VAL_ip_table_3:
		           eeprom_update_block(&dataBuffer[1], &eeignition_point_tbl3, sizeof(eeignition_point_tbl3));
				   break;
		};
		if (parameter.active_ip_tbl == dataBuffer[0])
			memcpy(&ignition_point_tbl, &dataBuffer[1], sizeof(ignition_point_tbl));
		return 0;	
	}
	return 1;
}
