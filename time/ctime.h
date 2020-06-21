#ifndef CTIME_H
#define CTIME_H
#include <time.h>
#include <string>
#include <stdio.h>
typedef struct TIMESTRUCT
{
    int year;
    int month;
    int day;
    int hour;
    int minute;
    int second;
}TIMESTRUCT;
using namespace std;
class Ctime
{
public:
    Ctime();
    bool SetCurrentTime();
    bool SetTime(TIMESTRUCT &time);
    TIMESTRUCT GetNormalTime();
    bool CompareDate(Ctime &date);
    int get_year();
    int get_month();
    int get_day();
    int get_hour();
    int get_minute();
    int get_second();
    int get_msec();
    int get_week();
    int set_system_time(int year, int month, int day, int hour, int minute, int second);
    int get_system_time(int *year, int *month, int *day, int *hour, int *minute, int *second);
    int get_week_day(int year, int month, int day);
    int get_day_num(int year, int month);
    int string_to_time(char *pStr, unsigned long *start_hour, unsigned long *start_min, unsigned long *end_hour, unsigned long *end_min);
public:
    TIMESTRUCT g_time;
    unsigned int m_utc_time;
    struct tm m_tm;
};
#endif // CTIME_H
