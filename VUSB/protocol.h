/*
 * protocol.h 
 *
 * Created: 26.09.2019 21:24:50
 *  Author: helge
 
 * define the different messages to read/write spark values
 */ 

#ifndef PROTOCOL_H_
#define PROTOCOL_H_

#define VAL_STATE_OFF						0
#define VAL_STATE_ON						1

#define VAL_ignition_timing_FIX				2  // set a fix point 
#define VAL_ignition_timing_DYN				3  // dyn adjustment depending on rpm

#define VAL_ip_table_1						0  // 
#define VAL_ip_table_2						1  // 
#define VAL_ip_table_3						2  // 

// logging to host handling
#define REQ_LOGGING							0
#define REQ_LOGGING_SET						1  // log text messages to the log window
#define REQ_LOGGING_GET						2  // 
static bool  LOG_stateon = VAL_STATE_OFF;		// values: STATE_ON or STATE_OFF 

// onboard LED handling
#define REQ_ONBOARD_LED_SET					3  // set onboard LED
#define REQ_ONBOARD_LED_GET					4
static bool  LED_stateon = VAL_STATE_OFF;		// values: STATE_ON or STATE_OFF 

// define if table controlled by egine rpm or a fix value is used
#define REQ_ignition_timing_GET				5  
#define REQ_ignition_timing_SET				6
static uchar ignition_timing = VAL_ignition_timing_FIX; // or VAL_ignition_timing_DYN

// On fixed config, define the shift from TDC, on dyn config define start point
#define TDC_startpoint						0 // OT = 0 Grad (degree) 
#define REQ_ignition_startpoint_GET			7 
#define REQ_ignition_startpoint_SET			8
static uchar ignition_startpoint = TDC_startpoint;  //degree - byte 0 - 30  

// which table is active
#define REQ_ip_tbl_GET						9 
#define REQ_ip_tbl_SET						10
static uchar active_ip_tbl = VAL_ip_table_1;				//current active table 1-3		
// set get a value in the tablex, 
// Format: 
// tbl number (0-2) - Byte (wValue.bytes[0]) Host -> Client  
// tbl position (0 - size) - Byte (wValue.bytes[1]) Host -> Client
// ignition_point rpm (0 - 20000) - 2 Bytes (wIndex.bytes[0+1] Host -> Client
// ignition_point degree (+/- 35) - 1 Byte, use etra data byte Host -> Client
// Client -> Host use serial bytes (5 bytes) 
#define REQ_ip_tbl_entry_GET					11 
#define REQ_ip_tbl_entry_SET					12

// actual measured rpm
#define REQ_rpm_GET								13 
volatile static unsigned int act_rpm = 0;	                      // revolutions per minute
#define REQ_rpc_GET								14 
volatile static unsigned long act_rpc = 0;	                      // revolutions per second

// active table entry
#define REQ_active_ip_GET						15 
									// return structure ignition_point_trans 
// time to next ignition in ms make it sense ?
#define REQ_next_ip_time_GET					16 
volatile static unsigned long next_ip_ms = 0;	           // in ms

typedef struct ignition_point {
    unsigned int 	rpm;     // if rpm > actl rpm
    char   degree;   // shift to degree (+35/-35)
} ignition_point_t;

typedef struct ignition_point_trans {
    uchar				tbl_nmb;   		// table number
    uchar				tbl_ip_pos;   	// table number
    ignition_point_t	ip;     		// if rpm > actl rpm
} ignition_point_trans_t;
	
#define ignition_point_tbl_SIZE  10	
volatile static const ignition_point_t ignition_point_tbl1[] = { //define rpm an assign ign point shift in degree
	{ 0, 0 },              // rpm <  = x degree shift
	{ 0, 0 },              
	{ 0, 0 },              
	{ 0, 0 },              
	{ 0, 0 },              
	{ 0, 0 },              
	{ 0, 0 },              
	{ 0, 0 }, 
	{ 0, 0 },                           
	{ 0, 0 }              
};

volatile static const ignition_point_t ignition_point_tbl2[] = { //define rpm an assign ign point shift in degree
	{ 0, 0 },              // rpm <  = x degree shift
	{ 0, 0 },              
	{ 0, 0 },              
	{ 0, 0 },              
	{ 0, 0 },              
	{ 0, 0 },              
	{ 0, 0 },              
	{ 0, 0 },              
	{ 0, 0 },  
	{ 0, 0 }                          
};

volatile static const ignition_point_t ignition_point_tbl3[] = { //define rpm an assign ign point shift in degree
	{ 0, 0 },              // rpm <  = x degree shift
	{ 0, 0 },              
	{ 0, 0 },              
	{ 0, 0 },              
	{ 0, 0 },              
	{ 0, 0 },              
	{ 0, 0 },              
	{ 0, 0 },              
	{ 0, 0 },  
	{ 0, 0 }                          
};

// to addess the active table 
volatile static const ignition_point_t* ignition_point_tbls[3] = {
	ignition_point_tbl1, 	
	ignition_point_tbl2,
 	ignition_point_tbl3
};

#define DATA_SIZE_IN_BYTE			0x04  // size in Byte


#endif /* PROTOCOL_H_ */