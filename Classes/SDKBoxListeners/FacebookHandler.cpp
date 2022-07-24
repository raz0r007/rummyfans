#include "FacebookHandler.h"
#include "LanguageManager.h"
#include "EventHandler.h"
#include "../ThirdPartys/SQLite/SQLite.h"
#include "NakamaServer.h"

/**
 @brief  Constructor
 @param  -
 @return -
*/

FacebookHandler::FacebookHandler()
{
}


/**
 @brief    Get always the same instance of the object (Singleton)
 @param    -
 @return   FacebookHandler*
*/

FacebookHandler* FacebookHandler::getInstance()
{
   static FacebookHandler sharedInstance;
   return &sharedInstance;
}


/**
 @brief  onLogin Facebook (initScene)
 @param  isLogin: if the login has been correct or not
         msg: facebook response login
 @return -
*/

void FacebookHandler::onLogin(bool isLogin, const string& msg)
{
    Director *director = Director::getInstance();
    UserInterface *ui = UserInterface::getInstance(director->getRunningScene());
    EventHandler *eventHandler = EventHandler::getInstance();
    
    if(!isLogin and ui->getCurrentWindowOpen() != WINDOW_ERROR) //Login failed
    {
        if(director->getRunningScene()->getName() == "initScene")
        {
            Button *btnPlay = (Button*)director->getRunningScene()->getChildByName("layoutUI")->getChildByName("btnPlay");
            btnPlay->setTouchEnabled(false); //we need to disable this button because of an overlapping between btnPlay and btnHome (window error)
        }
        
        ui->removeCurrentWindow(); //remove login window
        ui->addWindow(WINDOW_ERROR, LanguageManager::getString("LABEL_ERROR_LOGIN_ACCOUNT"));
    }
    else if(isLogin and msg != "autologin") //manual login
    {
        SQLite::getInstance()->setLoginMethod("Facebook");
        eventHandler->onTouchButtonPlay();
    }
}


/**
 @brief  Facebook API response callback
         FB Graph API Web: https://developers.facebook.com/tools/explorer/?method=GET&path=me
 @param  key: key sent by http request
         jsonData: request response
 @return -
*/

void FacebookHandler::onAPI(const string& key, const string& jsonData)
{
    Document httpRequestResponse;
    httpRequestResponse.Parse(jsonData.c_str());
    if(httpRequestResponse.HasParseError())
    {
        UserInterface *ui = UserInterface::getInstance(Director::getInstance()->getRunningScene());
        ui->addWindow(WINDOW_ERROR, LanguageManager::getString("LABEL_ERROR_GENERIC"));
        return;
    }
    
    if(key == "me") //Info requested: id,first_name,picture{url} -first time / register to NakamaServer-
    {
        //Update my info on my Nakama account
        NakamaServer *nakama = NakamaServer::getInstance();
        
        map<string, string> info;
        info.insert(pair<string, string>("username", sdkbox::PluginFacebook::getUserID()));
        info.insert(pair<string, string>("display_name", httpRequestResponse["first_name"].GetString()));
        info.insert(pair<string, string>("avatar_url", httpRequestResponse["picture"]["data"]["url"].GetString()));
        info.insert(pair<string, string>("lang_tag", ""));
        info.insert(pair<string, string>("location", ""));
        info.insert(pair<string, string>("timezone", ""));

        nakama->updateMyAccountInfo(info, true); //On this method we call onLoginSucceeded()
    }
}


/**
 @brief  Destructor
 @param  -
 @return -
*/

FacebookHandler::~FacebookHandler()
{
}
