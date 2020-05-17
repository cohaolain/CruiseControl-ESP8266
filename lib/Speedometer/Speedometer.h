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
    int min;
    int max;
    int limit;
    const double min_angle = PI / 12;
    const double angle_limit = PI * 1.5;
    void drawNeedle(double speed);
};