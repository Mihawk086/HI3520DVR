#include "Config_Client.h"
#include<cstdio>
#include<cstdlib>
#include<cstring>
#include<sys/stat.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<sys/un.h>
#include<errno.h>
#include<stddef.h>
#include<unistd.h>
#include <iostream>
using namespace std;
Config_Client &Config_Client::get_instance() {
    static Config_Client config_client;
    return config_client;
}
static const char *filename="uds-tmp";
Config_Client::Config_Client(){
    if(access(INICONFIG_PATH,0) == 0) {
        m_inifile.load(INICONFIG_PATH);
    }
    else{
        m_inifile.setValue("COMMON","ip","192.168.1.105","");
        m_inifile.setValue("COMMON","user","root","");
        m_inifile.setValue("COMMON","password","123456","");
        for(int i = 1; i <= 4; i++){
            char buf[20];
            sprintf(buf,"CHANNEL_%d",i);
            m_inifile.setValue(buf,"Enable","OFF");
            m_inifile.setValue(buf,"Result","720P");
            m_inifile.setValue(buf,"Mirror","OFF");
            m_inifile.setValue(buf,"BitRate","Middle");
        }
        m_inifile.setValue("BASESETTING","Standard","720P");
        m_inifile.setValue("BASESETTING","Audio","OFF");
        m_inifile.setValue("BASESETTING","FileSize","10min");
        m_inifile.setValue("BASESETTING","RecordMode","Auto");
        m_inifile.setValue("SUBCHANNEL","Result","D1");
        m_inifile.setValue("SUBCHANNEL","BitRate","Middle");
        m_inifile.setValue("TIMERECORD","StartTime","00::00");
        m_inifile.setValue("TIMERECORD","EndTime","24::00");
        m_inifile.saveas(INICONFIG_PATH);
    }
}
bool Config_Client::send_change_ipc() {
    struct sockaddr_un un;
    int sock_fd;
    un.sun_family = AF_UNIX;
    strcpy(un.sun_path,filename);
    sock_fd = socket(AF_UNIX,SOCK_STREAM,0);
    if(sock_fd < 0){
        printf("Request socket failed\n");
        return -1;
    }
    if(connect(sock_fd,(struct sockaddr *)&un,sizeof(un)) < 0){
        printf("connect socket failed\n");
        return -1;
    }
    send(sock_fd,"change",100,0);

    close(sock_fd);
    return 0;
}
T_TimeConfig Config_Client::get_time_config() {
    T_TimeConfig t;
    string starttime;
    string stoptime;
    int start_hour,start_minute,end_hour,end_minute;
    m_inifile.getValue("TIMERECORD","StartTime",starttime);
    m_inifile.getValue("TIMERECORD","EndTime",stoptime);
    sscanf(starttime.c_str(),"%2d::%2d",&start_hour,&start_minute);
    sscanf(stoptime.c_str(),"%2d::%2d",&end_hour,&end_minute);
    t.starthour = start_hour;
    t.endhour = end_hour;
    t.startmin = start_minute;
    t.endmin = end_minute;
    return t;
}
T_SubChannalSetting Config_Client::get_sub_channel_config() {
    T_SubChannalSetting t;
    string result,bitrate;
    m_inifile.getValue("SUBCHANNEL","Result",result);
    m_inifile.getValue("SUBCHANNEL","BitRate",bitrate);
    if(result == string("D1")){
        t.result = _D1;
    }
    else if(result == string("CIF")){
        t.result = _CIF;
    }
    else if(result == string("HD1")){
        t.result = _HD1;
    }

    if(bitrate == string("High")){
        t.coderate = HIGH;
    }
    else if(bitrate == string("Middle")){
        t.coderate == MIDDLE;
    }
    else if(bitrate == string("Low")){
        t.coderate = LOW;
    }
    return t;
}
T_RecordMenu Config_Client::get_base_config() {
    string standard,audio,filesize,recordmode;
    T_RecordMenu t;
    m_inifile.getValue("BASESETTING","Standard",standard);
    m_inifile.getValue("BASESETTING","Audio",audio);
    m_inifile.getValue("BASESETTING","FileSize",filesize);
    m_inifile.getValue("BASESETTING","RecordMode",recordmode);
    if(audio == string("ON")){
        t.audio = true;
    }
    else if(audio == string("OFF")){
        t.audio = false;
    }
    if(standard == string("720P")){
        t.standard = Standard_720P;
    }
    else if(standard == string("1080P")){
        t.standard = Standard_1080P;
    }
    else if(standard == string("PAL")){
        t.standard = Standard_PAL;
    }
    else if(standard == string("NTSC")){
        t.standard = Standard_NTSC;
    }
    if(filesize == string("10min")){
        t.filesize = _10min;
    } else if(filesize == string("15min")){
        t.filesize = _15min;
    } else if(filesize == string("30min")){
        t.filesize = _30min;
    }

    if(recordmode == string("Auto")){
        t.recordmode = AutoRecord;
    }
    else if(recordmode == string("OFF")){
        t.recordmode = Close;
    }
    else if(recordmode == string("Timing")){
        t.recordmode = Timing;
    }
    return t;
}
T_ChannalSetting Config_Client::get_channel_config(int channel) {
    if(channel > 4 || channel <1){
        printf("input channel error\n");
        return T_ChannalSetting();
    }
    T_ChannalSetting t;
    string enable,result,mirror,bitrate;
    char channel_str[20];
    sprintf(channel_str,"CHANNEL_%d",channel);
    m_inifile.getValue(channel_str,"Enable",enable);
    m_inifile.getValue(channel_str,"Result",result);
    m_inifile.getValue(channel_str,"Mirror",mirror);
    m_inifile.getValue(channel_str,"BitRate",bitrate);
    t.channalnum = channel;
    if(mirror == string("ON")){
        t.mirror = true;
    }
    else if(mirror == string("OFF")){
        t.mirror = false;
    }
    if(enable == string("ON")){
        t.enable = true;
    }
    else if(enable == string("OFF")){
        t.enable = false;
    }
    if(result == string("D1")){
        t.result = _D1;
    }
    else if(result == string("CIF")){
        t.result = _CIF;
    }
    else if(result == string("720P")){
        t.result = _720p;
    }
    if(bitrate == string("High")){
        t.coderate = HIGH;
    }
    else if(bitrate == string("Middle")){
        t.coderate == MIDDLE;
    }
    else if(bitrate == string("Low")){
        t.coderate = LOW;
    }
    return t;
}
bool Config_Client::wrtie_time_config(T_TimeConfig &t) {
    char buf1[256],buf2[256];
    sprintf(buf1,"%d::%d",t.starthour,t.startmin);
    sprintf(buf2,"%d::%d",t.endhour,t.endmin);
    m_inifile.setValue("TIMERECORD","StartTime",string(buf1));
    m_inifile.setValue("TIMERECORD","EndTime",string(buf2));
    m_inifile.saveas(INICONFIG_PATH);
    return true;
}
bool Config_Client::wrtie_sub_channel_config(T_SubChannalSetting &t) {
    switch (t.result) {
        case _D1:
            m_inifile.setValue("SUBCHANNEL", "Result", "D1");
            break;
        case _HD1:
            m_inifile.setValue("SUBCHANNEL", "Result", "HD1");
            break;
        case _CIF:
            m_inifile.setValue("SUBCHANNEL", "Result", "D1");
            break;
    }
    switch(t.coderate) {
        case MIDDLE:
            m_inifile.setValue("SUBCHANNEL", "BitRate", "Middle");
            break;
        case LOW:
            m_inifile.setValue("SUBCHANNEL", "BitRate", "Low");
            break;
        case HIGH:
            m_inifile.setValue("SUBCHANNEL", "BitRate", "High");
            break;
    }
    m_inifile.saveas(INICONFIG_PATH);
    return true;
}
bool Config_Client::write_base_config(T_RecordMenu &t) {
    switch(t.audio){
        case true:
            m_inifile.setValue("BASESETTING","Audio","ON");
            break;
        case false:
            m_inifile.setValue("BASESETTING","Audio","OFF");
            break;
    }
    switch (t.standard){
        case Standard_NTSC:
            m_inifile.setValue("BASESETTING","Standard","NTSC");
            break;
        case Standard_PAL:
            m_inifile.setValue("BASESETTING","Standard","PAL");
            break;
        case Standard_720P:
            m_inifile.setValue("BASESETTING","Standard","720P");
            break;
        case Standard_1080P:
            m_inifile.setValue("BASESETTING","Standard","1080P");
            break;
    }
    switch (t.filesize){
        case _10min:
            m_inifile.setValue("BASESETTING","FileSize","10min");
            break;
        case _15min:
            m_inifile.setValue("BASESETTING","FileSize","15min");
            break;
        case _30min:
            m_inifile.setValue("BASESETTING","FileSize","30min");
            break;
    }
    switch (t.recordmode){
        case AutoRecord:
            m_inifile.setValue("BASESETTING","RecordMode","Auto");
            break;
        case Close:
            m_inifile.setValue("BASESETTING","RecordMode","OFF");
            break;
        case Timing:
            m_inifile.setValue("BASESETTING","RecordMode","Timing");
            break;
    }
    m_inifile.saveas(INICONFIG_PATH);
    return true;
}
bool Config_Client::write_channel_config(int channel, T_ChannalSetting &t) {
    if(channel > 4 || channel <1) {
        printf("input channel error\n");
        return false;
    }
    char channel_str[20];
    sprintf(channel_str,"CHANNEL_%d",channel);
    switch (t.enable){
        case true:
            m_inifile.setValue(channel_str,"Enable","ON");
            break;
        case false:
            m_inifile.setValue(channel_str,"Enable","OFF");
            break;
    }
    switch (t.mirror){
        case true:
            m_inifile.setValue(channel_str,"Mirror","ON");
            break;
        case false:
            m_inifile.setValue(channel_str,"Mirror","OFF");
            break;
    }
    switch (t.result) {
        case _D1:
            m_inifile.setValue(channel_str,"Result", "D1");
            break;
        case _720p:
            m_inifile.setValue(channel_str,"Result", "720P");
            break;
        case _CIF:
            m_inifile.setValue(channel_str,"Result", "CIF");
            break;
    }
    switch(t.coderate) {
        case MIDDLE:
            m_inifile.setValue(channel_str, "BitRate", "Middle");
            break;
        case LOW:
            m_inifile.setValue(channel_str, "BitRate", "Low");
            break;
        case HIGH:
            m_inifile.setValue(channel_str,"BitRate", "High");
            break;
    }
    m_inifile.saveas(INICONFIG_PATH);
    return true;
}

