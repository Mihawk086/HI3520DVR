#include "MediaFile.h"
static uint32_t getTimeStamp()
{
    struct timeval tv = {0};
    gettimeofday(&tv, NULL);
    uint32_t ts = ((tv.tv_sec*1000)+((tv.tv_usec+500)/1000)); // 90: _clockRate/1000;
    return ts;
}
static string timeStr(const char *fmt) {
    std::tm tm_snapshot;
    auto time = ::time(NULL);
    localtime_r(&time, &tm_snapshot); // POSIX
    const size_t size = 1024;
    char buffer[size];
    auto success = std::strftime(buffer, size, fmt, &tm_snapshot);
    if (0 == success)
        return string(fmt);
    return buffer;
}
MediaFile::MediaFile(const string &strpath,CHANNEL channel)
        :m_bStart(false),m_strPath(strpath),
         m_recordSec(300),m_strFile(""),m_channel(channel),m_bRecordState(false){
    switch (channel){
        case CHANNEL_1:m_strCH = "CH1";break;
        case CHANNEL_2:m_strCH = "CH2";break;
        case CHANNEL_3:m_strCH = "CH3";break;
        case CHANNEL_4:m_strCH = "CH4";break;
    }
}
bool MediaFile::CreateFile() {
    CloseFile();
    auto strDate = timeStr("%Y-%m-%d");
    auto strTime = timeStr("%H-%M-%S");
    auto strFile =	m_strPath + strDate + "/" + m_strCH +"_" + strDate+"_"+strTime + ".ts";
    string fileName = m_strCH +"_" + strDate+"_"+strTime + ".ts";
    string Path = m_strPath + strDate;
    m_strFile = strFile;
    time_t tt = time(NULL);//这句返回的只是一个时间cuo
    tm* t= localtime(&tt);
    switch (m_channel){
        case CHANNEL_1:m_file_info.channel = 1;break;
        case CHANNEL_2:m_file_info.channel = 2;break;
        case CHANNEL_3:m_file_info.channel = 3;break;
        case CHANNEL_4:m_file_info.channel = 4;break;
    }
    m_file_info.year = t->tm_year + 1900;
    m_file_info.month =t->tm_mon + 1;
    m_file_info.day = t->tm_mday;
    m_file_info.hour = t->tm_hour;
    m_file_info.minute = t->tm_min;
    m_file_info.second = t->tm_sec;
    m_file_info.utctime = getTimeStamp();
    strcpy(m_file_info.file_path,strFile.c_str());
    strcpy(m_file_info.file_name,fileName.c_str());
    strcpy(m_file_info.path,Path.c_str());
    DVR_TABLE table = m_file_info;
    if(m_writedb_cb) {
        m_writedb_cb(table);
    }
    m_ticker.resetTime();
    if(File::createfile_path(strFile.data(), S_IRWXO | S_IRWXG | S_IRWXU) == false) {
        cout<<"create path error"<<endl;
    };
    m_TSMaker.init(strFile,32*1024);
    m_bRecordState = true;
    return true;
}
bool MediaFile::CloseFile() {
    m_TSMaker.clear();
    m_bRecordState = false;
    return false;
}
bool MediaFile::PutFrame(const xop::AVFrame &frame) {
    RecordControl();
    if(m_bStart && frame.type == VIDEO_FRAME_I && ((m_bRecordState==false)||m_ticker.elapsedTime() > m_recordSec * 1000)){
        CreateFile();
    }
    if (m_bStart) {
        m_TSMaker.putframe(frame);
    }
    return false;
}
bool MediaFile::RecordControl() {
    if(m_channel == CHANNEL_3 || m_channel == CHANNEL_4) {
        StartRecord();
    }
    else{
        StopRecord();
    }
    return false;
}
bool MediaFile::LoadConfig() {

    return false;
}
