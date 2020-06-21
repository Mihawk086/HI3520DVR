#include <cstdio>
#include <cstring>
#include <mntent.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <sys/mount.h>
#include <iostream>
#include "DevStorage.h"
DevStorage::DevStorage() {
}
bool DevStorage::IsFat32(DevStorage::parititions_info_t t_paritition) {
    char devPath[512];
    int devFD = 0;
    FILE_SYS_TYPE fsType = FS_ERROR_TYPE
    snprintf(devPath,511,"/dev/%s",t_paritition.chDeviceName.c_str());
    devFD = open(devPath, O_RDONLY);
    if(!devFD){
        printf("Open device failed!\n");
        return false;
    }
    fsType = _getFSType(devFD);
    close(devFD);
    if(fsType == FS_FAT32) {
        return true;
    }else{
        return false;
    }
}
bool DevStorage::isMounted(DevStorage::parititions_info_t t_paritition) {
    char *filename = "/proc/mounts";
    FILE *mntfile;
    struct mntent *mntent;
    mntfile = setmntent(filename, "r");
    if (!mntfile) {
        printf("Failed to read mtab file, error [%s]\n",
               strerror(errno));
        return false;
    }
    while(mntent = getmntent(mntfile))
    {
        if(strstr(mntent->mnt_fsname,t_paritition.chDeviceName.c_str())!=NULL){
            if(SDCARD_DEBUG) {
                printf("%s mount on %s\n", mntent->mnt_fsname, mntent->mnt_dir);
            }
            return true;
        }
    }
    endmntent(mntfile);
    return false;
}
bool DevStorage::MountPartition(DevStorage::parititions_info_t t_paritition) {
    if(t_paritition.nPrimacyDeviceFlag == 1){
        printf("mount error\n");
        return false;
    }
    char devPath[512];
    snprintf(devPath,511,"/dev/%s",t_paritition.chDeviceName.c_str());
    if(SDCARD_DEBUG){
        printf("devPath: %s\n",devPath);
    }
    int ret = mount((const char*)devPath, MOUNT_PATH, (const char*)"vfat", 0, NULL);
    if (ret == -1){
        return false;
    }
    return true;
}
int DevStorage::UnMountPartition(DevStorage::parititions_info_t t_paritition) {
    return 0;
}
int DevStorage::UnMountDirectory() {
    umount(MOUNT_PATH);
    char buf[100];
    sprintf(buf,"umount %s",MOUNT_PATH);
    if(SDCARD_DEBUG)
        printf("%s\n",buf);
    system(buf);
    return 0;
}
bool DevStorage::GetFirstDeviceFirstPartition(DevStorage::parititions_info_t &t_paritition) {
    for(auto const &item:m_Device){
        if(item.nPrimacyDeviceFlag == 0){
            t_paritition = item;
            return true;
        }
    }
    return false;
}
bool DevStorage::CheckDevice() {
    DevStorage::parititions_info_t t_paritition;
    m_Device.clear();
    if(Read_Proc_Partition() == -1){
        std::cout<<"get device error"<<std::endl;
        return false;
    }
    PrintDevice();
    if(GetFirstDeviceFirstPartition(t_paritition) == false){
        std::cout<<"get first error"<<std::endl;
        return false;
    }
    if(IsFat32(t_paritition) == false){
        std::cout<<t_paritition.chDeviceName<<"is no fat32"<<std::endl;
        return false;
    }
    if(isMounted(t_paritition) == true){
        std::cout<<t_paritition.chDeviceName<<"is mounted"<<std::endl;
        return true;
    }
    if(MountPartition(t_paritition) == false){
        std::cout<<t_paritition.chDeviceName<<"mount fail"<<std::endl;
        return false;
    }
    return true;
}
int DevStorage::Read_Proc_Partition() {
    FILE *fp = NULL;
    int ret = -1;
    int i = 0;
    int nSscanfNum = 0;
    unsigned long nMajor = 0;
    unsigned long nMinor = 0;
    unsigned long nBlocks = 0;
    char chDeviceName[50] = {0};
    char chBuffer[1024] = {0};
    int nDeviceNum = 0;
    int nFindDeviceFlag = 0;
    parititions_info_t PartitionsInfo;
    std::string nums("0123456789");
    m_Device.clear();
    fp = fopen("/proc/partitions","r");
    if (NULL == fp) {
        printf("\n fopen /proc/partitions failed \n");
        return -1;
    }
    while(1){
        nFindDeviceFlag = 0;
        memset(chBuffer,0,sizeof(chBuffer));
        if(NULL == fgets(chBuffer,sizeof(chBuffer),fp)) {
            break;
        }
        memset(chDeviceName,0,sizeof(chDeviceName));
        nSscanfNum = sscanf(chBuffer ," %lu %lu %lu %[^\n]",&nMajor,&nMinor,&nBlocks,chDeviceName);
        if(4 != nSscanfNum) {
            continue;
        }
        if(0 != strncmp(chDeviceName,"sd",strlen("sd"))) {
            continue;
        }
        PartitionsInfo.chDeviceName.assign(chDeviceName);
        PartitionsInfo.nMajor = nMajor;
        PartitionsInfo.nMinor = nMinor;
        PartitionsInfo.nBlocks = nBlocks;
        if(PartitionsInfo.chDeviceName.find_first_of(nums) == std::string::npos){
            PartitionsInfo.nPrimacyDeviceFlag = 1;
        }
        else{
            PartitionsInfo.nPrimacyDeviceFlag = 0;
        }
        if(SDCARD_DEBUG) {
            printf("\n  nMajor[%lu],nMinor[%lu],nBlocks[%lu],chDeviceName[%s] \n", nMajor, nMinor, nBlocks,
                   chDeviceName);
        }
        nDeviceNum++;
        m_Device.push_back(PartitionsInfo);
    }
    fclose(fp);
    return 0;
}
bool DevStorage::PrintDevice() {
    for(auto const &temp:m_Device){
        std::cout<<"DeviceName "<<temp.chDeviceName<<std::endl;
        std::cout<<"Major "<<temp.nMajor<<std::endl;
        std::cout<<"Minor "<<temp.nMinor<<std::endl;
        std::cout<<"Blocks "<<temp.nBlocks<<std::endl;
        std::cout<<"PrimacyDeviceFlag "<<temp.nPrimacyDeviceFlag<<std::endl;
    }
    return true;
}
FILE_SYS_TYPE DevStorage::_getFSType(int devFD)
{
    unsigned char tmpBuffer[0x400];
    unsigned char* pOffset = NULL;
    unsigned int readSize = 0;
    pOffset = &tmpBuffer[0];
    memset(pOffset, 0x00, 0x400);
    readSize = read(devFD, pOffset, 0x100);
    if(!readSize){
        printf("read device file failed!\n");
        return FS_ERROR_TYPE;
    }
    if(!memcmp((const void*)(pOffset+0x52), (const void*)"FAT32", 5))//th offset with 0x52 is fat32 tag;{
        printf("filesystem:fat32!\n");
        return FS_FAT32;
    }
    else if(!memcmp((const void*)(pOffset+0x3), (const void*)"NTFS", 4))//the offset with 0x3 is ntfs tag;
    {
        printf("filesystem:ntfs!\n");
        return FS_NTFS;
    }
    else{
        memset(pOffset, 0x00, 0x400);
        lseek(devFD,0x400, SEEK_SET); //seek to superblock1;
        readSize = read(devFD, pOffset, 0x400);//read the superblock1 to buffer;
        if(!readSize){
            printf("read1 device file failed!\n");
            return FS_ERROR_TYPE;
        }
        pOffset = pOffset+0x38; //the offset 0x438 is the tag of ext2:0x53 0xef;
        if((pOffset[0] == 0x53) && (pOffset[1] == 0xef)){
            printf("filesystem:ext2 or ext3!\n");
            return FS_EXT2;
        }
        else{
            printf("the offset 0x438 vaule is:0x%x 0x%x!\n", pOffset[0], pOffset[1]);
        }
    }
    printf("Unknown filesystem type!\n");
    return FS_ERROR_TYPE;
}

