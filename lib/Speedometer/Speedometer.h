#pragma once
#include <U8g2lib.h>

class Speedometer
{
public:
    Speedometer(U8G2 u8g2Instance, int height, int width, int x, int initLimit);
    void draw(double speed);
    void updateRange(int newLimit);

private:
    U8G2 u8g2;
    int radius;
    int center_x;
    int center_y;
    int lowerBound;
    int limit;
    const double normal_range = PI / 12;
    const double overflow_range = PI / 6;

    void drawTick(double speed, double shadeFrom, double shadeTo);
    void drawTick(double speed);
    void drawNeedle(double speed);
};