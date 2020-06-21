#ifndef HISI3520_RECORDLOOP_H
#define HISI3520_RECORDLOOP_H
#include <memory>
#include <map>

#include "media.h"
#include "EventLoop.h"
#include "cMp4maker.h"
#include "Mp4Recorder.h"
#include "TSMaker.h"
#include "MpegMaker.h"
#include "Hotplug.h"
#include "MediaFile.h"
#include "DevStorage.h"
class RecordLoop {
public:
    static RecordLoop* Instance();
    static RecordLoop* _instance;
    bool getSpace();
    bool putframe(const xop::AVFrame &farme);
    bool putframe(int channel, const xop::AVFrame &frame);
    bool startrecord();
    bool stoprecord();
    void write_sqlite(const DVR_TABLE &table);
    int loop();
    int loop2();
private:
    RecordLoop();
    string m_Path;
    mutable std::mutex _mutex;
    xop::EventLoop m_eventLoop;
    xop::EventLoop m_fileindex_eventLoop;
    bool m_IsRecord;
    bool m_bSDcardReady;
    std::shared_ptr<Hotplug> m_hotplug;
    std::map<int,std::shared_ptr<MediaFile>> m_MapMediaFile;
    uint32_t m_totalsize_mb;
    uint32_t m_availablesize_mb;
    const uint32_t  m_remove_mb;
    DevStorage m_dev;
};
#endif //HISI3520_RECORDLOOP_H
