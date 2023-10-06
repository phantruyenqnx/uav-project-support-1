#ifndef _DEFINE_H
#define _DEFINE_H

//#define MASTER
#define SLAVE

#define BAUDRATE 1000000

#define SLAVE_ID 0x72

#define FR0 0x15
#define FR1 0x25
#define FR2 0x35
#define FR3 0x45
#define FR4 0x57 // dong co
#define FR5 0x65 // led
#define READ 0x17
#define WRITE 0x20

enum
{
    FUNC1, // read sens 1
    FUNC2, // read sens 2 
    FUNC3, // read sens 3
    FUNC4, // read dong co 1
    FUNC5, // read led 1
    TOTAL_FUNC,
};

 #endif
