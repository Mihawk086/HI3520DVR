#ifndef CMEDIAFILE_H
#define CMEDIAFILE_H
#include <string>
#include <time.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include "csqlitemanager.h"
#include "sample_comm.h"
#include "ctime.h"
#define USE_DIRECT_IO   0
#define USE_FSYNC       1
#define USE_OSYNC_FLAG  0
#if USE_DIRECT_IO + USE_FSYNC + USE_OSYNC_FLAG != 1
#error "Only one IO method can be used"
#endif
using namespace std;
class CMediaFile
{
    enum
    {
        BLOCK_SIZE  =   32768,
        BLOCK_NUM   =   16,
        BLOCK_ALIGN =   32768
    };
    enum
    {
        STATE_OPEND =   1,
        STATE_CLOSE
    };
public:
    CMediaFile();
    CMediaFile(int channal);
    ~CMediaFile();
    bool CreateMediaFile(string path);
    int NormalWrite(void* buf, int size);
    int NormalOpen();
    int NormalClose();
private:
    string m_path;
    string m_filepath;
    string m_filename;
    int m_channal;
    Ctime m_createtime;
public:
    int Open();
    void Close();
    int Write(void* buf, int size);
    bool IsOpened() { return m_state & STATE_OPEND; }
protected:
    static void* WorkThreadEntry(void* arg);
    void StartWorkThread();
private:
    char*               m_buf;
    volatile int        m_startPos;
    volatile int        m_size;
    pthread_rwlock_t    m_lock;
    pthread_t           m_workThread;
    sem_t               m_dataReady;
    int                 m_flagExit;
    int                 m_state;
    int                 m_fd;
    CSQLiteManager*     m_qlite;

};
#endif // CMEDIAFILE_H
