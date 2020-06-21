#if 1
#ifndef HISI3520_CMP4MAKER_H
#define HISI3520_CMP4MAKER_H
#include <mutex>
#include <memory>
#include <mp4v2/mp4v2.h>
#include "Util/util.h"
#include "Util/logger.h"
#include "Util/TimeTicker.h"
#include "Util/TimeTicker.h"
#include "CRingFifo.h"
#include "media.h"
extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
};
using namespace ZL::Util;
using namespace std;
#ifndef MAX
#define MAX(a,b) ((a) > (b) ? (a) : (b) )
#endif //MAX
#ifndef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b) )
#endif //MIN
class Mp4Info
{
public:
    time_t ui64StartedTime; //GMT标准时间，单位秒
    time_t ui64TimeLen;//录像长度，单位秒
    off_t ui64FileSize;//文件大小，单位BYTE
    string strFilePath;//文件路径
    string strFileName;//文件名称
    string strFolder;//文件夹路径
    string strUrl;//播放路径
    string strAppName;//应用名称
    string strStreamId;//流ID
    string strVhost;//vhost
};
class Mp4Maker {
public:
    typedef std::shared_ptr<Mp4Maker> Ptr;
    Mp4Maker(const string &strPath,
             int m_recordSec);
    virtual ~Mp4Maker();
    void inputH264(const struct ringbuffer &buf);
    void inputH264(const xop::AVFrame &frame);
private:
    MP4FileHandle m_hMp4 = MP4_INVALID_FILE_HANDLE;
    MP4TrackId m_hVideo = MP4_INVALID_TRACK_ID;
    string m_strPath;
    string m_strFile;
    string m_strFileTmp;
    Ticker m_ticker;
    SmoothTicker m_mediaTicker[2];
    void createFile();
    void closeFile();
    void _inputH264(void *pData, uint32_t ui32Length, uint32_t ui64Duration, int iType);
    string m_strLastVideo;
    uint32_t m_ui32LastVideoTime = 0;
    int m_iLastVideoType = 0;
    Mp4Info m_info;
    int m_recordSec ;
    std::string m_sps;
    std::string m_pps;
};
#endif
#endif