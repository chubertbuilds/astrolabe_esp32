#include<vector>
#include<array>
class PotentiometerCalibrator {
    public:
        PotentiometerCalibrator(std::vector<std::array<float, 2>> calibrationPoints, float offset = 0);
        float getAngle(float count);
        void setPoints(std::vector<std::array<float, 2>>  calibrationPoints);

    private: 
        std::vector<std::array<float, 2>> _calibrationPoints;
        int n;
        float maximumCounts = 4038;
        float endSlope;
        float _offset;
        void init(std::vector<std::array<float, 2>> calibrationPoints, float offset);

};