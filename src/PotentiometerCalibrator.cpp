#include "src/PotentiometerCalibrator.h"

//[counts, angle]

PotentiometerCalibrator::PotentiometerCalibrator(std::vector<std::array<float, 2>>  calibrationPoints, float offset) {
    init(calibrationPoints, offset);
} 

void PotentiometerCalibrator::init(std::vector<std::array<float, 2>>  calibrationPoints, float offset) {
    _calibrationPoints = calibrationPoints;
    _offset = offset;
    n = calibrationPoints.size();
    endSlope = (calibrationPoints[n-1][1] - calibrationPoints[n-2][1]) / (calibrationPoints[n-1][0] - calibrationPoints[n-2][0]);
}

void PotentiometerCalibrator::setPoints(std::vector<std::array<float, 2>>  calibrationPoints) {
    init(calibrationPoints, _offset);
}

float PotentiometerCalibrator::getAngle(float count){

    if (count < _calibrationPoints[n-1][0]) {
        float x2 = _calibrationPoints[0][0];
        int i = 0;
        while (count > x2) {
            i++;
            x2 = _calibrationPoints[i][0];
        }
        float x1 = _calibrationPoints[i-1][0];
        float y2 = _calibrationPoints[i][1];
        float y1 = _calibrationPoints[i-1][1];
        float slope = (y2 - y1) / (x2 - x1);
        return y1 + slope * (count - x1) + _offset;
    }
    else {
        float x = _calibrationPoints[n-1][0];
        float y = _calibrationPoints[n-1][1];
        return y + endSlope * (count - x) + _offset;
    }
}

