#include "MainLoop.h"
#include "CTimeManager.h"
#include "RecordLoop.h"
#include "RtspLoop.h"
#include "RtspPusherLoop.h"
#include <signal.h>
using namespace std;
void SAMPLE_VIO_HandleSig(HI_S32 signo)
{
    if (SIGINT == signo || SIGTSTP == signo){
        SAMPLE_COMM_SYS_Exit();
    }
    exit(-1);
}
int main()
{
    signal(SIGINT, SAMPLE_VIO_HandleSig);
    signal(SIGTERM, SAMPLE_VIO_HandleSig);
    CTimeManager::get_instance()->SetSystemTime();
    RtspLoop::get_instance();
    RecordLoop::Instance();
    RtspPusherLoop::Instance();
    MainLoop::Instance().runloop();
}
