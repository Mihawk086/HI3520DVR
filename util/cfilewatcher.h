#ifndef CFILEWATCHER_H
#define CFILEWATCHER_H
#include "cinotifythread.h"
#include "cconfig.h"
class CFileWatcher
{
public:
    static CFileWatcher* _instance;
    static CFileWatcher* get_instance();
    int Start();
    void* onConfigInotify();
    static void* StartConfigOnInotify(void* para);
private:
    CFileWatcher();
    CInotifythread* m_configwatch;
};
#endif // CFILEWATCHER_H
