#ifndef CDEVSTORAGE_H
#define CDEVSTORAGE_H
#include <vector>
#include <string>
typedef struct  parititions_info_s
{
    unsigned long nMajor;
    unsigned long nMinor;
    unsigned long nBlocks;
    char chDeviceName[50];
    int  nPrimacyDeviceFlag;
}parititions_info_t;
class CDevStorage
{
public:
    static CDevStorage* get_instance();
    static CDevStorage* pCDevStorage;
    int GetDiskTable();
    int WriteDiskTable();
    int AddPartition();
    int DeletePartition();
    int FormatPartition(parititions_info_t t_paritition);
    int GetDeviceInfo();
    int GetDeviceName();
    int GetSerialNo();
    int IsFat32(parititions_info_t t_paritition);
    int isMounted(parititions_info_t t_paritition);
    int RemoveAllPart();
    int MountPartition(parititions_info_t t_paritition);
    int UnMountPartition(parititions_info_t t_paritition);
    int UnMountDirectory();
    int GetFirstDeviceFirstPartition(parititions_info_t *pstuPartitionsInfo,int num,parititions_info_t &t_paritition);
    int CheckDevice();
    int Sign_Primacy(parititions_info_t *pstuPartitionsInfo,int nDeviceNum);
    int Read_Proc_Partition(parititions_info_t * pstuPartitionsInfo,int nMaxNum,int *pnNum);
private:
    CDevStorage();
    const std::string m_MntDirectory;
    parititions_info_t m_stuPartitionsInfo[30];
    std::vector<parititions_info_t> m_Device;
};
#endif
