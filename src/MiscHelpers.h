#pragma once 

inline int getDayOfWeek(int y, int m, int d) {
    //https://en.wikipedia.org/wiki/Determination_of_the_day_of_the_week
    return (d+=m<3 ? y-- : y-2, 23*m/9+d+4+y/4-y/100+y/400) % 7;
}