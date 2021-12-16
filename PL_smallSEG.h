/* *****************************************************************************************
PL_smallSEG - A library for Segmented E-Paper displays (EPDs) from Plastic Logic based on 
UC8156 driver IC. The communication is SPI-based, for more information about hook-up please 
check: https://github.com/plasticlogic.

Created by Robert Poser, Dec 16th 2021, Dresden/Germany. Released under BSD license
(3-clause BSD license), check license.md for more information.

We invested time and resources providing this open source code, please support Plasticlogic 
and open source hardware by purchasing this product @Plasticlogic
***************************************************************************************** */
#ifndef PL_smallEPD_h
#define PL_smallEPD_h

#include <SPI.h>

#define EPD_BLACK   0x00
#define EPD_DGRAY   0x55
#define EPD_LGRAY   0xAA
#define EPD_WHITE   0xFF
#define EPD_YELLOW  0x04
#define EPD_GREEN   0x05
#define EPD_RED     0x06
#define EPD_BLUE    0x07

#define EPD_UPD_FULL  0x00            // Triggers a Full update, 4 GL, 800ms
#define EPD_UPD_PART  0x01            // Triggers a Partial update, 4 GL, 800ms
#define EPD_UPD_MONO  0x02            // Triggers a Partial Mono update, 2 GL, 250ms

#define EPD_REVISION          0x00  // Revision, Read only
#define EPD_PANELSETTING      0x01
#define EPD_DRIVERVOLTAGE     0x02
#define EPD_POWERCONTROL      0x03
#define EPD_BOOSTSETTING      0x04  
#define EPD_INTERVALSYNC      0x05
#define EPD_TCOMTIMING        0x06
#define EPD_INTTEMPERATURE    0x07
#define EPD_SETRESOLUTION     0x0C
#define EPD_WRITEPXRECTSET    0x0D
#define EPD_PIXELACESSPOS     0x0E
#define EPD_DATENTRYMODE      0x0F
#define EPD_DISPLAYENGINE     0x14
#define EPD_VCOMCONFIG        0x18
#define EPD_BORDERSETTING     0x1D
#define EPD_POWERSEQUENCE     0x1F
#define EPD_SOFTWARERESET     0x20
#define EPD_PROGRAMMTP        0x40
#define EPD_MTPADDRESSSETTING 0x41
#define EPD_LOADMONOWF        0x44
#define EPD_REGREAD           0x80  

class PL_smallSEG { 

public:
    PL_smallSEG(int8_t _cs, int8_t _rst=-1, int8_t _busy=-1);
    void begin();
    void clear(byte c = EPD_WHITE);
    void clearScreen();
    void invert(bool b2=false);
    void update(int updateMode=EPD_UPD_FULL, bool manPow=false);
    void updateLegio(byte color=EPD_BLACK);
    void writeToPreviousBuffer(byte color=EPD_BLACK);    
    byte buffer[8760];
    void powerOn(void);
    void powerOff(void);
    void writeRegister(uint8_t address, int16_t val1, int16_t val2, int16_t val3, int16_t val4);
    void setTPCOM(int v);
    void setSourceVoltage(int v);
    byte seg_mapping[22] = { 113, 105, 97, 89, 81, 73, 65, 57, 49, 41, 33, 234, 226, 218, 210, 202, 194, 186, 178, 170, 162, 154};
    void drawSegments(long segments, byte color=EPD_BLACK);

private:
    int _buffersize;
    int cs, rst, busy;
    void waitForBusyInactive(void);
    byte readRegister(char address);
    void writeBuffer(bool previous=false);
};

#endif
