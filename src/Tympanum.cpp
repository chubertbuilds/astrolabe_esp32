#include "src/Tympanum.h"

Tympanum::Tympanum(double latitude) {
    _latitude = latitude * (std::numbers::pi / 180.0);
    _almucantar = calculateAlmucantar();
    _horizon = _almucantar[0];
    _azimuth = calculateAzimuth();
    _tropics = calculateTropics();
    _ecliptic = calculateEcliptic();
    _hours = calculateHours();
    
}

void Tympanum::setLatitude(double latitude) {
    _latitude = latitude * (std::numbers::pi / 180.0);
    _almucantar = calculateAlmucantar();
    _horizon = _almucantar[0];
    _azimuth = calculateAzimuth();
    _tropics = calculateTropics();
    _ecliptic = calculateEcliptic();
    _hours = calculateHours();
}

std::array<Circle2D, n_almucantar> Tympanum::getAlmucantar() {
    return _almucantar;
}

std::array<Circle2D, n_azimuth> Tympanum::getAzimuth() {
    return _azimuth;
}

std::array<Circle2D, n_hours> Tympanum::getHours() {
    return _hours;
}

std::array<Circle2D, n_tropics> Tympanum::getTropics() {
    return _tropics;
}

Circle2D Tympanum::getHorizon() {
    return _horizon;
}

std::array<float, n_houses> Tympanum::getHouseCusps(float LSH_angle) {
    std::array<float, n_houses> cusps;

    float x = _ecliptic.x;
    float y = _ecliptic.y;
    float x_p = x * std::cos(LSH_angle) - y * std::sin(LSH_angle);
    float y_p = x * std::sin(LSH_angle) + y * std::cos(LSH_angle);
    Circle2D rotated_ecliptic = Circle2D{.x = x_p, .y = y_p, .r = _ecliptic.r};
    Circle2D hour;
    std::array<Point2D, 2> intersections;
    std::array<Point2D, n_houses / 2> cusp_points;
    for (int i = 0; i < n_houses / 2; i++) {
        switch(i) {
            case 0:
                intersections = getCircleIntersection(rotated_ecliptic, _horizon);
                cusp_points[i] = intersections[0].x < intersections[1].x ? intersections[0] : intersections[1];
                continue;
            case 1:
                hour = _hours[1];
                hour = Circle2D{.x = -hour.x, .y = hour.y, .r = hour.r};
                break;
            case 2:
                hour = _hours[3];
                hour = Circle2D{.x = -hour.x, .y = hour.y, .r = hour.r};
                break;
            case 3:
                cusp_points[i] = Point2D{
                                .x = 0, 
                                .y = - std::sqrt(_ecliptic.r * _ecliptic.r - rotated_ecliptic.x * rotated_ecliptic.x) + rotated_ecliptic.y
                            };
                continue;
            case 4:
                hour = _hours[3];
                break;
            case 5:
                hour = _hours[1];
                break;
        }
        intersections = getCircleIntersection(rotated_ecliptic, hour);
        cusp_points[i] = isBelowHorizon(intersections[0]) ? intersections[0] : intersections[1];
    }

    SphereVector equ_latlong, ecl_latlong;
    int index;
    for (int i = 0; i < n_houses / 2; i++) {
        equ_latlong = getLatLongFromStereo(getStereoCoordsFromPoint(cusp_points[i]));
        equ_latlong = SphereVector{.lat = equ_latlong.lat, .lon = equ_latlong.lon - LSH_angle};
        ecl_latlong = getRotatedLatLong(equ_latlong, -e);
        index = n_houses - i - 1;
        cusps[index] = getNormalizedAngle(-ecl_latlong.lon);
        cusps[index - n_houses/2] = getNormalizedAngle(-(ecl_latlong.lon + std::numbers::pi));
    }

    return cusps;

}

bool Tympanum::isBelowHorizon(Point2D p) {
    double d = (p.x - _horizon.x) * (p.x - _horizon.x) + (p.y - _horizon.y) * (p.y - _horizon.y);
    return d > (_horizon.r * _horizon.r);
}


std::array<Circle2D, n_almucantar> Tympanum::calculateAlmucantar() {
    std::array<Circle2D, n_almucantar> almucantar;
    for (int i = 0; i < n_almucantar; i++) {
        StereoCoords a = getStereoCoords((std::numbers::pi/2-_latitude) + i * std::numbers::pi/2/n_almucantar, - std::numbers::pi/2);
        StereoCoords b = getStereoCoords(-(std::numbers::pi/2-_latitude) + i * std::numbers::pi/2/n_almucantar, std::numbers::pi/2);
        double r = (a.r + b.r)/2;
        almucantar[i] = Circle2D{.x = 0, .y = -a.r + r, .r = r};
    }
    return almucantar;
}

std::array<Circle2D, n_azimuth> Tympanum::calculateAzimuth() {
    std::array<Circle2D, n_azimuth> azimuth;
    StereoCoords a = getStereoCoords(_latitude, std::numbers::pi/2);
    StereoCoords b, c;
    SphereVector v1, v2;
    for (int i = 0; i < n_azimuth; i++) {
        v1 = getRotatedLatLong(SphereVector{.lat = 0, .lon = 0 + i * std::numbers::pi/n_azimuth}, -(std::numbers::pi/2 - _latitude));
        v2 = getRotatedLatLong(SphereVector{.lat = 0, .lon = std::numbers::pi + i * std::numbers::pi/n_azimuth}, -(std::numbers::pi/2 - _latitude));
        b = getStereoCoords(v1.lat, v1.lon);
        c = getStereoCoords(v2.lat, v2.lon);
        azimuth[i] = getCircleFromThreePoints(
            Point2D{.x = a.r * std::cos(a.theta), .y = a.r * std::sin(a.theta)},
            Point2D{.x = b.r * std::cos(b.theta), .y = b.r * std::sin(b.theta)},
            Point2D{.x = c.r * std::cos(c.theta), .y = c.r * std::sin(c.theta)});
    }
    return azimuth;
}

std::array<Circle2D, n_tropics> Tympanum::calculateTropics() {
    std::array<Circle2D, n_tropics> tropics;
    StereoCoords r0 = getStereoCoords(e, 0);
    StereoCoords r1 = getStereoCoords(0, 0);
    StereoCoords r2 = getStereoCoords(-e, 0);
    tropics[0] = Circle2D{.x = 0, .y = 0, .r = r0.r};
    tropics[1] = Circle2D{.x = 0, .y = 0, .r = r1.r};
    tropics[2] = Circle2D{.x = 0, .y = 0, .r = r2.r};
    return tropics;
}

Circle2D Tympanum::calculateEcliptic() {
    double r1 = -(_tropics[0].r);
    double r2 = (_tropics[2].r);
    return Circle2D{.x = 0, .y = -(r2 + r1)/2, .r = (r2 - r1)/2};
}


std::array<Circle2D, n_hours> Tympanum::calculateHours() {
    std::array<Circle2D, n_hours> hours;
    Point2D intersection_temp;
    std::array<double, 3> intersection_theta;
    std::array<double, 3> theta_interval;
    for (int i = 0; i < 3; i++) {
        intersection_temp = getCircleIntersection(_horizon, _tropics[i])[0];
        intersection_theta[i] = std::atan2(intersection_temp.y, std::abs(intersection_temp.x));
        theta_interval[i] = (std::numbers::pi + intersection_theta[i] * 2) / (2.0 * (n_hours + 1));
    }

    Point2D a, b, c;
    double theta_a, theta_b, theta_c;
    for (int i = 0; i < n_hours; i++) {
        theta_a = intersection_theta[0] - theta_interval[0] * (i + 1);
        theta_b = intersection_theta[1] - theta_interval[1] * (i + 1);
        theta_c = intersection_theta[2] - theta_interval[2] * (i + 1);
        a = Point2D{ .x = _tropics[0].r * std::cos(theta_a), .y = _tropics[0].r * std::sin(theta_a) };
        b = Point2D{ .x = _tropics[1].r * std::cos(theta_b), .y = _tropics[1].r * std::sin(theta_b) };
        c = Point2D{ .x = _tropics[2].r * std::cos(theta_c), .y = _tropics[2].r * std::sin(theta_c) };
        hours[i] = getCircleFromThreePoints(a, b, c);
    }
    return hours;
}

