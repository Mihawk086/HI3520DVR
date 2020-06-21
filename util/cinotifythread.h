#ifndef CINOTIFYTHREAD_H
#define CINOTIFYTHREAD_H
#include "cthread.h"
#include <string>
using namespace std;
#define EVENT_SIZE  ( sizeof (struct inotify_event) )
#define EVENT_BUF_LEN     ( 1024 * ( EVENT_SIZE + 16 ) )
class CInotifythread :public Thread
{
public:
    CInotifythread();
    CInotifythread(string filepath,void* (*callback)(void* arg),void *ptr);
    bool Config(string filepath,void* (*callback)(void* arg),void *ptr);
    virtual void* run(void);
private:
    string m_filepath;
    void *_ptr;
    void* (*_callback)(void* arg);
};
#endif // CINOTIFYTHREAD_H
