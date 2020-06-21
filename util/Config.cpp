#include "Config.h"
#include<cstdio>
#include<cstdlib>
#include<cstring>
#include<sys/stat.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<sys/un.h>
#include<errno.h>
#include<stddef.h>
#include<unistd.h>
static const char *filename="uds-tmp";
Config::Config(xop::EventLoop *loop):m_ConfigPath("./config.ini"){
    m_loop = loop;
    m_readBuf.reset(new xop::BufferReader());
    m_inifile_ptr.reset(new inifile::IniFile());
    if(access(m_ConfigPath.c_str(),0) == 0) {
        m_inifile_ptr->load(m_ConfigPath);
    }
    else{
        m_inifile_ptr->setValue("COMMON","ip","192.168.1.105","");
        m_inifile_ptr->setValue("COMMON","user","root","");
        m_inifile_ptr->setValue("COMMON","password","123456","");
        for(int i = 1; i <= 4; i++){
            char buf[20];
            sprintf(buf,"CHANNEL_%d",i);
            m_inifile_ptr->setValue(buf,"Enable","OFF");
            m_inifile_ptr->setValue(buf,"Result","720P");
            m_inifile_ptr->setValue(buf,"Mirror","OFF");
            m_inifile_ptr->setValue(buf,"BitRate","Middle");
        }
        m_inifile_ptr->saveas(m_ConfigPath);
    }
    m_fd = inotify_init();
    if ( m_fd < 0 ) {
        perror( "inotify_init" );
        cout<<"inotify_init error"<<endl;
    }
    m_wd = inotify_add_watch( m_fd,m_ConfigPath.c_str(), IN_MODIFY);
    if(m_wd < 0) {
        perror("inotify_add_watch");
        cout<<"inotify_add_watch error"<<endl;
    }
    m_ConfigChannel.reset(new xop::Channel(m_fd));
    m_ConfigChannel->setReadCallback([this](){this->handleRead();});
    m_ConfigChannel->enableReading();
    m_loop->updateChannel(m_ConfigChannel);
    set_socket();
}
bool Config::setConfigChangeCB(const ConfigChangeCB &cb) {
    m_ConfigChangeCB = cb;
    return false;
}
bool Config::handleRead() {
    int i = 0;
    fprintf(stderr,"fd is readable.\n");
    int length = read(m_fd,m_buf,EVENT_BUF_LEN);
    //printf("length=%d\n",length);
    if(length < 0) {
        perror("read");
    }
    while(i < length) {
        //fprintf(stderr,"inside while ...\n");
        struct inotify_event *event = (struct inotify_event*)&m_buf[i];
        //printf("event->len = %d\n",event->len);
        printf("%x\n",event->mask);
        if(event->mask & IN_MODIFY)
        {
            /*文件发生变化,调用回调函数*/
            if(m_ConfigChangeCB){
                m_ConfigChangeCB();
            }
            printf("modified\n");
        }
        i += EVENT_SIZE + event->len;
    }
    i = 0;
    inotify_rm_watch(m_fd,m_wd);
    close(m_fd);
    m_fd = inotify_init();
    m_wd = inotify_add_watch(m_fd,m_ConfigPath.c_str(), IN_MODIFY);
    m_loop->removeChannel(m_ConfigChannel);
    m_ConfigChannel.reset(new xop::Channel(m_fd));
    m_ConfigChannel->setReadCallback([this](){this->handleRead();});
    m_ConfigChannel->enableReading();
    m_loop->updateChannel(m_ConfigChannel);
    return false;
}
int Config::test_read() {
    int res;
    char event_buf[1024];
    int event_size;
    int event_pos = 0;
    struct inotify_event *event;
    res = read(m_fd, event_buf, sizeof(event_buf));
    if(res < (int)sizeof(*event)) {
        if(errno == EINTR)
            return 0;
        printf("could not get event, %s\n", strerror(errno));
        return -1;
    }
    printf("event \n");
    while(res >= (int)sizeof(*event)) {
        event = (struct inotify_event *)(event_buf + event_pos);
        if(event->len) {
            if(event->mask & IN_CREATE) {
                printf("create file: %s\n", event->name);
            } else {
                printf("delete file: %s\n", event->name);
            }
            if(event->mask & IN_MODIFY){
                printf("modified file\n");
            }
        }
        event_size = sizeof(*event) + event->len;
        res -= event_size;
        event_pos += event_size;
    }
}
bool Config::set_socket() {
    int fd;
    struct sockaddr_un un;
    fd = socket(AF_UNIX,SOCK_STREAM,0);
    if(fd < 0){
        printf("Request socket failed!\n");
        return -1;
    }
    un.sun_family = AF_UNIX;
    unlink(filename);
    strcpy(un.sun_path,filename);
    if(bind(fd,(struct sockaddr *)&un,sizeof(un)) <0 ){
        printf("bind failed!\n");
        return -1;
    }
    if(listen(fd,1000) < 0){
        printf("listen failed!\n");
        return -1;
    }
    m_fd_2 = fd;
    m_ConfigChannel_2.reset(new xop::Channel(fd));
    m_ConfigChannel_2->enableReading();
    m_ConfigChannel_2->setReadCallback([this](){
        handleRead2();
    });
    m_loop->updateChannel(m_ConfigChannel_2);
    return true;
}
bool Config::handleRead2() {
    int fd = accept(m_fd_2,NULL,NULL);
    if(fd < 0){
        printf("accept failed\n");
        return -1;
    }
    printf("new connection\n");
    xop::ChannelPtr chnptr(new xop::Channel(fd));
    chnptr->setReadCallback([fd,this](){
        char buf[1024];
        printf("read fd = %d\n",fd);
        int ret = recv(fd,buf,1024,0);
        if(ret < 0){
            printf("recv faild\n");
        }
        printf("%s\n",buf);
        if(m_ConfigChangeCB_2){
            m_ConfigChangeCB_2();
        }
    });
    chnptr->setCloseCallback([this,chnptr](){
        auto ptr = chnptr;
        m_loop->removeChannel(ptr);
    });
    chnptr->enableReading();
    m_loop->updateChannel(chnptr);
    return true;
}
bool Config::reload_inifile() {
    if(access(m_ConfigPath.c_str(),0) == 0) {
        m_inifile_ptr->load(m_ConfigPath);
    }
    return true;
}

