#include <Adafruit_XCA9554.h>
#define TFT_SCK 0
#define TFT_CS 1
#define TFT_MOSI 7
#define BACKLIGHT 4
#define TFT_RESET 2
#define BTN_DN 5
#define BTN_UP 6
#define QUALIA_EXPANDER_ADDRESS 0X3F


class PCA9554_Qualia {
public:
    PCA9554_Qualia();
    void initHD40015C40();
    void begin();
    bool readUp();
    bool readDown();
    void setBacklight(bool val);

private:
    Adafruit_XCA9554 expander;
    void writeNV3052CCommand(uint8_t command, uint8_t param);
    void toggleClock();
    void writeWord(uint8_t word);
};

