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
	   
#define MAX_TIME_BETWEEN (UINT_MAX +1) * 256   // round about 1 second  

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
 uchar i;
	// Timer 0 config
//	TCCR0B |= (1<<CS00); // Prescaler = 0
      // do Overflow Interrupt
//	TIMSK |= (1<<TOIE0);

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
    bool led = true;
	uchar b[2] = "a";

 	init_device();
	//stdout = &mystdout;
   b[1] = 0;
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
		USBWriteStr(&b[0]);
		b[0] = b[0] + 1;
		
		//BlinkLEDD();               
	 	USBDelay_ms(1000);
 	}
}

