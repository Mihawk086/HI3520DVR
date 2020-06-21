#ifndef CHOTPLUG_H
#define CHOTPLUG_H
#include "cthread.h"
#define UEVENT_BUFFER_SIZE 2048
class CHotPlug :public Thread
{
public:
    bool Config(void* (*callback)(void* arg),void *ptr);
    virtual void* run();
    static CHotPlug* _instance;
    static CHotPlug* getInstance();
    static void* StartAddAction(void* p);
    int AddAction();
    int RemoveAction();
    int onAddActionThread();
private:
    CHotPlug();
    bool m_bAddThread;
    void *_ptr;
    void* (*_callback)(void* arg);
    char m_DiskName[10];
};
#endif // CHOTPLUG_H
