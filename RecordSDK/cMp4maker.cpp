#if 1
#include <sys/stat.h>
#include <netinet/in.h>
#include "cMp4maker.h"
#include "Util/File.h"
#include "Util/mini.h"
#include "Util/util.h"
#include "Util/NoticeCenter.h"
using namespace ZL::Util;
static unsigned long GetTimeStamp()
{
    struct timeval tv;
    if (gettimeofday(&tv, NULL) != 0)
        return 0;
    return (tv.tv_sec*1000  + tv.tv_usec / 1000);
}
string timeStr(const char *fmt) {
    std::tm tm_snapshot;
    auto time = ::time(NULL);
#if defined(_WIN32)
    localtime_s(&tm_snapshot, &time); // thread-safe
#else
    localtime_r(&time, &tm_snapshot); // POSIX
#endif
    const size_t size = 1024;
    char buffer[size];
    auto success = std::strftime(buffer, size, fmt, &tm_snapshot);
    if (0 == success)
        return string(fmt);
    return buffer;
}
Mp4Maker::Mp4Maker(const string& strPath,
                   int recordSec
                  ) {
    DebugL << strPath;
    m_strPath = strPath;
    m_info.strFolder = strPath;
    m_recordSec = recordSec;
}
Mp4Maker::~Mp4Maker() {
    closeFile();
}
void Mp4Maker::inputH264(const xop::AVFrame &frame)
{
    int iType;
    if(frame.type == VIDEO_FRAME_I) {
        iType = 1;
    }else{
        iType = 0;
    }
    uint32_t ui32TimeStamp = GetTimeStamp();
    void *pData = frame.buffer.get();
    uint32_t ui32Length = frame.size;
    if(iType == 1){
        m_sps.assign((char*)frame.sps,frame.sps_len);
        m_pps.assign((char*)frame.pps,frame.pps_len);
    }
    switch (iType) {
        case 0:
        case 1: {
            if (m_strLastVideo.size()) {
                int64_t iTimeInc = (int64_t)ui32TimeStamp - (int64_t)m_ui32LastVideoTime;
                iTimeInc = MAX(0,MIN(iTimeInc,500));
                if(iTimeInc == 0 ||  iTimeInc == 500){
                    WarnL << "abnormal time stamp increment:" << ui32TimeStamp << " " << m_ui32LastVideoTime;
                }
                _inputH264((char *) m_strLastVideo.data(), m_strLastVideo.size(), iTimeInc, m_iLastVideoType);
            }
            uint32_t *p = (uint32_t *) pData;
            *p = htonl(ui32Length - 4);
            m_strLastVideo.assign((char *) pData, ui32Length);
            memcpy(pData, "\x00\x00\x00\x01", 4);
            m_ui32LastVideoTime = ui32TimeStamp;
            m_iLastVideoType = iType;
        }
            break;
        default:
            break;
    }
}
void Mp4Maker::inputH264(const struct ringbuffer &buf){
    int iType = buf.frame_type;
    uint32_t ui32TimeStamp = buf.PTS/1000;
    void *pData = buf.buffer;
    uint32_t ui32Length = buf.size;
    if(iType == 1){
        m_sps.assign((char*)buf.sps,buf.spslen);
        m_pps.assign((char*)buf.pps,buf.ppslen);
    }
    switch (iType) {
        case 0:
        case 1: {
            if (m_strLastVideo.size()) {
                int64_t iTimeInc = (int64_t)ui32TimeStamp - (int64_t)m_ui32LastVideoTime;
                iTimeInc = MAX(0,MIN(iTimeInc,500));
                if(iTimeInc == 0 ||  iTimeInc == 500){
                    WarnL << "abnormal time stamp increment:" << ui32TimeStamp << " " << m_ui32LastVideoTime;
                }
                _inputH264((char *) m_strLastVideo.data(), m_strLastVideo.size(), iTimeInc, m_iLastVideoType);
            }
            uint32_t *p = (uint32_t *) pData;
            *p = htonl(ui32Length - 4);
            m_strLastVideo.assign((char *) pData, ui32Length);
            memcpy(pData, "\x00\x00\x00\x01", 4);
            m_ui32LastVideoTime = ui32TimeStamp;
            m_iLastVideoType = iType;
        }
            break;
        default:
            break;
    }
}
void Mp4Maker::_inputH264(void* pData, uint32_t ui32Length, uint32_t ui32Duration, int iType) {
    if(iType == 1 && (m_hMp4 == MP4_INVALID_FILE_HANDLE || m_ticker.elapsedTime() > m_recordSec * 1000)){
        createFile();
    }
    if (m_hVideo != MP4_INVALID_TRACK_ID) {
        MP4WriteSample(m_hMp4, m_hVideo, (uint8_t *) pData, ui32Length,ui32Duration*90,0,iType == 1);
    }
}
void Mp4Maker::createFile() {
    closeFile();
    auto strDate = timeStr("%Y-%m-%d");
    auto strTime = timeStr("%H-%M-%S");
    auto strFileTmp = m_strPath + strDate + "/" + strTime + ".mp4";
    auto strFile =	m_strPath + strDate + "/" + strTime + ".mp4";
    m_info.ui64StartedTime = ::time(NULL);
    m_info.strFileName = strTime + ".mp4";
    m_info.strFilePath = strFile;
#if !defined(_WIN32)
    File::createfile_path(strFileTmp.data(), S_IRWXO | S_IRWXG | S_IRWXU);
#else
    File::createfile_path(strFileTmp.data(), 0);
#endif
    m_ticker.resetTime();
    m_hMp4 = MP4CreateEx(strFileTmp.data(),  0, 1, 1, 0, 0, 0, 0);
    if (m_hMp4 == MP4_INVALID_FILE_HANDLE)    {
        printf("open file fialed.\n");
        return ;
    }
    MP4SetTimeScale(m_hMp4, 90000);
    m_hVideo = MP4AddH264VideoTrack(m_hMp4, 90000, MP4_INVALID_DURATION,
                                    1280, 720,
                                    m_sps[5], m_sps[6], m_sps[7], 3);
    if (m_hVideo == MP4_INVALID_TRACK_ID)
    {
        printf("add video track fialed.\n");
        return ;
    }
    MP4AddH264SequenceParameterSet(m_hMp4, m_hVideo, (uint8_t *)m_sps.data() + 4, m_sps.size() - 4);
    MP4AddH264PictureParameterSet(m_hMp4, m_hVideo, (uint8_t *)m_pps.data() + 4, m_pps.size() - 4);
}
void Mp4Maker::closeFile() {
    if (m_hMp4 != MP4_INVALID_FILE_HANDLE) {
        {
            TimeTicker();
            MP4Close(m_hMp4,MP4_CLOSE_DO_NOT_COMPUTE_BITRATE);
        }
        rename(m_strFileTmp.data(),m_strFile.data());
        m_hMp4 = MP4_INVALID_FILE_HANDLE;
        m_hVideo = MP4_INVALID_TRACK_ID;
        m_info.ui64TimeLen = ::time(NULL) - m_info.ui64StartedTime;
        struct stat fileData;
        stat(m_strFile.data(), &fileData);
        m_info.ui64FileSize = fileData.st_size;
    }
}
#endif