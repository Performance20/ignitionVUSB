/*
 * defintions.h
 *
 * Created: 11.10.2019 14:29:23
 *  Author: helge
 */ 


#ifndef __DEFINITIONS_H__
#define __DEFINITIONS_H__

#define LED_BUILTIN   B,1
#define FIRMWARE_REV_MAIN	0  	// 0-255
#define FIRMWARE_REV_SUB	0	//0-255

volatile static uint8_t firmwareMain = FIRMWARE_REV_MAIN;  // main.rev  = 1-255
volatile static uint8_t firmwareSub = FIRMWARE_REV_SUB;  // sub.rev  = 1-255


#endif /* __DEFINITIONS_H__ */