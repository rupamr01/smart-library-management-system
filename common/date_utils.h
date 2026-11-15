#ifndef COMMON_DATE_UTILS_H
#define COMMON_DATE_UTILS_H

#include <string>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <cctype>
using namespace std;

inline bool isLeapYear(int year)
{
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

inline int daysInMonth(int year, int month)
{
    static const int days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    if (month == 2 && isLeapYear(year))
    {
        return 29;
    }

    return days[month - 1];
}

inline bool isValidDate(int year, int month, int day)
{
    if (year < 1 || month < 1 || month > 12 || day < 1)
    {
        return false;
    }

    return day <= daysInMonth(year, month);
}

inline bool parseDate(const string& text, int& year, int& month, int& day)
{
    if (text.size() != 10 || text[4] != '-' || text[7] != '-')
    {
        return false;
    }

    for (size_t i = 0; i < text.size(); ++i)
    {
        if (i == 4 || i == 7)
        {
            continue;
        }

        if (!isdigit(static_cast<unsigned char>(text[i])))
        {
            return false;
        }
    }

    year = (text[0] - '0') * 1000 + (text[1] - '0') * 100 + (text[2] - '0') * 10 + (text[3] - '0');
    month = (text[5] - '0') * 10 + (text[6] - '0');
    day = (text[8] - '0') * 10 + (text[9] - '0');

    return isValidDate(year, month, day);
}

inline long long toSerialDays(int year, int month, int day)
{
    year -= (month <= 2);
    long long era = (year >= 0 ? year : year - 399) / 400;
    unsigned int yearOfEra = static_cast<unsigned int>(year - era * 400);
    unsigned int dayOfYear = (153 * (month + (month > 2 ? -3 : 9)) + 2) / 5 + day - 1;
    unsigned int dayOfEra = yearOfEra * 365 + yearOfEra / 4 - yearOfEra / 100 + dayOfYear;

    return era * 146097 + static_cast<long long>(dayOfEra) - 719468;
}

inline void fromSerialDays(long long serial, int& year, int& month, int& day)
{
    serial += 719468;
    long long era = (serial >= 0 ? serial : serial - 146096) / 146097;
    unsigned int dayOfEra = static_cast<unsigned int>(serial - era * 146097);
    unsigned int yearOfEra = (dayOfEra - dayOfEra / 1460 + dayOfEra / 36524 - dayOfEra / 146096) / 365;
    year = static_cast<int>(yearOfEra) + static_cast<int>(era * 400);
    unsigned int dayOfYear = dayOfEra - (365 * yearOfEra + yearOfEra / 4 - yearOfEra / 100);
    unsigned int monthPart = (5 * dayOfYear + 2) / 153;
    day = static_cast<int>(dayOfYear - (153 * monthPart + 2) / 5 + 1);
    month = static_cast<int>(monthPart + (monthPart < 10 ? 3 : -9));
    year += (month <= 2);
}

inline string formatDate(int year, int month, int day)
{
    ostringstream output;
    output << setfill('0') << setw(4) << year << '-'
           << setw(2) << month << '-' << setw(2) << day;
    return output.str();
}

inline string todayString()
{
    time_t now = time(nullptr);
    tm* local = localtime(&now);

    return formatDate(local->tm_year + 1900, local->tm_mon + 1, local->tm_mday);
}

inline string addDays(const string& date, int days)
{
    int year = 0;
    int month = 0;
    int day = 0;

    if (!parseDate(date, year, month, day))
    {
        return "";
    }

    fromSerialDays(toSerialDays(year, month, day) + days, year, month, day);
    return formatDate(year, month, day);
}

inline int daysBetween(const string& from, const string& to)
{
    int fromYear = 0;
    int fromMonth = 0;
    int fromDay = 0;
    int toYear = 0;
    int toMonth = 0;
    int toDay = 0;

    if (!parseDate(from, fromYear, fromMonth, fromDay) ||
        !parseDate(to, toYear, toMonth, toDay))
    {
        return 0;
    }

    return static_cast<int>(toSerialDays(toYear, toMonth, toDay) -
                            toSerialDays(fromYear, fromMonth, fromDay));
}

#endif
