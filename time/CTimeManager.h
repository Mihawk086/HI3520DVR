#ifndef HISI3520_CTIMEMANAGER_H
#define HISI3520_CTIMEMANAGER_H
#include "hi_rtc.h"
class CTimeManager {
public:
    int GetRtcTime();
    int SetRtcTime();
    int SetSystemTime();
    static CTimeManager* get_instance();
    static CTimeManager* pCTimerManager;
private:
    CTimeManager();
    rtc_time_t m_tm;
};
#endif //HISI3520_CTIMEMANAGER_H
