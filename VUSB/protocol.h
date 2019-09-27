/*
 * protocol.h 
 *
 * Created: 26.09.2019 21:24:50
 *  Author: helge
 
 * define the different messages to read/write spark values
 */ 


#ifndef PROTOCOL_H_
#define PROTOCOL_H_

#define REQ_LOGGING				1  // log text messages to the log window
#define REQ_ONBOARD_LED_SET		2
#define REQ_ONBOARD_LED_STATUS	3

#define VAL_ONBOARD_LED_ON		1
#define VAL_ONBOARD_LED_OFF		2

#define DATA_SIZE_IN_BYTE		0x04  // size in Byte


#endif /* PROTOCOL_H_ */