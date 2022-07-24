#ifndef __ENCRYPTIONMANAGER_H__
#define __ENCRYPTIONMANAGER_H__

#include "cocos2d.h"
#include "xxtea.h"

using namespace std;
using namespace cocos2d;

class EncryptionManager {
private:
    unsigned char key[23];
    xxtea_long keyLength;
    xxtea_long retLength;
    
public:
    EncryptionManager();
    ~EncryptionManager();
    static EncryptionManager* getInstance();

    string encryptString(string data);
    string decryptString(string data);
};

#endif
