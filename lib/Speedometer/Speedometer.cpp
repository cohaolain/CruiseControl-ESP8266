#include <Speedometer.h>

Speedometer::Speedometer(U8G2 u8g2Instance, int height, int width, int x, int initLimit)
{
    u8g2 = u8g2Instance;
    updateRange(initLimit);

    radius = (4 * pow(height, 2) + pow(width, 2)) / (8 * height);
    center_x = x + width / 2;
    center_y = u8g2.getDisplayHeight() + (radius - height);
}

void Speedometer::draw(double speed)
{
    u8g2.drawCircle(center_x, center_y, radius);

    drawNeedle(speed);
}

void Speedometer::updateRange(int newLimit)
{
    limit = newLimit;
    min = limit * 0.7;
    max = limit * 1.2;
}

void Speedometer::drawNeedle(double speed)
{
    double theta;
    if (speed < min)
        theta = min_angle;
    else if (speed < max)
        theta = ((speed - min) / (max - min)) * 2 * PI + min_angle;
    else
        return;
    Serial.printf("speed: %f, min: %d, max: %d, theta: %f\n", speed, min, max, theta);
    if (theta > angle_limit || theta < 0)
        return;
    const int intersect_x = center_x - (cos(theta) * radius);
    const int intersect_y = center_y - (sin(theta) * radius);

    u8g2.drawLine(center_x, center_y, intersect_x, intersect_y);
}