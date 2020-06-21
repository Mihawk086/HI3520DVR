#ifndef CWORKDIRECTORY_H
#define CWORKDIRECTORY_H
#include <string>
#include <iostream>
#include <pthread.h>
#include <sys/statfs.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "cconfig.h"
#include "cfilewatcher.h"
#include "ctime.h"
#include "csaveinterface.h"
#include "csqlitemanager.h"
#include "Timer.h"
using namespace std;
class CWorkDirectory
{
public:
    ~CWorkDirectory();
    static CWorkDirectory* get_Instance();
    bool SetCurrentDirectory(string& Currentpath);
    bool SetWorkDirectory(string& path);
    bool CreatNewFolder();
    bool ReadConfig();
    string getCurrentDirectory();
    string getWorkDirectory();
    bool getSpace();
    bool Start();
    bool Stop();
    void* OnTimerInfo();
    void* OnTimerSpaceCheck();
    void* OnTimerLoop();
    static void StartOnTimerLoop(void* para);
    static void StartOnTimerInfo(void* para);
    static void StartOnTimerSpaceCheck(void* para);
private:
    CWorkDirectory();
    static CWorkDirectory* m_pWorkDirectory;
    Ctime m_currenttime;          //当前时间
    string m_CurrentDirectory;    //当前存储目录,既日期目录
    string m_WorkDirectory;       //当前工作目录
    pthread_mutex_t m_directorymutex;
    bool m_IsRecording;
    unsigned int m_totalsize_mb;    //总容量
    unsigned int m_availablesize_mb;    //可用容量
    unsigned int m_sizetofree_mb;    //剩余容量释放
    Timer m_timer_spacecheck ,m_timer_loopcoverage;
    CSaveInterface* m_saveinterface;    //存储文件接口
    CSQLiteManager* m_sqlitemanager;    //数据库接口
    CConfig* m_config;                  //配置文件
};
#endif // CWORKDIRECTORY_H
