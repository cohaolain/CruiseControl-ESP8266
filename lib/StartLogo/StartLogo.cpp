#include <StartLogo.h>

StartLogo::StartLogo(int x_pos, int y_pos)
{
    x = x_pos;
    y = y_pos;
}

void StartLogo::displayLogo(int duration)
{
    u8g2.clearBuffer();
    u8g2.drawXBMP(x, y, start_logo_width, start_logo_height, start_logo_bits);
    u8g2.sendBuffer();
    delay(duration);
}