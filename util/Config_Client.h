#ifndef HISI3520_CONFIG_CLIENT_H
#define HISI3520_CONFIG_CLIENT_H
#include "inifile.h"
#define INICONFIG_PATH "./config.ini"
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
    int starthour;
    int startmin;
    int endhour;
    int endmin;
}T_TimeConfig;
typedef struct T_SubChannalSetting
{
    enum Result result;
    enum CodeRate coderate;
}T_SubChannalSetting;
class Config_Client {
public:
    static Config_Client& get_instance();
    bool send_change_ipc();
    inifile::IniFile m_inifile;
    T_TimeConfig get_time_config();
    T_SubChannalSetting get_sub_channel_config();
    T_RecordMenu get_base_config();
    T_ChannalSetting get_channel_config(int channel);
    bool wrtie_time_config(T_TimeConfig& t);
    bool wrtie_sub_channel_config(T_SubChannalSetting& t);
    bool write_base_config(T_RecordMenu& t);
    bool write_channel_config(int channel,T_ChannalSetting& t);
private:
    Config_Client();
};
#endif //HISI3520_CONFIG_CLIENT_H
