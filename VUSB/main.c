/*
 * main.c
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
//#include "ringbf.h"
//#include "osccal.c"
#include <limits.h> 
#include "tools.h"
#include "definitions.h"
#include "usbconnect.h"
#include "eeprom_circular_buffer.h"
	   
#define MAX_TIME_BETWEEN (UINT_MAX + 1) * 256   // round about 1 second  
#define MAIN_LOOP_DELAY		1					// 1 sec
#define MAIN_LOOP_DELAY_in_ms	MAIN_LOOP_DELAY * 1000  // 1 sec
#define OP_CNT_POINT		10  // each 10 sec write to the eeprom the counter

// EEprom parameter handling
uint8_t  EEMEM eeprom_start;
uint8_t  EEMEM eeprom_optm_start;
EE_PARAM EEMEM eeprom;
EE_PARAM parameter;
ignition_point_t EEMEM eeignition_point_tbl1[ignition_point_tbl_SIZE];
ignition_point_t EEMEM eeignition_point_tbl2[ignition_point_tbl_SIZE];
ignition_point_t EEMEM eeignition_point_tbl3[ignition_point_tbl_SIZE];
ignition_point_t ignition_point_tbl[ignition_point_tbl_SIZE];

uint8_t restartflag = 0;

#define DATA_SIZE sizeof(uint32_t)
#define OP_MAX_ENTRIES  60 // 100k per entry writing possible until eeprom die - = 100 * (4+1) = 500 Byte
#define MEM_SIZE ((DATA_SIZE) + 1) * OP_MAX_ENTRIES

uint8_t  EEMEM eeprom_operationTime[MEM_SIZE];
volatile uint32_t operationTime;
struct ee_cb cb;

void ee_read(uint8_t *addr, uint8_t *dst, size_t size) {
	 eeprom_read_block(dst, addr, size);

}

void ee_write(uint8_t *addr, const uint8_t *src, size_t size) {
	eeprom_update_block(src, addr, size);
}

volatile  uint16_t act_rpm = 0;	                  // revolutions per minute
volatile  uint32_t act_rps = 0;	                  // revolutions per second
volatile  int8_t act_IP = 0;	                      // current injection point in degree
volatile  int16_t act_DWA = 0;	                      // current dwell angle in degree
// time to next ignition in ms make it sense ?
volatile  uint16_t next_ip_ms = 0;	           // in ms


volatile long fire_cnt = 0;
volatile unsigned char ticks_cnt = 0;
volatile unsigned int ticksTCNT0_cnt = 0;
volatile unsigned long pre_ticks_cnt = 0;


/*
ISR (TIMER0_OVF_vect) //  overflow count between the signals to get the current rpm value
{
	if (ticks_cnt <= UINT_MAX) ticks_cnt++; // count min 1 round per second
}

ISR (INT0_vect)
{
	// raw ticks count per second between 2 sparkle fires
	ticksTCNT0_cnt = TCNT0;
	TCNT0 = 0;
	pre_ticks_cnt = ticks_cnt;
	ticks_cnt = 0;
}
*/
void init_device()
{
 uint8_t i;
 uint8_t status;
 
	// Timer 0 config
//	TCCR0B |= (1<<CS00); // Prescaler = 0
      // do Overflow Interrupt
//	TIMSK |= (1<<TOIE0);

	ee_cb_init(&cb, eeprom_operationTime, DATA_SIZE, OP_MAX_ENTRIES, ee_write, ee_read); // init the eeprom ring buffer
	// load eeprom data parameter data
	status = eeprom_read_byte(&eeprom_start);
	if (status != 1) // init the eeprom with default value
	{
		parameter.LOG_state = DEF_LOG_state;
		parameter.LED_state = DEF_LED_state;
		parameter.ignition_mode = DEF_ignition_mode;
		parameter.ithelper_startpoint = DEF_ithelper_startpoint;
		parameter.ithelper_RPM = DEF_ithelper_RPM;
		parameter.ignition_fix_startpoint = DEF_ignition_fix_startpoint;
		parameter.dwell_angle_fix = DEF_dwell_angle_fix;
		parameter.active_ip_tbl = DEF_active_ip_tbl;
		eeprom_update_block(&parameter, &eeprom, sizeof(eeprom));
		for (i=0; i<ignition_point_tbl_SIZE; i++) 
		{
			ignition_point_tbl[i].rpm = DEF_tbl_pos;
			ignition_point_tbl[i].rpm = DEF_tbl_rpm;
			ignition_point_tbl[i].degree = DEF_tbl_degree;
			ignition_point_tbl[i].dwa = DEF_tbl_dwa;
		} 
		eeprom_update_block(&ignition_point_tbl, &eeignition_point_tbl1, sizeof(eeignition_point_tbl1));
		eeprom_update_block(&ignition_point_tbl, &eeignition_point_tbl2, sizeof(eeignition_point_tbl2));
		eeprom_update_block(&ignition_point_tbl, &eeignition_point_tbl3, sizeof(eeignition_point_tbl3));
		eeprom_update_byte(&eeprom_start, 1);
	}
	else
	{
		eeprom_read_block(&parameter, &eeprom, sizeof(parameter));
		if (parameter.active_ip_tbl == VAL_active_ip_table_2)
			eeprom_read_block(&ignition_point_tbl, &eeignition_point_tbl2, sizeof(ignition_point_tbl));
		else if (parameter.active_ip_tbl == VAL_active_ip_table_3)
			eeprom_read_block(&ignition_point_tbl, &eeignition_point_tbl3, sizeof(ignition_point_tbl));
		else
			eeprom_read_block(&ignition_point_tbl, &eeignition_point_tbl1, sizeof(ignition_point_tbl));
	}
	// operation counter
	status = eeprom_read_byte(&eeprom_optm_start);	
	if (status != 1) // set operation time back to zero
	{
		operationTime = 0;
		ee_cb_write(&cb, (uint8_t*) &operationTime);
		eeprom_update_byte(&eeprom_optm_start, 1);
	}
	else
	{
		ee_cb_read(&cb, (uint8_t*) &operationTime);
		operationTime += OP_CNT_POINT / 2; // max. wrong value could be 10 sec, therefore I add 5 sec to middle the error
	}
	
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
}

int main(void)
{
	int act_rps, act_rpm;
	unsigned long ticks_cnt_peroid;
	   
    //bool led = true;
	//char b[6] = "Hallo";

 	init_device();
	//stdout = &mystdout;
   //b[5] = 0;
 	for(;;){  
	 	 
	//	ticks_cnt_peroid = (pre_ticks_cnt * (UINT_MAX +1)) + ticksTCNT0_cnt;
	//	if (ticks_cnt_peroid > F_CPU)
 	//		act_rps = 1;
	//	else
	//		act_rps = (int) (F_CPU / ticks_cnt_peroid);
	//	act_rpm = act_rps * 60;
	//	LOG("act_rpm = %d ,  ",act_rpm);
	// 	USBDelay_ms(100);
//		LOG("act_rps = %d\n",act_rps);
//		USBWriteStr("act_rps = ");
//		USBWriteStr(&bf[0]);
//		USBWriteStr(&b[0]);
//		b[0] = b[0] + 1;
		
		//BlinkLEDD();  
		//LOG("abcdefghi\n");
		if (restartflag)		
		{
			restartflag -= 1;
			if (!restartflag)  // restart to clean the eeprom
			{
				USBDelay_ms(1);
				while(1); 	
			}
		}

		// update operation counter
		operationTime += MAIN_LOOP_DELAY;
		if (!(operationTime % OP_CNT_POINT)) 
		{
			ee_cb_write(&cb, (uint8_t*) &operationTime);	// save operation timer 
			//LOGHINT2(operationTime);
		}
	 	USBDelay_ms(MAIN_LOOP_DELAY_in_ms);
 	}
}
