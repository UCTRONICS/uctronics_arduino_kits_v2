

#ifndef __RTCDS1302_H__
#define __RTCDS1302_H__

#include <Arduino.h>
#include "RtcDateTime.h"
#include "RtcUtility.h"


const uint8_t daysArray [] PROGMEM = { 31,28,31,30,31,30,31,31,30,31,30,31 };
const uint8_t dowArray[] PROGMEM = { 0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4 };

//DS1302 Register Addresses
const uint8_t DS1302_REG_TIMEDATE   = 0x80;
const uint8_t DS1302_REG_TIMEDATE_BURST = 0xBE;
const uint8_t DS1302_REG_TCR    = 0x90;
const uint8_t DS1302_REG_RAM_BURST = 0xFE;
const uint8_t DS1302_REG_RAMSTART   = 0xc0;
const uint8_t DS1302_REG_RAMEND     = 0xfd;
// ram read and write addresses are interleaved
const uint8_t DS1302RamSize = 31;


struct DateTime
{
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    uint8_t dayOfWeek;
    uint32_t unixtime;
};

// DS1302 Trickle Charge Control Register Bits
enum DS1302TcrResistor {
    DS1302TcrResistor_Disabled = 0,
    DS1302TcrResistor_2KOhm = B00000001,
    DS1302TcrResistor_4KOhm = B00000010,
    DS1302TcrResistor_8KOhm = B00000011,
    DS1302TcrResistor_MASK  = B00000011,
};

enum DS1302TcrDiodes {
    DS1302TcrDiodes_None = 0,
    DS1302TcrDiodes_One      = B00000100,
    DS1302TcrDiodes_Two      = B00001000,
    DS1302TcrDiodes_Disabled = B00001100,
    DS1302TcrDiodes_MASK     = B00001100,
};

enum DS1302TcrStatus {
    DS1302TcrStatus_Enabled  = B10100000,
    DS1302TcrStatus_Disabled = B01010000,
    DS1302TcrStatus_MASK     = B11110000,
};

const uint8_t DS1302Tcr_Disabled = DS1302TcrStatus_Disabled | DS1302TcrDiodes_Disabled | DS1302TcrResistor_Disabled;

// DS1302 Clock Halt Register & Bits
const uint8_t DS1302_REG_CH = 0x80; // bit in the seconds register
const uint8_t DS1302_CH     = 7;

// Write Protect Register & Bits
const uint8_t DS1302_REG_WP = 0x8E; 
const uint8_t DS1302_WP = 7;

template<class T_WIRE_METHOD> class RtcDS1302
{
public:
    RtcDS1302(T_WIRE_METHOD& wire) :
        _wire(wire)
    {
    }

    void Begin()
    {
        _wire.begin();
    }

    
    void Begin(int sda, int scl)
    {
        _wire.begin(sda, scl);
    }

    bool GetIsWriteProtected()
    {
        uint8_t wp = getReg(DS1302_REG_WP);
        return !!(wp & _BV(DS1302_WP));
    }

    void SetIsWriteProtected(bool isWriteProtected)
    {
        uint8_t wp = getReg(DS1302_REG_WP);
        if (isWriteProtected)
        {
            wp |= _BV(DS1302_WP);
        }
        else
        {
            wp &= ~_BV(DS1302_WP);
        }
        setReg(DS1302_REG_WP, wp);
    }

    bool IsDateTimeValid()
    {
        return GetDateTime().IsValid();
    }

    bool GetIsRunning()
    {
        uint8_t ch = getReg(DS1302_REG_CH);
        return !(ch & _BV(DS1302_CH));
    }

    void SetIsRunning(bool isRunning)
    {
        uint8_t ch = getReg(DS1302_REG_CH);
        if (isRunning)
        {
            ch &= ~_BV(DS1302_CH);
        }
        else
        {
            ch |= _BV(DS1302_CH);
        }
        setReg(DS1302_REG_CH, ch);
    }

    uint8_t GetTrickleChargeSettings()
    {
        uint8_t setting = getReg(DS1302_REG_TCR);
        return setting;
    }

    void SetTrickleChargeSettings(uint8_t setting)
    {
        if ((setting & DS1302TcrResistor_MASK) == DS1302TcrResistor_Disabled) {
            // invalid resistor setting, set to disabled
            setting = DS1302Tcr_Disabled;
            goto apply;
        }
        if ((setting & DS1302TcrDiodes_MASK) == DS1302TcrDiodes_Disabled ||
            (setting & DS1302TcrDiodes_MASK) == DS1302TcrDiodes_None) {
            // invalid diode setting, set to disabled
            setting = DS1302Tcr_Disabled;
            goto apply;
        }
        if ((setting & DS1302TcrStatus_MASK) != DS1302TcrStatus_Enabled) {
            // invalid status setting, set to disabled
            setting = DS1302Tcr_Disabled;
            goto apply;
        }

     apply:
        setReg(DS1302_REG_TCR, setting);
    }
    // uint8_t dec2bcd(uint8_t dec)
    // {
    //   return ((dec / 10) * 16) + (dec % 10);
    // }

    
    void setDateTime(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second,const RtcDateTime& dt)
    {
        // set the date time
        _wire.beginTransmission(DS1302_REG_TIMEDATE_BURST);

        _wire.write(Uint8ToBcd(second));
        _wire.write(Uint8ToBcd(minute));
        _wire.write(Uint8ToBcd(hour)); // 24 hour mode only
        _wire.write(Uint8ToBcd(day));
        _wire.write(Uint8ToBcd(month));
        // RTC Hardware Day of Week is 1-7, 1 = Monday
        // convert our Day of Week to Rtc Day of Week
        uint8_t rtcDow = RtcDateTime::ConvertDowToRtc(dt.DayOfWeek());
        _wire.write(Uint8ToBcd(rtcDow));
        _wire.write(Uint8ToBcd(year - 2000));
        _wire.write(0); // no write protect, as all of this is ignored if it is protected
        _wire.endTransmission();
    }

    void SetDateTime(const RtcDateTime& dt)
    {
        // set the date time
        _wire.beginTransmission(DS1302_REG_TIMEDATE_BURST);

        _wire.write(Uint8ToBcd(dt.Second()));
        _wire.write(Uint8ToBcd(dt.Minute()));
        _wire.write(Uint8ToBcd(dt.Hour())); // 24 hour mode only
        _wire.write(Uint8ToBcd(dt.Day()));
        _wire.write(Uint8ToBcd(dt.Month()));

        // RTC Hardware Day of Week is 1-7, 1 = Monday
        // convert our Day of Week to Rtc Day of Week
        uint8_t rtcDow = RtcDateTime::ConvertDowToRtc(dt.DayOfWeek());

        _wire.write(Uint8ToBcd(rtcDow));
        _wire.write(Uint8ToBcd(dt.Year() - 2000));
        _wire.write(0); // no write protect, as all of this is ignored if it is protected

        _wire.endTransmission();
    }
    bool isLeapYear(uint16_t year)
    {
        return (year % 4 == 0);
    }
    char *strDaySufix(uint8_t day)
    {
        if (day % 10 == 1)
        {
            return "st";
        } else
        if (day % 10 == 2)
        {
            return "nd";
        }
        if (day % 10 == 3)
        {
            return "rd";
        }

        return "th";
    }
    char *strAmPm(uint8_t hour, bool uppercase)
    {
        if (hour < 12)
        {
            if (uppercase)
            {
                return "AM";
            } else
            {
                return "am";
            }
        } else
        {
            if (uppercase)
            {
                return "PM";
            } else
            {
                return "pm";
            }
        }
    }
    char *strMonth(uint8_t month)
    {
        switch (month) {
            case 1:
                return "January";
                break;
            case 2:
                return "February";
                break;
            case 3:
                return "March";
                break;
            case 4:
                return "April";
                break;
            case 5:
                return "May";
                break;
            case 6:
                return "June";
                break;
            case 7:
                return "July";
                break;
            case 8:
                return "August";
                break;
            case 9:
                return "September";
                break;
            case 10:
                return "October";
                break;
            case 11:
                return "November";
                break;
            case 12:
                return "December";
                break;
            default:
                return "Unknown";
        }
    }
    uint8_t hour12(uint8_t hour24)
    {
        if (hour24 == 0)
        {
            return 12;
        }

        if (hour24 > 12)
        {
        return (hour24 - 12);
        }

        return hour24;
    }

    uint8_t daysInMonth(uint16_t year, uint8_t month)
    {
        uint8_t days;

        days = pgm_read_byte(daysArray + month - 1);

        if ((month == 2) && isLeapYear(year))
        {
            ++days;
        }

        return days;
    }

    long time2long(uint16_t days, uint8_t hours, uint8_t minutes, uint8_t seconds)
    {
        return ((days * 24L + hours) * 60 + minutes) * 60 + seconds;
    }
    uint32_t unixtime(void)
    {
        uint32_t u;

        u = time2long(date2days(Timemessage.year, Timemessage.month, Timemessage.day), Timemessage.hour, Timemessage.minute, Timemessage.second);
        u += 946681200;

        return u;
    }
    DateTime DataTime()
    {
        _wire.beginTransmission(DS1302_REG_TIMEDATE_BURST | THREEWIRE_READFLAG);
        uint8_t second = BcdToUint8(_wire.read() & 0x7F);
        uint8_t minute = BcdToUint8(_wire.read());
        uint8_t hour = BcdToBin24Hour(_wire.read());
        uint8_t day = BcdToUint8(_wire.read());
        uint8_t month = BcdToUint8(_wire.read());

        uint8_t dayOfWeek=BcdToUint8(_wire.read());  

        uint16_t year = BcdToUint8(_wire.read()) + 2000;

        _wire.read();  // throwing away write protect flag
        _wire.endTransmission();
        Timemessage.year=year;
        Timemessage.month=month;
        Timemessage.day=day;
        Timemessage.hour=hour;
        Timemessage.minute=minute;
        Timemessage.second=second;
        Timemessage.dayOfWeek=dayOfWeek;
        Timemessage.unixtime = unixtime();
        return Timemessage;
    }
    RtcDateTime GetDateTime()
    {
        _wire.beginTransmission(DS1302_REG_TIMEDATE_BURST | THREEWIRE_READFLAG);

        uint8_t second = BcdToUint8(_wire.read() & 0x7F);
        uint8_t minute = BcdToUint8(_wire.read());
        uint8_t hour = BcdToBin24Hour(_wire.read());
        uint8_t dayOfMonth = BcdToUint8(_wire.read());
        uint8_t month = BcdToUint8(_wire.read());

       uint8_t week=BcdToUint8(_wire.read());  // throwing away day of week as we calculate it

        uint16_t year = BcdToUint8(_wire.read()) + 2000;

        _wire.read();  // throwing away write protect flag

        _wire.endTransmission();
        return RtcDateTime(year, month, dayOfMonth, hour, minute, second);
    }
    uint16_t date2days(uint16_t year, uint8_t month, uint8_t day)
    {
        year = year - 2000;

        uint16_t days16 = day;

        for (uint8_t i = 1; i < month; ++i)
        {
            days16 += pgm_read_byte(daysArray + i - 1);
        }

        if ((month == 2) && isLeapYear(year))
        {
            ++days16;
        }

        return days16 + 365 * year + (year + 3) / 4 - 1;
    }
    uint16_t dayInYear(uint16_t year, uint8_t month, uint8_t day)
    {
        uint16_t fromDate;
        uint16_t toDate;

        fromDate = date2days(year, 1, 1);
        toDate = date2days(year, month, day);

        return (toDate - fromDate);
    }
    char *strDayOfWeek(uint8_t dayOfWeek)
    {
        switch (dayOfWeek) {
            case 1:
                return "Monday";
                break;
            case 2:
                return "Tuesday";
                break;
            case 3:
                return "Wednesday";
                break;
            case 4:
                return "Thursday";
                break;
            case 5:
                return "Friday";
                break;
            case 6:
                return "Saturday";
                break;
            case 7:
                return "Sunday";
                break;
            default:
                return "Unknown";
        }
    }
    char* dateFormat(const char* dateFormat, DateTime dt,char* buffer)
    {

        buffer[0] = 0;
        char helper[11]={0};
        while (*dateFormat != '\0')
        {
            switch (dateFormat[0])
            {
                // Day decoder
                case 'd':
                    sprintf(helper, "%02d", dt.day); 
                    strcat(buffer, (const char *)helper); 
                    break;
                case 'j':
                    sprintf(helper, "%d", dt.day);
                    strcat(buffer, (const char *)helper);
                    break;
                case 'l':
                    strcat(buffer, (const char *)strDayOfWeek(dt.dayOfWeek));
                    break;
                case 'D':
                    strncat(buffer, strDayOfWeek(dt.dayOfWeek), 3);
                    break;
                case 'N':
                    sprintf(helper, "%d", dt.dayOfWeek);
                    strcat(buffer, (const char *)helper);
                    break;
                case 'w':
                    sprintf(helper, "%d", (dt.dayOfWeek + 7) % 7);
                    strcat(buffer, (const char *)helper);
                    break;
                case 'z':
                    sprintf(helper, "%d", dayInYear(dt.year, dt.month, dt.day));
                    strcat(buffer, (const char *)helper);
                    break;
                case 'S':
                    strcat(buffer, (const char *)strDaySufix(dt.day));
                    break;

                // Month decoder
                case 'm':
                    sprintf(helper, "%02d", dt.month);
                    strcat(buffer, (const char *)helper);
                    break;
                case 'n':
                    sprintf(helper, "%d", dt.month);
                    strcat(buffer, (const char *)helper);
                    break;
                case 'F':
                    strcat(buffer, (const char *)strMonth(dt.month));
                    break;
                case 'M':
                    strncat(buffer, (const char *)strMonth(dt.month), 3);
                    break;
                case 't':
                    sprintf(helper, "%d", daysInMonth(dt.year, dt.month));
                    strcat(buffer, (const char *)helper);
                    break;

                // Year decoder
                case 'Y':
                    sprintf(helper, "%d", dt.year); 
                    strcat(buffer, (const char *)helper); 
                    break;
                case 'y': sprintf(helper, "%02d", dt.year-2000);
                    strcat(buffer, (const char *)helper);
                    break;
                case 'L':
                    sprintf(helper, "%d", isLeapYear(dt.year)); 
                    strcat(buffer, (const char *)helper); 
                    break;

                // Hour decoder
                case 'H':
                    sprintf(helper, "%02d", dt.hour);
                    strcat(buffer, (const char *)helper);
                    break;
                case 'G':
                    sprintf(helper, "%d", dt.hour);
                    strcat(buffer, (const char *)helper);
                    break;
                case 'h':
                    sprintf(helper, "%02d", hour12(dt.hour));
                    strcat(buffer, (const char *)helper);
                    break;
                case 'g':
                    sprintf(helper, "%d", hour12(dt.hour));
                    strcat(buffer, (const char *)helper);
                    break;
                case 'A':
                    strcat(buffer, (const char *)strAmPm(dt.hour, true));
                    break;
                case 'a':
                    strcat(buffer, (const char *)strAmPm(dt.hour, false));
                    break;

                // Minute decoder
                case 'i': 
                    sprintf(helper, "%02d", dt.minute);
                    strcat(buffer, (const char *)helper);
                    break;

                // Second decoder
                case 's':
                    sprintf(helper, "%02d", dt.second); 
                    strcat(buffer, (const char *)helper); 
                    break;

                // Misc decoder
                case 'U': 
                    sprintf(helper, "%lu", dt.unixtime);
                    strcat(buffer, (const char *)helper);
                    break;

                default: 
                    strncat(buffer, dateFormat, 1);
                    break;
            }
            dateFormat++;
        }

        return buffer;
    }
    void SetMemory(uint8_t memoryAddress, uint8_t value)
    {
        // memory addresses interleaved read and write addresses
        // so we need to calculate the offset
        uint8_t address = memoryAddress * 2 + DS1302_REG_RAMSTART;
        if (address <= DS1302_REG_RAMEND)
        {
            setReg(address, value);
        }
    }

    uint8_t GetMemory(uint8_t memoryAddress)
    {
        uint8_t value = 0;
        // memory addresses interleaved read and write addresses
        // so we need to calculate the offset
        uint8_t address = memoryAddress * 2 + DS1302_REG_RAMSTART;
        if (address <= DS1302_REG_RAMEND)
        {
            value = getReg(address);
        }
        return value;
    }

    uint8_t SetMemory(const uint8_t* pValue, uint8_t countBytes)
    {
        uint8_t countWritten = 0;

        _wire.beginTransmission(DS1302_REG_RAM_BURST);

        while (countBytes > 0 && countWritten < DS1302RamSize)
        {
            _wire.write(*pValue++);
            countBytes--;
            countWritten++;
        }

        _wire.endTransmission();

        return countWritten;
    }

    uint8_t GetMemory(uint8_t* pValue, uint8_t countBytes)
    {
        uint8_t countRead = 0;

        _wire.beginTransmission(DS1302_REG_RAM_BURST | THREEWIRE_READFLAG);

        while (countBytes > 0 && countRead < DS1302RamSize)
        {
            *pValue++ = _wire.read();
            countRead++;
            countBytes--;
        }

        _wire.endTransmission();

        return countRead;
    }

private:
    T_WIRE_METHOD& _wire;
    DateTime Timemessage;

    uint8_t getReg(uint8_t regAddress)
    {

        _wire.beginTransmission(regAddress | THREEWIRE_READFLAG);
        uint8_t regValue = _wire.read();
        _wire.endTransmission();
        return regValue;
    }

    void setReg(uint8_t regAddress, uint8_t regValue)
    {
        _wire.beginTransmission(regAddress);
        _wire.write(regValue);
        _wire.endTransmission();
    }
};

#endif // __RTCDS1302_H__