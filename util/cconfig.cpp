#include "cconfig.h"
CConfig* CConfig::_instance = new CConfig();
CConfig::CConfig()
{
    m_configpath = "./config/con.json";
    if(access(m_configpath.c_str(), F_OK) == -1)
    {
        cout<<"open config fail"<<endl;
        CreateJSON();
        WriteJSON();
    }
}
CConfig* CConfig::get_instance(){
    return _instance;
}
bool CConfig::CreateJSON()
{
    Json::Value req;
    Json::Value RecordMenu;
    RecordMenu["RecordMode"] = AutoRecord;
    RecordMenu["Standard"]  = Standard_720P;
    RecordMenu["FileSize"]  = _10min;
    RecordMenu["Audio"] = false;
    Json::Value ChannalMenu;
    Json::Value Channal[4];
    for(int i = 0; i < 4; i++)
    {
        Channal[i]["ChannalNum"] = i+1;
        Channal[i]["Enable"] = true;
        Channal[i]["Result"] = _720p;
        Channal[i]["CodeRate"] = MIDDLE;
        Channal[i]["Mirror"] = false;
        ChannalMenu.append(Channal[i]);
    }
    Json::Value SubChannal;
    SubChannal["Result"] = _CIF;
    SubChannal["CodeRate"] = MIDDLE;
    Json::Value TimeConfig;
    TimeConfig["RecordStartTimeHour"] = 0;
    TimeConfig["RecordStartTimeMin"] = 0;
    TimeConfig["RecordEndTimeHour"] = 24;
    TimeConfig["RecordEndTimeMin"] = 0;
    req["RecordMenu"] = RecordMenu;
    req["ChannalMenu"] =  ChannalMenu;
    req["TimeConfig"] = TimeConfig;
    req["SubChannal"] = SubChannal;
    m_RecordMenu = RecordMenu;
    m_ChannalMenu = ChannalMenu;
    m_TimeConfig = TimeConfig;
    m_SubChannal = SubChannal;
    Json::StyledWriter writer;
    string jsonstr = writer.write(req);
    cout<<jsonstr<<endl;
    return true;
}
bool CConfig::PrintJSON()
{
    Json::Value req;
    req["RecordMenu"] = m_RecordMenu;
    req["ChannalMenu"] =  m_ChannalMenu;
    req["TimeConfig"] = m_TimeConfig;
    req["SubChannal"] = m_SubChannal;
    Json::StyledWriter writer;
    string jsonstr = writer.write(req);
    cout<<jsonstr<<endl;
    cout<<"===="<<endl;
    return true;
}
bool CConfig::WriteJSON()
{
    FILE *fp;
    if ((fp = fopen(m_configpath.c_str(), "w")) == NULL)
    {
        printf("open file %s fail \n", m_configpath.c_str());
        return false;
    }
    Json::Value req;
    req["RecordMenu"] = m_RecordMenu;
    req["ChannalMenu"] = m_ChannalMenu;
    req["TimeConfig"] = m_TimeConfig;
    req["SubChannal"] = m_SubChannal;
    Json::StyledWriter writer;
    string jsonstr = writer.write(req);
    fprintf(fp, "%s", jsonstr.c_str());
    fclose(fp);
    return 0;
}
bool CConfig::ReadJSON()
{
    FILE *fp;
    string str;
    char txt[1000];
    int filesize;
    if ((fp=fopen(m_configpath.c_str(),"r"))==NULL)
    {
        printf("open file %s fail \n",m_configpath.c_str());
        return false;
    }
    fseek(fp,0,SEEK_END);
    filesize = ftell(fp);
    rewind(fp);
    while((fgets(txt,1000,fp))!=NULL){
        str += txt;
    }
    fclose(fp);
    m_read = str;
    Json::Reader reader;
    Json::Value  resp;
    if (!reader.parse(str, resp, false))
    {
        cout<<"bad json format!"<<endl;
        return false;
    }
    m_ChannalMenu = resp["ChannalMenu"];
    m_RecordMenu = resp["RecordMenu"];
    m_TimeConfig = resp["TimeConfig"];
    m_SubChannal = resp["SubChannal"];
    m_tRecordMenu.recordmode = RecordMode(m_RecordMenu["RecordMode"].asInt());
    m_tRecordMenu.standard = Standard(m_RecordMenu["Standard"].asInt());
    m_tRecordMenu.filesize = FileSize(m_RecordMenu["FileSize"].asInt());
    m_tRecordMenu.audio = m_RecordMenu["Audio"].asBool();
    for(int i = 0 ; i < 4 ; i++)
    {
        m_tChannalMenu[i].coderate = CodeRate(m_ChannalMenu[i]["CodeRate"].asInt());
        m_tChannalMenu[i].result = Result(m_ChannalMenu[i]["Result"].asInt());
        m_tChannalMenu[i].enable = m_ChannalMenu[i]["Enable"].asBool();
        m_tChannalMenu[i].mirror = m_ChannalMenu[i]["Mirror"].asBool();
        m_tChannalMenu[i].channalnum = m_ChannalMenu[i]["ChannalNum"].asInt();
    }
    m_ttimeconfig.starthour = m_TimeConfig["RecordStartTimeHour"].asInt();;
    m_ttimeconfig.startmin = m_TimeConfig["RecordStartTimeMin"].asInt();;
    m_ttimeconfig.endhour = m_TimeConfig["RecordEndTimeHour"].asInt();;
    m_ttimeconfig.endmin = m_TimeConfig["RecordEndTimeMin"].asInt();;
    m_tsubchannalsetting.coderate =  CodeRate(m_SubChannal["CodeRate"].asInt());
    m_tsubchannalsetting.result = Result(m_SubChannal["Result"].asInt());
    return true;
}
string CConfig::getConfigFilePath()
{
    return m_configpath;
}
T_RecordMenu CConfig::GetRecordMenu()
{
    return m_tRecordMenu;
}
T_TimeConfig CConfig::GetTimeConfig()
{
    return m_ttimeconfig;
}
T_SubChannalSetting CConfig::GetSubChannalSetting()
{
    return m_tsubchannalsetting;
}
T_ChannalSetting CConfig::GetChannalSetting(int channal)
{
    return m_tChannalMenu[channal];
}
bool CConfig::SetChannalSetting(T_ChannalSetting t_channalsetting,int channal)
{
    if((channal > 3) || (channal <0))
    {
        cout<<"channal input error"<<endl;
    }
    m_tChannalMenu[channal] = t_channalsetting;
    m_ChannalMenu[channal]["CodeRate"] = m_tChannalMenu[channal].coderate;
    m_ChannalMenu[channal]["Result"] = m_tChannalMenu[channal].result;
    m_ChannalMenu[channal]["Enable"] = m_tChannalMenu[channal].enable;
    m_ChannalMenu[channal]["Mirror"] = m_tChannalMenu[channal].mirror;
    return true;
}
bool CConfig::SetRecordMenu(T_RecordMenu t_recordmenu)
{
    m_tRecordMenu = t_recordmenu;
    m_RecordMenu["RecordMode"] = t_recordmenu.recordmode;
    m_RecordMenu["Standard"]  = t_recordmenu.standard;
    m_RecordMenu["FileSize"]  = t_recordmenu.filesize;
    m_RecordMenu["Audio"] = t_recordmenu.audio;
    return true;
}
bool CConfig::SetTimeConfig(T_TimeConfig t_timeconfig)
{
    m_ttimeconfig = t_timeconfig;
    m_TimeConfig["RecordStartTimeHour"] = m_ttimeconfig.starthour;
    m_TimeConfig["RecordStartTimeMin"] = m_ttimeconfig.startmin;
    m_TimeConfig["RecordEndTimeHour"] = m_ttimeconfig.endhour;
    m_TimeConfig["RecordEndTimeMin"] = m_ttimeconfig.endmin;
    return true;
}
bool CConfig::SetSubChannalSetting(T_SubChannalSetting t_subchannalsetting)
{
    m_tsubchannalsetting = t_subchannalsetting;
    m_SubChannal["Result"] = m_tsubchannalsetting.result;
    m_SubChannal["CodeRate"] = m_tsubchannalsetting.coderate;
    return true;
}

