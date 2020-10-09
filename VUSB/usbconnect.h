
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

#define LOGSUPPORT

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define LINENB TOSTRING(__LINE__)

//int usb_putchar(char c, FILE *stream);
//static FILE mystdout = FDEV_SETUP_STREAM( usb_putchar, NULL, _FDEV_SETUP_WRITE );

//inline void Log(X)  if (LOG_stateon == VAL_STATE_ON) {  printf(X); }
//#define TM_PRINTF(f_, ...) printf((f_), ##__VA_ARGS__)
//#define PRINTF(...) printf(__VA_ARGS__)
//#define PTF(A,...) fprintf(file,A,##__VA_ARGS__); printf(A,##__VA_ARGS__);

#ifdef LOGSUPPORT
#define LOGHINTSHOW
//#define LOGERRSHOW

//static const char logtestStr[]  PROGMEM = "%s:%s\n";
//static const char logStr[]  PROGMEM = "%s\n";
#ifdef LOGERRSHOW
static const char errStr[]  PROGMEM = "E:%s\n";
static const char errStr2[]  PROGMEM = "E:%s %d\n";
#endif
#ifdef LOGHINTSHOW
static const char logHint[]   = "H:%s\n";
static const char logHint2[]   = "H:%s %d\n";
//static const char logHint3[] PROGMEM  = "H:%s W:%d, %d\n";
#endif

//#define LOGTEST(...) printfss(logtestStr, ATT, __VA_ARGS__);
//#define LOGTEST(...) // printfss(logtestStr, LINENB, __VA_ARGS__);
//#define LOG(...) //printfss(logStr, __VA_ARGS__);   // Log Text
//#define LOG(...) if (LOG_state == VAL_LOG_ON) printfss(logStr, __VA_ARGS__);   // Log Text

#ifdef LOGERRSHOW
#define LOGERR  if (LOG_state == VAL_LOG_ON) printfss(errStr, LINENB) 
#define LOGERR2(...) if (LOG_state == VAL_LOG_ON) printfss(errStr2, LINENB, __VA_ARGS__)
#else
#define LOGERR  
#define LOGERR2(...)  
#endif

#ifdef LOGHINTSHOW
#define LOGHINT if (LOG_state == VAL_LOG_ON) printfss(logHint, LINENB) //if (LOG_state == VAL_LOG_ON) printfss(errHint, LINENB) 
#define LOGHINT2(...) if (LOG_state == VAL_LOG_ON) printfss(logHint2, LINENB, __VA_ARGS__)
//#define LOGHINT3(...) //if (LOG_state == VAL_LOG_ON) printfss(errHint3, LINENB, __VA_ARGS__)
#else
#define LOGHINT
#define LOGHINT2(...)
#endif
//uchar USBWriteStr(const char* data);
//uchar USBWriteStrT(const char* strdata);
#else
#define LOGERR
#define LOGERR2(...)
#define LOGHINT 
#define LOGHINT2(...) 
#endif

void USBDelay_ms(unsigned int milli);

//USB_PUBLIC uchar usbFunctionWrite(uchar *data, uchar len);


#endif  // 


