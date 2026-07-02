#include "src/PCA9554_Qualia.h"

PCA9554_Qualia::PCA9554_Qualia():expander(){}

void PCA9554_Qualia::begin() {
    expander.begin(QUALIA_EXPANDER_ADDRESS);
    expander.pinMode(TFT_CS, OUTPUT);
    expander.pinMode(TFT_MOSI, OUTPUT);
    expander.pinMode(TFT_RESET, OUTPUT);
    expander.pinMode(TFT_SCK, OUTPUT);
    expander.pinMode(BACKLIGHT, OUTPUT);
    expander.pinMode(BTN_DN, INPUT);
    expander.pinMode(BTN_UP, INPUT);
}

bool PCA9554_Qualia::readUp() {
    return expander.digitalRead(BTN_UP);
}

bool PCA9554_Qualia::readDown() {
    return expander.digitalRead(BTN_DN);
}

void PCA9554_Qualia::setBacklight(bool val) {
    expander.digitalWrite(BACKLIGHT, val);
}

void PCA9554_Qualia::initHD40015C40() {
    //toggle TFT reset pin
    expander.digitalWrite(TFT_RESET, HIGH);
    delay(100);
    expander.digitalWrite(TFT_RESET, LOW);
    delay(100);
    expander.digitalWrite(TFT_RESET, HIGH);
    delay(100);


    //falling edge on CSX enables serial interface 
    expander.digitalWrite(TFT_CS, HIGH);
    expander.digitalWrite(TFT_CS, LOW);
    writeNV3052CCommand(0xFF,0x30); 
    

    //Page 3052 Subpage 01: Main Timing, Power, GIP
    writeNV3052CCommand(0xFF,0x30); writeNV3052CCommand(0xFF,0x52); writeNV3052CCommand(0xFF,0x01); writeNV3052CCommand(0xE3,0x00); writeNV3052CCommand(0x0A,0x11); writeNV3052CCommand(0x23,0xA0); writeNV3052CCommand(0x24,0x32); writeNV3052CCommand(0x25,0x12); writeNV3052CCommand(0x26,0x2E); writeNV3052CCommand(0x27,0x2E); writeNV3052CCommand(0x29,0x02); writeNV3052CCommand(0x2A,0xCF); writeNV3052CCommand(0x32,0x34); writeNV3052CCommand(0x38,0x9C); writeNV3052CCommand(0x39,0xA7); writeNV3052CCommand(0x3A,0x27); writeNV3052CCommand(0x3B,0x94); writeNV3052CCommand(0x42,0x6D); writeNV3052CCommand(0x43,0x83); writeNV3052CCommand(0x81,0x00); writeNV3052CCommand(0x91,0x67); writeNV3052CCommand(0x92,0x67); writeNV3052CCommand(0xA0,0x52); writeNV3052CCommand(0xA1,0x50); writeNV3052CCommand(0xA4,0x9C); writeNV3052CCommand(0xA7,0x02); writeNV3052CCommand(0xA8,0x02); writeNV3052CCommand(0xA9,0x02); writeNV3052CCommand(0xAA,0xA8); writeNV3052CCommand(0xAB,0x28); writeNV3052CCommand(0xAE,0xD2); writeNV3052CCommand(0xAF,0x02); writeNV3052CCommand(0xB0,0xD2); writeNV3052CCommand(0xB2,0x26); writeNV3052CCommand(0xB3,0x26);
    
    //Page 3052 Subpage 02: Analog Gamma Curves
    writeNV3052CCommand(0xFF,0x30); writeNV3052CCommand(0xFF,0x52); writeNV3052CCommand(0xFF,0x02); writeNV3052CCommand(0xB1,0x0A); writeNV3052CCommand(0xD1,0x0E); writeNV3052CCommand(0xB4,0x2F); writeNV3052CCommand(0xD4,0x2D); writeNV3052CCommand(0xB2,0x0C); writeNV3052CCommand(0xD2,0x0C); writeNV3052CCommand(0xB3,0x30); writeNV3052CCommand(0xD3,0x2A); writeNV3052CCommand(0xB6,0x1E); writeNV3052CCommand(0xD6,0x16); writeNV3052CCommand(0xB7,0x3B); writeNV3052CCommand(0xD7,0x35); writeNV3052CCommand(0xC1,0x08); writeNV3052CCommand(0xE1,0x08); writeNV3052CCommand(0xB8,0x0D); writeNV3052CCommand(0xD8,0x0D); writeNV3052CCommand(0xB9,0x05); writeNV3052CCommand(0xD9,0x05); writeNV3052CCommand(0xBD,0x15); writeNV3052CCommand(0xDD,0x15); writeNV3052CCommand(0xBC,0x13); writeNV3052CCommand(0xDC,0x13); writeNV3052CCommand(0xBB,0x12); writeNV3052CCommand(0xDB,0x10); writeNV3052CCommand(0xBA,0x11); writeNV3052CCommand(0xDA,0x11); writeNV3052CCommand(0xBE,0x17); writeNV3052CCommand(0xDE,0x17); writeNV3052CCommand(0xBF,0x0F); writeNV3052CCommand(0xDF,0x0F); writeNV3052CCommand(0xC0,0x16); writeNV3052CCommand(0xE0,0x16); writeNV3052CCommand(0xB5,0x2E); writeNV3052CCommand(0xD5,0x3F); writeNV3052CCommand(0xB0,0x03); writeNV3052CCommand(0xD0,0x02);
    
    //Page 3052 Subpage 03: Digital Gamma, Panel Mapping
    writeNV3052CCommand(0xFF,0x30); writeNV3052CCommand(0xFF,0x52); writeNV3052CCommand(0xFF,0x03); writeNV3052CCommand(0x08,0x09); writeNV3052CCommand(0x09,0x0A); writeNV3052CCommand(0x0A,0x0B); writeNV3052CCommand(0x0B,0x0C); writeNV3052CCommand(0x28,0x22); writeNV3052CCommand(0x2A,0xE9); writeNV3052CCommand(0x2B,0xE9); writeNV3052CCommand(0x34,0x51); writeNV3052CCommand(0x35,0x01); writeNV3052CCommand(0x36,0x26); writeNV3052CCommand(0x37,0x13); writeNV3052CCommand(0x40,0x07); writeNV3052CCommand(0x41,0x08); writeNV3052CCommand(0x42,0x09); writeNV3052CCommand(0x43,0x0A); writeNV3052CCommand(0x44,0x22); writeNV3052CCommand(0x45,0xDB); writeNV3052CCommand(0x46,0xdC); writeNV3052CCommand(0x47,0x22); writeNV3052CCommand(0x48,0xDD); writeNV3052CCommand(0x49,0xDE); writeNV3052CCommand(0x50,0x0B); writeNV3052CCommand(0x51,0x0C); writeNV3052CCommand(0x52,0x0D); writeNV3052CCommand(0x53,0x0E); writeNV3052CCommand(0x54,0x22); writeNV3052CCommand(0x55,0xDF); writeNV3052CCommand(0x56,0xE0); writeNV3052CCommand(0x57,0x22); writeNV3052CCommand(0x58,0xE1); writeNV3052CCommand(0x59,0xE2); writeNV3052CCommand(0x80,0x1E); writeNV3052CCommand(0x81,0x1E); writeNV3052CCommand(0x82,0x1F); writeNV3052CCommand(0x83,0x1F); writeNV3052CCommand(0x84,0x05); writeNV3052CCommand(0x85,0x0A); writeNV3052CCommand(0x86,0x0A); writeNV3052CCommand(0x87,0x0C); writeNV3052CCommand(0x88,0x0C); writeNV3052CCommand(0x89,0x0E); writeNV3052CCommand(0x8A,0x0E); writeNV3052CCommand(0x8B,0x10); writeNV3052CCommand(0x8C,0x10); writeNV3052CCommand(0x8D,0x00); writeNV3052CCommand(0x8E,0x00); writeNV3052CCommand(0x8F,0x1F); writeNV3052CCommand(0x90,0x1F); writeNV3052CCommand(0x91,0x1E); writeNV3052CCommand(0x92,0x1E); writeNV3052CCommand(0x93,0x02); writeNV3052CCommand(0x94,0x04); writeNV3052CCommand(0x96,0x1E); writeNV3052CCommand(0x97,0x1E); writeNV3052CCommand(0x98,0x1F); writeNV3052CCommand(0x99,0x1F); writeNV3052CCommand(0x9A,0x05); writeNV3052CCommand(0x9B,0x09); writeNV3052CCommand(0x9C,0x09); writeNV3052CCommand(0x9D,0x0B); writeNV3052CCommand(0x9E,0x0B); writeNV3052CCommand(0x9F,0x0D); writeNV3052CCommand(0xA0,0x0D); writeNV3052CCommand(0xA1,0x0F); writeNV3052CCommand(0xA2,0x0F); writeNV3052CCommand(0xA3,0x00); writeNV3052CCommand(0xA4,0x00); writeNV3052CCommand(0xA5,0x1F); writeNV3052CCommand(0xA6,0x1F); writeNV3052CCommand(0xA7,0x1E); writeNV3052CCommand(0xA8,0x1E); writeNV3052CCommand(0xA9,0x01); writeNV3052CCommand(0xAA,0x03);  
    
    //Return to Subpage 0, turn the display on.
    writeNV3052CCommand(0xFF,0x30);
    writeNV3052CCommand(0xFF,0x52);
    writeNV3052CCommand(0xFF,0x00);
    writeNV3052CCommand(0x36,0x0A);             
    writeNV3052CCommand(0x11,0x00);
    delay(200); 
    writeNV3052CCommand(0x29,0x00);
    delay(100);


    //Turn off CSX.
    expander.digitalWrite(TFT_CS, HIGH);
}

void PCA9554_Qualia::writeNV3052CCommand(uint8_t command, uint8_t param) {
    //see page 165 of NV3052C datasheet here: https://www.phoenixdisplay.com/wp-content/uploads/2019/05/NV3052C-Datasheet-V0.2.pdf
    //set SDI to low before sending command 
    expander.digitalWrite(TFT_MOSI, LOW);
    toggleClock();
    writeWord(command);
    //set SDI to high before sending param
    expander.digitalWrite(TFT_MOSI, HIGH);
    toggleClock();
    writeWord(param);
}

void PCA9554_Qualia::toggleClock() {
    expander.digitalWrite(TFT_SCK, LOW);
    //delay(10);
    expander.digitalWrite(TFT_SCK, HIGH);
    //delay(10);
    expander.digitalWrite(TFT_SCK, LOW);
    //delay(10);
}

void PCA9554_Qualia::writeWord(uint8_t word) {
    uint8_t bitmask = 0x80;
    for(int i = 0; i<8; i++) {
        expander.digitalWrite(TFT_MOSI, (bitmask & word));
        toggleClock();
        bitmask >>= 1;
    }
}