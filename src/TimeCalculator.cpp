#include "src/TimeCalculator.h"

TimeCalculator::TimeCalculator(int _sidereal_day_offset, double _longitude, int _offset, bool _DST) {
    sidereal_day_offset = _sidereal_day_offset;
    longitude = (std::numbers::pi/180.0)*_longitude;
    enable_US_DST = _DST;
    UT1_offset = _offset;
}

void TimeCalculator::setSettings(int _sidereal_day_offset, double _longitude, int _offset, bool _DST) {
    sidereal_day_offset = _sidereal_day_offset;
    longitude = (std::numbers::pi/180.0)*_longitude;
    enable_US_DST = _DST;
    UT1_offset = _offset;
}

void TimeCalculator::setOffset(int _sidereal_day_offset) {
    sidereal_day_offset = _sidereal_day_offset;
}

double TimeCalculator::getJulianDate(float rete_angle) {
    double _julian_date = j2000 + sidereal_day * sidereal_day_offset;
    double LSHoffset_j2000_local = LSHoffset_j2000_greenwich + (longitude / (2.0*std::numbers::pi));
    _julian_date += sidereal_day*((rete_angle + instrument_offset)/(2.0*std::numbers::pi) - LSHoffset_j2000_local);
    julian_date = _julian_date;
    return _julian_date;
}

DateTime TimeCalculator::getDateTime(double _julian_date) {

    double hour_remainder = std::fmod(_julian_date, 1.0);
    const double hour_frac = 1/24.0;
    const double minute_frac = hour_frac/60.0;
    int hour = 12 + std::floor(hour_remainder/hour_frac); //julian days...start at noon...
    double minute_remainder = std::fmod(hour_remainder, hour_frac);
    int minute = std::round(minute_remainder/minute_frac);
    bool DST;

    if (minute==60) {
        hour += 1;
        minute = 0;
    }

    hour += UT1_offset;

    int day_offset = 0;
    
    if (hour >= 24) {
        hour -= 24;
        day_offset = 1;
    }
    if (hour < 0) {
        hour += 24;
        day_offset = -1;
    }

    Date d = getDate(std::floor(_julian_date + day_offset));
    DST = enable_US_DST && isDST(d);
    if (DST) {
        hour += 1;
        if (hour >= 24) {
            hour -= 24;
            day_offset = 1;
        }
        if (hour < 0) {
            hour += 24;
            day_offset = -1;
        }
    }
    d = getDate(std::floor(_julian_date + day_offset));

    return DateTime{.year = d.year, .month = d.month, .day = d.day, .hour = hour, .minute = minute, .DST = DST};
}

//https://en.wikipedia.org/wiki/Julian_day#Julian_or_Gregorian_calendar_from_Julian_day_number
Date TimeCalculator::getDate(int JD) {
    const int y = 4716;
    const int j = 1401;
    const int m = 2;
    const int n = 12;
    const int r = 4;
    const int p = 1461;
    const int v = 3;
    const int u = 5;
    const int s = 153;
    const int w = 2;
    const int B = 274277;
    const int C = -38;
    const int x = 146097;

    int f = JD + j + (((4 * JD + B) / x) * 3) / 4 + C;
    int e = r * f + v;
    int g = (e % p) / r;
    int h = u * g + w;

    int day = (h % s) / u + 1;
    int month = (h / s + m) % n + 1;
    int year = (e / p) - y + (n + m - month) / n;

    return Date{.year = year, .month = month, .day = day};

}

//crying, sobbing
bool TimeCalculator::isDST(Date d) {
    int month = d.month;
    int year = d.year;
    int day = d.day;
    int spring_forward;
    int fall_back;
    if (year < 1967) return false;
    if (year == 1974) {
        if (month >= 2 && month <= 9) return true;
        if (month != 1 && month != 10) return false;
        if (month == 1) {
            spring_forward = 6;
            return day >= spring_forward;
        }
        if (month == 10) {
            fall_back = 27;
            return day <= fall_back;
        }  
    }
    if (year == 1975) {
        if (month >= 3 && month <= 9) return true;
        if (month != 2 && month != 10) return false;
        if (month == 2) {
            spring_forward = 23;
            return day >= spring_forward;
        }
        if (month == 10) {
            fall_back = 26;
            return day <= fall_back;
        }  
    }
    if ((year >= 1967 && year <= 1973) || (year >= 1976 && year <= 1986)) {
        if (month >= 5 && month <= 9) return true;
        if (month != 4 && month != 10) return false;
        if (month == 4) {
            spring_forward = getLastSunday(year, 4);
            return day >= spring_forward;
        }
        if (month == 10) {
            fall_back = getLastSunday(year, 10);
            return day <= fall_back;
        }   
    }
    if (year >= 1987 && year <= 2006) {
        if (month >= 5 && month <= 9) return true;
        if (month != 4 && month != 10) return false;
        if (month == 4) {
            spring_forward = getNthSunday(year, 1, 4);
            return day >= spring_forward;
        }
        if (month == 10) {
            fall_back = getLastSunday(year, 10);
            return day <= fall_back;
        }   
    }
    if (year >= 2007) {
        if (month >= 4 && month <= 10) return true;
        if (month != 3 && month != 11) return false;
        if (month == 3) {
            spring_forward = getNthSunday(year, 2, 3);
            return day >= spring_forward;
        }
        if (month == 11) {
            fall_back = getNthSunday(year, 1, 11);
            return day <= fall_back;
        }
    }
    return false;

}

int TimeCalculator::getNthSunday(int year, int n, int month) {
    int nday_first_of_month = getDayOfWeek(year, month, 1);
    int first_sunday = (nday_first_of_month == 0) ? 1 : 1 + 7 - (nday_first_of_month);
    return first_sunday + 7 * (n - 1);
}

//february is not an option, but we don't need it. or error handling.
int TimeCalculator::getLastSunday(int year, int month) {
    int last_day;
    switch (month) {
        case (1):
        case (3):
        case (5):
        case (7):
        case (8):
        case (10):
        case (12):
            last_day = 31;
            break;
        default:
            last_day = 30;
            break;
    }
    int nday_last_of_month = getDayOfWeek(year, month, last_day);
    return last_day - nday_last_of_month;

}
