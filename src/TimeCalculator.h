#pragma once

#include <numbers>
#include <cmath>
#include "src/MiscHelpers.h"

const double sidereal_day = 0.9972696;

const double j2000 = 2451545.0;

//LSH offsets are in proportion of a day
const double LSHoffset_j2000_greenwich = 0.77905092592; //on 2000/01/01 12 PM UT1 in Greenwich, the LSH is 18:41:50 https://aa.usno.navy.mil/data/siderealtime

const double instrument_offset = 6.5 * std::numbers::pi; //the reported instrument angle is -6.5pi at the lowest 0 hr LSH

struct DateTime {
    int year;
    int month;
    int day;
    int hour;
    int minute;
    bool DST;
};

struct Date {
    int year;
    int month;
    int day;
};

class TimeCalculator {
    public:
        TimeCalculator(int _sidereal_day_offset = 0, double _longitude = 0.0, int _offset = 0, bool _DST = false);
        DateTime getDateTime(double _julian_date);
        double getJulianDate(float rete_angle);
        void setOffset(int _sidereal_day_offset);
        void setSettings(int _sidereal_day_offset, double _longitude, int _offset, bool _DST);


    private:
        double LSHoffset;
        int sidereal_day_offset; //number of sidereal days between rete range and the 1/1/2000 rete range
        double julian_date;
        double longitude;
        int UT1_offset;
        bool enable_US_DST;
        Date getDate(int JD);
        bool isDST(Date d);
        int getNthSunday(int year, int n, int month);
        int getLastSunday(int year, int month);
};
