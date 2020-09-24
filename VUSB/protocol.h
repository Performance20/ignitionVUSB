#pragma once

/*
 * protocol.h
 *
 * Created: 26.09.2019 21:24:50
 *  Author: helge

 * define the different messages to read/write spark/ignition values
 */

#ifndef PROTOCOL_H_
#define PROTOCOL_H_

#ifndef uchar
#define uchar   unsigned char
#endif

#ifndef uint
#define uint   unsigned int
#endif

//general states
#define VAL_STATE_OFF						0
#define VAL_STATE_ON						1

 // logging to host handling
#define REQ_LOGGING							0  // wants to have the current log
#define REQ_LOGGING_SET						REQ_LOGGING + 1  // start logging
#define REQ_LOGGING_GET						REQ_LOGGING_SET + 1  // stopp logging
#define VAL_LOG_OFF							0
#define VAL_LOG_ON							1
#define MAX_LOG_state					VAL_LOG_ON
#define MIN_LOG_state					VAL_LOG_OFF
#define DEF_LOG_state					VAL_LOG_OFF
static uint8_t  LOG_state = DEF_LOG_state;

// onboard LED handling
#define REQ_ONBOARD_LED_SET					REQ_LOGGING_GET + 1  // set onboard LED
#define REQ_ONBOARD_LED_GET					REQ_ONBOARD_LED_SET + 1
#define VAL_LED_STATE_OFF					0
#define VAL_LED_STATE_ON					1
#define VAL_LED_STATE_BLINK					2
#define VAL_LED_STATE_INPUT					3
#define VAL_LED_STATE_DIAG					4
#define MAX_LED_state					VAL_LED_STATE_DIAG
#define MIN_LED_state					VAL_LED_STATE_OFF
#define DEF_LED_state					VAL_LED_STATE_INPUT
static uint8_t  LED_state = DEF_LED_state;

// define mode of operation
#define REQ_ignition_mode_GET				REQ_ONBOARD_LED_GET + 1  
#define REQ_ignition_mode_SET				REQ_ignition_mode_GET + 1
#define VAL_ignition_mode_M0				0  // manuell
#define VAL_ignition_mode_M1				1  // fix
#define VAL_ignition_mode_M2				2  // fix with start helper
#define VAL_ignition_mode_M3				3  // dyn via table
#define MAX_ignition_mode					VAL_ignition_mode_M3
#define MIN_ignition_mode					VAL_ignition_mode_M0
#define DEF_ignition_mode					VAL_ignition_mode_M2
static uint8_t ignition_mode = DEF_ignition_mode; // or VAL_ignition_timing_DYN

// On mode M2, define the ignition timing (IT) point as start support, in round per minutes 
#define REQ_ITH_startpoint_GET				REQ_ignition_mode_SET + 1 
#define REQ_ITH_startpoint_SET				REQ_ITH_startpoint_GET + 1
#define MAX_ithelper_startpoint				20
#define MIN_ithelper_startpoint				-30
#define DEF_ithelper_startpoint				0
static int8_t ithelper_startpoint = DEF_ithelper_startpoint;  //degree: -30 to 20  

// On mode M2, define the max RPM for support starter help
#define REQ_starthelp_RPM_GET				REQ_ITH_startpoint_SET + 1 
#define REQ_starthelp_RPM_SET				REQ_starthelp_RPM_GET + 1
#define MAX_ignition_fix_startpoint			2000
#define MIN_ignition_fix_startpoint			0
#define DEF_ignition_fix_startpoint			500
static  int16_t ithelper_RPM_max = DEF_ignition_fix_startpoint;  //RPM: 0 - 2000  

// On fixed config = mode M1 + M2, define the shift from TDC
#define REQ_ignition_fix_startpoint_GET		REQ_starthelp_RPM_SET + 1
#define REQ_ignition_fix_startpoint_SET		REQ_ignition_fix_startpoint_GET + 1
#define MAX_ignition_fix_startpoint			20
#define MIN_ignition_fix_startpoint			(-30)
#define DEF_ignition_fix_startpoint			(-22)
static int8_t ignition_fix_startpoint = DEF_ignition_fix_startpoint;  //degree: -30 to 20  

// Dwell angle
#define REQ_Dwell_Angle_GET					REQ_ignition_fix_startpoint_SET + 1
#define REQ_Dwell_Angle_SET					REQ_Dwell_Angle_GET + 1
#define MAX_dwell_angle_fix					(-140)
#define MIN_dwell_angle_fix					(-240)
#define DEF_dwell_angle_fix					(-180)
static int16_t dwell_angle_fix = DEF_dwell_angle_fix;  //degree: -30 to 20  

// which table is active
#define REQ_ip_tbl_GET						REQ_Dwell_Angle_SET + 1 
#define REQ_ip_tbl_SET						REQ_ip_tbl_GET + 1
#define VAL_ip_table_1						0  // 
#define VAL_ip_table_2						1  // 
#define VAL_ip_table_3						2  // 
#define MAX_active_ip_tbl					VAL_ip_table_3
#define MIN_active_ip_tbl					VAL_ip_table_1
#define DEF_active_ip_tbl					VAL_ip_table_1
static uint8_t active_ip_tbl = VAL_ip_table_1;   //current active table 1-3		


															// set get a value in the tablex, 
// Format: 
// tbl number (0-2) - Byte (wValue.bytes[0]) Host -> Client  
// tbl position (0 - size) - Byte (wValue.bytes[1]) Host -> Client
// ignition_point rpm (0 - 20000) - 2 Bytes (wIndex.bytes[0+1] Host -> Client
// ignition_point degree (+/- 35) - 1 Byte, use etra data byte Host -> Client
// Client -> Host use serial bytes (5 bytes) 
#define REQ_ip_tbl_entry_GET					REQ_ip_tbl_SET + 1  
#define REQ_ip_tbl_entry_SET					REQ_ip_tbl_entry_GET + 1
#define MAX_ip_tbl_entry						ignition_point_tbl_SIZE - 1
#define MIN_ip_tbl_entry						0


typedef struct ignition_point {
	uint16_t 	rpm;     // if rpm > actl rpm
	int8_t		degree;   // shift to degree (-30/20)
	int16_t 	dwa;     // dwell angle -240/-140
	
} ignition_point_t;

typedef struct ignition_point_trans {
	uint8_t				tbl_nmb;   		// table number
	uint8_t				tbl_ip_pos;   	// table position
	ignition_point_t	ip;     		// if rpm > actl rpm
} ignition_point_trans_t;

#define ignition_point_tbl_SIZE  10	
volatile static const ignition_point_t ignition_point_tbl1[] = { //define rpm an assign ign point shift in degree
	{ 0, 0, 0 },              // rpm <  = x degree shift
	{ 0, 0, 0 },
	{ 0, 0, 0 },
	{ 0, 0, 0 },
	{ 0, 0, 0 },
	{ 0, 0, 0 },
	{ 0, 0, 0 },
	{ 0, 0, 0 },
	{ 0, 0, 0 },
	{ 0, 0, 0 }
};

volatile static const ignition_point_t ignition_point_tbl2[] = { //define rpm an assign ign point shift in degree
	{ 0, 0, 0 },              // rpm <  = x degree shift
	{ 0, 0, 0 },
	{ 0, 0, 0 },
	{ 0, 0, 0 },
	{ 0, 0, 0 },
	{ 0, 0, 0 },
	{ 0, 0, 0 },
	{ 0, 0, 0 },
	{ 0, 0, 0 },
	{ 0, 0, 0 }
};

volatile static const ignition_point_t ignition_point_tbl3[] = { //define rpm an assign ign point shift in degree
	{ 0, 0, 0 },              // rpm <  = x degree shift
	{ 0, 0, 0 },
	{ 0, 0, 0 },
	{ 0, 0, 0 },
	{ 0, 0, 0 },
	{ 0, 0, 0 },
	{ 0, 0, 0 },
	{ 0, 0, 0 },
	{ 0, 0, 0 },
	{ 0, 0, 0 }
};

// to address the active table 
volatile static const ignition_point_t* ignition_point_tbls[3] = {
	ignition_point_tbl1,
	ignition_point_tbl2,
	ignition_point_tbl3
};

/**********************************************************************************/
//
// actual measured rpm

// actual measured rpm
#define REQ_rpm_GET								REQ_ip_tbl_entry_SET + 1 
volatile static uint16_t act_rpm = 0;	                  // revolutions per minute
#define REQ_rps_GET								REQ_rpm_GET + 1
volatile static uint32_t act_rps = 0;	                  // revolutions per second
#define REQ_act_IP_GET							REQ_rps_GET + 1
volatile static int8_t act_IP = 0;	                      // current injection point in degree
#define REQ_act_DWA_GET							REQ_act_IP_GET + 1
volatile static int16_t act_DWA = 0;	                      // current dwell angle in degree
// time to next ignition in ms make it sense ?
#define REQ_next_ip_time_in_ms_GET				REQ_act_DWA_GET + 1
volatile static uint16_t next_ip_ms = 0;	           // in ms

// Overal running time
#define REQ_operation_sec_GET		REQ_next_ip_time_in_ms_GET + 1  // running in seconds or better minutes?
#define REQ_operation_sec_SET		REQ_operation_sec_GET + 1  //
volatile static uint32_t operationTime = 0;


#define DATA_SIZE_IN_BYTE			0x04  // size in Byte

#define REQ_firmware_version_GET		21  //manf
#endif /* PROTOCOL_H_ */