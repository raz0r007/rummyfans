#include "PlayHandler.h"
#include "EventHandler.h"
#include "LanguageManager.h"
#include "../ThirdPartys/SQLite/SQLite.h"

/**
 @brief  Constructor
 @param  -
 @return -
*/

PlayHandler::PlayHandler()
{
}


/**
 @brief    Get always the same instance of the object (Singleton)
 @param    -
 @return   PlayHandler*
*/

PlayHandler* PlayHandler::getInstance()
{
   static PlayHandler sharedInstance;
   return &sharedInstance;
}


/**
 @brief    On connection status chaged on GameCenter or Google Play
 @param    Possible status: GPS_CONNECTED, GPS_DISCONNECTED, GPS_CONNECTION_ERROR
 @return   -
*/

void PlayHandler::onConnectionStatusChanged(int status)
{
    if(status == sdkbox::GPS_CONNECTED)
    {
        #if defined(__ANDROID__)
            SQLite *sqlite = SQLite::getInstance();
            EventHandler *eventHandler = EventHandler::getInstance();
            
            sqlite->setLoginMethod("Google");
            eventHandler->onTouchButtonPlay();
        #else
            //If I log-in to GameCenter, I need to request signature information in order to log-in in Nakama Server
            sdkbox::PluginSdkboxPlay::generateIdentityVerificationSignature(); //On callback 'onGenerateIdentityVerificationSignature' we to the redirection..
        #endif
    }
    else if(status == sdkbox::GPS_CONNECTION_ERROR)
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
}


/**
 @brief  Callback generateIdentityVerificationSignature()
         https://developer.apple.com/documentation/gamekit/gklocalplayer/1515407-generateidentityverificationsign
 @param  publicKey: key url
         signature: user signature
         salt: random string generated by Apple
         timest: date/time of signature
         error: error message (if exists)
 @return -
*/

void PlayHandler::onGenerateIdentityVerificationSignature(const string& publicKey, const string& signature, const string& salt, unsigned long long timest, const string& error)
{
    #if !defined(__ANDROID__)

        if(error.empty())
        {
            gcPublicKey = publicKey;
            gcSignature = signature;
            gcSalt = salt;
            gcTimestamp = timest;

            SQLite *sqlite = SQLite::getInstance();
            EventHandler *eventHandler = EventHandler::getInstance();

            sqlite->setLoginMethod("GameCenter");
            eventHandler->onTouchButtonPlay();
        }
        else
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

    #endif
}


/**
 @brief  Destructor
 @param  -
 @return -
*/

PlayHandler::~PlayHandler()
{
}