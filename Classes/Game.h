#ifndef __GAME_H__
#define __GAME_H__

#include "Player.h"

#ifdef SDKBOX_ENABLED
    #include "PluginFacebook/PluginFacebook.h"
    #include "SDKBoxListeners/FacebookHandler.h"

    #include "PluginIAP/PluginIAP.h"
    #include "SDKBoxListeners/IAPHandler.h"

    #include "PluginAdMob/PluginAdMob.h"
    #include "SDKBoxListeners/AdMobHandler.h"

    #include "PluginSdkboxPlay/PluginSdkboxPlay.h"
    #include "SDKBoxListeners/PlayHandler.h"

    #include "PluginSignInWithApple/PluginSignInWithApple.h"
    #include "SDKBoxListeners/AppleAuthHandler.h"
#endif

using namespace std;
using namespace cocos2d;

class Game
{
private:
    static Game* m_pInstance;
    
public:
    Scene* scene;
    stack<card> deckOfCards;
    stack<card> cardsSeen;
    Player *players[2];
    
    string matchId; //Match Id in Nakama Server
    string matchToken;
    
    Game();
    ~Game();
    static Game* getInstance();
    static void destroyInstance();
  
    list<card> getFullDeckOfCards();
    
    void findOpponent();
    
    void dealCards();
    
    bool start();
    void stop();
    
    void finishGameRound(Player *playerWinner, string gameModeFinished);
};

#endif
