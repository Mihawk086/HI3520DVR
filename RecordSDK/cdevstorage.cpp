#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <mntent.h>
#include <errno.h>

#include "cdevstorage.h"

#define MOUNTDIR "/mnt_1"

#define SDCARD_DEBUG 0


CDevStorage* CDevStorage::get_instance()
{
    return pCDevStorage;
}

CDevStorage* CDevStorage::pCDevStorage = new CDevStorage();

CDevStorage::CDevStorage():m_MntDirectory(MOUNTDIR) {
}

//获取分区表内容
int CDevStorage::GetDiskTable() {

}

//写分区信息
int CDevStorage::WriteDiskTable() {

}

//添加分区
int CDevStorage::AddPartition()
{

}

//删除分区
int CDevStorage::DeletePartition()
{

}

//得到设备序号的磁盘信息
int CDevStorage::GetDeviceInfo()
{

}

//得到设备名
int CDevStorage::GetDeviceName()
{

}

//得到设备序号
int CDevStorage::GetSerialNo()
{

}

//删除所有分区
int CDevStorage::RemoveAllPart()
{

}

//判断分区是否挂载
int CDevStorage::isMounted(parititions_info_t t_paritition)
{
    char *filename = "/proc/mounts";
    FILE *mntfile;
    struct mntent *mntent;

    mntfile = setmntent(filename, "r");
    if (!mntfile) {
        printf("Failed to read mtab file, error [%s]\n",
                        strerror(errno));
        return -1;
    }
    /*
    struct mntent 结构体
    */
#if 0
struct mntent
{
    char *mnt_fsname;           /* 文件系统对应的设备路径或者服务器地址  */
    char *mnt_dir;              /* 文件系统挂载到的系统路径 */
    char *mnt_type;             /* 文件系统类型: ufs, nfs, 等  */
    char *mnt_opts;             /* 文件系统挂载参数，以逗号分隔  */
    int mnt_freq;               /* 文件系统备份频率（以天为单位）  */
    int mnt_passno;             /* 开机fsck的顺序，如果为0，不会进行check */
};
#endif
    while(mntent = getmntent(mntfile))
    {
/*
        printf("mnt_dir:%s, mnt_fsname:%s, mnt_type:%s, mnt_opts:%s\n",
                        mntent->mnt_dir,
                        mntent->mnt_fsname,
                        mntent->mnt_type,
                        mntent->mnt_opts);
*/
        if(strstr(mntent->mnt_fsname,t_paritition.chDeviceName)!=NULL)
        {
            if(SDCARD_DEBUG) {
                printf("%s mount on %s\n", mntent->mnt_fsname, mntent->mnt_dir);
            }
            return 0;
        }
    }
    endmntent(mntfile);
    return -1;
}

//格式化分区
int CDevStorage::FormatPartition(parititions_info_t t_paritition)
{
    char cmd[100];
    sprintf(cmd,"mkfs.vfat /dev/%s",t_paritition.chDeviceName);
    system(cmd);
    return 0;
}

//挂载分区
int CDevStorage::MountPartition(parititions_info_t t_paritition)
{
    char cmd[100];
    sprintf(cmd,"mount -t vfat /dev/%s %s",t_paritition.chDeviceName,m_MntDirectory.c_str());
    if(SDCARD_DEBUG)
        printf("%s\n",cmd);
    system(cmd);
    return 0;
}

//卸载分区
int CDevStorage::UnMountPartition(parititions_info_t t_paritition)
{
    char cmd[100];
    sprintf(cmd,"umount /dev/%s",t_paritition.chDeviceName);
    if(SDCARD_DEBUG)
        printf("%s\n",cmd);
    system(cmd);
    return 0;
}


int CDevStorage::UnMountDirectory()
{
    char buf[100];
    sprintf(buf,"umount %s",m_MntDirectory.c_str());
    if(SDCARD_DEBUG)
        printf("%s\n",buf);
    system(buf);
}

int CDevStorage::CheckDevice()
{
    parititions_info_t t_FirstParitition;
    memset(&t_FirstParitition,0,sizeof(t_FirstParitition));
    int ret = -1;
    memset(m_stuPartitionsInfo,0,sizeof(m_stuPartitionsInfo));
    int nMaxNum = sizeof(m_stuPartitionsInfo)/sizeof(m_stuPartitionsInfo[0]);
    int nNum = 0;

    Read_Proc_Partition(m_stuPartitionsInfo, nMaxNum, &nNum);
    if(SDCARD_DEBUG) {
        printf("\n Num[%d] \n", nNum);
    }

    //获取第一个设备的第一个分区
    ret = GetFirstDeviceFirstPartition(m_stuPartitionsInfo,nNum,t_FirstParitition);
    if(ret == 0) {
        if(SDCARD_DEBUG) {
            printf("get first device successful\n");
            printf("%s\n", t_FirstParitition.chDeviceName);
        }
    }
    else {
        printf("get first device fail\n");
        return -1;
    }

    //检查分区是否fat32文件系统
    ret = IsFat32(t_FirstParitition);
    if(ret == -1) {
        printf("is not FAT32\n");
        return -1;
    }

    //判断分区是否挂载
    ret = isMounted(t_FirstParitition);
    if(ret == -1) {
        MountPartition(t_FirstParitition);
        return 0;
    }
    return 0;
    //UnMountPartition(t_FirstParitition);
}


int CDevStorage::Sign_Primacy(parititions_info_t *pstuPartitionsInfo,int nDeviceNum)
{
    int i = 0;
    int j = 0;
    char chDeviceNameLastSign[100] = {0};
    if((NULL == pstuPartitionsInfo) ||
        (nDeviceNum <= 0))
    {
        printf("\nparam error\n");
        return -1;
    }

    for(i = 0; i < (nDeviceNum - 1);i++)
    {
        if((0 != strlen(chDeviceNameLastSign)) &&
            (NULL != strstr(pstuPartitionsInfo[i].chDeviceName,chDeviceNameLastSign)))
        {
            continue;
        }

        for(j = i + 1 ; j < nDeviceNum; j++)
        {
            if(pstuPartitionsInfo[i].nMajor != pstuPartitionsInfo[j].nMajor) //major相同才比较
            {
                break;
            }
            if(NULL != strstr(pstuPartitionsInfo[j].chDeviceName, pstuPartitionsInfo[i].chDeviceName))
            {
                 pstuPartitionsInfo[i].nPrimacyDeviceFlag = 1;

                 memset(chDeviceNameLastSign,0,sizeof(chDeviceNameLastSign));
                 strncpy(chDeviceNameLastSign,pstuPartitionsInfo[i].chDeviceName,sizeof(chDeviceNameLastSign) - 1);
                 if(SDCARD_DEBUG) {
                     printf("\n zzh_test primacy  chDeviceName=%s,nMajor[%lu] minor=%lu Flag = %d\n",
                            pstuPartitionsInfo[i].chDeviceName,
                            pstuPartitionsInfo[i].nMajor,
                            pstuPartitionsInfo[i].nMinor,
                            pstuPartitionsInfo[i].nPrimacyDeviceFlag);
                 }
                    i = j;

                 break;
            }
        }
    }

    return 0;
}

int CDevStorage::Read_Proc_Partition(parititions_info_t * pstuPartitionsInfo,int nMaxNum,int *pnNum)
{
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

    if(NULL == pstuPartitionsInfo ||
        NULL == pnNum)
    {
        printf("\n param error \n");
        return -1;
    }

    fp = fopen("/proc/partitions","r");

    if (NULL == fp)
    {
        printf("\n fopen /proc/partitions failed \n");
        return -1;
    }

    while(1)
    {
        nFindDeviceFlag = 0;
        memset(chBuffer,0,sizeof(chBuffer));
        if(NULL == fgets(chBuffer,sizeof(chBuffer),fp))
        {
            break;
        }
        memset(chDeviceName,0,sizeof(chDeviceName));
        nSscanfNum = sscanf(chBuffer ," %lu %lu %lu %[^\n]",&nMajor,&nMinor,&nBlocks,chDeviceName);

        if(4 != nSscanfNum)
        {
            continue;
        }

        if(0 != strncmp(chDeviceName,"sd",strlen("sd")))
        {
            continue;
        }

        memset(&pstuPartitionsInfo[nDeviceNum], 0, sizeof(parititions_info_t));
        strncpy(pstuPartitionsInfo[nDeviceNum].chDeviceName,chDeviceName,sizeof(pstuPartitionsInfo[nDeviceNum].chDeviceName) -1);
        pstuPartitionsInfo[nDeviceNum].nMajor = nMajor;
        pstuPartitionsInfo[nDeviceNum].nMinor = nMinor;
        pstuPartitionsInfo[nDeviceNum].nBlocks = nBlocks;

        if(SDCARD_DEBUG) {
            printf("\n  nMajor[%lu],   nMinor[%lu],    nBlocks[%lu],    chDeviceName[%s] \n", nMajor, nMinor, nBlocks,
                   chDeviceName);
        }
        //find device
        nDeviceNum++;

        if(nDeviceNum >= nMaxNum)//最多处理nMaxNum个
        {
            fclose(fp);
            *pnNum = nDeviceNum;
            Sign_Primacy(pstuPartitionsInfo,nDeviceNum);
            return 0;
        }
    }
    fclose(fp);

    *pnNum = nDeviceNum;
    if(nDeviceNum > 0)
    {
        Sign_Primacy( pstuPartitionsInfo,nDeviceNum);
    }
    return 0;
}


int CDevStorage::GetFirstDeviceFirstPartition(parititions_info_t *pstuPartitionsInfo,int num,parititions_info_t &t_paritition)
{
    int i = 0;
    for(i = 0; i < num ; i++)
    {
        //printf("\n%d\n",pstuPartitionsInfo[i].nPrimacyDeviceFlag);
        if(pstuPartitionsInfo[i].nPrimacyDeviceFlag == 1)
        {
            if(SDCARD_DEBUG) {
                printf("\n%s\n", pstuPartitionsInfo[i].chDeviceName);
            }
            if((i+1) < num)
            {
                if(pstuPartitionsInfo[i+1].nPrimacyDeviceFlag == 0)
                {
                    if(SDCARD_DEBUG) {
                        printf("\n%s\n", pstuPartitionsInfo[i + 1].chDeviceName);
                    }
                    t_paritition = pstuPartitionsInfo[i+1];
                    return 0;
                }
            }
        }
    }
    return -1;
}

int CDevStorage::IsFat32(parititions_info_t t_paritition)
{
    FILE * p_file = NULL;
    char buf[1024];
    int ret = -1;

    p_file = popen("fdisk -l", "r");
    if (!p_file)
    {
        fprintf(stderr, "Erro to popen");
    }
    while (fgets(buf,sizeof(buf), p_file) != NULL)
    {
        //fprintf(stdout, "%s", buf);
        if(strstr(buf,t_paritition.chDeviceName) != NULL)
        {
            if(strstr(buf,"FAT32") != NULL)
            {
                if(SDCARD_DEBUG) {
                    printf("%s", buf);
                    printf("paritition is FAT32\n");
                }
                ret = 0;
            }
        }
    }
    pclose(p_file);

    return ret;
}


