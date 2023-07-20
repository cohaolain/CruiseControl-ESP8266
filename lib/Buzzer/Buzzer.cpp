#include <Buzzer.h>

void ackBeep(bool doTwice)
{
    tone(buzzerPin, 880, doTwice ? 75 : 100);
    if (doTwice)
    {
        delay(125);
        ackBeep(false);
    }
}

void ackBeep()
{
    ackBeep(false);
}

void actionBeep(bool isNice)
{
    if (isNice)
    {
        tone(buzzerPin, 650, 75);
        delay(75);
        tone(buzzerPin, 880, 75);
    }
    else
    {
        tone(buzzerPin, 880, 75);
        delay(75);
        tone(buzzerPin, 650, 75);
    }
}
