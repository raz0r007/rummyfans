#include "AppleAuthHandler.h"
#include "LanguageManager.h"
#include "UserInterface.h"
#include "../ThirdPartys/SQLite/SQLite.h"
#include "EventHandler.h"

/**
 @brief  Constructor
 @param  -
 @return -
*/

AppleAuthHandler::AppleAuthHandler()
{
}


/**
 @brief    Get always the same instance of the object (Singleton)
 @param    -
 @return   AppleAuthHandler*
*/

AppleAuthHandler* AppleAuthHandler::getInstance()
{
   static AppleAuthHandler sharedInstance;
   return &sharedInstance;
}


/**
 @brief  Callback onAuthorizationDidComplete (on Login ok)
 @param  authInfo: login info
 @return -
*/

void AppleAuthHandler::onAuthorizationDidComplete(const string& authInfo)
{
    Document authInfoResponse;
    authInfoResponse.Parse(authInfo.c_str());
    if(authInfoResponse.HasParseError())
    {
        UserInterface *ui = UserInterface::getInstance(Director::getInstance()->getRunningScene());
        ui->addWindow(WINDOW_ERROR, LanguageManager::getString("LABEL_ERROR_GENERIC"));
        return;
    }

    SQLite *sqlite = SQLite::getInstance();
    sqlite->setLoginMethod("Apple");
    
    if(authInfoResponse.HasMember("givenName"))
    {
        userName = authInfoResponse["givenName"].GetString(); //Only first time login Apple send this info
        sqlite->setNakamaUser(userName);
    }
    
    userId = authInfoResponse["user"].GetString();
    sqlite->setNakamaPassword(userId);
    
    EventHandler *eventHandler = EventHandler::getInstance();
    eventHandler->onTouchButtonPlay();
}


/**
 @brief  Callback onAuthorizationStatus (on Plugin Init)
 @param  authState: State (0 = Not Connected / 1 = Already Connected)
 @return -
*/

void AppleAuthHandler::onAuthorizationStatus(const string& authState) {
    
    Document authStateResponse;
    authStateResponse.Parse(authState.c_str());
    SQLite *sqlite = SQLite::getInstance();
    if(!authStateResponse.HasParseError() and sqlite->getLoginMethod() == "Apple" and authStateResponse["state"].GetInt() == 1)
    {
        userName = sqlite->getNakamaUser();
        userId = sqlite->getNakamaPassword();
    }
}


/**
 @brief  Callback onAuthorizationCompleteWithError (on Login Error)
 @param  authInfo: login info
 @return -
*/

void AppleAuthHandler::onAuthorizationCompleteWithError(const string& authInfo)
{
    Director *director = Director::getInstance();
    UserInterface *ui = UserInterface::getInstance(director->getRunningScene());

    if(director->getRunningScene()->getName() == "initScene")
    {
        Button *btnPlay = (Button*)director->getRunningScene()->getChildByName("layoutUI")->getChildByName("btnPlay");
        btnPlay->setTouchEnabled(false); //we need to disable this button because of an overlapping between btnPlay and btnHome (window error)
    }

    ui->removeCurrentWindow(); //remove login window
    ui->addWindow(WINDOW_ERROR, LanguageManager::getString("LABEL_ERROR_LOGIN_ACCOUNT"));
}


/**
 @brief  Destructor
 @param  -
 @return -
*/

AppleAuthHandler::~AppleAuthHandler()
{
}
