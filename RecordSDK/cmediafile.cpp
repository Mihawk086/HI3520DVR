#include "cmediafile.h"
CMediaFile::CMediaFile()
    : m_startPos(0),
      m_size(0),
      m_state(0),
      m_fd(-1)
{
    int ret;
    ret = posix_memalign((void**)&m_buf, BLOCK_ALIGN, BLOCK_SIZE * BLOCK_NUM);
    if(ret != 0) m_buf = NULL;
    ret = pthread_rwlock_init(&m_lock, NULL);
    ret = sem_init(&m_dataReady, 0, 0);
    if(ret < 0)
    {
        printf("Cannot init sem\n");
    }
}
CMediaFile::CMediaFile(int channal)
    : m_startPos(0),
      m_size(0),
      m_state(0),
      m_fd(-1)
{
    m_channal = channal;
    int ret;
    ret = posix_memalign((void**)&m_buf, BLOCK_ALIGN, BLOCK_SIZE * BLOCK_NUM);
    if(ret != 0) m_buf = NULL;
    ret = pthread_rwlock_init(&m_lock, NULL);
    ret = sem_init(&m_dataReady, 0, 0);
    if(ret < 0)
    {
        printf("Cannot init sem\n");
    }
}

bool CMediaFile::CreateMediaFile(string path)
{
    m_path = path;
    m_createtime.SetCurrentTime();
    TIMESTRUCT time = m_createtime.GetNormalTime();
    char filename[255];
    sprintf(filename,"CH%1d_%04d%02d%02d_%02d%02d%02d.h264",m_channal,time.year,\
            time.month,time.day,time.hour,time.minute,time.second);
    m_filename = filename;
    m_filepath = m_path;
    m_filepath += "/";
    m_filepath += m_filename;
    int fd = open(m_filepath.c_str(),O_RDWR | O_CREAT,0777);
    close(fd);
    CSQLiteManager* psqlite = CSQLiteManager::get_instance();
    psqlite->WriteDB(m_channal,m_createtime.m_utc_time,m_filename,m_path,m_filepath,\
                      m_createtime.GetNormalTime());
    return true;
}
CMediaFile::~CMediaFile()
{
    Close();
    pthread_rwlock_destroy(&m_lock);
    sem_destroy(&m_dataReady);
    free(m_buf);
}
int CMediaFile::NormalOpen()
{
    const char* path = m_filepath.c_str();
#if USE_DIRECT_IO
    m_fd = open(path, O_WRONLY | O_CREAT | O_TRUNC | O_DIRECT | O_SYNC);
#elif USE_OSYNC_FLAG
    m_fd = open(path, O_WRONLY | O_CREAT | O_TRUNC | O_SYNC);
#elif USE_FSYNC
    m_fd = open(path, O_WRONLY | O_CREAT | O_TRUNC);
#endif
    if(m_fd < 0)
    {
        printf("Cannot open file %s\n", path);
        goto err;
    }
    else
    {
        printf("open media file successfully\n");
    }
    m_state = STATE_OPEND;
    return 0;
err:
    if(m_fd >= 0)
    {
        close(m_fd);
        unlink(path);
    }
    return -1;
}
int CMediaFile::Open()
{
    const char* path = m_filepath.c_str();
    int ret;
    pthread_t idThread;
    Close();
    m_startPos = 0;
    m_size = 0;
#if USE_DIRECT_IO
    m_fd = open(path, O_WRONLY | O_CREAT | O_TRUNC | O_DIRECT | O_SYNC);
#elif USE_OSYNC_FLAG
    m_fd = open(path, O_WRONLY | O_CREAT | O_TRUNC | O_SYNC);
#elif USE_FSYNC
    m_fd = open(path, O_WRONLY | O_CREAT | O_TRUNC);
#endif
    if(m_fd < 0)
    {
        printf("Cannot create file %s\n", path);
        goto err;
    }
    while(sem_trywait(&m_dataReady) == 0) ;
    m_flagExit = 0;
    ret = pthread_create(&idThread, NULL, WorkThreadEntry, this);
    if(ret != 0)
    {
        printf("Cannot create writer thread\n");
        goto err;
    }
    m_workThread = idThread;
    m_state |= STATE_OPEND;
    return 0;
err:
    if(m_fd >= 0)
    {
        close(m_fd);
        unlink(path);
    }
    return -1;
}
int CMediaFile::NormalClose()
{
    if(m_fd>0)
    {
        close(m_fd);
    }
    m_state = STATE_CLOSE;
    m_fd = -1;
    m_filename.clear();
    m_filepath.clear();
    m_path.clear();
    return 0;
}
void CMediaFile::Close()
{
    if(!(m_state & STATE_OPEND)) return;
    m_flagExit = 1;
    sem_post(&m_dataReady);
    pthread_join(m_workThread, NULL);
    fcntl(m_fd, F_SETFL, O_SYNC);
    if(m_startPos + m_size > BLOCK_SIZE * BLOCK_NUM)
    {
        write(m_fd, &m_buf[m_startPos], BLOCK_SIZE * BLOCK_NUM - m_startPos);
        m_size -= BLOCK_SIZE * BLOCK_NUM - m_startPos;
        m_startPos = 0;
    }
    if(m_size) write(m_fd, &m_buf[m_startPos], m_size);
    fsync(m_fd);
    close(m_fd);
    m_state &= ~STATE_OPEND;
    m_fd = -1;
    m_filename.clear();
    m_filepath.clear();
    m_path.clear();
}
int CMediaFile::NormalWrite(void* buf, int size)
{
    if(!(m_state & STATE_OPEND)) return -1;
    int ret = write(m_fd, buf, size);
    return 0;
}
int CMediaFile::Write(void *buf, int size)
{
    int start, emptybufsize;
    int tmp = size;
    if(!(m_state & STATE_OPEND)) return -1;
    while(size > 0)
    {
        pthread_rwlock_rdlock(&m_lock);
        start = m_startPos + m_size;
        if(start >= BLOCK_SIZE * BLOCK_NUM) start -= BLOCK_SIZE * BLOCK_NUM;
        emptybufsize = BLOCK_SIZE * BLOCK_NUM -  m_size;
        pthread_rwlock_unlock(&m_lock);
        if(emptybufsize == 0)
        {
            static int i = 0;
            printf("no emptybufsize %d\n",i++);
            usleep(10 * 1000);
            continue;
        }
        int bytes = size < emptybufsize ? size : emptybufsize;
        if(start + bytes > BLOCK_SIZE * BLOCK_NUM) bytes = BLOCK_SIZE * BLOCK_NUM - start;
        memcpy(&m_buf[start], buf, bytes);
        buf = (char*)buf + bytes;
        size -= bytes;
        for(int i = (bytes + BLOCK_SIZE - emptybufsize % BLOCK_SIZE) / BLOCK_SIZE; i; --i)
            sem_post(&m_dataReady);
        pthread_rwlock_wrlock(&m_lock);
        m_size += bytes;
        pthread_rwlock_unlock(&m_lock);
    }
    return tmp;
}
void* CMediaFile::WorkThreadEntry(void *arg)
{
    CMediaFile* pWriter = static_cast<CMediaFile*>(arg);
    pWriter->StartWorkThread();
    return NULL;
}
void CMediaFile::StartWorkThread()
{
    int start, size;
    while(!m_flagExit)
    {
        sem_wait(&m_dataReady);
        pthread_rwlock_rdlock(&m_lock);
        start = m_startPos;
        size = m_size;
        pthread_rwlock_unlock(&m_lock);
        if(size >= BLOCK_SIZE)
        {
            write(m_fd, &m_buf[start], BLOCK_SIZE);
#if USE_FSYNC
            fsync(m_fd);
#endif
            pthread_rwlock_wrlock(&m_lock);
            m_startPos += BLOCK_SIZE;
            if(m_startPos >= BLOCK_SIZE * BLOCK_NUM) m_startPos -= BLOCK_SIZE * BLOCK_NUM;
            m_size -= BLOCK_SIZE;
            pthread_rwlock_unlock(&m_lock);
        }
    }
}


