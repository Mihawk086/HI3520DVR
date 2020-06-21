#ifndef HISI3520_DEVSTORAGE_H
#define HISI3520_DEVSTORAGE_H
#include <iostream>
#include <string>
#include <vector>
typedef enum
{
    FS_NTFS,
    FS_FAT32,
    FS_EXT2,
    FS_ERROR_TYPE,
}FILE_SYS_TYPE;
#define SDCARD_DEBUG 1
#define MOUNT_PATH "/mnt_1"
class DevStorage {
public:
    typedef struct  parititions_info_s
    {
        unsigned long nMajor;
        unsigned long nMinor;
        unsigned long nBlocks;
        std::string chDeviceName;
        int  nPrimacyDeviceFlag;
    }parititions_info_t;
    DevStorage();
    bool IsFat32(parititions_info_t t_paritition);
    bool isMounted(parititions_info_t t_paritition);
    bool MountPartition(parititions_info_t t_paritition);
    int UnMountPartition(parititions_info_t t_paritition);
    int UnMountDirectory();
    bool GetFirstDeviceFirstPartition(parititions_info_t &t_paritition);
    bool CheckDevice();
    int Read_Proc_Partition();
    bool PrintDevice();
    FILE_SYS_TYPE _getFSType(int devFD);
private:
    const std::string m_MntDirectory;
    std::vector<parititions_info_t> m_Device;
};
#endif //HISI3520_DEVSTORAGE_H
