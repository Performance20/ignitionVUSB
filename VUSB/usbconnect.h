
#ifndef __usbconnect_h_included__
#define __usbconnect_h_included__

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <stdbool.h>
#include <util/delay.h>
//#include <stdio.h>
#include <string.h>
#include <stddef.h>

#include "printf.h"


//int usb_putchar(char c, FILE *stream);
int usb_putchar(char c);

//static FILE mystdout = FDEV_SETUP_STREAM( usb_putchar, NULL, _FDEV_SETUP_WRITE );

//inline void Log(X)  if (LOG_stateon == VAL_STATE_ON) {  printf(X); }
//#define TM_PRINTF(f_, ...) printf((f_), ##__VA_ARGS__)
//#define PRINTF(...) printf(__VA_ARGS__)
//#define PTF(A,...) fprintf(file,A,##__VA_ARGS__); printf(A,##__VA_ARGS__);
static const char errStr[]  = "E: %d\n";
static const char errHint[]  = "H: %d\n";
static const char errStr2[]  = "E: % W: %d\n";
static const char errHint2[]  = "H: %d W: %d\n";
static const char errHint3[]  = "H: %d W: %d, %d\n";

#define LOG(...) if (LOG_stateon == VAL_STATE_ON) printfss(__VA_ARGS__);
//#define LOG(...)
#define LOGERR  if (LOG_stateon == VAL_STATE_ON) printfss(errStr, __LINE__) 
#define LOGHINT if (LOG_stateon == VAL_STATE_ON) printfss(errHint, __LINE__) 
#define LOGERR2(...)  if (LOG_stateon == VAL_STATE_ON) printfss(errStr2, __LINE__, __VA_ARGS__)
#define LOGHINT2(...) if (LOG_stateon == VAL_STATE_ON) printfss(errHint2, __LINE__, __VA_ARGS__)
#define LOGHINT3(...) if (LOG_stateon == VAL_STATE_ON) printfss(errHint3, __LINE__, __VA_ARGS__)

uchar USBWriteStr(const char* data);
uchar USBWriteStrT(const char* strdata);

void USBDelay_ms(unsigned int milli);

//USB_PUBLIC uchar usbFunctionWrite(uchar *data, uchar len);


#endif  // 


