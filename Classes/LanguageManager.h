#ifndef __LANGUAGEMANAGER_H__
#define __LANGUAGEMANAGER_H__

#include "cocos2d.h"
#include "external/json/document.h"

using namespace std;
using namespace rapidjson;
using namespace cocos2d;

class LanguageManager
{
private:
    Document document;
    string getStringForKey(string key);
    static LanguageManager* getInstance();
    
public:
    LanguageManager();
    ~LanguageManager();
    
    static string getString(string key);
};

#endif
