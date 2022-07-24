#include "LanguageManager.h"

/**
 @brief  Constructor
 @param  -
 @return -
*/

LanguageManager::LanguageManager()
{
    string fileName;
    switch(Application::getInstance()->getCurrentLanguage())
    {
        case LanguageType::ENGLISH:
             fileName = "lang/en.json";
             break;
        default:
            CCLOG("Unknown language. Use english");
            fileName = "lang/en.json";
            break;
    };
    
    string clearContent = FileUtils::getInstance()->getStringFromFile(fileName);
    document.Parse<0>(clearContent.c_str());
    if(document.HasParseError())
    {
        CCLOG("Language file parsing error!");
        return;
    }
}


/**
 @brief    Get always the same instance of the object (Singleton)
 @param    -
 @return   LanguageManager*
*/

LanguageManager* LanguageManager::getInstance()
{
    static LanguageManager sharedInstance;
    return &sharedInstance;
}


/**
 @brief   Get a string with a key
 @param   key: json key
 @return  string
*/

string LanguageManager::getStringForKey(string key)
{
    return document[key.c_str()].GetString();
}


/**
 @brief  Get a string
 @param  key: json key
 @return string
*/

string LanguageManager::getString(string key)
{
    return getInstance()->getStringForKey(key);
}


/**
 @brief  Destructor
 @param  -
 @return -
*/

LanguageManager::~LanguageManager()
{
}
