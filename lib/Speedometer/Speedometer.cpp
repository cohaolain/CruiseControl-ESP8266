#include <Speedometer.h>

Speedometer::Speedometer(int height, int width, int x, int initLimit)
{
    updateRange(initLimit);

    radius = (4 * pow(height, 2) + pow(width, 2)) / (8 * height);
    center_x = x + width / 2;
    center_y = u8g2.getDisplayHeight() + (radius - height);
}

void Speedometer::draw(double speed)
{
    u8g2.drawCircle(center_x, center_y, radius);

    drawTick(limit);
    drawNeedle(speed);
}

void Speedometer::updateRange(int newLimit)
{
    limit = newLimit;
    lowerBound = limit * 0.8;
}

double mapf(double x, double in_min, double in_max, double out_min, double out_max)
{
    const double delta = x - in_min;
    const double out_range = out_max - out_min;
    const double in_range = in_max - in_min;

    return delta * out_range / in_range + out_min;
}

void Speedometer::drawTick(double speed, double shadeFrom, double shadeTo)
{
    double theta = mapf(speed, lowerBound, limit, normal_range, PI - normal_range);

    theta = min(theta, PI + overflow_range);
    theta = max(theta, -overflow_range);

    const int inner_x = center_x - (cos(theta) * (radius * shadeFrom));
    const int inner_y = center_y - (sin(theta) * (radius * shadeFrom));
    const int outer_x = center_x - (cos(theta) * (radius * shadeTo));
    const int outer_y = center_y - (sin(theta) * (radius * shadeTo));

    u8g2.drawLine(inner_x, inner_y, outer_x, outer_y);
}

void Speedometer::drawTick(double speed)
{
    drawTick(speed, 0.7, 1);
}

void Speedometer::drawNeedle(double speed)
{
    drawTick(speed, 0, 1);
}
