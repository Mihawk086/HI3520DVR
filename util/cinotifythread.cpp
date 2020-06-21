#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/inotify.h>
#include "cinotifythread.h"
CInotifythread::CInotifythread(){
}
CInotifythread::CInotifythread(string filepath,void* (*callback)(void* arg),void *ptr)
{
    m_filepath = filepath;
    _callback = callback;
    _ptr = ptr;
}
bool CInotifythread::Config(string filepath,void* (*callback)(void* arg),void *ptr){
    m_filepath = filepath;
    _callback = callback;
    _ptr = ptr;
    return true;
}
void* CInotifythread::run(void)
{
    int length,i=0;
    int fd,maxfd;
    int wd;
    int ret;
    fd_set rfds;
    struct timeval tv;
    char buffer[EVENT_BUF_LEN];
    fd = inotify_init();
    if ( fd < 0 )
    {
        perror( "inotify_init" );
    }
    wd = inotify_add_watch( fd,m_filepath.c_str(), IN_ALL_EVENTS);
    if(wd < 0)
    {
        perror("inotify_add_watch");
    }
    while(1)
        {
            FD_ZERO(&rfds);
            FD_SET(fd,&rfds);
            maxfd = fd + 1;
            tv.tv_sec = 10;
            tv.tv_usec = 0;
            ret = select(maxfd,&rfds,NULL,NULL,&tv);
            switch(ret)
            {
                case -1:
                    fprintf(stderr,"select failed\n");
                    break;
                case 0:
                    continue;
                default:
                    fprintf(stderr,"fd is readable.\n");
                    length = read(fd,buffer,EVENT_BUF_LEN);
                    if(length < 0)
                    {
                        perror("read");
                    }
                    while(i < length)
                    {
                        struct inotify_event *event = (struct inotify_event*)&buffer[i];
                        if(event->mask & IN_MODIFY)
                        {
                            _callback(_ptr);
                        }
                        i += EVENT_SIZE + event->len;
                    }
                    i = 0;
                    inotify_rm_watch(fd,wd);
                    close(fd);
                    fd = inotify_init();
                    wd = inotify_add_watch( fd,m_filepath.c_str(), IN_MODIFY);
                    break;
            }
        }
        inotify_rm_watch(fd,wd);
}

