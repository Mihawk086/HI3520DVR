#ifndef CCONFIG_H
#define CCONFIG_H
#include <string>
#include <iostream>

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

#include "json/json-forwards.h"
#include "json/json.h"
using namespace std;
enum RecordMode
{
    AutoRecord,
    Timing,
    Close
};
enum Standard
{
    Standard_PAL,
    Standard_NTSC,
    Standard_720P,
    Standard_1080P
};
enum Result
{
    _D1,
    _CIF,
    _HD1,
    _720p,
    _1080P
};
enum CodeRate
{
    LOW,
    MIDDLE,
    HIGH
};

enum FileSize
{
    _10min,
    _15min,
    _30min
};
typedef struct T_ChannalSetting
{
    int channalnum;
    bool enable;
    enum Result result;
    enum CodeRate coderate;
    bool mirror;
}T_ChannalSetting;
typedef struct T_RecordMenu
{
    RecordMode recordmode;
    Standard standard;
    enum FileSize filesize;
    bool audio;
}T_RecordMenu;
typedef struct T_TimeConfig
{
    unsigned int starthour;
    unsigned int startmin;
    unsigned int endhour;
    unsigned int endmin;
}T_TimeConfig;
typedef struct T_SubChannalSetting
{
    enum Result result;
    enum CodeRate coderate;
}T_SubChannalSetting;
class CConfig
{
public:
    static CConfig* get_instance();
    bool CreateJSON();
    bool WriteJSON();
    bool ReadJSON();
    bool PrintJSON();
    T_RecordMenu GetRecordMenu();
    T_TimeConfig GetTimeConfig();
    T_SubChannalSetting GetSubChannalSetting();
    T_ChannalSetting GetChannalSetting(int channal);
    bool SetChannalSetting(T_ChannalSetting t_channalsetting,int channal);
    bool SetRecordMenu(T_RecordMenu t_recordmenu);
    bool SetTimeConfig(T_TimeConfig t_timeconfig);
    bool SetSubChannalSetting(T_SubChannalSetting t_subchannalsetting);
    string getConfigFilePath();
public:
    static CConfig* _instance;
private:
    CConfig();
    string m_configpath;
    Json::Value m_RecordMenu;
    Json::Value m_ChannalMenu;
    Json::Value m_TimeConfig;
    Json::Value m_SubChannal;
    string m_read;
    T_RecordMenu m_tRecordMenu;
    T_ChannalSetting m_tChannalMenu[4];
    T_TimeConfig m_ttimeconfig;
    T_SubChannalSetting m_tsubchannalsetting;
};
#endif // CCONFIG_H
