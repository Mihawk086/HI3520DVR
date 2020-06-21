#ifndef THREAD_H_
#define THREAD_H_
#include <pthread.h>
class Thread {
public:
    enum THREADSTATE
    {
        IDLE,
        WORK,
        BUSY,
    };
public:
    Thread();
    virtual ~Thread();
    virtual void* run(void) = 0;
    virtual int start(void);
    virtual int cancel(void);
    pthread_t get_pid() const
    {
        return pid;
    }
protected:
    pthread_t pid;
    THREADSTATE _thread_state;
private:
    static void* thread_entry(void* para);
};
#endif /* THREAD_H_ */
