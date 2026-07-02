#pragma once

#include <cmath>
#include <array>
#include "src/CircleMath.h"
#include "src/Ephemerides.h"

const int n_almucantar = 9;
const int n_azimuth = 12; //functionally 24, because one azimuth circle describes both e.g. 0h and 6h
const int n_hours = 5; //functionally 12, 5*2 + horizon + vertical
const int n_tropics = 3;
const int n_houses = 12;

class Tympanum {
    public:
        Tympanum(double latitude = 0.1);
        std::array<Circle2D, n_almucantar> getAlmucantar();
        std::array<Circle2D, n_azimuth> getAzimuth();
        std::array<Circle2D, n_tropics> getTropics();
        std::array<Circle2D, n_hours> getHours();
        std::array<float, n_houses> getHouseCusps(float LSH_angle);
        Circle2D getHorizon();
        void setLatitude(double latitude);

    private:
        std::array<Circle2D, n_almucantar> calculateAlmucantar();
        std::array<Circle2D, n_azimuth> calculateAzimuth();
        std::array<Circle2D, n_tropics> calculateTropics();
        std::array<Circle2D, n_hours> calculateHours();
        Circle2D calculateEcliptic();
        std::array<Circle2D, n_almucantar> _almucantar;
        std::array<Circle2D, n_azimuth> _azimuth;
        std::array<Circle2D, n_tropics> _tropics;
        std::array<Circle2D, n_hours> _hours;
        bool isBelowHorizon(Point2D p);
        Circle2D _horizon;
        Circle2D _ecliptic;
        double _latitude;

};