//
// Created by fuck on 18-10-17.
//


#include "Hotplug.h"
#include "EventLoop.h"
#include "DevStorage.h"

#include <iostream>

using namespace std;
int main(){
    xop::EventLoop loop;
    Hotplug hotplug(&loop);
    DevStorage dev;
    hotplug.setAddDeviceCallback([&dev,&loop](){
        loop.addTimer([&dev](){
            cout<<"add device "<<endl;
            auto ret = dev.CheckDevice();
            cout<<ret<<endl;
        },5000,false);

    });

    hotplug.setRemoveDeviceCallback([&dev](){
        cout<<"remove deviec"<<endl;
        dev.UnMountDirectory();
    });

    loop.loop();
}