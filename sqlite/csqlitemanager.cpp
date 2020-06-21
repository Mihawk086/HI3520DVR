#include "csqlitemanager.h"
#include <dirent.h>
CSQLiteManager* CSQLiteManager::get_instance() {
    static CSQLiteManager instance;
    return &instance;
}
CSQLiteManager::CSQLiteManager()
        :m_DBname("fileindex.db"),m_workdir("/mnt_1"),m_DBpath(),m_bopen(false)
{
    m_DBpath = m_workdir+"/"+m_DBname;
}
bool CSQLiteManager::CreateDB()
{
    std::lock_guard<std::mutex> locker(_mutex);
    sqlite3 *db;
    if (!access(m_DBpath.c_str(),0)) {
        cout<<m_DBpath<<" exist"<<endl;
        return true;
    }
    else
    {
        int ret;
        ret = sqlite3_open(m_DBpath.c_str(), &db);
        if( ret )
        {
           fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
           return false;
        }else
        {
           fprintf(stderr, "Opened database successfully\n");
           sqlite3_close(db);
        }
    }
    char *sql;
    sql = "CREATE TABLE dvr("  \
                       "utctime UNSIGNED BIG INT," \
                       "filename char(255)," \
                       "path VARCHAR(255)," \
                       "filepath VARCHAR(255)," \
                       "channal int," \
                       "year int,"\
                       "month int,"\
                       "day int,"\
                       "hour int,"\
                       "minute int,"\
                       "second int);";
    CppSQLite3DB DB;
    try {
        DB.open(m_DBpath.c_str());
    }
    catch (CppSQLite3Exception& e) {
        cout<<"open sqlite false"<<endl;
        cout<<e.errorMessage()<<endl;
        return false;
    }
    if(DB.tableExists("dvr") == false) {
        cout<<"creat table dvr"<<endl;
        DB.execDML(sql);
    }
    try {
        DB.close();
    }
    catch (CppSQLite3Exception& e) {
        cout<<e.errorMessage()<<endl;
        return false;
    }
    return true;
}
bool CSQLiteManager::WriteDB(const DVR_TABLE &table) {
    std::lock_guard<std::mutex> locker(_mutex);
    CppSQLite3DB db;
    try {
        db.open(m_DBpath.c_str());
    }
    catch (CppSQLite3Exception& e) {
        cout<<"open sqlite false"<<endl;
        cout<<e.errorMessage()<<endl;
        return false;
    }
    char sql[500];
    sprintf(sql,"insert into dvr values(%lu,'%s','%s','%s',%d,%d,%d,%d,%d,%d,%d);",table.utctime,table.file_name,\
            table.path,table.file_path,table.channel,table.year,table.month,table.day,table.hour,table.minute,table.second);
    try{
        db.execDML(sql);
    }
    catch(CppSQLite3Exception& e) {
        cout<<e.errorMessage()<<endl;
        return false;
    }
    return false;
}
bool CSQLiteManager::WriteDB(int channal,unsigned int utctime,string filename,string path,\
                             string filepath, TIMESTRUCT time)
{
    std::lock_guard<std::mutex> locker(_mutex);
    CppSQLite3DB db;
    try {
        db.open(m_DBpath.c_str());
    }
    catch (CppSQLite3Exception& e) {
        cout<<"open sqlite false"<<endl;
        cout<<e.errorMessage()<<endl;
        return false;
    }
    char sql[500];
    sprintf(sql,"insert into dvr values(%u,'%s','%s','%s',%d,%d,%d,%d,%d,%d,%d);",utctime,filename.c_str(),\
            path.c_str(),filepath.c_str(),channal,time.year,time.month,time.day,time.hour,time.minute,time.second);
    try{
        db.execDML(sql);
    }
    catch(CppSQLite3Exception& e) {
        cout<<e.errorMessage()<<endl;
        return false;
    }
    return true;
}

bool CSQLiteManager::SearchOldestFile(string& path,string& filepath)
{
    std::lock_guard<std::mutex> locker(_mutex);
    CppSQLite3DB db;
    try {
        db.open(m_DBpath.c_str());
    }
    catch (CppSQLite3Exception& e) {
        cout<<"open sqlite false"<<endl;
        cout<<e.errorMessage()<<endl;
        return false;
    }
    string sql1= "select min(utctime) from dvr;";
    string utctime;
    CppSQLite3Query q1;
    try {
        q1 = db.execQuery(sql1.c_str());
    }
    catch(CppSQLite3Exception& e) {
        cout<<e.errorMessage()<<endl;
        return false;
    }
    if (q1.eof()) {
        cout << "find oldest file error" << endl;
        return false;
    } else {
        utctime = q1.fieldValue("min(utctime)");
    }
    char buf[500];
    sprintf(buf,"select filename, path, filepath from dvr where utctime = %s;",utctime.c_str());
    CppSQLite3Query q2;
    try {
        q2 = db.execQuery(buf);
    }
    catch(CppSQLite3Exception& e) {
        cout<<e.errorMessage()<<endl;
        return false;
    }
    if (!q2.eof()) {
        path = q2.fieldValue("path");
        filepath = q2.fieldValue("filepath");
    } else {
        cout << "find oldest file error" << endl;
        return false;
    }
    return true;
}
bool CSQLiteManager::DeletebyFilepath(string& filepath)
{
    std::lock_guard<std::mutex> locker(_mutex);
    char buf[500];
    sprintf(buf,"delete from dvr where filepath = '%s';",filepath.c_str());
    CppSQLite3DB db;
    try {
        db.open(m_DBpath.c_str());
    }
    catch (CppSQLite3Exception& e) {
        cout<<"open sqlite false"<<endl;
        cout<<e.errorMessage()<<endl;
        return false;
    }
    try {
        CppSQLite3Query q = db.execQuery(buf);
    }
    catch(CppSQLite3Exception& e) {
        cout<<e.errorMessage()<<endl;
        false;
    }
    return true;
}
bool CSQLiteManager::SearchDB(string starttime,string endtime)
{
    std::lock_guard<std::mutex> locker(_mutex);
    CppSQLite3DB db;
    try {
        db.open(m_DBpath.c_str());
    }
    catch (CppSQLite3Exception& e) {
        cout<<"open sqlite false"<<endl;
        cout<<e.errorMessage()<<endl;
        return false;
    }
    char buf[500];
    sprintf(buf,"select * from dvr where utctime >= %s and utctime<= %s;",
           starttime.c_str(),endtime.c_str());
    CppSQLite3Query q;
    try{
        q = db.execQuery(buf);
    }
    catch(CppSQLite3Exception& e) {
        cout<<e.errorMessage()<<endl;
        return false;
    }
    while (!q.eof())
    {
        q.nextRow();
    }
    return true;
}
bool CSQLiteManager::SearchDB(string starttime,string endtime,unsigned int channal,vector<std::string> &files)
{
    char buf[500];
    std::lock_guard<std::mutex> locker(_mutex);
    CppSQLite3DB db;
    try {
        db.open(m_DBpath.c_str());
    }
    catch (CppSQLite3Exception& e) {
        cout<<"open sqlite false"<<endl;
        cout<<e.errorMessage()<<endl;
        return false;
    }
    if((channal >= 1) && (channal <= 4))
    {
        cout<<"channal = 1~4 "<<endl;
        sprintf(buf,"select * from dvr where UTCtime >= %s and UTCtime<= %s and channal == %d;",
                starttime.c_str(),endtime.c_str(),channal);
    }
    else if(channal == 5)
    {
        cout<<"channal = 5 "<<endl;
        sprintf(buf,"select * from dvr where UTCtime >= %s and UTCtime<= %s;",
                starttime.c_str(),endtime.c_str());
    }
    else
    {
        cout<<"channal error"<<endl;
        return false;
    }
    CppSQLite3Query q = db.execQuery(buf);
    while (!q.eof())
    {
        cout<<q.fieldValue("filename")<<endl;
        files.push_back(q.fieldValue("filename"));
        q.nextRow();
    }
    return true;
}
bool CSQLiteManager::CheckDB()
{
    std::lock_guard<std::mutex> locker(_mutex);
    CppSQLite3DB db;
    try {
        db.open(m_DBpath.c_str());
    }
    catch (CppSQLite3Exception& e) {
        cout<<"open sqlite false"<<endl;
        cout<<e.errorMessage()<<endl;
        return false;
    }
    string buf;
    CppSQLite3Query q;
    try {
        q = db.execQuery("select * from dvr;");
    }
    catch(CppSQLite3Exception& e) {
        cout<<e.errorMessage()<<endl;
        return false;
    }
    while (!q.eof())
    {
        buf = q.fieldValue("filepath");
        if((access(buf.c_str(),F_OK)) == -1)
        {
            DeletebyFilepath(buf);
        }
        q.nextRow();
    }
    return true;
}
bool CSQLiteManager::PrintDB()
{
    std::lock_guard<std::mutex> locker(_mutex);
    CppSQLite3DB db;
    try {
        db.open(m_DBpath.c_str());
    }
    catch (CppSQLite3Exception& e) {
        cout<<"open sqlite false"<<endl;
        cout<<e.errorMessage()<<endl;
        return false;
    }
    CppSQLite3Query q;
    try {
        q = db.execQuery("select * from dvr;");
    }
    catch(CppSQLite3Exception& e) {
        cout<<e.errorMessage()<<endl;
        return false;
    }
    while (!q.eof())
    {
        cout<<q.fieldValue("utctime")<<endl;
        cout<<q.fieldValue("filepath")<<endl;
        cout<<q.fieldValue("filename")<<endl;
        q.nextRow();
    }
    return true;
}
bool CSQLiteManager::ReadFileList(string path,vector<string> &vfile)
{
    DIR *dir;
    struct dirent *ptr;
    if ((dir=opendir(path.c_str())) == NULL) {
        perror("Open dir error...");
        return false;
    }
    while ((ptr=readdir(dir)) != NULL)
    {
        if(strcmp(ptr->d_name,".")==0 || strcmp(ptr->d_name,"..")==0)
            continue;
        else if(ptr->d_type == 8) {
            string filepath = path + "/" + ptr->d_name;
            vfile.push_back(filepath);
        }
        else if(ptr->d_type == 4)    ///dir
        {
            string newpath = path + "/" + ptr->d_name;
            ReadFileList(newpath,vfile);
        }
    }
    closedir(dir);
    return true;
}
