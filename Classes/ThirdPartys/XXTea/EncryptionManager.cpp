#include "EncryptionManager.h"

/**
 @brief  Constructor
 @param  -
 @return -
*/

EncryptionManager::EncryptionManager()
{
    strcpy((char*)key, "w7Kni1L9v6T04Pj2Fa9iMgR");
    keyLength = sizeof(key);
    retLength = 0;
}


/**
 @brief    Get always the same instance of the object (Singleton)
 @param    -
 @return   EncryptionManager*
*/

EncryptionManager* EncryptionManager::getInstance() {
    static EncryptionManager sharedInstance;
    return &sharedInstance;
}


/**
 @brief    Encrypt string
 @param    data: string to encrypt
 @return   Return the encrypted string
*/

string EncryptionManager::encryptString(string data)
{
    if(data != "")
    {
        xxtea_long retLength = 0;
        unsigned const char *encryptedData = reinterpret_cast<unsigned const char*>(xxtea_encrypt((unsigned char *)data.c_str(), (xxtea_long)data.size(), (unsigned char *)key, keyLength, &retLength));
        
        char* out = NULL;
        base64Encode(encryptedData, retLength, &out); //We convert it to base64
        return string(out);
    }
    else
        return "";
}


/**
 @brief    Decrypt string
 @param    data: string to decrypt
 @return   Return the decrypted string
*/

string EncryptionManager::decryptString(string data)
{
    if(data != "")
    {
        unsigned char* output = NULL;
        int outLength = base64Decode((unsigned char*)data.c_str(), (unsigned int)strlen(data.c_str()), &output);
        char *decryptedData = reinterpret_cast<char*>(xxtea_decrypt(output, outLength, (unsigned char *)key, keyLength, &retLength));

        return decryptedData;
    }
    else
        return "";
}


/**
 @brief  Destructor
 @param  -
 @return -
*/

EncryptionManager::~EncryptionManager()
{
}
