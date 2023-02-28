#include <Arduino.h>
#include "RTC_SAMD21.h"
#include "DateTime.h"

RTC_SAMD21 rtc;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
void setup()
{
    Serial.begin(115200);
    rtc.begin();
    RTC->MODE2.FREQCORR.reg = 0x7A;
    DateTime now = DateTime(F(__DATE__), F(__TIME__));
     //!!! notic The year is limited to 2000-2099
    while (!Serial) ;
    Serial.println("adjust time!");
    rtc.adjust(now);
    Serial.print(" FREQCORR: ");
    Serial.println(RTC->MODE2.FREQCORR.reg, HEX);
    
}

int doprint = 1;
void loop()
{
    if (Serial.available()>0) {
        doprint = Serial.parseInt();
        while (Serial.available() > 0) {
            Serial.read();
        }
    }
    if (doprint > 0) {
        DateTime now = rtc.now();

	Serial.print(now.year(), DEC);
	Serial.print('/');
	Serial.print(now.month(), DEC);
	Serial.print('/');
	Serial.print(now.day(), DEC);

	Serial.print(" (");
	Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
	Serial.print(") ");
	Serial.print(now.hour(), DEC);
	Serial.print(':');
	Serial.print(now.minute(), DEC);
	Serial.print(':');
	Serial.print(now.second(), DEC);
	Serial.println();
/***
    Serial.print(" since midnight 1/1/1970 = ");
    Serial.print(now.unixtime());
    Serial.print("s = ");
    Serial.print(now.unixtime() / 86400L);
    Serial.println("d");

    // calculate a date which is 7 days, 12 hours, 30 minutes, and 6 seconds into the future
    DateTime future(now + TimeSpan(7, 12, 30, 6));

    Serial.print(" now + 7d + 12h + 30m + 6s: ");
    Serial.print(future.year(), DEC);
    Serial.print('/');
    Serial.print(future.month(), DEC);
    Serial.print('/');
    Serial.print(future.day(), DEC);
    Serial.print(' ');
    Serial.print(future.hour(), DEC);
    Serial.print(':');
    Serial.print(future.minute(), DEC);
    Serial.print(':');
    Serial.print(future.second(), DEC);
    Serial.println();

    Serial.println();
    ***/
	doprint = 0;
    }
    delay(1000);
}
