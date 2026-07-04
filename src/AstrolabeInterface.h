#include "Adafruit_LiquidCrystal.h"
#include "Adafruit_seesaw.h"
#include "src/TimeCalculator.h"
#include "src/Ephemerides.h"
#include <Adafruit_PCF8574.h>
#include <cstdio>
#include <cmath>
#include <array>
#include "src/CircleMath.h"
#include <algorithm>
#include <iterator>
#include "src/MiscHelpers.h"


#define DATETIME 0
#define DATETIME_EDIT_DAY 1
#define DATETIME_EDIT_MONTH 2
#define DATETIME_EDIT_YEAR 3
#define INFO_SUN 4
#define INFO_MOON 5
#define INFO_MERCURY 6
#define INFO_VENUS 7
#define INFO_MARS 8
#define INFO_JUPITER 9
#define INFO_SATURN 10
#define INFO_HOROSCOPE 11
#define SETTINGS 12
#define SETTINGS_EDIT_LONG_TEN 13
#define SETTINGS_EDIT_LONG_ONE 14
#define SETTINGS_EDIT_LONG_DEC 15
#define SETTINGS_EDIT_LAT_TEN 16
#define SETTINGS_EDIT_LAT_ONE 17
#define SETTINGS_EDIT_LAT_DEC 18
#define SETTINGS_EDIT_DST 19
#define SETTINGS_EDIT_UTC 20
#define CALIBRATION 21
#define HOROSCOPE 22
#define INFO_STAR_A 23
#define INFO_STAR_B 24
#define INFO_STAR_C 25
#define INFO_STAR_D 26
#define INFO_STAR_E 27
#define INFO_STAR_F 28
#define INFO_STAR_G 29
#define INFO_STAR_H 30
#define INFO_STAR_I 31
#define INFO_STAR_J 32
#define INFO_STAR_K 33
#define INFO_STAR_L 34

#define SEESAW_ADDR 0x36
#define LCD_ADDR 0x21
#define PCF_ADDR 0X20
#define SS_SWITCH 24

#define ENCODER_INTERRUPT 7
#define BUTTON_INTERRUPT 6

#define RIGHT_BLUE 0
#define RIGHT_GREEN 1
#define RIGHT_RED 2
#define RIGHT_BUTTON 3
#define LEFT_BLUE 4
#define LEFT_GREEN 5
#define LEFT_RED 6
#define LEFT_BUTTON 7

#define MONTH_INTERVAL 31
#define YEAR_INTERVAL 366

#define LAMBDA_SYMBOL 0
#define SUN_SYMBOL 1
#define MOON_SYMBOL 2
#define MERCURY_SYMBOL 3
#define VENUS_SYMBOL 4
#define MARS_SYMBOL 5
#define JUPITER_SYMBOL 6
#define SATURN_SYMBOL 7
#define DEGREE_SYMBOL (char)223
#define BETA_SYMBOL (char)226

#define FIRE 0
#define EARTH 1
#define AIR 2
#define WATER 3

extern const int max_blink_counter;


struct EncoderReturn {
    int date_offset;
    bool settings_changed;
    bool calibration_changed;
};

struct SignLongitude {
    int sign;
    int deg;
    int min;
};

struct RetrogradeInfo {
    bool mercury_r = false;
    bool venus_r = false;
    bool mars_r = false;
    bool jupiter_r = false;
    bool saturn_r = false;
};

struct Settings {
    float latitude;
    float longitude;
    int UTC_offset;
    bool DST;
};

struct InterfaceParams {
    DateTime dt{};
    PlanetData pd{};
    std::array<float, 12> cusps;
    RetrogradeInfo ri;
    std::array<SphereVector, 7> planets_altaz;
    std::array<SphereVector, 12> stars_altaz;
};

class AstrolabeInterface {
    public:
        AstrolabeInterface(InterfaceParams p, Adafruit_PCF8574* _pcf, Settings _settings, float _calibration_k);
        void process(InterfaceParams p);
        EncoderReturn serviceEncoder();
        void serviceButton();
        void initializeLoading();
        Settings getSettings();
        float getCalibration();

    private:
        Adafruit_LiquidCrystal* lcd;
        bool needs_initialize;
        void initializeState(InterfaceParams p);
        void initializeDatetime(InterfaceParams p);
        void initializeInfo(InterfaceParams p);
        void initializeStar(InterfaceParams p);
        void initializeSettings();
        void initializeCalibration();
        void initializeHoroscope(InterfaceParams p);
        InterfaceParams old_p;
        Adafruit_seesaw* ss;
        Adafruit_PCF8574* pcf;
        int32_t old_encoder_position;
        int state;
        bool blink_clk;
        int blink_clk_counter;
        bool year_blank;
        bool month_blank;
        bool day_blank;
        void processDatetime(InterfaceParams p);
        void processInfo(InterfaceParams p);
        void processStar(InterfaceParams p);
        void processSettings();
        void processCalibration();
        void processHoroscope(InterfaceParams p);
        SignLongitude getSignLongitude(float lon);
        bool inHouse(float cusp_a, float cusp_b, float lon);
        float getHouse(std::array<float, 12> cusps, float lon);
        int led_planet;
        int current_led;
        void planetLED(InterfaceParams p);
        bool led_status_indicated;
        Settings settings;
        Settings old_settings;
        bool long_ten_blank;
        bool long_one_blank;
        bool long_dec_blank;
        bool lat_ten_blank;
        bool lat_one_blank;
        bool lat_dec_blank;
        bool DST_blank;
        bool UTC_blank;
        float calibration_k;
        float old_calibration_k;
};