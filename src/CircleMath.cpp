#include "src/CircleMath.h"

Circle2D getCircleFromThreePoints(Point2D a, Point2D b, Point2D c) {
    //https://math.stackexchange.com/questions/213658/get-the-equation-of-a-circle-when-given-3-points
    std::complex<double> z1(a.x, a.y);
    std::complex<double> z2(b.x, b.y);
    std::complex<double> z3(c.x, c.y);

    std::complex<double> w = (z3 - z1)/(z2 - z1);
    std::complex<double> center = (z2 - z1)*(w - std::abs(w)*std::abs(w))/(w - std::conj(w)) + z1;
    return Circle2D{.x = center.real(), .y = center.imag(), .r = std::abs(z1-center)};
}

std::array<Point2D, 2> getCircleIntersection(Circle2D a, Circle2D b) {
    //https://math.stackexchange.com/questions/256100/how-can-i-find-the-points-at-which-two-circles-intersect
    double x1 = a.x;
    double x2 = b.x;
    double y1 = a.y;
    double y2 = b.y;
    double r1 = a.r;
    double r2 = b.r;

    double d = std::sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
    double l = (r1 * r1 - r2 * r2 + d * d) / (2 * d);
    double h = std::sqrt(r1 * r1 - l * l);

    double xe = h / d * (y2 - y1);
    double ye = - h / d * (x2 - x1);
    double xi = l / d * (x2 - x1) + x1;
    double yi = l / d * (y2 - y1) + y1;

    return {Point2D{.x = xi + xe, .y = yi + ye}, Point2D{.x = xi - xe, .y = yi - ye}};
}

StereoCoords getStereoCoords(double latitude, double longitude) {
    return StereoCoords{.r = 1 / std::tan((std::numbers::pi/2 + latitude)/2), .theta = longitude};
}

StereoCoords getStereoCoordsFromPoint(Point2D p) {
    return StereoCoords{.r = std::sqrt(p.x * p.x + p.y * p.y), .theta = atan2(p.y, p.x)};
}

SphereVector getLatLongFromStereo(StereoCoords c) {
    return SphereVector{.lat = 2 * std::atan(1 / c.r) - std::numbers::pi/2.0, .lon = c.theta};
}

SphereVector getRotatedLatLong(SphereVector v, double theta) {
    double z = std::sin(v.lat);
    double x = std::cos(v.lat) * std::cos(v.lon);
    double y = std::cos(v.lat) * std::sin(v.lon);
    RectVector rotated = rotateVectorAboutX(RectVector{.x = x, .y = y, .z = z}, theta);
    return getLatLongFromRect(rotated);
}

RectVector rotateVectorAboutX(RectVector p, double theta) {
    double y = p.y * std::cos(theta) - p.z * std::sin(theta);
    double z = p.y * std::sin(theta) + p.z * std::cos(theta);
    RectVector coords = {.x = p.x, .y = y, .z = z};
    return coords;
}

SphereVector getLatLongFromRect(RectVector p) {
    double lat = std::atan(p.z/(std::sqrt(p.x*p.x + p.y*p.y)));
    double lon = std::atan2(p.y, p.x);
    SphereVector coords = {.lat = lat, .lon = lon};
    return coords;
}

RectVector getNegativeVector(RectVector p) {
    RectVector negative_v = {.x = -(p.x), .y = -(p.y), .z=-(p.z)};
    return negative_v;
}

RectVector addVectors(RectVector a, RectVector b) {
    RectVector sum = {.x = a.x + b.x, .y = a.y + b.y, .z = a.z + b.z};
    return sum;
}

double getNormalizedAngle(double angle) {
    double normalizedAngle = std::fmod(angle, 2*std::numbers::pi);
    if (normalizedAngle < 0) {
        normalizedAngle += 2*std::numbers::pi;
    }
    return normalizedAngle;
}

float getNormalizedAngle(float angle) {
    float normalizedAngle = std::fmod(angle, 2*std::numbers::pi);
    if (normalizedAngle < 0) {
        normalizedAngle += 2*std::numbers::pi;
    }
    return normalizedAngle;
}

SphereVector getAltAz(float rete_angle, SphereVector equ_coords, float latitude) {
    latitude = latitude * std::numbers::pi / 180.0;
    SphereVector geo_equ_coords = SphereVector{.lat = equ_coords.lat, .lon = equ_coords.lon - rete_angle};
    return getRotatedLatLong(geo_equ_coords, std::numbers::pi/2 - latitude);
}

