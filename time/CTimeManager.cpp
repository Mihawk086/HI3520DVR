#include "CTimeManager.h"
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <errno.h>
#include <cctype>
#include "hi_rtc.h"
void usage(void){
    printf("\n"
            "Usage: ./test [options] [parameter1] ...\n"
            "Options: \n"
            "	-s(set)		    Set time/alarm,     e.g '-s time 2012/7/15/13/37/59'\n"
            "	-g(get)		    Get time/alarm,     e.g '-g alarm'\n"
            "	-w(write)	    Write RTC register, e.g '-w <reg> <val>'\n"
            "	-r(ead)		    Read RTC register,  e.g '-r <reg>'\n"
            "	-a(alarm)	    Alarm ON/OFF',      e.g '-a ON'\n"
            "	-reset		    RTC reset\n"
            "	-b(battery monitor) battery ON/OFF,     e.g '-b ON'\n"
            "	-f(requency)        frequency precise adjustment, e.g '-f <val>'\n"
            "\n");
    exit(1);
}
static int _atoul(const char *str, unsigned char *pvalue){
    unsigned int result=0;
    while (*str){
        if (isdigit((int)*str)){
            if ((result<429496729) || ((result==429496729) && (*str<'6'))){
                result = result*10 + (*str)-48;
            }
            else{
                *pvalue = result;
                return -1;
            }
        }else{
            *pvalue=result;
            return -1;
        }
        str++;
    }
    *pvalue=result;
    return 0;
}
#define ASC2NUM(ch) (ch - '0')
#define HEXASC2NUM(ch) (ch - 'A' + 10)
static int  _atoulx(const char *str, unsigned char *pvalue){
    unsigned int   result=0;
    unsigned char  ch;
    while (*str){
        ch=toupper(*str);
        if (isdigit(ch) || ((ch >= 'A') && (ch <= 'F' ))){
            if (result < 0x10000000){
                result = (result << 4) + ((ch<='9')?(ASC2NUM(ch)):(HEXASC2NUM(ch)));
            }else{
                *pvalue=result;
                return -1;
            }
        }else{
            *pvalue=result;
            return -1;
        }
        str++;
    }
    *pvalue=result;
    return 0;
}
static int str_to_num(const char *str, unsigned char *pvalue)
{
    if ( *str == '0' && (*(str+1) == 'x' || *(str+1) == 'X') ){
        if (*(str+2) == '\0'){
            return -1;
        }else{
            return _atoulx(str+2, pvalue);
        }
    }else {
        return _atoul(str,pvalue);
    }
}
static int parse_string(char *string, rtc_time_t *p_tm)
{
    char *comma, *head;
    int value[10];
    int i;
    if (!string || !p_tm)
        return -1;
    if (!strchr(string, '/'))
        return -1;
    head = string;
    i = 0;
    comma = NULL;
    for(;;) {
        comma = strchr(head, '/');
        if (!comma){
            value[i++] = atoi(head);
            break;
        }
        *comma = '\0';
        value[i++] = atoi(head);
        head = comma+1;
    }
    if (i < 5)
        return -1;
    p_tm->year   = value[0];
    p_tm->month  = value[1];
    p_tm->date   = value[2];
    p_tm->hour   = value[3];
    p_tm->minute = value[4];
    p_tm->second = value[5];
    p_tm->weekday = 0;
    return 0;
}
int CTimeManager::GetRtcTime()
{
    rtc_time_t tm;
    int ret = -1;
    int fd = -1;
    const char *dev_name = "/dev/hirtc";
    printf("[RTC_RD_TIME]\n");
    fd = open(dev_name, O_RDWR);
    if (!fd){
        printf("open %s failed\n", dev_name);
        return -1;
    }
    ret = ioctl(fd, HI_RTC_RD_TIME, &tm);
    if (ret < 0){
        printf("ioctl: HI_RTC_RD_TIME failed\n");
        goto err1;
    }
    m_tm = tm;
    close(fd);
    return 0;
    err1:
    close(fd);
    return -1;
}
int CTimeManager::SetRtcTime()
{
    rtc_time_t tm;
    int ret = -1;
    int fd = -1;
    const char *dev_name = "/dev/hi_rtc";
    rtc_time_t set_tm;
    set_tm.year = 2018;
    set_tm.month = 6;
    set_tm.date = 20;
    set_tm.hour = 16;
    set_tm.minute = 55;
    set_tm.second = 0;
    set_tm.weekday = 0;
    tm = set_tm;
    printf("set time\n");
    fd = open(dev_name, O_RDWR);
    if (!fd)
    {
        printf("open %s failed\n", dev_name);
        return -1;
    }
    printf("year:%d\n", tm.year);
    printf("month:%d\n",tm.month);
    printf("date:%d\n", tm.date);
    printf("hour:%d\n", tm.hour);
    printf("minute:%d\n", tm.minute);
    printf("second:%d\n", tm.second);
    ret = ioctl(fd, HI_RTC_SET_TIME, &tm);
    if (ret < 0){
        printf("ioctl: HI_RTC_SET_TIME failed\n");
        goto err1;
    }
    close(fd);
    return 0;
    err1:
    close(fd);
    return -1;
}
int CTimeManager::SetSystemTime()
{
    GetRtcTime();
    rtc_time_t time;
    struct tm t;
    time_t tt;
    int ret = -1;
    time = m_tm;
    t.tm_year = time.year - 1900;
    t.tm_mon = time.month - 1;
    t.tm_mday = time.date;
    t.tm_hour = time.hour;
    t.tm_min = time.minute;
    t.tm_sec = time.second;
    tt = mktime(&t);
    ret = stime(&tt);
    if(ret < 0)
    {
        printf("stime error\n");
        printf("errno = %d\n",errno);
    }
    return 0;
}
CTimeManager *CTimeManager::get_instance(){
    return pCTimerManager;
}
CTimeManager* CTimeManager::pCTimerManager = new CTimeManager();
CTimeManager::CTimeManager(){
}
