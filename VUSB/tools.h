/*
 * tools.h
 *
 * Created: 20.09.2019 20:55:23
 *  Author: helge
 */ 


#ifndef TOOLS_H_
#define TOOLS_H_

#define _SET(type,name,bit)          type ## name  |= _BV(bit)
#define _CLEAR(type,name,bit)        type ## name  &= ~ _BV(bit)
#define _TOGGLE(type,name,bit)       type ## name  ^= _BV(bit)
#define _GET(type,name,bit)          ((type ## name >> bit) &  1)
#define _PUT(type,name,bit,value)    type ## name = ( type ## name & ( ~ _BV(bit)) ) | ( ( 1 & (unsigned char)value ) << bit )
#define _BIT(name,bit)               (bit)

//these macros are used by end user
#define OUTPUT_SET(pin)         _SET(DDR,pin)
#define INPUT_SET(pin)          _CLEAR(DDR,pin)
#define HIGH_SET(pin)           _SET(PORT,pin)
#define LOW_SET(pin)            _CLEAR(PORT,pin)
#define TOGGLE_SET(pin)         _TOGGLE(PORT,pin)
#define READ_PIN(pin)           _GET(PIN,pin)
#define BIT_GET(pin)            _BIT(pin)

#define FIREBIT             BIT_GET(FIRE)
#define SIGNALBIT           BIT_GET(SIGNAL)




#endif /* TOOLS_H_ */