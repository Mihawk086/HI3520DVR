#include "cthread.h"
Thread::Thread(){
}
Thread::~Thread() {
    Thread::cancel();
}
void* Thread::thread_entry(void* para)
{
    Thread *pThread = static_cast<Thread *>(para);
    return pThread->run();
}
int Thread::start(void)
{
    if(pthread_create(&pid,0,thread_entry,static_cast<void *>(this)) < 0)
    {
        pthread_detach(this->pid);
        return -1;
    }
    return 0;
}
int Thread::cancel(void)
{
    pthread_cancel(this->pid);
    return 0;
}
