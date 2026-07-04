#include "src/AstrolabeInterface.h"

const char* week_names[7] = {"SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"};
const char* sign_names[12] = {"ARI", "TAU", "GEM", "CAN", "LEO", "VIR", "LIB", "SCO", "SAG", "CAP", "AQU", "PIS"};
const char* sign_names_long[12] = {
    "ARIES      ",
    "TAURUS     ",
    "GEMINI     ",
    "CANCER     ",
    "LEO        ",
    "VIRGO      ",
    "LIBRA      ",
    "SCORPIO    ",
    "SAGITTARIUS",
    "CAPRICORN  ",
    "AQUARIUS   ",
    "PISCES     "
};
const char* sign_descriptions[12] = {
    "       THE RAM      ",
    "      THE BULL      ",
    "      THE TWINS     ",
    "      THE CRAB      ",
    "      THE LION      ",
    "     THE MAIDEN     ",
    "     THE SCALES     ",
    "    THE SCORPION    ",
    "     THE ARCHER     ",
    "    THE SEA-GOAT    ",
    "  THE WATER-BEARER  ",
    "     THE FISHES     "
};

const char* planet_names[7] = {
    "  SUN", 
    " MOON", 
    "MERCURY", 
    " VENUS", 
    " MARS", 
    "JUPITER", 
    "SATURN"};

const char* star_names[12] = {
    "       ALGOL        ",
    "       RIGEL        ",
    "      CAPELLA       ",
    "     BETELGEUSE     ",
    "       SIRIUS       ",
    "      PROCYON       ",
    "       DUBHE        ",
    "       ALKAID       ",
    "      ARCTURUS      ",
    "        VEGA        ",
    "       ALTAIR       ",
    "       DENEB        "
};

const char* star_descriptions[12] = {
    "  THE DEMON'S HEAD  ",
    " THE HUNTER'S FOOT  ",
    "    THE SHE-GOAT    ",
    " THE HUNTER'S HAND  ",
    "  THE SEARING ONE   ",
    "   THE LITTLE DOG   ",
    "  THE BEAR'S HEAD   ",
    "  THE BEAR'S TAIL   ",
    "    THE GUARDIAN    ",
    "  THE DIVING EAGLE  ",
    " THE SOARING EAGLE  ",
    "      THE SWAN      "
};

const int max_blink_counter = 1;

const float sign_interval = 30.0;

const int buffer_size = 100;

uint8_t lambda_symbol[8] = {0,8,4,4,10,10,17,17};
uint8_t sun_symbol[8] = {14,17,21,17,14,0,0,0};
uint8_t moon_symbol[8] = {12,18,9,5,5,9,18,12};
uint8_t mercury_symbol[8] = {10,4,14,10,14,4,14,4};
uint8_t venus_symbol[8] = {14,10,14,4,14,4,4,0};
uint8_t mars_symbol[8] = {0,7,3,5,12,18,18,12};
uint8_t jupiter_symbol[8] = {8,4,4,4,8,18,31,2};
uint8_t saturn_symbol[8] = {8,30,8,10,13,9,1,2};

AstrolabeInterface::AstrolabeInterface(InterfaceParams p, Adafruit_PCF8574* _pcf, Settings _settings, float _calibration_k) {
    lcd = new Adafruit_LiquidCrystal(LCD_ADDR);
    lcd->begin(20, 4);
    lcd->createChar(LAMBDA_SYMBOL, lambda_symbol);
    lcd->createChar(SUN_SYMBOL, sun_symbol);
    lcd->createChar(MOON_SYMBOL, moon_symbol);
    lcd->createChar(MERCURY_SYMBOL, mercury_symbol);
    lcd->createChar(VENUS_SYMBOL, venus_symbol);
    lcd->createChar(MARS_SYMBOL, mars_symbol);
    lcd->createChar(JUPITER_SYMBOL, jupiter_symbol);
    lcd->createChar(SATURN_SYMBOL, saturn_symbol);

    lcd->clear();
    lcd->setBacklight(HIGH);

    ss = new Adafruit_seesaw();
    ss->begin(SEESAW_ADDR);
    ss->pinMode(SS_SWITCH, INPUT_PULLUP);
    ss->setGPIOInterrupts((uint32_t)1 << SS_SWITCH, 1);
    ss->enableEncoderInterrupt();

    pcf = _pcf;
    settings = _settings;
    old_settings = settings;
    calibration_k = _calibration_k;
    old_calibration_k = calibration_k;

    pcf->pinMode(RIGHT_RED, OUTPUT);
    pcf->digitalWrite(RIGHT_RED, HIGH);
    pcf->pinMode(RIGHT_GREEN, OUTPUT);
    pcf->digitalWrite(RIGHT_GREEN, HIGH);
    pcf->pinMode(RIGHT_BLUE, OUTPUT);
    pcf->digitalWrite(RIGHT_BLUE, HIGH);
    pcf->pinMode(LEFT_RED, OUTPUT);
    pcf->digitalWrite(LEFT_RED, HIGH);
    pcf->pinMode(LEFT_GREEN, OUTPUT);
    pcf->digitalWrite(LEFT_GREEN, HIGH);
    pcf->pinMode(LEFT_BLUE, OUTPUT);
    pcf->digitalWrite(LEFT_BLUE, HIGH);
    pcf->pinMode(LEFT_BUTTON, INPUT);
    pcf->pinMode(RIGHT_BUTTON, INPUT);
    current_led = AIR;
    led_planet = INFO_SUN;
    old_encoder_position = ss->getEncoderPosition();

    state = DATETIME;
    needs_initialize = true;

    blink_clk = false;
    blink_clk_counter = 0;

    led_status_indicated = false;

    this->initializeLoading();

    old_p = p;
}

void AstrolabeInterface::initializeLoading() {
    lcd->setCursor(0,1);
    lcd->print("     LOADING...     ");
}

void AstrolabeInterface::initializeDatetime(InterfaceParams p) {
    lcd->setCursor(5,1);
    lcd->print(p.dt.year);
    lcd->setCursor(9,1);
    lcd->print("/");
    lcd->setCursor(10,1);
    lcd->print(p.dt.month / 10);
    lcd->setCursor(11,1);
    lcd->print(p.dt.month % 10);
    lcd->setCursor(12,1);
    lcd->print("/");   
    lcd->setCursor(13,1);
    lcd->print(p.dt.day / 10);
    lcd->setCursor(14,1);
    lcd->print(p.dt.day % 10);

    lcd->setCursor(10,2);
    lcd->print(p.dt.hour / 10);
    lcd->setCursor(11,2);
    lcd->print(p.dt.hour % 10);
    lcd->setCursor(12,2);
    lcd->print(":");
    lcd->setCursor(13,2);
    lcd->print(p.dt.minute / 10);
    lcd->setCursor(14,2);
    lcd->print(p.dt.minute % 10);

    lcd->setCursor(9, 2);
    if (p.dt.DST) {
        lcd->print("*");
    }

    lcd->setCursor(5, 2);
    int week_day = getDayOfWeek(p.dt.year, p.dt.month, p.dt.day);
    lcd->print(week_names[week_day]);
    old_p = p;
}

void AstrolabeInterface::initializeInfo(InterfaceParams p) {
    int planet_n = state - INFO_SUN;
    SphereVector latlong_ecl;
    bool retrograde = false;
    SphereVector altaz;
    switch (state) {
        case INFO_SUN:
            latlong_ecl = p.pd.sun_ecl_latlong;
            altaz = p.planets_altaz[0];
            break;
        case INFO_MOON:
            latlong_ecl = p.pd.moon_ecl_latlong;
            altaz = p.planets_altaz[1];
            break;
        case INFO_MERCURY:
            latlong_ecl = p.pd.mercury_ecl_latlong;
            retrograde = p.ri.mercury_r;
            altaz = p.planets_altaz[2];
            break;
        case INFO_VENUS:
            latlong_ecl = p.pd.venus_ecl_latlong;
            retrograde = p.ri.venus_r;
            altaz = p.planets_altaz[3];
            break;
        case INFO_MARS:
            latlong_ecl = p.pd.mars_ecl_latlong;
            retrograde = p.ri.mars_r;
            altaz = p.planets_altaz[4];
            break;
        case INFO_JUPITER:
            latlong_ecl = p.pd.jupiter_ecl_latlong;
            retrograde = p.ri.jupiter_r;
            altaz = p.planets_altaz[5];
            break;
        case INFO_SATURN:
            latlong_ecl = p.pd.saturn_ecl_latlong;
            retrograde = p.ri.saturn_r;
            altaz = p.planets_altaz[6];
            break;
    }
    SignLongitude sl = getSignLongitude(latlong_ecl.lon);
    float ecl_lat_f = latlong_ecl.lat * 180.0 / std::numbers::pi;
    int ecl_lat = ecl_lat_f;
    ecl_lat = ecl_lat % 10;
    int ecl_lat_min = (int) std::abs(std::fmod(ecl_lat_f, 1.0) * 60.0);
    lcd->setCursor(1, 0);
    lcd->print(planet_names[planet_n]);
    lcd->setCursor(9, 0);
    lcd->write(planet_n + 1);
    if (retrograde) {
        lcd->setCursor(10, 0);
        lcd->print("R");
    }
    lcd->setCursor(14, 0);
    lcd->print(".");
    lcd->setCursor(17,0);
    lcd->print("H");

    float house = getHouse(p.cusps, latlong_ecl.lon);
    int house_i = (int) std::floor(house);
    int house_d = (int) (std::fmod(house, 1.0) * 100.0);
    lcd->setCursor(12, 0);
    lcd->print(house_i / 10);
    lcd->setCursor(13, 0);
    lcd->print(house_i % 10);
    lcd->setCursor(15, 0);
    lcd->print(house_d / 10);
    lcd->setCursor(16, 0);
    lcd->print(house_d % 10);

    lcd->setCursor(0, 1);
    lcd->write(LAMBDA_SYMBOL);
    lcd->setCursor(1, 1);
    lcd->print(":");
    lcd->setCursor(2, 1);
    lcd->print(sign_names[sl.sign]);
    lcd->setCursor(5, 1);
    lcd->print(sl.deg / 10);
    lcd->setCursor(6, 1);
    lcd->print(sl.deg % 10);
    lcd->setCursor(7, 1);
    lcd->print(DEGREE_SYMBOL);
    lcd->setCursor(8, 1);
    lcd->print(sl.min / 10);
    lcd->setCursor(9, 1);
    lcd->print(sl.min % 10);
    lcd->setCursor(10, 1);
    lcd->print("'");
    lcd->setCursor(12, 1);
    lcd->print(BETA_SYMBOL);
    lcd->setCursor(13, 1);
    lcd->print(":");
    lcd->setCursor(14, 1);
    lcd->print(ecl_lat<0 ? "-" : " ");
    lcd->setCursor(15, 1);
    lcd->print(std::abs(ecl_lat));
    lcd->setCursor(16, 1);
    lcd->print(DEGREE_SYMBOL);
    lcd->setCursor(17, 1);
    lcd->print(ecl_lat_min / 10);
    lcd->setCursor(18, 1);
    lcd->print(ecl_lat_min % 10);
    lcd->setCursor(19, 1);
    lcd->print("'");

    int alt = altaz.lat * 180.0 / std::numbers::pi;
    int az = getNormalizedAngle(-(altaz.lon + std::numbers::pi/2)) * 180.0 / std::numbers::pi;
    lcd->setCursor(2, 2);
    lcd->print("ALT:");
    if (alt < 0) {
        lcd->setCursor(6, 2);
        lcd->print("-");
    }
    lcd->setCursor(7, 2);
    lcd->print(std::abs(alt / 10));
    lcd->setCursor(8, 2);
    lcd->print(std::abs(alt % 10));
    lcd->setCursor(11, 2);
    lcd->print("AZ:");
    lcd->setCursor(14, 2);
    lcd->print(az / 100);
    lcd->setCursor(15, 2);
    lcd->print((az / 10) % 10);
    lcd->setCursor(16, 2);
    lcd->print(az % 10);
    lcd->setCursor(9, 2);
    lcd->print(DEGREE_SYMBOL);
    lcd->setCursor(17, 2);
    lcd->print(DEGREE_SYMBOL);

    if (led_planet == state) {
        lcd->setCursor(19,0);
        lcd->print("*");
        led_status_indicated = true;
    }
}

void AstrolabeInterface::initializeStar(InterfaceParams p) {
    int star_n = state - INFO_STAR_A;
    SphereVector altaz = p.stars_altaz[star_n];

    lcd->setCursor(0, 0);
    lcd->print(star_names[star_n]);
    lcd->setCursor(0, 1);
    lcd->print(star_descriptions[star_n]);

    int alt = altaz.lat * 180.0 / std::numbers::pi;
    int az = getNormalizedAngle(-(altaz.lon + std::numbers::pi/2)) * 180.0 / std::numbers::pi;
    lcd->setCursor(2, 2);
    lcd->print("ALT:");
    if (alt < 0) {
        lcd->setCursor(6, 2);
        lcd->print("-");
    }
    lcd->setCursor(7, 2);
    lcd->print(std::abs(alt / 10));
    lcd->setCursor(8, 2);
    lcd->print(std::abs(alt % 10));
    lcd->setCursor(11, 2);
    lcd->print("AZ:");
    lcd->setCursor(14, 2);
    lcd->print(az / 100);
    lcd->setCursor(15, 2);
    lcd->print((az / 10) % 10);
    lcd->setCursor(16, 2);
    lcd->print(az % 10);
    lcd->setCursor(9, 2);
    lcd->print(DEGREE_SYMBOL);
    lcd->setCursor(17, 2);
    lcd->print(DEGREE_SYMBOL);
}

void AstrolabeInterface::initializeSettings() {
    lcd->setCursor(1, 0);
    lcd->print("LONGITUDE:    .");
    lcd->setCursor(2, 1);
    lcd->print("LATITUDE:    .");
    lcd->setCursor(1, 2);
    lcd->setCursor(17, 0);
    lcd->print(DEGREE_SYMBOL);
    lcd->setCursor(17, 1);
    lcd->print(DEGREE_SYMBOL);
    lcd->setCursor(18, 1);
    lcd->print("N");


    lcd->setCursor(1, 2);
    lcd->print("DST:");
    lcd->setCursor(8, 2);
    lcd->print("TIME: UTC");

    int longitude_i = std::abs(settings.longitude);
    int longitude_d = std::abs(std::fmod(settings.longitude, 1.0) * 10);
    int latitude_i = settings.latitude;
    int latitude_d = std::fmod(settings.latitude, 1.0) * 10;
    lcd->setCursor(12, 0);
    lcd->print(longitude_i / 100);
    lcd->setCursor(13, 0);
    lcd->print((longitude_i / 10) % 10);
    lcd->setCursor(14, 0);
    lcd->print(longitude_i % 10);
    lcd->setCursor(16, 0);
    lcd->print(longitude_d);
    lcd->setCursor(18, 0);
    lcd->print(settings.longitude > 0 ? "E" : "W");

    lcd->setCursor(13, 1);
    lcd->print(latitude_i / 10);
    lcd->setCursor(14, 1);
    lcd->print(latitude_i % 10);
    lcd->setCursor(16, 1);
    lcd->print(latitude_d);

    lcd->setCursor(6, 2);
    lcd->print(settings.DST ? "Y" : "N");
    lcd->setCursor(17, 2);
    lcd->print(settings.UTC_offset >= 0 ? "+" : "-");
    lcd->setCursor(18, 2);
    lcd->print(std::abs(settings.UTC_offset / 10));
    lcd->setCursor(19, 2);
    lcd->print(std::abs(settings.UTC_offset % 10));
}

void AstrolabeInterface::initializeCalibration() {
    int k_ones = calibration_k >= 1 ? 1 : 0;
    int k_dec = calibration_k >= 1 ? (calibration_k - 1) * 1000.0 : calibration_k * 1000.0;
    lcd->setCursor(5, 0);
    lcd->print("CALIBRATION");
    lcd->setCursor(6, 1);
    lcd->print("K:  .");
    lcd->setCursor(9, 1);
    lcd->print(k_ones);
    lcd->setCursor(11, 1);
    lcd->print(k_dec / 100);
    lcd->setCursor(12, 1);
    lcd->print((k_dec / 10) % 10);
    lcd->setCursor(13, 1);
    lcd->print(k_dec % 10);
}

void AstrolabeInterface::initializeHoroscope(InterfaceParams p) {
    float horoscope = p.cusps[11];
    SignLongitude sl = getSignLongitude(horoscope);
    lcd->setCursor(0, 0);
    lcd->print("     HOROSCOPUS     ");
    lcd->setCursor(15, 1);
    lcd->print(DEGREE_SYMBOL);
    lcd->setCursor(18, 1);
    lcd->print("'");

    lcd->setCursor(1, 1);
    lcd->print(sign_names_long[sl.sign]);
    lcd->setCursor(0, 2);
    lcd->print(sign_descriptions[sl.sign]);

    lcd->setCursor(13, 1);
    lcd->print(sl.deg / 10);
    lcd->setCursor(14, 1);
    lcd->print(sl.deg % 10);
    lcd->setCursor(16, 1);
    lcd->print(sl.min / 10);
    lcd->setCursor(17, 1);
    lcd->print(sl.min % 10);

    old_p = p;
}


void AstrolabeInterface::initializeState(InterfaceParams p) {
    lcd->clear();

    switch(state) {
        case DATETIME:
            initializeDatetime(p);
            break;
        case INFO_SUN:
        case INFO_MOON:
        case INFO_MERCURY:
        case INFO_VENUS:
        case INFO_MARS:
        case INFO_JUPITER:
        case INFO_SATURN: 
            initializeInfo(p);
            break;
        case SETTINGS:
            initializeSettings();
            break;
        case CALIBRATION:
            initializeCalibration();
            break;
        case HOROSCOPE:
            initializeHoroscope(p);
            break;
        case INFO_STAR_A:
        case INFO_STAR_B:
        case INFO_STAR_C:
        case INFO_STAR_D:
        case INFO_STAR_E:
        case INFO_STAR_F:
        case INFO_STAR_G:
        case INFO_STAR_H:
        case INFO_STAR_I:
        case INFO_STAR_J:
        case INFO_STAR_K:
        case INFO_STAR_L:
            initializeStar(p);
            break;
    }

}

void AstrolabeInterface::processHoroscope(InterfaceParams p) {
    float horoscope = p.cusps[11];
    float old_horoscope = old_p.cusps[11];
    SignLongitude sl = getSignLongitude(horoscope);
    SignLongitude old_sl = getSignLongitude(old_horoscope);

    if (sl.sign != old_sl.sign) {
        lcd->setCursor(1, 1);
        lcd->print(sign_names_long[sl.sign]);
        lcd->setCursor(0, 2);
        lcd->print(sign_descriptions[sl.sign]);
    }

    if (sl.deg != old_sl.deg) {
        lcd->setCursor(13, 1);
        lcd->print(sl.deg / 10);
        lcd->setCursor(14, 1);
        lcd->print(sl.deg % 10);
    }

    if (sl.min != old_sl.min) {
        lcd->setCursor(16, 1);
        lcd->print(sl.min / 10);
        lcd->setCursor(17, 1);
        lcd->print(sl.min % 10);
    }
    old_p = p;

}

void AstrolabeInterface::processSettings() {
    int longitude_i = std::abs(settings.longitude);
    int old_longitude_i = std::abs(old_settings.longitude);
    int longitude_d = std::abs(std::fmod(settings.longitude, 1.0) * 10);
    int latitude_i = settings.latitude;
    int latitude_d = std::fmod(settings.latitude, 1.0) * 10;

    blink_clk_counter++;
    bool blink_restore = false;
    if (blink_clk_counter >= max_blink_counter) {
        blink_clk_counter = 0;
        blink_clk = !blink_clk;
        if (!blink_clk) blink_restore = true;
    }

    bool blink_long_ten = blink_clk && state == SETTINGS_EDIT_LONG_TEN;
    bool blink_long_one = blink_clk && state == SETTINGS_EDIT_LONG_ONE;
    bool blink_long_dec = blink_clk && state == SETTINGS_EDIT_LONG_DEC;

    bool blink_lat_ten = blink_clk && state == SETTINGS_EDIT_LAT_TEN;
    bool blink_lat_one = blink_clk && state == SETTINGS_EDIT_LAT_ONE;
    bool blink_lat_dec = blink_clk && state == SETTINGS_EDIT_LAT_DEC;

    bool blink_DST = blink_clk && state == SETTINGS_EDIT_DST;
    bool blink_UTC = blink_clk && state == SETTINGS_EDIT_UTC;

    bool long_ten_restore = blink_restore && long_ten_blank;
    bool long_one_restore = blink_restore && long_one_blank;
    bool long_dec_restore = blink_restore && long_dec_blank;

    bool lat_ten_restore = blink_restore && lat_ten_blank;
    bool lat_one_restore = blink_restore && lat_one_blank;
    bool lat_dec_restore = blink_restore && lat_dec_blank;

    bool DST_restore = blink_restore && DST_blank;    
    bool UTC_restore = blink_restore && UTC_blank;


    if (lat_ten_blank && state != SETTINGS_EDIT_LAT_TEN) lat_ten_restore = true;
    if (lat_one_blank && state != SETTINGS_EDIT_LAT_ONE) lat_one_restore = true;
    if (lat_dec_blank && state != SETTINGS_EDIT_LAT_DEC) lat_dec_restore = true;

    if (long_ten_blank && state != SETTINGS_EDIT_LONG_TEN) long_ten_restore = true;
    if (long_one_blank && state != SETTINGS_EDIT_LONG_ONE) long_one_restore = true;
    if (long_dec_blank && state != SETTINGS_EDIT_LONG_DEC) long_dec_restore = true;

    if (DST_blank && state != SETTINGS_EDIT_DST) DST_restore = true;
    if (UTC_blank && state != SETTINGS_EDIT_UTC) UTC_restore = true;

    if (blink_lat_ten) {
        lcd->setCursor(13, 1);
        lcd->print(" ");
        lat_ten_blank = true;
    }
    if (blink_lat_one) {
        lcd->setCursor(14, 1);
        lcd->print(" ");
        lat_one_blank = true;
    }
    if (blink_lat_dec) {
        lcd->setCursor(16, 1);
        lcd->print(" ");
        lat_dec_blank = true;
    }
    if (blink_long_ten) {
        lcd->setCursor(13, 0);
        lcd->print(" ");
        long_ten_blank = true;
    }
    if (blink_long_one) {
        lcd->setCursor(14, 0);
        lcd->print(" ");
        long_one_blank = true;
    }
    if (blink_long_dec) {
        lcd->setCursor(16, 0);
        lcd->print(" ");
        long_dec_blank = true;
    }
    if (blink_DST) {
        lcd->setCursor(6, 2);
        lcd->print(" ");
        DST_blank = true;
    }
    if (blink_UTC) {
        lcd->setCursor(18, 2);
        lcd->print("  ");
        UTC_blank = true;
    }

    if (longitude_i / 100 != old_longitude_i / 100) {
        lcd->setCursor(12, 0);
        lcd->print(longitude_i / 100);
    }

    if ((settings.DST != old_settings.DST && !blink_DST) || DST_restore) {
        lcd->setCursor(6, 2);
        lcd->print(settings.DST ? "Y" : "N");
        DST_blank = false;
    }
    if ((settings.UTC_offset != old_settings.UTC_offset && !blink_UTC) || UTC_restore) {
        lcd->setCursor(17, 2);
        lcd->print(settings.UTC_offset >= 0 ? "+" : "-");
        lcd->setCursor(18, 2);
        lcd->print(std::abs(settings.UTC_offset / 10));
        lcd->setCursor(19, 2);
        lcd->print(std::abs(settings.UTC_offset % 10));
        UTC_blank = false;
    }

    if (settings.longitude > 0 ^ old_settings.longitude > 0) {
        lcd->setCursor(18, 0);
        lcd->print(settings.longitude > 0 ? "E" : "W");
    }

    if ((settings.longitude != old_settings.longitude && !blink_long_ten) || long_ten_restore) {
        lcd->setCursor(13, 0);
        lcd->print((longitude_i / 10) % 10);
        long_ten_blank = false;
    }
    if ((settings.longitude != old_settings.longitude && !blink_long_one) || long_one_restore) {
        lcd->setCursor(14, 0);
        lcd->print(longitude_i % 10);
        long_one_blank = false;
    }
    if ((settings.longitude != old_settings.longitude && !blink_long_dec) || long_dec_restore) {
        lcd->setCursor(16, 0);
        lcd->print(longitude_d);
        long_dec_blank = false;
    }
    if ((settings.latitude != old_settings.latitude && !blink_lat_ten) || lat_ten_restore) {
        lcd->setCursor(13, 1);
        lcd->print(latitude_i / 10);
        lat_ten_blank = false;
    }
    if ((settings.latitude != old_settings.latitude && !blink_lat_one) || lat_one_restore) {
        lcd->setCursor(14, 1);
        lcd->print(latitude_i % 10);
        lat_one_blank = false;
    }
    if ((settings.latitude != old_settings.latitude && !blink_lat_dec) || lat_dec_restore) {
        lcd->setCursor(16, 1);
        lcd->print(latitude_d);
        lat_dec_blank = false;
    }

    old_settings = settings;
}

void AstrolabeInterface::processInfo(InterfaceParams p) {
    SphereVector latlong_ecl, old_latlong_ecl, altaz, old_altaz;
    bool retrograde, old_retrograde;
    switch (state) {
        case INFO_SUN:
            latlong_ecl = p.pd.sun_ecl_latlong;
            old_latlong_ecl = old_p.pd.sun_ecl_latlong;
            altaz = p.planets_altaz[0];
            old_altaz = old_p.planets_altaz[0];
            break;
        case INFO_MOON:
            latlong_ecl = p.pd.moon_ecl_latlong;
            old_latlong_ecl = old_p.pd.moon_ecl_latlong;
            altaz = p.planets_altaz[1];
            old_altaz = old_p.planets_altaz[1];
            break;
        case INFO_MERCURY:
            latlong_ecl = p.pd.mercury_ecl_latlong;
            old_latlong_ecl = old_p.pd.mercury_ecl_latlong;
            altaz = p.planets_altaz[2];
            old_altaz = old_p.planets_altaz[2];
            retrograde = p.ri.mercury_r;
            old_retrograde = old_p.ri.mercury_r;
            break;
        case INFO_VENUS:
            latlong_ecl = p.pd.venus_ecl_latlong;
            old_latlong_ecl = old_p.pd.venus_ecl_latlong;
            altaz = p.planets_altaz[3];
            old_altaz = old_p.planets_altaz[3];
            retrograde = p.ri.venus_r;
            old_retrograde = old_p.ri.venus_r;
            break;
        case INFO_MARS:
            latlong_ecl = p.pd.mars_ecl_latlong;
            old_latlong_ecl = old_p.pd.mars_ecl_latlong;
            altaz = p.planets_altaz[4];
            old_altaz = old_p.planets_altaz[4];
            retrograde = p.ri.mars_r;
            old_retrograde = old_p.ri.mars_r;
            break;
        case INFO_JUPITER:
            latlong_ecl = p.pd.jupiter_ecl_latlong;
            old_latlong_ecl = old_p.pd.jupiter_ecl_latlong;
            altaz = p.planets_altaz[5];
            old_altaz = old_p.planets_altaz[5];
            retrograde = p.ri.jupiter_r;
            old_retrograde = old_p.ri.jupiter_r;
            break;
        case INFO_SATURN:
            latlong_ecl = p.pd.saturn_ecl_latlong;
            old_latlong_ecl = old_p.pd.saturn_ecl_latlong;
            altaz = p.planets_altaz[6];
            old_altaz = old_p.planets_altaz[6];
            retrograde = p.ri.saturn_r;
            old_retrograde = old_p.ri.saturn_r;
            break;
    }
    SignLongitude sl = getSignLongitude(latlong_ecl.lon);
    SignLongitude old_sl = getSignLongitude(old_latlong_ecl.lon);
    float ecl_lat_f = latlong_ecl.lat * 180.0 / std::numbers::pi;
    int ecl_lat = ecl_lat_f;
    ecl_lat = ecl_lat % 10;
    int ecl_lat_min = (int) std::abs(std::fmod(ecl_lat_f, 1.0) * 60.0);
    float old_ecl_lat_f = old_latlong_ecl.lat * 180.0 / std::numbers::pi;
    int old_ecl_lat = old_latlong_ecl.lat;
    old_ecl_lat = old_ecl_lat % 10;
    int old_ecl_lat_min = (int) std::abs(std::fmod(old_latlong_ecl.lat, 1.0) * 60.0);

    if (sl.sign != old_sl.sign) {
        lcd->setCursor(2, 1);
        lcd->print(sign_names[sl.sign]);
    }
    if (sl.deg != old_sl.deg) {
        lcd->setCursor(5, 1);
        lcd->print(sl.deg / 10);
        lcd->setCursor(6, 1);
        lcd->print(sl.deg % 10);
    }
    if (sl.min != old_sl.min) {
        lcd->setCursor(8, 1);
        lcd->print(sl.min / 10);
        lcd->setCursor(9, 1);
        lcd->print(sl.min % 10);
    }
    if (ecl_lat != old_ecl_lat) {
        lcd->setCursor(14, 1);
        lcd->print(ecl_lat<0 ? "-" : " ");
        lcd->setCursor(15, 1);
        lcd->print(std::abs(ecl_lat));
    }
    if (ecl_lat_min != old_ecl_lat_min) {
        lcd->setCursor(17, 1);
        lcd->print(ecl_lat_min / 10);
        lcd->setCursor(18, 1);
        lcd->print(ecl_lat_min % 10);
    }

    if (retrograde != old_retrograde) {
        lcd->setCursor(10, 0);
        if (retrograde) {
            lcd->print("R");
        }
        else {
            lcd->print(" ");
        }
    }

    if (altaz.lat != old_altaz.lat) {
        int alt = altaz.lat * 180.0 / std::numbers::pi;
        lcd->setCursor(6, 2);
        if (alt < 0) {
            lcd->print("-");
        }
        else {
            lcd->print(" ");
        }
        lcd->setCursor(7, 2);
        lcd->print(std::abs(alt / 10));
        lcd->setCursor(8, 2);
        lcd->print(std::abs(alt % 10));
    }
    if (altaz.lon != old_altaz.lon) {
        int az = getNormalizedAngle(-(altaz.lon + std::numbers::pi/2)) * 180.0 / std::numbers::pi;
        int old_az = getNormalizedAngle(-(old_altaz.lon + std::numbers::pi/2)) * 180.0 / std::numbers::pi;
        lcd->setCursor(14, 2);
        lcd->print(az / 100);
        lcd->setCursor(15, 2);
        lcd->print((az / 10) % 10);
        lcd->setCursor(16, 2);
        lcd->print(az % 10);
    }

    float house = getHouse(p.cusps, latlong_ecl.lon);
    float old_house = getHouse(old_p.cusps, old_latlong_ecl.lon);
    int house_i = (int) std::floor(house);
    int house_d = (int) (std::fmod(house, 1.0) * 100.0);
    int old_house_i = (int) std::floor(old_house);
    int old_house_d = (int) (std::fmod(old_house, 1.0) * 100.0);
    if (house_i != old_house_i) {
        lcd->setCursor(12, 0);
        lcd->print(house_i / 10);
        lcd->setCursor(13, 0);
        lcd->print(house_i % 10);
    }
    if (house_d != old_house_d) {
        lcd->setCursor(15, 0);
        lcd->print(house_d / 10);
        lcd->setCursor(16, 0);
        lcd->print(house_d % 10);
    }

    if (led_planet == state && !led_status_indicated) {
        lcd->setCursor(19,0);
        lcd->print("*");
        led_status_indicated = true;
    }
    old_p = p;
}

void AstrolabeInterface::processStar(InterfaceParams p) {
    int n_star = state - INFO_STAR_A;
    SphereVector altaz = p.stars_altaz[n_star];
    SphereVector old_altaz = old_p.stars_altaz[n_star];
    if (altaz.lat != old_altaz.lat) {
        int alt = altaz.lat * 180.0 / std::numbers::pi;
        lcd->setCursor(6, 2);
        if (alt < 0) {
            lcd->print("-");
        }
        else {
            lcd->print(" ");
        }
        lcd->setCursor(7, 2);
        lcd->print(std::abs(alt / 10));
        lcd->setCursor(8, 2);
        lcd->print(std::abs(alt % 10));
    }
    if (altaz.lon != old_altaz.lon) {
        int az = getNormalizedAngle(-(altaz.lon + std::numbers::pi/2)) * 180.0 / std::numbers::pi;
        int old_az = getNormalizedAngle(-(old_altaz.lon + std::numbers::pi/2)) * 180.0 / std::numbers::pi;
        lcd->setCursor(14, 2);
        lcd->print(az / 100);
        lcd->setCursor(15, 2);
        lcd->print((az / 10) % 10);
        lcd->setCursor(16, 2);
        lcd->print(az % 10);
    }
    old_p = p;
}

void AstrolabeInterface::processDatetime(InterfaceParams p) {
    blink_clk_counter++;
    bool blink_restore = false;
    if (blink_clk_counter >= max_blink_counter) {
        blink_clk_counter = 0;
        blink_clk = !blink_clk;
        if (!blink_clk) blink_restore = true;
    }

    bool blink_year = blink_clk && state == DATETIME_EDIT_YEAR;
    bool blink_month = blink_clk && state == DATETIME_EDIT_MONTH;
    bool blink_day = blink_clk && state == DATETIME_EDIT_DAY;

    bool blink_year_restore = blink_restore && year_blank;
    bool blink_month_restore = blink_restore && month_blank;
    bool blink_day_restore = blink_restore && day_blank;

    if (year_blank && state != DATETIME_EDIT_YEAR) blink_year_restore = true;
    if (month_blank && state != DATETIME_EDIT_MONTH) blink_month_restore = true;
    if (day_blank && state != DATETIME_EDIT_DAY) blink_day_restore = true;

    if(blink_year) {
        lcd->setCursor(5,1);
        lcd->print("    ");
        year_blank = true;
    }
    if(blink_month) {
        lcd->setCursor(10,1);
        lcd->print("  ");
        month_blank = true;
    }
    if(blink_day) {
        lcd->setCursor(13,1);
        lcd->print("  ");
        day_blank = true;
    }

    if ((old_p.dt.year != p.dt.year && !blink_year) || blink_year_restore) {
        lcd->setCursor(5,1);
        lcd->print(p.dt.year);
        year_blank = false;
    }
    if ((old_p.dt.month != p.dt.month && !blink_month) || blink_month_restore) {
        lcd->setCursor(10,1);
        lcd->print(p.dt.month / 10);
        lcd->setCursor(11,1);
        lcd->print(p.dt.month % 10);
        month_blank = false;
    }
    if ((old_p.dt.day != p.dt.day && !blink_day) || blink_day_restore) {
        lcd->setCursor(13,1);
        lcd->print(p.dt.day / 10);
        lcd->setCursor(14,1);
        lcd->print(p.dt.day % 10);
        day_blank = false;
    }
    
    lcd->setCursor(5, 2);
    int week_day = getDayOfWeek(p.dt.year, p.dt.month, p.dt.day);
    lcd->print(week_names[week_day]);

    if (old_p.dt.hour != p.dt.hour) {
        lcd->setCursor(10,2);
        lcd->print(p.dt.hour / 10);
        lcd->setCursor(11,2);
        lcd->print(p.dt.hour % 10);
    }
    if (old_p.dt.minute != p.dt.minute) {
        lcd->setCursor(13,2);
        lcd->print(p.dt.minute / 10);
        lcd->setCursor(14,2);
        lcd->print(p.dt.minute % 10);
    }
    if (old_p.dt.DST != p.dt.DST) {
        lcd->setCursor(9, 2);
        lcd->print(p.dt.DST ? "*" : " ");
    }
    old_p = p;
}

void AstrolabeInterface::planetLED(InterfaceParams p) {
    SphereVector latlong_ecl;
    switch (led_planet) {
        case INFO_SUN:
            latlong_ecl = p.pd.sun_ecl_latlong;
            break;
        case INFO_MOON:
            latlong_ecl = p.pd.moon_ecl_latlong;
            break;
        case INFO_MERCURY:
            latlong_ecl = p.pd.mercury_ecl_latlong;
            break;
        case INFO_VENUS:
            latlong_ecl = p.pd.venus_ecl_latlong;
            break;
        case INFO_MARS:
            latlong_ecl = p.pd.mars_ecl_latlong;
            break;
        case INFO_JUPITER:
            latlong_ecl = p.pd.jupiter_ecl_latlong;
            break;
        case INFO_SATURN:
            latlong_ecl = p.pd.saturn_ecl_latlong;
            break;
    }
    SignLongitude sl = getSignLongitude(latlong_ecl.lon);
    switch (sl.sign % 4) {
        case FIRE:
            if (current_led == FIRE) break;
            current_led = FIRE;
            pcf->digitalWrite(LEFT_RED, LOW);
            pcf->digitalWrite(LEFT_GREEN, HIGH);
            pcf->digitalWrite(LEFT_BLUE, HIGH);
            pcf->digitalWrite(RIGHT_RED, LOW);
            pcf->digitalWrite(RIGHT_GREEN, HIGH);
            pcf->digitalWrite(RIGHT_BLUE, HIGH);
            break;
        case WATER:
            if (current_led == WATER) break;
            current_led = WATER;
            pcf->digitalWrite(LEFT_RED, HIGH);
            pcf->digitalWrite(LEFT_GREEN, HIGH);
            pcf->digitalWrite(LEFT_BLUE, LOW);
            pcf->digitalWrite(RIGHT_RED, HIGH);
            pcf->digitalWrite(RIGHT_GREEN, HIGH);
            pcf->digitalWrite(RIGHT_BLUE, LOW);
            break;
        case AIR:
            if (current_led == AIR) break;
            current_led = AIR;
            pcf->digitalWrite(LEFT_RED, HIGH);
            pcf->digitalWrite(LEFT_GREEN, HIGH);
            pcf->digitalWrite(LEFT_BLUE, HIGH);
            pcf->digitalWrite(RIGHT_RED, HIGH);
            pcf->digitalWrite(RIGHT_GREEN, HIGH);
            pcf->digitalWrite(RIGHT_BLUE, HIGH);
            break;
        case EARTH:
            if (current_led == EARTH) break;
            current_led = EARTH;
            pcf->digitalWrite(LEFT_RED, HIGH);
            pcf->digitalWrite(LEFT_GREEN, LOW);
            pcf->digitalWrite(LEFT_BLUE, HIGH);
            pcf->digitalWrite(RIGHT_RED, HIGH);
            pcf->digitalWrite(RIGHT_GREEN, LOW);
            pcf->digitalWrite(RIGHT_BLUE, HIGH);
            break;
    }
}

void AstrolabeInterface::processCalibration() {
    if (old_calibration_k == calibration_k) return;
    int k_ones = calibration_k >= 1 ? 1 : 0;
    int k_dec = calibration_k >= 1 ? (calibration_k - 1) * 1000.0 : calibration_k * 1000.0;
    lcd->setCursor(9, 1);
    lcd->print(k_ones);
    lcd->setCursor(11, 1);
    lcd->print(k_dec / 100);
    lcd->setCursor(12, 1);
    lcd->print((k_dec / 10) % 10);
    lcd->setCursor(13, 1);
    lcd->print(k_dec % 10);
    old_calibration_k = calibration_k;

}

void AstrolabeInterface::process(InterfaceParams p) {
    planetLED(p);

    if (needs_initialize) {
        initializeState(p);
        needs_initialize = false;
        return;
    }

    switch(state) {
        case DATETIME:
        case DATETIME_EDIT_DAY:
        case DATETIME_EDIT_MONTH:
        case DATETIME_EDIT_YEAR:
            processDatetime(p);
            break;
        case INFO_SUN:
        case INFO_MOON:
        case INFO_MERCURY:
        case INFO_VENUS:
        case INFO_MARS:
        case INFO_JUPITER:
        case INFO_SATURN:
            processInfo(p);
            break;
        case SETTINGS:
        case SETTINGS_EDIT_LONG_TEN:
        case SETTINGS_EDIT_LONG_ONE:
        case SETTINGS_EDIT_LONG_DEC:
        case SETTINGS_EDIT_LAT_TEN:
        case SETTINGS_EDIT_LAT_ONE:
        case SETTINGS_EDIT_LAT_DEC:
        case SETTINGS_EDIT_DST:
        case SETTINGS_EDIT_UTC:
            processSettings();
            break;
        case CALIBRATION:
            processCalibration();
            break;
        case HOROSCOPE:
            processHoroscope(p);
            break;
        case INFO_STAR_A:
        case INFO_STAR_B:
        case INFO_STAR_C:
        case INFO_STAR_D:
        case INFO_STAR_E:
        case INFO_STAR_F:
        case INFO_STAR_G:
        case INFO_STAR_H:
        case INFO_STAR_I:
        case INFO_STAR_J:
        case INFO_STAR_K:
        case INFO_STAR_L:
            processStar(p);
            break;
    }
}

EncoderReturn AstrolabeInterface::serviceEncoder() {
    bool settings_changed = false;
    bool calibration_changed = false;
    if (! ss->digitalRead(SS_SWITCH)) {
      if (state == DATETIME) state = DATETIME_EDIT_DAY;
      else if (state == DATETIME_EDIT_DAY) state = DATETIME;
      else if (state == DATETIME_EDIT_MONTH) state = DATETIME;
      else if (state == DATETIME_EDIT_YEAR) state = DATETIME;
      if (state >= INFO_SUN && state <= INFO_SATURN) {
        led_planet = state;
        led_status_indicated = false;
      }
      if (state == SETTINGS) state = SETTINGS_EDIT_LONG_TEN;
      else if (state >= SETTINGS_EDIT_LONG_TEN && state <= SETTINGS_EDIT_UTC) {
        state = SETTINGS;
        settings_changed = true;
      }
    }

    int date_offset = 0;
    int32_t encoder_position = ss->getEncoderPosition();
    int32_t encoder_change = encoder_position - old_encoder_position;
    bool encoder_direction = encoder_change > 0;
    old_encoder_position = encoder_position;
    if (state == DATETIME_EDIT_DAY) date_offset = encoder_change;
    if (state == DATETIME_EDIT_MONTH) date_offset = encoder_change * MONTH_INTERVAL;
    if (state == DATETIME_EDIT_YEAR) date_offset = encoder_change * YEAR_INTERVAL;
    if (state == SETTINGS_EDIT_LONG_DEC) settings.longitude = settings.longitude + encoder_change * 0.1;
    if (state == SETTINGS_EDIT_LONG_ONE) settings.longitude = settings.longitude + encoder_change * 1;
    if (state == SETTINGS_EDIT_LONG_TEN) settings.longitude = settings.longitude + encoder_change * 10;
    if (state == SETTINGS_EDIT_LAT_DEC) settings.latitude = settings.latitude + encoder_change * 0.1;
    if (state == SETTINGS_EDIT_LAT_ONE) settings.latitude = settings.latitude + encoder_change * 1;
    if (state == SETTINGS_EDIT_LAT_TEN) settings.latitude = settings.latitude + encoder_change * 10;
    if (state == SETTINGS_EDIT_UTC) settings.UTC_offset = settings.UTC_offset + encoder_change * 1;


    if (settings.latitude > 66.0) settings.latitude = 66.0;
    if (settings.latitude < 0.1) settings.latitude = 0.1;
    if (settings.longitude > 179.9) settings.longitude = 179.9;
    if (settings.longitude < -179.9) settings.longitude = -179.9;
    if (settings.UTC_offset < -12) settings.UTC_offset = -12;
    if (settings.UTC_offset > 12) settings.UTC_offset = 12;

    if (state == CALIBRATION) {
        calibration_k = calibration_k + encoder_change * .001;
        calibration_changed = true;
        if (calibration_k > 1.05) calibration_k = 1.05;
        if (calibration_k < .95) calibration_k = .95;
    }


    if (encoder_change != 0) {
        if (state == SETTINGS_EDIT_DST) settings.DST = !settings.DST;
        if (state >= INFO_SUN && state <= INFO_SATURN) {
            if (state == INFO_SUN) {
                state = encoder_direction ? INFO_MOON : INFO_SATURN;
            }
            else if (state == INFO_SATURN) {
                state = encoder_direction ? INFO_SUN : INFO_JUPITER;
            }
            else {
                state = encoder_direction ? state + 1 : state - 1;
            }
            needs_initialize = true;
        }
        if (state >= INFO_STAR_A && state <= INFO_STAR_L) {
            if (state == INFO_STAR_A) {
                state = encoder_direction ? INFO_STAR_B : INFO_STAR_L;
            }
            else if (state == INFO_STAR_L) {
                state = encoder_direction ? INFO_STAR_A : INFO_STAR_K;
            }
            else {
                state = encoder_direction? state + 1 : state - 1;
            }
            needs_initialize = true;
        }
    }
    return EncoderReturn{.date_offset = date_offset, .settings_changed = settings_changed, .calibration_changed = calibration_changed};
}

void AstrolabeInterface::serviceButton() {
    if (! pcf->digitalRead(LEFT_BUTTON)) {
        if (state == CALIBRATION) {
            state = SETTINGS;
            needs_initialize = true;
        }
        else if (state == SETTINGS) {
            state = HOROSCOPE;
            needs_initialize = true;
        }
        else if (state == HOROSCOPE) {
            state = INFO_STAR_A;
            needs_initialize = true;
        }
        else if (state >= INFO_STAR_A && state <= INFO_STAR_L) {
            state = INFO_SUN;
            needs_initialize = true;
        }
        else if (state >= INFO_SUN && state <= INFO_SATURN) {
            state = DATETIME;
            needs_initialize = true;
        }
        else if (state == DATETIME_EDIT_DAY) state = DATETIME_EDIT_MONTH;
        else if (state == DATETIME_EDIT_MONTH) state = DATETIME_EDIT_YEAR;
        else if (state == DATETIME_EDIT_YEAR) state = DATETIME_EDIT_DAY;
        else if (state >= SETTINGS_EDIT_LONG_TEN && state <= SETTINGS_EDIT_UTC) {
            if (state == SETTINGS_EDIT_LONG_TEN) state = SETTINGS_EDIT_UTC;
            else state--;
        }
        else if (state == DATETIME) {
            state = CALIBRATION;
            needs_initialize = true;
        }
    }
    if (! pcf->digitalRead(RIGHT_BUTTON)) {
        if (state >= INFO_SUN && state <= INFO_SATURN) {
            state = INFO_STAR_A;
            needs_initialize = true;
        }
        else if (state >= INFO_STAR_A && state <= INFO_STAR_L) {
            state = HOROSCOPE;
            needs_initialize = true;
        }
        else if (state == HOROSCOPE) {
            state = SETTINGS;
            needs_initialize = true;
        }
        else if (state == DATETIME) {
            state = INFO_SUN;
            needs_initialize = true;
        }
        else if (state == DATETIME_EDIT_MONTH) state = DATETIME_EDIT_DAY;
        else if (state == DATETIME_EDIT_YEAR) state = DATETIME_EDIT_MONTH;
        else if (state == DATETIME_EDIT_DAY) state = DATETIME_EDIT_YEAR;
        else if (state >= SETTINGS_EDIT_LONG_TEN && state <= SETTINGS_EDIT_UTC) {
            if (state == SETTINGS_EDIT_UTC) state = SETTINGS_EDIT_LONG_TEN;
            else state++;
        } 
        else if (state == SETTINGS) {
            state = CALIBRATION;
            needs_initialize = true;
        }
        else if (state == CALIBRATION) {
            state = DATETIME;
            needs_initialize = true;
        }
    }
}

SignLongitude AstrolabeInterface::getSignLongitude(float lon) {
    lon = getNormalizedAngle(lon) * 180.0 / std::numbers::pi;
    int sign_n = std::floor(lon/sign_interval);
    float deg = std::fmod(lon, sign_interval);
    float min = std::fmod(deg, 1.0) * 60.0;
    return SignLongitude{.sign = sign_n, .deg = std::floor(deg), .min = std::floor(min)};
}

float AstrolabeInterface::getHouse(std::array<float, 12> cusps, float lon) {
    lon = getNormalizedAngle(lon);
    int upper_i, lower_i;
    for (lower_i = 0; lower_i < 12; lower_i++) {
        upper_i = lower_i == 11 ? 0 : lower_i + 1;
        if (inHouse(cusps[lower_i], cusps[upper_i], lon)) break;
    }
    float upper_cusp, lower_cusp;
    lower_cusp = cusps[lower_i];
    upper_cusp = cusps[upper_i] < cusps[lower_i] ? cusps[upper_i] + 2 * std::numbers::pi : cusps[upper_i];
    if (lon < lower_cusp) lon = lon + 2 * std::numbers::pi;
    float decimal = (lon - lower_cusp) / (upper_cusp - lower_cusp);
    lower_i+=2;
    lower_i = lower_i > 12 ? lower_i - 12 : lower_i;
    return lower_i + decimal;
}

bool AstrolabeInterface::inHouse(float cusp_a, float cusp_b, float lon) {
    return (lon > cusp_a && lon < cusp_b) || (lon > cusp_a && cusp_b < cusp_a) || (lon < cusp_a && lon < cusp_b && cusp_b < cusp_a);
}

Settings AstrolabeInterface::getSettings() {
    return settings;
}

float AstrolabeInterface::getCalibration() {
    return calibration_k;
}