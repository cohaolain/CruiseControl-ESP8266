#pragma once
#include <U8g2lib.h>

extern U8G2 globalu8g2instance;

class Speedometer
{
public:
    Speedometer(int height, int width, int x, int initLimit);
    void draw(double speed);
    void updateRange(int newLimit);

private:
    U8G2 u8g2 = globalu8g2instance;
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