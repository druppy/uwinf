#ifndef __PORTTYPE_H
#define __PORTTYPE_H

//character types
typedef unsigned char   uchar;
typedef signed char     schar;

//fixed-bitwidth integral types
typedef unsigned char   uint8;
typedef signed char     sint8;

typedef unsigned short  uint16;
typedef short           sint16;

typedef unsigned long   uint32;
typedef long            sint32;

#define MAKEUINT16(high,low)    ((((uint16)(high))<<8)|((uint8)(low)))
#define MAKEUINT32(high,low)    ((((uint32)(high))<<16)|((uint16)(low)))
#define LOWUINT8(s)             ((uint8)(s))
#define HIGHUINT8(s)            ((uint8)((s)>>8))
#define LOWUINT16(l)            ((uint16)(l))
#define HIGHUINT16(l)           ((uint16)((l)>>16))

#endif
