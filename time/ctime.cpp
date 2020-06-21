#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <string.h>
#include "ctime.h"
Ctime::Ctime()
{
}
bool Ctime::SetCurrentTime()
{
    struct tm *t;
    time_t tt;
    time(&tt);
    t = localtime(&tt);
    g_time.year = t->tm_year + 1900;
    g_time.month = t->tm_mon + 1;
    g_time.day = t->tm_mday;
    g_time.hour = t->tm_hour;
    g_time.minute =  t->tm_min;
    g_time.second = t->tm_sec;
    this->m_utc_time = tt;
    this->m_tm = *t;
    return true;
}
bool Ctime::SetTime(TIMESTRUCT &time)
{
    struct tm t;
    time_t tt;
    t.tm_year = time.year - 1900;
    t.tm_mon = time.month - 1;
    t.tm_mday = time.day;
    t.tm_hour = time.hour;
    t.tm_min = time.minute;
    t.tm_sec = time.second;
    tt = mktime(&t);
    this->m_utc_time = tt;
    this->m_tm = t;
}
bool Ctime::CompareDate(Ctime &Cdate)
{
    struct TIMESTRUCT g_date;
    g_date = Cdate.g_time;
    if((this->g_time.year == g_date.year)&&
            (this->g_time.month == g_date.month)&&
            (this->g_time.day == g_date.day))
    {
        return true;
    }
    else
    {
        return false;
    }
}
TIMESTRUCT Ctime::GetNormalTime()
{
    return this->g_time;
}
int Ctime::get_year()
{
    time_t timep;
    struct tm *p;
    time(&timep);
    p = localtime(&timep);
    return (p->tm_year + 1900);
}
int Ctime::get_month()
{
    time_t timep;
    struct tm *p;
    time(&timep);
    p = localtime(&timep);
    return (p->tm_mon + 1);
}
int Ctime::get_day()
{
    time_t timep;
    struct tm *p;
    time(&timep);
    p = localtime(&timep);

    return p->tm_mday;
}
int Ctime::get_hour()
{
    time_t timep;
    struct tm *p;
    time(&timep);
    p = localtime(&timep);
    return p->tm_hour;
}
int Ctime::get_minute()
{
    time_t timep;
    struct tm *p;
    time(&timep);
    p = localtime(&timep);
    return p->tm_min;
}
int Ctime::get_second()
{
    time_t timep;
    struct tm *p;
    time(&timep);
    p = localtime(&timep);
    return p->tm_sec;
}
int Ctime::get_msec()
{
    struct timeval tv;
    struct timezone tz;
    int val;
    gettimeofday(&tv,&tz);
    val = tv.tv_sec * 1000 + (int)(tv.tv_usec/1000);
    return val;
}
int Ctime::get_week()
{
    time_t timep;
    struct tm *p;
    time(&timep);
    p = localtime(&timep);
    return p->tm_wday;
}
int Ctime::set_system_time(int year,int month,int day,int hour,int minute,int second)
{
    time_t timep;
    struct tm *p;
    struct timeval tv;
    struct timezone tz;
    time(&timep);
    p = localtime(&timep);
    p->tm_year = year - 1900;
    p->tm_mon =  month -1;
    p->tm_mday = day;
    p->tm_hour = hour;
    p->tm_min =  minute;
    p->tm_sec =  second;
    timep = mktime(p);
    gettimeofday(&tv,&tz);
    tv.tv_sec = timep;
    tv.tv_usec = 0;
    settimeofday(&tv,&tz);
    return 0;
}
int Ctime::get_system_time(int *year, int *month, int *day, int *hour, int *minute, int *second)
{
    time_t timep;
    struct tm *p;
    if (year==NULL || month==NULL || day==NULL || hour==NULL || minute==NULL || second==NULL)
    {
        return -1;
    }
    time(&timep);
    p = localtime(&timep);
    *year = p->tm_year + 1900;
    *month = p->tm_mon + 1;
    *day = p->tm_mday;
    *hour = p->tm_hour;
    *minute = p->tm_min;
    *second = p->tm_sec;
    return 0;
}
int Ctime::get_week_day(int year, int month, int day)
{
    struct tm time;
    struct tm *p;
    time_t timep;
    memset(&time,0,sizeof(struct tm));
    time.tm_year = year - 1900;
    time.tm_mon = month - 1;
    time.tm_mday = day;
    timep = mktime(&time);
    p = localtime(&timep);
    if(p->tm_wday == 0)
    {
        return 6;
    }
    else
    {
        return p->tm_wday - 1;
    }
}
int Ctime::get_day_num(int year,int month)
{
    int days[] = {31,28,31,30,31,30,31,31,30,31,30,31};
    if((year % 4 == 0 && year % 100 != 0) || year % 400 == 0)
    {
        if(month == 2)
        {
            return days[month-1] + 1;
        }
        else
        {
            return days[month-1];
        }
    }
    else
    {
        return days[month-1];
    }
}
int Ctime::string_to_time(char *pStr, unsigned long *start_hour, unsigned long *start_min, unsigned long *end_hour, unsigned long *end_min)
{
    char sztmp[5];
    if (pStr==NULL || start_hour==NULL || start_min==NULL || end_hour==NULL || end_min==NULL)
    {
        return -1;
    }
    bzero(sztmp,5);
    memcpy(sztmp,pStr,2);
    *start_hour = atoi(sztmp);
    memcpy(sztmp,pStr+3,2);
    *start_min = atoi(sztmp);
    memcpy(sztmp,pStr+10,2);
    *end_hour = atoi(sztmp);
    memcpy(sztmp,pStr+13,2);
    *end_min = atoi(sztmp);
    return 0;
}



