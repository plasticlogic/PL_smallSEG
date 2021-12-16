/* *****************************************************************************************
PL_smallSEG - Demo for Segmented Color E-Paper displays (EPDs) from Plastic Logic based on 
UC8156 driver IC. The communication is SPI-based, for more information about hook-up please 
check: https://github.com/plasticlogic.

Created by Robert Poser, Dec 16th 2021, Dresden/Germany. Released under BSD license
(3-clause BSD license), check license.md for more information.

We invested time and resources providing this open source code, please support Plasticlogic 
and open source hardware by purchasing this product @Plasticlogic
***************************************************************************************** */
#include "PL_smallSEG.h"          // Display-specific library for 4pigment media (Legio, 6colors)

#define EPD_RST     12
#define EPD_BUSY    9
#define EPD_CS      5
                                    // Declare 4pigment based EPD instance with 6 colors
PL_smallSEG epd(EPD_CS, EPD_RST, EPD_BUSY);   

void setup() {  
    SPI.begin();                    // SPI initialisation, UC8156 supp. max 10Mhz (writing) & 
    SPI.beginTransaction(SPISettings(6000000, MSBFIRST, SPI_MODE0));      // 6.6Mhz (reading)

    epd.begin();                    // EPD initialisation & ClearScreen
    
    epd.drawSegments(0b010101010101010101010101);
    epd.updateLegio(EPD_BLACK);
    
    epd.clear();
    epd.drawSegments(0b000110111001101111100010);
    epd.updateLegio(EPD_YELLOW);
   
    epd.clear();
    epd.drawSegments(0b000100000001000001000000);
    epd.updateLegio(EPD_GREEN);

    epd.clear();
    epd.drawSegments(0b001000000010000000001000);
    epd.updateLegio(EPD_RED);

    epd.clear();
    epd.drawSegments(0b000000101000000000000010);
    epd.updateLegio(EPD_BLUE);
}

void loop() {  
}
