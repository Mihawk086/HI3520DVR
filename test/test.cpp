//
// Created by fuck on 18-10-15.
//

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/stat.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<sys/un.h>
#include<errno.h>
#include<stddef.h>
#include<unistd.h>
#define BUFFER_SIZE 1024
const char *filename="uds-tmp";

#include "DevStorage.h"


int main00()
{
    struct sockaddr_un un;
    int sock_fd;
    const char* buffer = "fuck";
    un.sun_family = AF_UNIX;
    strcpy(un.sun_path,filename);
    sock_fd = socket(AF_UNIX,SOCK_STREAM,0);
    if(sock_fd < 0){
        printf("Request socket failed\n");
        return -1;
    }
    if(connect(sock_fd,(struct sockaddr *)&un,sizeof(un)) < 0){
        printf("connect socket failed\n");
        return -1;
    }

    //send(sock_fd,buffer, sizeof(buffer),0);

    sleep(5);
    send(sock_fd,buffer, sizeof(buffer),0);

    close(sock_fd);

    return 0;
}


#include <iostream>

#include<stdio.h>
#include<unistd.h>
#include<fcntl.h>
#include <sys/types.h>
#include <string>
#include <string.h>
#include <errno.h>
#include <sys/time.h>


static uint32_t getTimeStamp()
{
    struct timeval tv = {0};
    gettimeofday(&tv, NULL);
    uint32_t ts = ((tv.tv_sec*1000)+((tv.tv_usec)/1000)); // 90: _clockRate/1000;
    return ts;
}

using namespace std;

int main01(){
    string filename("/mnt_1/test.hole");
    mode_t f_attrib = S_IRUSR|S_IWUSR;
    int fd=open(filename.c_str(),O_RDWR|O_CREAT|O_TRUNC, f_attrib);
    if(fd<0) {
        perror("creat file fail");
    }
    int time = getTimeStamp();
    cout<<"start "<<time<<"ms"<<endl;
    if(ftruncate(fd,1024*1024*90) < 0) {
        perror("could not deallocate");
    }
    int end = getTimeStamp();
    cout<<"end "<<end<<"ms"<<endl;
    int du = end - time;
    printf("%d ms\n",du);
    if(lseek(fd,0,SEEK_SET)==-1) {
        perror("could not sleek");
    }

}


int main02(){
    DevStorage dev;
    if(dev.CheckDevice()){
        cout<<"find"<<endl;
        sleep(4);
        dev.UnMountDirectory();
    }

}


#include "Config_Client.h"

int main(){
    Config_Client::get_instance().get_time_config();
}