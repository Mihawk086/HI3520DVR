#ifndef HISI3520_MEDIAFILE_H
#define HISI3520_MEDIAFILE_H
#include "Util/File.h"
#include "Util/TimeTicker.h"
#include <string>
#include "csqlitemanager.h"
#include "TSMaker.h"
#include "media.h"
#include "MpegMaker.h"
enum CHANNEL
{
  CHANNEL_1,
  CHANNEL_2,
  CHANNEL_3,
  CHANNEL_4
};
typedef std::function<void(const DVR_TABLE&)> WRTITEDB_CB;
typedef struct RECORD_PARAM{
}RECORD_PARAM;
class MediaFile {
public:
    MediaFile(const string& strpath,CHANNEL channel);
    bool CreateFile();
    bool CloseFile();
    bool PutFrame(const xop::AVFrame &frame);
    bool LoadConfig();
    bool RecordControl();
    bool StartRecord(){m_bStart = true;}
    bool setWriteSqilteCB(const WRTITEDB_CB &cb){m_writedb_cb = cb;}
    bool StopRecord(){
        m_bStart = false;
        CloseFile();
    }
private:
    CHANNEL m_channel;
    WRTITEDB_CB m_writedb_cb;
    string m_strCH;
    ZL::Util::Ticker m_ticker;
    DVR_TABLE m_file_info;
    string m_strPath;
    string m_strFile;
    bool m_bStart;
    bool m_bRecordState;
    TSMaker m_TSMaker;
    int m_recordSec;
};
#endif //HISI3520_MEDIAFILE_H
