#ifndef CVENCMANAGER_H
#define CVENCMANAGER_H
#include <string.h>
#include <stdlib.h>

#include "cthread.h"
#include "sample_comm.h"
#include "cconfig.h"
class CVencManager
{
public:
    ~CVencManager();
    static CVencManager* get_instance();
    bool Setting();
    bool ReadConfig();
    void* onMainStream();
    void* onSubStream();
    int VencStart();
    static void* StartMainStream(void* para);
    static void* StartSubStream(void* para);
    int VencExit();
public:
    static CVencManager* _instance;
private:
    CVencManager();
    int m_u32TotalChn;
    bool m_bMainThreadStart;
    bool m_bSubThreadStart;
    CConfig* m_pConfig;
    VIDEO_NORM_E m_enNorm;
    unsigned int m_u32MainProfile;
    PIC_SIZE_E m_enMainSize;
    PIC_SIZE_E m_enSubSize;
    pthread_t m_idMainThread;
    pthread_t m_idSubThread;
};
#endif // CVENCMANAGER_H
