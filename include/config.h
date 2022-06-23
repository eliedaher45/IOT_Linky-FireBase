#pragma once 

#define DEBUG 0
#define POST 0

#define rxPin D3
#define txPin D4

#define ticRx D2
#define ticTx D8

#define START_HOUR 17
#define END_HOUR 8

#define SERIAL Serial
#define FORMAT SERIAL_7E1

#define LD2 PA5
#define BTN_MODE D5
#define B1 PC13
#define LD_H A5
#define LD_S A4

#define WAIT_S 10  // in seconds //
#define BOOT_WAIT 5000
#define BUFF_SIZE 1000


#if DEBUG == 1
#define debug(x) Serial.print(x)
#define debugln(x) Serial.println(x)
#else
#define debug(x)
#define debugln(x)
#endif