/* *****************************************************************************************
PL_smallSEG - A library for Segmented E-Paper displays (EPDs) from Plastic Logic based on 
UC8156 driver IC. The communication is SPI-based, for more information about hook-up please 
check: https://github.com/plasticlogic.

Created by Robert Poser, Dec 16th 2021, Dresden/Germany. Released under BSD license
(3-clause BSD license), check license.md for more information.

We invested time and resources providing this open source code, please support Plasticlogic 
and open source hardware by purchasing this product @Plasticlogic
***************************************************************************************** */
#include "PL_smallSEG.h"

PL_smallSEG::PL_smallSEG(int8_t _cs, int8_t _rst, int8_t _busy) { //: Adafruit_GFX(EPD_WIDTH, EPD_HEIGHT) {
    cs      = _cs;
    rst     = _rst;
    busy    = _busy;
}

// PUBLIC

// ******************************************************************************************
// BEGIN - Resetting UC8156 driver IC and configuring all sorts of behind-the-scenes-settings
// By default (WHITEERASE=TRUE) a clear screen update is triggered once to erase the screen.
// ******************************************************************************************
void PL_smallSEG::begin() {
    pinMode(cs, OUTPUT);  
    if (busy!=-1)
        pinMode(busy, INPUT);

    if (rst!=-1) {
        pinMode(rst, OUTPUT);                   //Trigger a global hardware reset...
        digitalWrite(rst, HIGH);  
        delay(5);   
        digitalWrite(rst, LOW);
        delay(5);        
        digitalWrite(rst, HIGH);
        delay(5);        
        waitForBusyInactive();
    } else
        writeRegister(EPD_SOFTWARERESET, -1, -1, -1, -1);    //... or do software reset if no pin defined

    _buffersize=8760;
    writeRegister(EPD_PANELSETTING, 0x10, -1, -1, -1);        
    writeRegister(EPD_WRITEPXRECTSET, 0, 239, 0, 4);  //147?
    writeRegister(EPD_VCOMCONFIG, 0x00, 0x00, 0x24, 0x00);
    writeRegister(EPD_INTERVALSYNC, 0, 0, -1, -1);
    writeRegister(EPD_DRIVERVOLTAGE, 0x25, 0xff, -1, -1);
    writeRegister(EPD_BORDERSETTING, 0x04, -1, -1, -1);
    writeRegister(EPD_LOADMONOWF, 0x60, -1, -1, -1);
    writeRegister(EPD_INTTEMPERATURE, 0x0A, -1, -1, -1);
    writeRegister(EPD_BOOSTSETTING, 0x22, 0x17, -1, -1);
    writeRegister(0x1B, 0, 0, -1, -1);
    clearScreen();                   //Start with a white refresh if TRUE
}

// ************************************************************************************
// CLEAR - Erases the image buffer and triggers an image update and sets the cursor
// back to the origin coordinates (0,0).
// ************************************************************************************
void PL_smallSEG::clear(byte c) {
  switch (c) {
    case EPD_WHITE:
      for (int i=0; i<_buffersize; i++) 
          buffer[i] = 0xff;
      break;
    case EPD_LGRAY:
      for (int i=0; i<_buffersize; i++) 
          buffer[i] = 0xaa;
      break;
    case EPD_DGRAY:
      for (int i=0; i<_buffersize; i++) 
          buffer[i] = 0x55;
      break;
    case EPD_BLACK:
      for (int i=0; i<_buffersize; i++) 
          buffer[i] = 0x00;
      break;
    }
}

// ************************************************************************************
// WHITE ERASE - Triggers two white updates to erase the screen and set back previous
// ghosting. Recommended after each power cycling.
// ************************************************************************************
void PL_smallSEG::clearScreen() {
   for (int i = 0; i < 2; i++) {
        setTPCOM(+15000);
        clear(EPD_BLACK);
        writeToPreviousBuffer(EPD_LGRAY);
        update();
        writeToPreviousBuffer(EPD_LGRAY);
        update();
        writeToPreviousBuffer(EPD_LGRAY);
        update();
        delay(1);
        setTPCOM(-15000);
        clear();
        writeToPreviousBuffer(EPD_LGRAY);
        update();
        writeToPreviousBuffer(EPD_LGRAY);
        update();
        writeToPreviousBuffer(EPD_LGRAY);
        update();
        delay(1);
    }
   clear();
   setTPCOM(0);
}

// ************************************************************************************
// DRAWSEGMENT - 
// ************************************************************************************
void PL_smallSEG::drawSegments(long segments, byte color) {                 
      for (int i=0; i<22; i++)
        if (bitRead(segments, i)) {
            byte s = seg_mapping[i]; 
            byte seg = buffer[s/4];        
            byte new_value;
            switch (s%4) {
              case 0: new_value = (seg & 0x3F) | ((uint8_t)color << 6); break;
              case 1: new_value = (seg & 0xCF) | ((uint8_t)color << 4); break;
              case 2: new_value = (seg & 0xF3) | ((uint8_t)color << 2); break;
              case 3: new_value = (seg & 0xFC) | (uint8_t)color; break;
            }
            buffer[s/4] = new_value;
            buffer[s/4+60] = new_value;
            buffer[s/4+120] = new_value;
            buffer[s/4+180] = new_value;
            buffer[s/4+240] = new_value;    
        }
}


// ************************************************************************************
// INVERT - Inverts the screen content from black to white and vice versa
// ************************************************************************************
void PL_smallSEG::invert(bool b2) {
    for (int i=0; i<_buffersize; i++) 
        buffer[i] = ~buffer[i];
}

// ************************************************************************************
// UPDATE - Triggers an image update based on the content written in the image buffer.
// ************************************************************************************


void PL_smallSEG::update(int updateMode, bool manPow) {
    writeBuffer();
    if (!manPow) 
        powerOn();
    switch (updateMode) {
        case 0:
            writeRegister(EPD_PROGRAMMTP, 0x00, -1, -1, -1);
            writeRegister(EPD_DISPLAYENGINE, 0x03, -1, -1, -1);
            waitForBusyInactive();
            break;
        case 1:
            writeRegister(EPD_PROGRAMMTP, 0x00, -1, -1, -1);
            writeRegister(EPD_DISPLAYENGINE, 0x03, -1, -1, -1);
            waitForBusyInactive();
            break;
        case 2:
            writeRegister(EPD_PROGRAMMTP, 0x02, -1, -1, -1);
            writeRegister(EPD_DISPLAYENGINE, 0x07, -1, -1, -1);
            waitForBusyInactive();
    }
    if (!manPow) 
        powerOff();
}

void PL_smallSEG::updateLegio(byte color) {
    switch (color) {
    case EPD_BLACK: // BLACK & WHITE --> +13V/-13V
        setTPCOM(0);
        setSourceVoltage(13000);
        invert();
        for (int i=0; i<5; i++) {
            writeToPreviousBuffer(EPD_LGRAY);
            update();
            writeToPreviousBuffer(EPD_LGRAY);
            update();   
        }
        invert();
        break;
    case EPD_YELLOW: // YELLOW    --> 0V/+30V
        setTPCOM(+15000);
        setSourceVoltage(15000);
        for (int i = 0; i < 3; i++) {
            writeToPreviousBuffer(EPD_LGRAY);
            update();
        }
        break;
    case EPD_GREEN: 
        setTPCOM(0);
        setSourceVoltage(13000);
        for (int i = 0; i < 3; i++) {
            writeToPreviousBuffer(EPD_WHITE);
            //invert();
            update(EPD_UPD_MONO);
            //invert();
        }
        break;
    case EPD_RED: // RED
        setSourceVoltage(10000);
        for (int i = 0; i < 2; i++) {
            writeToPreviousBuffer(EPD_LGRAY);
            //invert();
            setTPCOM(10000);
            update(EPD_UPD_MONO);
            invert();
            delay(10);
            writeToPreviousBuffer(EPD_LGRAY);
            setTPCOM(-10000);
            update(EPD_UPD_MONO);
            invert();
            delay(10);
        }
        break;/*
    case EPD_BLUE:
        for (int i = 0; i < 2; i++) {
            writeToPreviousBuffer(EPD_LGRAY);
            setTPCOM(-12000);
            setSourceVoltage(12000);
            update(EPD_UPD_MONO);
            delay(1);
            invert();
            writeToPreviousBuffer(EPD_LGRAY);
            setTPCOM(12000);
            setSourceVoltage(12000);
            update(EPD_UPD_MONO);
            invert();
            delay(1);
        }
        setSourceVoltage(12000);
        setTPCOM(-12000);
        for (int i = 0; i < 2; i++) {
            writeToPreviousBuffer(EPD_LGRAY);
            update(EPD_UPD_MONO);
            delay(1);
        }
        break;
    }*/
    case EPD_BLUE: // RED
        setSourceVoltage(10000);
        for (int i = 0; i < 2; i++) {
            writeToPreviousBuffer(EPD_LGRAY);
            //invert();
            setTPCOM(10000);
            update(EPD_UPD_MONO);
            invert();
            delay(10);
            writeToPreviousBuffer(EPD_LGRAY);
            setTPCOM(-10000);
            update(EPD_UPD_MONO);
            invert();
            delay(10);
        } 
        setSourceVoltage(10000);
        setTPCOM(-10000);
        invert();
        for (int i = 0; i < 2; i++) {
            writeToPreviousBuffer(EPD_LGRAY);
            update(EPD_UPD_MONO);
            delay(1);
        }
        break;
}
}

void PL_smallSEG::setSourceVoltage(int v) {
    v = round(v / 1000);
    int x = (v - 8) / 1 * 2 << 4;
    x = x | (v - 8) / 1 * 2;
    writeRegister(EPD_DRIVERVOLTAGE, 0x25, x, -1, -1);
}

void PL_smallSEG::setTPCOM(int v){
    if (v >= 0)
      writeRegister(0x1B, lowByte((v)/30), highByte((v)/30), -1, -1);
    else 
      writeRegister(0x1B, lowByte((v)/30*(-1)), highByte((v)/30*(-1))|4, -1, -1);
}


void PL_smallSEG::writeToPreviousBuffer(byte color){
    writeRegister(EPD_PIXELACESSPOS, 0, 0, -1, -1); 
    writeRegister(EPD_DATENTRYMODE, 0x30, -1, -1, -1);        //Previous buffer @UC8156
    digitalWrite(cs, LOW);
    SPI.transfer(0x10);
    for (int i=0; i < (240*5/4); i++) 
        SPI.transfer(color);
    digitalWrite(cs, HIGH);
    waitForBusyInactive();
}

// PRIVATE

// ************************************************************************************
// POWERON - Activates the defined high voltages needed to update the screen. The
// command should always be called before triggering an image update.
// ************************************************************************************
void PL_smallSEG::powerOn() {
    waitForBusyInactive();
    writeRegister(EPD_SETRESOLUTION, 0, 239, 0, 4);  
    writeRegister(EPD_TCOMTIMING, 0xFF, 0xFF, -1, -1);
    writeRegister(EPD_POWERSEQUENCE, 0x00, 0x00, 0x00, -1);
    writeRegister(EPD_POWERCONTROL, 0x51, -1, -1, -1);
    while (readRegister(0x15) == 0) {}          // Wait until Internal Pump is ready    
}

// ************************************************************************************
// POWEROFF - Deactivates the high voltages needed to update the screen. The
// command should always be called after triggering an image update.
// ************************************************************************************
void PL_smallSEG::powerOff() {
    writeRegister(EPD_POWERCONTROL, 0x10, -1, -1, -1);
    waitForBusyInactive();
    writeRegister(EPD_POWERCONTROL, 0xC0, -1, -1, -1);
    waitForBusyInactive();
}

// ************************************************************************************
// WRITEBUFFER - Sends the content of the memory buffer to the UC8156 driver IC.
// ************************************************************************************
void PL_smallSEG::writeBuffer(bool previous){
    writeRegister(EPD_PIXELACESSPOS, 0, 0, -1, -1); 
    if (previous)
        writeRegister(EPD_DATENTRYMODE, 0x30, -1, -1, -1);        //Previous buffer @UC8156
    else
        writeRegister(EPD_DATENTRYMODE, 0x20, -1, -1, -1);        
    
    digitalWrite(cs, LOW);
    SPI.transfer(0x10);
    for (int i=0; i < (240*5/4); i++) 
            SPI.transfer(buffer[i]);
    digitalWrite(cs, HIGH);
    waitForBusyInactive();
}


// ************************************************************************************
// WRITE REGISTER - Sets register ADDRESS to value VAL1 (optional: VAL2, VAL3, VAL4)
// ************************************************************************************
void PL_smallSEG::writeRegister(uint8_t address, int16_t val1, int16_t val2, 
    int16_t val3, int16_t val4) {
    digitalWrite(cs, LOW);
    SPI.transfer(address);
    if (val1!=-1) SPI.transfer((byte)val1);
    if (val2!=-1) SPI.transfer((byte)val2);
    if (val3!=-1) SPI.transfer((byte)val3);
    if (val4!=-1) SPI.transfer((byte)val4);
    digitalWrite(cs, HIGH);
    waitForBusyInactive();
}

// ************************************************************************************
// READREGISTER - Returning the value of the register at the specified address
// ************************************************************************************
byte PL_smallSEG::readRegister(char address){
    byte data;
    digitalWrite(cs, LOW);                                      
    SPI.transfer(address | EPD_REGREAD);
    data = SPI.transfer(0xFF);                         
    digitalWrite(cs, HIGH);
    waitForBusyInactive();
    return data;                                        // can be improved
}


// ************************************************************************************
// WAITFORBUSYINACTIVE - Sensing to ‘Busy’ pin to detect the UC8156 driver status.
// Function returns only after driver IC is free again for listening to new commands.
// ************************************************************************************
void PL_smallSEG::waitForBusyInactive(){
    while (digitalRead(busy) == LOW) {}
}
