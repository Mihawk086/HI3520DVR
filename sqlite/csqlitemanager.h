#ifndef CSQLITEMANAGER_H
#define CSQLITEMANAGER_H
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <memory>
#include <vector>
#include <mutex>
#include "CppSQLite3.h"
#include "ctime.h"
#include "sqlite3.h"
using namespace std;
typedef struct DVR_TABLE
{
    uint32_t utctime;
    char file_name[255];
    char path[255];
    char file_path[255];
    int channel;
    int year;
    int month;
    int day;
    int hour;
    int minute;
    int second;
}DVR_TABLE;
class CSQLiteManager
{
public:
    static CSQLiteManager* get_instance();
    CSQLiteManager(const CSQLiteManager&) = delete;
    CSQLiteManager&operator=(const CSQLiteManager&) = delete;
    bool ReadFileList(string path,vector<string> &vfile);
    bool CreateDB();
    bool WriteDB(const DVR_TABLE &table);
    bool WriteDB(int channal,unsigned int utctime,string filename,string path,\
                                 string filepath, TIMESTRUCT time);
    bool CheckDB();
    bool PrintDB();
    bool SearchOldestFile(string& path , string& filepath);
    bool DeletebyFilepath(string& filepath);
    bool SearchDB(string starttime,string endtime);
    bool SearchDB(string starttime,string endtime,unsigned int channal,vector<std::string> &files);
private:
    bool m_bopen;
    mutable std::mutex _mutex;
    CSQLiteManager();
    string m_workdir;
    string m_DBpath;
    string m_DBname;
};
#endif // CSQLITEMANAGER_H
