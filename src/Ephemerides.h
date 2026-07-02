#pragma once

#include <cmath>
#include "src/CircleMath.h"

const double e = 0.4090926291;

struct PlanetData {
    struct SphereVector sun_ecl_latlong;
    struct SphereVector moon_ecl_latlong;
    struct SphereVector mercury_ecl_latlong;
    struct SphereVector venus_ecl_latlong;
    struct SphereVector mars_ecl_latlong;
    struct SphereVector jupiter_ecl_latlong;
    struct SphereVector saturn_ecl_latlong;
    struct SphereVector sun_equ_latlong;
    struct SphereVector moon_equ_latlong;
    struct SphereVector mercury_equ_latlong;
    struct SphereVector venus_equ_latlong;
    struct SphereVector mars_equ_latlong;
    struct SphereVector jupiter_equ_latlong;
    struct SphereVector saturn_equ_latlong;
};

PlanetData getPlanetData(double jd);




