#ifndef CSAVEINTERFACE_H
#define CSAVEINTERFACE_H
#include <pthread.h>
#include <string>
#include "cconfig.h"
#include "sample_comm.h"
#include "cmediafile.h"
#include "csqlitemanager.h"
using namespace std;
class CSaveInterface
{
public:
    ~CSaveInterface();
    static CSaveInterface *fnGetInstance();
    bool Create(int channal,string& path);
    bool Create(string& path);
    bool WriteH264(int channal,VENC_STREAM_S *pstStream,VENC_CHN VeChn);
    bool CloseAll();
    bool WriteInterface(int channal,VENC_STREAM_S *pstStream,VENC_CHN VeChn);
private:
    CSaveInterface();
    static  CSaveInterface* _m_pSaveInterface;
    CConfig* m_pConfig;
    CMediaFile* m_mediafile[4];
    CSQLiteManager* m_sqlite;
    pthread_mutex_t m_mutex;
    bool m_newfile[4];
};
#endif
