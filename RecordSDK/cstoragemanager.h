#ifndef CSTORAGEMANAGER_H
#define CSTORAGEMANAGER_H
class CStorageManager
{
public:
    static CStorageManager* get_instance();
    static CStorageManager* pCStorageManager;
    int Start();
private:
    CStorageManager();
};
#endif // CSTORAGEMANAGER_H
