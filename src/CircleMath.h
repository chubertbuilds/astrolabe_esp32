#pragma once

#ifndef CIRCLEMATH_H
#define CIRCLEMATH_H

#include <tuple>
#include <complex>
#include <cmath>
#include <array>

struct Circle2D {
    double x;
    double y;
    double r;
};

struct Point2D {
    double x;
    double y;
};

struct StereoCoords {
    double r;
    double theta;
};

struct RectVector {
    double x;
    double y;
    double z;
};

struct SphereVector {
    double lat;
    double lon;
};
Circle2D getCircleFromThreePoints(Point2D a, Point2D b, Point2D c);

std::array<Point2D, 2> getCircleIntersection(Circle2D a, Circle2D b);

StereoCoords getStereoCoords(double latitude, double longitude);

SphereVector getRotatedLatLong(SphereVector v, double theta);

RectVector rotateVectorAboutX(RectVector p, double theta);

SphereVector getLatLongFromRect(RectVector p);

RectVector addVectors(RectVector a, RectVector b);

RectVector getNegativeVector(RectVector p);

double getNormalizedAngle(double angle);

float getNormalizedAngle(float angle);

StereoCoords getStereoCoordsFromPoint(Point2D p);

SphereVector getLatLongFromStereo(StereoCoords c);

SphereVector getAltAz(float rete_angle, SphereVector equ_coords, float latitude);

#endif // CIRCLEMATH_H