#include "NakamaServer.h"
#include "NakamaCocos2d/NCocosHelper.h"
#include "Game.h"
#include "LanguageManager.h"
#include "EventHandler.h"
#include "Bot/Bot.h"
#include "audio/include/AudioEngine.h"
#include "ThirdPartys/SQLite/SQLite.h"


/**
 @brief  Constructor
 @param  -
 @return -
*/

NakamaServer::NakamaServer()
{
    m_session = nullptr;
    m_rtClient = nullptr;
    m_rtListener = nullptr;
    matchMakerOpponentFound = false;
}


/**
 @brief    Get always the same instance of the object (Singleton)
 @param    -
 @return   NakamaServer*
*/

NakamaServer* NakamaServer::getInstance() {
    static NakamaServer sharedInstance;
    return &sharedInstance;
}


/**
 @brief  The server's initialization consists in schedule the tick() method for update everything each 50ms (the time recommended by Nakama team)
 @param  -
 @return -
*/

void NakamaServer::init()
{
    auto tickCallback = [this](float dt)
    {
        if(m_client)
            m_client->tick();
        
        if (m_rtClient)
            m_rtClient->tick();
    };

    auto scheduler = Director::getInstance()->getScheduler();
    scheduler->schedule(tickCallback, this, 0.05f, CC_REPEAT_FOREVER, 0, false, "nakama-tick");
}


/**
 @brief  Login to Nakama server using e-mail (if it's the first time, the user is registered)
 @param  email: email to login
         password: user password
 @return bool
*/

bool NakamaServer::loginWithEmail(string email, string password)
{
    if(m_session != nullptr)
        return false;
    
    string username = email.substr(0, email.find("@"));

    auto errorCallbackLogin = [this, email, password, username](const NError& error)
    {
        if(error.code == ErrorCode::NotFound)
        {
            auto succeededCallbackRegister = [username, email](const NSessionPtr session)
            {
                NakamaServer *nakama = NakamaServer::getInstance();
                nakama->m_session = session;
                
                map<string, string> info;
                info.insert(pair<string, string>("username", email));
                info.insert(pair<string, string>("display_name", username));
                info.insert(pair<string, string>("avatar_url", "https://anstra.net/rummyfans/defaultAvatar.png"));
                info.insert(pair<string, string>("lang_tag", ""));
                info.insert(pair<string, string>("location", ""));
                info.insert(pair<string, string>("timezone", ""));
             
                nakama->updateMyAccountInfo(info, true); //On this method we call onLoginSucceeded()
            };
            
            auto errorCallbackRegister = [](const NError& error)
            {
                UserInterface *ui = UserInterface::getInstance(Director::getInstance()->getRunningScene());
                ui->addWindow(WINDOW_ERROR, LanguageManager::getString("LABEL_ERROR_GENERIC"));
            };
            
            //Register
            m_client->authenticateEmail(email, password, email, true, {}, succeededCallbackRegister, errorCallbackRegister);
        }
        else
        {
            Game *game = Game::getInstance();
            game->players[0]->fillDefaultPlayerInfo(); //If we can't login, maybe the server is offline, so we fill default info and we play with a bot!
        }
    };
    
    m_client->authenticateEmail(email, password, email, false, {}, std::bind(&NakamaServer::onLoginSucceeded, NakamaServer::getInstance(), std::placeholders::_1), errorCallbackLogin);
    
    return true;
}


/**
 @brief  Login to Nakama server using facebook (if it's the first time, the user is registered)
 @param  -
 @return bool
*/

bool NakamaServer::loginWithFacebook()
{
    if(sdkbox::PluginFacebook::isLoggedIn() == false or sdkbox::PluginFacebook::getAccessToken().empty() == true or m_session != nullptr)
        return false;
    
    auto errorCallbackLogin = [this](const NError& error)
    {
        if(error.code == ErrorCode::NotFound)
        {
            auto succeededCallbackRegister = [](const NSessionPtr session)
            {
                NakamaServer *nakama = NakamaServer::getInstance();
                nakama->m_session = session;
              
                //My Facebook Info:
                sdkbox::FBAPIParam params;
                params["fields"] = "id,first_name,picture.type(large){url}";
                sdkbox::PluginFacebook::api("me", "GET", params, "me"); //FB server responses on FacebookHandler::onAPI, and there I update my info on Nakama and we call onLoginSucceeded()
                //
            };
            
            auto errorCallbackRegister = [](const NError& error)
            {
                UserInterface *ui = UserInterface::getInstance(Director::getInstance()->getRunningScene());
                ui->addWindow(WINDOW_ERROR, LanguageManager::getString("LABEL_ERROR_GENERIC"));
            };
            
            //Register
            m_client->authenticateFacebook(sdkbox::PluginFacebook::getAccessToken(), sdkbox::PluginFacebook::getUserID(), true, false, {}, succeededCallbackRegister, errorCallbackRegister);
        }
        else
        {
            Game *game = Game::getInstance();
            game->players[0]->fillDefaultPlayerInfo(); //If we can't login, maybe the server is offline, so we fill default info and we play with a bot!
        }
    };
    
    m_client->authenticateFacebook(sdkbox::PluginFacebook::getAccessToken(), sdkbox::PluginFacebook::getUserID(), false, false, {}, std::bind(&NakamaServer::onLoginSucceeded, NakamaServer::getInstance(), std::placeholders::_1), errorCallbackLogin);
    
    return true;
}


/**
 @brief  Login to Nakama server using GameCenter (if it's the first time, the user is registered)
 @param  -
 @return bool
*/

bool NakamaServer::loginWithGameCenter()
{
    #if defined(__ANDROID__) //Only iOS
        return false;
    #else
        PlayHandler *playHandler = PlayHandler::getInstance();
        if(sdkbox::PluginSdkboxPlay::isSignedIn() == false or m_session != nullptr or playHandler->gcPublicKey.empty() or playHandler->gcSignature.empty() or playHandler->gcSalt.empty())
                return false;

        auto errorCallbackLogin = [this](const NError& error)
        {
            if(error.code == ErrorCode::NotFound)
            {
                auto succeededCallbackRegister = [](const NSessionPtr session)
                {
                    NakamaServer *nakama = NakamaServer::getInstance();
                    nakama->m_session = session;

                    map<string, string> info;
                    info.insert(pair<string, string>("username", sdkbox::PluginSdkboxPlay::getPlayerAccountField("player_id")));
                    info.insert(pair<string, string>("display_name", sdkbox::PluginSdkboxPlay::getPlayerAccountField("name")));
                    info.insert(pair<string, string>("avatar_url", "https://anstra.net/rummyfans/defaultAvatar.png"));
                    info.insert(pair<string, string>("lang_tag", ""));
                    info.insert(pair<string, string>("location", ""));
                    info.insert(pair<string, string>("timezone", ""));

                    nakama->updateMyAccountInfo(info, true); //On this method we call onLoginSucceeded()
                };

                auto errorCallbackRegister = [](const NError& error)
                {
                    UserInterface *ui = UserInterface::getInstance(Director::getInstance()->getRunningScene());
                    ui->addWindow(WINDOW_ERROR, LanguageManager::getString("LABEL_ERROR_GENERIC"));
                };

                //Register
                PlayHandler *playHandler = PlayHandler::getInstance();
                m_client->authenticateGameCenter(sdkbox::PluginSdkboxPlay::getPlayerId(), "net.anstra.rummyfans", playHandler->gcTimestamp, playHandler->gcSalt, playHandler->gcSignature, playHandler->gcPublicKey, sdkbox::PluginSdkboxPlay::getPlayerAccountField("player_id"), true, {}, succeededCallbackRegister, errorCallbackRegister);
            }
            else
            {
                Game *game = Game::getInstance();
                game->players[0]->fillDefaultPlayerInfo(); //If we can't login, maybe the server is offline, so we fill default info and we play with a bot!
            }
        };

        m_client->authenticateGameCenter(sdkbox::PluginSdkboxPlay::getPlayerId(), "net.anstra.rummyfans", playHandler->gcTimestamp, playHandler->gcSalt, playHandler->gcSignature, playHandler->gcPublicKey, sdkbox::PluginSdkboxPlay::getPlayerAccountField("player_id"), false, {}, std::bind(&NakamaServer::onLoginSucceeded, NakamaServer::getInstance(), std::placeholders::_1), errorCallbackLogin);

        return true;
    #endif
}


/**
 @brief  Login to Nakama server using Apple Auth (if it's the first time, the user is registered)
         IMPORTANT NOTE: we are using 'authenticateCustom' because Nakama Team didn't implement 'authenticateApple' yet. They already commited this change in master: https://github.com/heroiclabs/nakama-cpp/commit/4bfe49748bade55c085d489851edd6e111c3eff5 but new Release Version is under construction.
         We're waiting new release version in order to use authenticateApple and new features..
  @param  -
 @return bool
*/

bool NakamaServer::loginWithApple()
{
    #if defined(__ANDROID__) //Only iOS
        return false;
    #else
        AppleAuthHandler *appleHandler = AppleAuthHandler::getInstance();
        if(m_session != nullptr or appleHandler->userId.empty())
            return false;

        auto errorCallbackLogin = [this](const NError& error)
        {
            if(error.code == ErrorCode::NotFound)
            {
                auto succeededCallbackRegister = [](const NSessionPtr session)
                {
                    NakamaServer *nakama = NakamaServer::getInstance();
                    AppleAuthHandler *appleHandler = AppleAuthHandler::getInstance();

                    nakama->m_session = session;
                    
                    map<string, string> info;
                    info.insert(pair<string, string>("username", appleHandler->userId));
                    info.insert(pair<string, string>("display_name", appleHandler->userName.empty() ? "Me" : appleHandler->userName));
                    info.insert(pair<string, string>("avatar_url", "https://anstra.net/rummyfans/defaultAvatar.png"));
                    info.insert(pair<string, string>("lang_tag", ""));
                    info.insert(pair<string, string>("location", ""));
                    info.insert(pair<string, string>("timezone", ""));

                    nakama->updateMyAccountInfo(info, true); //On this method we call onLoginSucceeded()
                };

                auto errorCallbackRegister = [](const NError& error)
                {
                    UserInterface *ui = UserInterface::getInstance(Director::getInstance()->getRunningScene());
                    ui->addWindow(WINDOW_ERROR, LanguageManager::getString("LABEL_ERROR_GENERIC"));
                };

                //Register
                AppleAuthHandler *appleHandler = AppleAuthHandler::getInstance();
                m_client->authenticateCustom(appleHandler->userId, appleHandler->userId, true, {}, succeededCallbackRegister, errorCallbackRegister);
            }
            else
            {
                Game *game = Game::getInstance();
                game->players[0]->fillDefaultPlayerInfo(); //If we can't login, maybe the server is offline, so we fill default info and we play with a bot!
            }
        };

        m_client->authenticateCustom(appleHandler->userId, appleHandler->userId, false, {}, std::bind(&NakamaServer::onLoginSucceeded, NakamaServer::getInstance(), std::placeholders::_1), errorCallbackLogin);

        return true;
    #endif
}


/**
 @brief  Login to Nakama server using Google Play Account (if it's the first time, the user is registered)
 @param  -
 @return bool
*/

bool NakamaServer::loginWithGoogle()
{
    #if !defined(__ANDROID__)
        return false;
    #endif

    if(sdkbox::PluginSdkboxPlay::isSignedIn() == false or m_session != nullptr)
        return false;
    
    auto errorCallbackLogin = [this](const NError& error)
    {
        if(error.code == ErrorCode::NotFound)
        {
            auto succeededCallbackRegister = [](const NSessionPtr session)
            {
                NakamaServer *nakama = NakamaServer::getInstance();
                nakama->m_session = session;
                
                map<string, string> info;
                info.insert(pair<string, string>("username", sdkbox::PluginSdkboxPlay::getPlayerAccountField("player_id")));
                info.insert(pair<string, string>("display_name", sdkbox::PluginSdkboxPlay::getPlayerAccountField("name")));
                info.insert(pair<string, string>("avatar_url", "https://anstra.net/rummyfans/defaultAvatar.png"));
                info.insert(pair<string, string>("lang_tag", ""));
                info.insert(pair<string, string>("location", ""));
                info.insert(pair<string, string>("timezone", ""));
             
                nakama->updateMyAccountInfo(info, true); //On this method we call onLoginSucceeded()
            };
            
            auto errorCallbackRegister = [](const NError& error)
            {
                UserInterface *ui = UserInterface::getInstance(Director::getInstance()->getRunningScene());
                ui->addWindow(WINDOW_ERROR, LanguageManager::getString("LABEL_ERROR_GENERIC"));
            };
            
            //Register
            m_client->authenticateGoogle(sdkbox::PluginSdkboxPlay::getPlayerAccountField("server_auth_code"), sdkbox::PluginSdkboxPlay::getPlayerAccountField("player_id"), true, {}, succeededCallbackRegister, errorCallbackRegister);
        }
        else
        {
            Game *game = Game::getInstance();
            game->players[0]->fillDefaultPlayerInfo(); //If we can't login, maybe the server is offline, so we fill default info and we play with a bot!
        }
    };
    
    m_client->authenticateGoogle(sdkbox::PluginSdkboxPlay::getPlayerAccountField("server_auth_code"), sdkbox::PluginSdkboxPlay::getPlayerAccountField("player_id"), false, {}, std::bind(&NakamaServer::onLoginSucceeded, NakamaServer::getInstance(), std::placeholders::_1), errorCallbackLogin);
    
    return true;
}


/**
 @brief  Callback onLoginSucceeded
 @param  session: users' s session
 @return -
*/

void NakamaServer::onLoginSucceeded(NSessionPtr session)
{
    NakamaServer *nakama = NakamaServer::getInstance();
 
    nakama->m_session = session;
    nakama->connect();
}


/**
 @brief  Connect to real-time multiplayer server
 @param  -
 @return bool
*/

bool NakamaServer::connect()
{
    if(m_session == nullptr)
        return false;

    //Callbacks
    m_rtListener.reset(new NRtDefaultClientListener());
    m_rtListener->setConnectCallback(std::bind(&NakamaServer::onConnectSucceeded, NakamaServer::getInstance()));
    m_rtListener->setErrorCallback(std::bind(&NakamaServer::onConnectError, NakamaServer::getInstance(), std::placeholders::_1));
    m_rtListener->setDisconnectCallback(std::bind(&NakamaServer::onDisconnect, NakamaServer::getInstance()));
    
    m_rtListener->setMatchmakerMatchedCallback(std::bind(&NakamaServer::onMatchmakerMatchedCallback, NakamaServer::getInstance(), std::placeholders::_1));
    m_rtListener->setMatchPresenceCallback(std::bind(&NakamaServer::onMatchPresenceCallback, NakamaServer::getInstance(), std::placeholders::_1));
    m_rtListener->setMatchDataCallback(std::bind(&NakamaServer::onReceiveDataFromMatch, NakamaServer::getInstance(), std::placeholders::_1));
    
    //Init real-time connection
    m_rtClient = NCocosHelper::createRtClient(m_client, DEFAULT_PORT);
    m_rtClient->setListener(m_rtListener.get());
    m_rtClient->connect(m_session, true);
    
    return true;
}


/**
 @brief  Callback onConnectSucceeded
 @param  -
 @return -
*/

void NakamaServer::onConnectSucceeded()
{
    NakamaServer *nakama = NakamaServer::getInstance();
    nakama->getMyAccountInfo();
}


/**
 @brief  Callback onConnectError
 @param  error: error msg
 @return -
*/

void NakamaServer::onConnectError(const NRtError &error)
{
    UserInterface *ui = UserInterface::getInstance(Director::getInstance()->getRunningScene());
    ui->addWindow(WINDOW_ERROR, LanguageManager::getString("LABEL_ERROR_CONNECT"));
}


/**
 @brief  Callback onDisconnect
 @param  -
 @return -
*/

void NakamaServer::onDisconnect()
{
    if(Bot::getInstance()->isPlaying)
        return;
    
    matchMakerOpponentFound = false;
    UserInterface *ui = UserInterface::getInstance(Director::getInstance()->getRunningScene());
    ui->addWindow(WINDOW_ERROR, LanguageManager::getString("LABEL_ERROR_DISCONNECT"));
    
    //Bug fix: if I disconnect I need to unschedule mainly 'waitForPlayersInfo'
    auto scheduler = Director::getInstance()->getScheduler();
    scheduler->unscheduleAllForTarget(Game::getInstance());
}


/**
 @brief  Logout from Nakama Server
 @param  -
 @return -
*/

void NakamaServer::logout()
{
    m_session = nullptr;
    m_rtClient = nullptr;
    m_rtListener = nullptr;
}


/**
 @brief  Disconnect from Nakama Server (real-time client)
 @param  -
 @return -
*/

void NakamaServer::disconnect()
{
    if (m_rtClient != nullptr and m_rtClient->isConnected())
        m_rtClient->disconnect();
}


/**
 @brief  Check if I'm connected to the server
 @param  -
 @return bool
*/

bool NakamaServer::isConnected()
{
    if(m_session == nullptr)
        return false;
    
    if (m_rtClient != nullptr)
        return m_rtClient->isConnected();
    
    return false;
}


/**
 @brief  Get my account information
 @param  -
 @return bool
*/

bool NakamaServer::getMyAccountInfo()
{
    if(m_session == nullptr)
        return false;
    
    auto succeededCallbackAccount = [](const NAccount &myAccount)
    {
        string defaultAvatarUrl = "https://anstra.net/rummyfans/defaultAvatar.png";
        Player *myPlayer = Game::getInstance()->players[0];
        SQLite *sqlite = SQLite::getInstance();
        
        myPlayer->pid = myAccount.user.id;
        myPlayer->name = myAccount.user.displayName;
        myPlayer->avatarUrl = myAccount.user.avatarUrl;
        
        #if !defined(__ANDROID__) //Only iOS
            if(sqlite->getLoginMethod() == "Apple" and AppleAuthHandler::getInstance()->userName.empty())
            {
                AppleAuthHandler::getInstance()->userName = myPlayer->name;
                sqlite->setNakamaUser(myPlayer->name);
            }
        #endif
        
        if(!myAccount.user.avatarUrl.empty() and myAccount.user.avatarUrl != defaultAvatarUrl)
        {
            //Get and transform my avatar to Sprite
            HttpRequest* requestPhoto = new HttpRequest();
            requestPhoto->setUrl(myAccount.user.avatarUrl);
            requestPhoto->setRequestType(HttpRequest::Type::GET);
            
            requestPhoto->setResponseCallback([](HttpClient* client, HttpResponse* response) {
                
                Player *myPlayer = Game::getInstance()->players[0];
                if(!response or response->getResponseCode() != 200)
                {
                    myPlayer->avatarUrl = "https://anstra.net/rummyfans/defaultAvatar.png";
                    
                    Image *m = new Image;
                    m->initWithImageFile("others/defaultAvatar.png");
                    myPlayer->avatarImg = m;
                    
                    return;
                }
                
                vector<char> *buffer = response->getResponseData();
                Image *m = new Image;
                m->initWithImageData(reinterpret_cast<unsigned char*>(&(buffer->front())), buffer->size());

                myPlayer->avatarImg = m;
            });
            
            HttpClient::getInstance()->send(requestPhoto);
            requestPhoto->release();
        }
        else
        {
            Image *m = new Image;
            m->initWithImageFile("others/defaultAvatar.png");
            myPlayer->avatarImg = m;
        }
        
        //When I logged in, connected and got my information, on UserInterface WINDOW_NEWMATCH (waitForPlayersInfo) we call addMatchMaker() for search opponents
    };
    
    auto errorCallbackAccount = [](const NError& error)
    {
        UserInterface *ui = UserInterface::getInstance(Director::getInstance()->getRunningScene());
        ui->addWindow(WINDOW_ERROR, LanguageManager::getString("LABEL_ERROR_GENERIC"));
    };
    
    m_client->getAccount(m_session, succeededCallbackAccount, errorCallbackAccount);
    
    return true;
}


/**
 @brief  Update my account information
 @param  info: info to update
         redirectToLoginOk: redirect to login OK
 @return bool
*/

bool NakamaServer::updateMyAccountInfo(map<string, string> info, bool redirectToLoginOk)
{
    if(m_session == nullptr)
        return false;
    
    auto succeededCallbackUpdate = [this, redirectToLoginOk]()
    {
        if(redirectToLoginOk)
            onLoginSucceeded(m_session);
    };
    
    auto errorCallbackUpdate = [](const NError& error)
    {
        UserInterface *ui = UserInterface::getInstance(Director::getInstance()->getRunningScene());
        ui->addWindow(WINDOW_ERROR, LanguageManager::getString("LABEL_ERROR_GENERIC"));
    };
    
    m_client->updateAccount(m_session, info.at("username"), info.at("display_name"), info.at("avatar_url"), info.at("lang_tag"), info.at("location"), info.at("timezone"), succeededCallbackUpdate, errorCallbackUpdate);
    
    return true;
}


/**
 @brief  Find a new opponent in the server
 @param  query: if we want to search a custom opponent
 @return bool
*/

bool NakamaServer::addMatchMaker(const string query)
{
    if(m_session == nullptr)
        return false;
    
    auto successCallbackMatchMaker = [this](const NMatchmakerTicket &ticket)
    {
        matchMakerCurrentTicket = ticket.ticket;
    };
    
    auto errorCallbackMatchMaker = [](const NRtError &error)
    {
        UserInterface *ui = UserInterface::getInstance(Director::getInstance()->getRunningScene());
        ui->addWindow(WINDOW_ERROR, LanguageManager::getString("LABEL_ERROR_GENERIC"));
    };
    
    Game *game = Game::getInstance();
    NStringMap stringProperties;

    //Custom information to send to my opponent when he found me
    stringProperties.insert(pair<string,string>("display_name", game->players[0]->name));
    stringProperties.insert(pair<string,string>("avatar_url", game->players[0]->avatarUrl));
    
    if(!game->matchId.empty())
        stringProperties.insert(pair<string,string>("nextRoundMatch", game->matchId)); //special for play next game round
    else
        stringProperties.insert(pair<string,string>("playNewMatch", "true")); //new match
    //
    
    //addMatchmaker for join in the pool. When a player/opponent is found, onMatchmakerMatchedCallback is called
    m_rtClient->addMatchmaker(2, 2, query, stringProperties, {}, successCallbackMatchMaker, errorCallbackMatchMaker);
    matchMakerOpponentFound = false;
    
    return true;
}


/**
 @brief  Remove current match maker
 @param  -
 @return bool
*/

bool NakamaServer::removeCurrentMatchmaker()
{
    if(m_session == nullptr)
        return false;
    
    auto successCallbackMatchMaker = []()
    {
    };
    
    auto errorCallbackMatchMaker = [](const NRtError &error)
    {
        UserInterface *ui = UserInterface::getInstance(Director::getInstance()->getRunningScene());
        ui->addWindow(WINDOW_ERROR, LanguageManager::getString("LABEL_ERROR_GENERIC"));
    };
    
    m_rtClient->removeMatchmaker(matchMakerCurrentTicket, successCallbackMatchMaker, errorCallbackMatchMaker);
    matchMakerOpponentFound = false;
    
    return true;
}


/**
 @brief  Callback called when an opponent is found to play
 @param  m: players information
 @return -
*/

void NakamaServer::onMatchmakerMatchedCallback(NMatchmakerMatchedPtr m)
{
    Director *director = Director::getInstance();
    string defaultAvatarUrl = "https://anstra.net/rummyfans/defaultAvatar.png";
    Game *game = Game::getInstance();
    Player *myPlayer = game->players[0];
    bool gameNextRound = false;
    bool alreadyJumpedMyUser = false;
    
    matchMakerOpponentFound = true;
    
    for(vector<NMatchmakerUser>::iterator it = m->users.begin(); it != m->users.end(); it++)
    {
        if((*it).presence.userId == myPlayer->pid)
        {
          if(alreadyJumpedMyUser == true)
          {
              //This is a bug fix. We shouldn't allow a match with players logged-in with the same credentials. It could cause problems/fatal errors in Nakama
              //In this case, we continue searching opponents...
              matchMakerOpponentFound = false;
              addMatchMaker();
              return;
          }
            
          alreadyJumpedMyUser = true;
          continue;
        }
        else if(game->players[1]->pid != "") //exeption when we play the next game round (we already have the information of my opponent)
        {
            gameNextRound = true;
            continue;
        }
        
        //I complete the opponent information
        game->players[1]->pid = (*it).presence.userId;
        
        for(map<string, string>::iterator mapInfo = (*it).stringProperties.begin(); mapInfo != (*it).stringProperties.end(); mapInfo++)
        {
            if((*mapInfo).first == "display_name")
            {
                game->players[1]->name = (*mapInfo).second;
            }
            else if((*mapInfo).first == "avatar_url")
            {
                if((*mapInfo).second == defaultAvatarUrl)
                {
                    game->players[1]->avatarUrl = defaultAvatarUrl;
                    
                    Image *m = new Image;
                    m->initWithImageFile("others/defaultAvatar.png");
                    game->players[1]->avatarImg = m;
                }
                else
                {
                    game->players[1]->avatarUrl = (*mapInfo).second;

                    //Get and transform the avatar to Sprite
                    HttpRequest* requestPhoto = new HttpRequest();
                    requestPhoto->setUrl((*mapInfo).second);
                    requestPhoto->setRequestType(HttpRequest::Type::GET);
                    
                    requestPhoto->setResponseCallback([](HttpClient* client, HttpResponse* response) {
                        
                        Game *game = Game::getInstance();
                        if(!response or response->getResponseCode() != 200)
                        {
                            game->players[1]->avatarUrl = "https://anstra.net/rummyfans/defaultAvatar.png";
                            
                            Image *m = new Image;
                            m->initWithImageFile("others/defaultAvatar.png");
                            game->players[1]->avatarImg = m;
                            
                            return;
                        }
                        
                        vector<char> *buffer = response->getResponseData();
                        Image *m = new Image;
                        m->initWithImageData(reinterpret_cast<unsigned char*>(&(buffer->front())), buffer->size());

                        game->players[1]->avatarImg = m;
                     });
                    
                    HttpClient::getInstance()->send(requestPhoto);
                    requestPhoto->release();
                }
            }
        }
    }
    
    //When I've the vector 'players' completed, it's time to join the match
    if(director->getRunningScene()->getName() == "matchScene")
    {
        UserInterface *ui = UserInterface::getInstance(director->getRunningScene());
        Layout* layoutWindow = (Layout*)director->getRunningScene()->getChildByName("layoutUI")->getChildByName("layoutUIWindow");
        Label *findingText = (Label*)layoutWindow->getChildByName("labelFindingText");
        Vec2 textPos = findingText->getPosition();

        findingText->stopAllActions();
        layoutWindow->removeChild(findingText); //we remove it and we add a new label opponentFound. We don't use setString() because there're some bugs in this cocos method..
        
        Label *labelOppFound = ui->addLabel(LanguageManager::getString("LABEL_OPPONENT_FOUND"), textPos, 14, 0, true, layoutWindow);
        labelOppFound->setName("labelOpponentFound");
    }
    
    game->matchToken = m->token; //first time we call joinMatchByToken() on UserInterface WINDOW_NEWMATCH (waitForPlayersInfo)
    if(gameNextRound == true)
        joinMatchByToken(m->token);
}


/**
 @brief  Join specific match by token
 @param  token: token to join to the specific match
 @return bool
*/

bool NakamaServer::joinMatchByToken(const string token)
{
    if(m_session == nullptr)
        return false;
    
    auto successCallbackJoin = [](const NMatch &match)
    {
        Game *game = Game::getInstance();
        NakamaServer *nakama = NakamaServer::getInstance();
        
        game->matchId = match.matchId;
        
        //When I join to the match, we can't start the game yet. We need to wait until my opponent and I are ready, so we schedule a method for check it:
        auto waitForPlayFunc = [](float) {
            Game *game = Game::getInstance();
            if(game->players[0]->readyForPlay and game->players[1]->readyForPlay)
            {
                auto scheduler = Director::getInstance()->getScheduler();
                scheduler->unschedule("threadWaitingForPlay", Game::getInstance());
                game->start();
            }
        };

        auto scheduler = Director::getInstance()->getScheduler();
        scheduler->unschedule("threadWaitingForPlay", Game::getInstance()); //for make sure that there's no similar threads already scheduled
        scheduler->schedule(waitForPlayFunc, Game::getInstance(), 0.1f, CC_REPEAT_FOREVER, 0, false, "threadWaitingForPlay");
        //
        
        if(match.presences.size() == 0)
        {
            game->dealCards(); //If I'm the first player who joined to the match, I've to create the stacks deckOfCards, cardsSeen and deal the cards for each player
            game->players[0]->readyForPlay = true; //Now I'm ready for play
            
            //Who starts playing? my opponent
            game->players[0]->myTurn = false;
            game->players[1]->myTurn = true;
        }
        else
        {
            nakama->sendMatchData(OPCODE_REQUEST_INIT_CARDS, {}); //If there's other player, I request him the initial cards for play
            
            //Who starts playing? me
            game->players[0]->myTurn = true;
            game->players[1]->myTurn = false;
        }
    };
    
    auto errorCallbackJoin = [](const NRtError &error)
    {
        UserInterface *ui = UserInterface::getInstance(Director::getInstance()->getRunningScene());
        ui->addWindow(WINDOW_ERROR, LanguageManager::getString("LABEL_ERROR_GENERIC"));
    };
    
    m_rtClient->joinMatchByToken(token, successCallbackJoin, errorCallbackJoin);
    return true;
}


/**
 @brief  Leave match
 @param  matchId: match Id to leave
 @return bool
*/

bool NakamaServer::leaveMatch(const string matchId)
{
    if(m_session == nullptr)
        return false;
    
    auto successCallbackLeave = []()
    {
    };
    
    auto errorCallbackLeave = [](const NRtError &error)
    {
        UserInterface *ui = UserInterface::getInstance(Director::getInstance()->getRunningScene());
        ui->addWindow(WINDOW_ERROR, LanguageManager::getString("LABEL_ERROR_GENERIC"));
    };
    
    m_rtClient->leaveMatch(matchId, successCallbackLeave, errorCallbackLeave);
    matchMakerOpponentFound = false;
    
    return true;
}


/**
 @brief  Callback when I receive info in the match
 @param  data: info received
 @return -
*/

void NakamaServer::onReceiveDataFromMatch(NMatchData data)
{
    Director *director = Director::getInstance();
    Game *game = Game::getInstance();
    NakamaServer *nakama = NakamaServer::getInstance();
    EventHandler *eventHandler = EventHandler::getInstance();
    
    switch(data.opCode)
    {
        case OPCODE_REQUEST_INIT_CARDS:
        {
            //If I receive this opCode, I should send the variables deckOfCards, cardsSeen and the cards of both players
            /*
            I send this information using this json structure (example):
             {
                 "myCards": [
                     {
                         "0": "card-Clubs-2"
                     },
                     {
                         "1": "card-Clubs-3"
                     },
                     {
                         "2": "card-Clubs-4"
                     }
                     .
                     .
                 ],
                 "opponentCards": [
                     {
                         "0": "card-Clubs-2"
                     },
                     {
                         "1": "card-Clubs-3"
                     },
                     {
                         "2": "card-Clubs-4"
                     }
                     .
                     .
                 ],
                 "deckOfCards": [
                     {
                         "10": "card-Clubs-2"
                     },
                     {
                         "9": "card-Clubs-3"
                     },
                     {
                         "8": "card-Clubs-4"
                     }
                     .
                     .
                 ],
                 "cardsSeen": "card-Clubs-2"
             }
            */
            
            //I construct the json in a string:
            NBytes dataToSend;
            
            //Their cards:
            vector<card> playerCards = game->players[1]->myCards;
            dataToSend = "{\"myCards\":["; //These are called 'myCards' because these are their cards
            
            for(int i = 0; i < playerCards.size(); i++)
                dataToSend += "{\"" + to_string(i) + "\":\"" + playerCards[i].imgName + "\"},";
            
            dataToSend[dataToSend.size()-1] = ']';
          
            //My cards:
            vector<card> myCards = game->players[0]->myCards;
            dataToSend += ",\"opponentCards\":["; //These are called 'opponentCards' because these are my cards
            
            for(int i = 0; i < myCards.size(); i++)
                dataToSend += "{\"" + to_string(i) + "\":\"" + myCards[i].imgName + "\"},";
            
            dataToSend[dataToSend.size()-1] = ']';
            
            //Deck of cards:
            dataToSend += ",\"deckOfCards\":[";
            stack<card> auxDeckOfCards = game->deckOfCards;
            
            while(!auxDeckOfCards.empty())
            {
                dataToSend += "{\"" + to_string(auxDeckOfCards.size()-1) + "\":\"" + auxDeckOfCards.top().imgName + "\"},";
                auxDeckOfCards.pop();
            }
            
            dataToSend[dataToSend.size()-1] = ']';
            
            //Cards seen:
            dataToSend += ",\"cardsSeen\":\"" + game->cardsSeen.top().imgName + "\"}";
            
            //I send the data:
            nakama->sendMatchData(OPCODE_SEND_INIT_CARDS, dataToSend);
            
            break;
        }
        case OPCODE_SEND_INIT_CARDS:
        {
            //If I receive this opCode, I've to fill my local variables with the info I received
            //I use rapidjson library for read the information
            Document infoReceived;
            infoReceived.Parse(data.data.c_str());
            if(infoReceived.HasParseError())
            {
                UserInterface *ui = UserInterface::getInstance(director->getRunningScene());
                ui->addWindow(WINDOW_ERROR, LanguageManager::getString("LABEL_ERROR_GENERIC"));
                return;
            }
            
            //My Cards:
            list<card> listCards = game->getFullDeckOfCards();
            const rapidjson::Value &myCards = infoReceived["myCards"];
            for (int i = 0; i < (int)myCards.Size(); i++)
            {
                card cardToInsert;
                cardToInsert.imgName = myCards[i][to_string(i).c_str()].GetString();

                list<card>::iterator itCard = find_if(listCards.begin(), listCards.end(), [cardToInsert](const card &card){ return card.imgName == cardToInsert.imgName; });
                if(itCard == listCards.end()) {
                    UserInterface *ui = UserInterface::getInstance(Director::getInstance()->getRunningScene());
                    ui->addWindow(WINDOW_ERROR, LanguageManager::getString("LABEL_ERROR_GENERIC"));
                    return;
                }
                
                cardToInsert.suit = (*itCard).suit;
                cardToInsert.value = (*itCard).value;
                cardToInsert.points = (*itCard).points;
                game->players[0]->myCards.push_back(cardToInsert);
            }
            
            //Opponent Cards:
            const rapidjson::Value &opponentCards = infoReceived["opponentCards"];
            for (int i = 0; i < (int)opponentCards.Size(); i++)
            {
                card cardToInsert;
                cardToInsert.imgName = opponentCards[i][to_string(i).c_str()].GetString();

                list<card>::iterator itCard = find_if(listCards.begin(), listCards.end(), [cardToInsert](const card &card){ return card.imgName == cardToInsert.imgName; });
                if(itCard == listCards.end()) {
                    UserInterface *ui = UserInterface::getInstance(Director::getInstance()->getRunningScene());
                    ui->addWindow(WINDOW_ERROR, LanguageManager::getString("LABEL_ERROR_GENERIC"));
                    return;
                }

                cardToInsert.suit = (*itCard).suit;
                cardToInsert.value = (*itCard).value;
                cardToInsert.points = (*itCard).points;
                game->players[1]->myCards.push_back(cardToInsert);
            }
            
            //Deck of Cards:
            const rapidjson::Value &deckOfCards = infoReceived["deckOfCards"];
            for (int i = (int)deckOfCards.Size() - 1; i >= 0; i--)
            {
                card cardToInsert;
                cardToInsert.imgName = deckOfCards[i][to_string(deckOfCards.Size() - i -  1).c_str()].GetString();

                list<card>::iterator itCard = find_if(listCards.begin(), listCards.end(), [cardToInsert](const card &card){ return card.imgName == cardToInsert.imgName; });
                if(itCard == listCards.end()) {
                    UserInterface *ui = UserInterface::getInstance(Director::getInstance()->getRunningScene());
                    ui->addWindow(WINDOW_ERROR, LanguageManager::getString("LABEL_ERROR_GENERIC"));
                    return;
                }
                
                cardToInsert.suit = (*itCard).suit;
                cardToInsert.value = (*itCard).value;
                cardToInsert.points = (*itCard).points;
                game->deckOfCards.push(cardToInsert);
            }
            
            //Card seen:
            card cardSeen;
            cardSeen.imgName = infoReceived["cardsSeen"].GetString();
            
            list<card>::iterator itCard = find_if(listCards.begin(), listCards.end(), [cardSeen](const card &card){ return card.imgName == cardSeen.imgName; });
            if(itCard == listCards.end()) {
                UserInterface *ui = UserInterface::getInstance(Director::getInstance()->getRunningScene());
                ui->addWindow(WINDOW_ERROR, LanguageManager::getString("LABEL_ERROR_GENERIC"));
                return;
            }
            
            cardSeen.suit = (*itCard).suit;
            cardSeen.value = (*itCard).value;
            cardSeen.points = (*itCard).points;
            game->cardsSeen.push(cardSeen);
            
            //Now, I'm ready for play:
            game->players[0]->readyForPlay = true;
            
            break;
        }
        case OPCODE_READY_FOR_PLAY:
        {
            //If I receive this code, it means that my opponent is ready for play
            game->players[1]->readyForPlay = true;
            
            break;
        }
        case OPCODE_GRAB_CARD:
        {
            //If I receive this opCode, it means that my opponent grab a card
            /*
             Example of format that I should receive:
             {
                 "From": "deckOfCards",
                 "Position": 10
             }
            */
            
            AudioEngine::play2d("sounds/cardTouch.wav");
            
            Document infoReceived;
            infoReceived.Parse(data.data.c_str());
            if(infoReceived.HasParseError())
            {
                UserInterface *ui = UserInterface::getInstance(director->getRunningScene());
                ui->addWindow(WINDOW_ERROR, LanguageManager::getString("LABEL_ERROR_GENERIC"));
                return;
            }
            
            string From = infoReceived["From"].GetString(); //The opponent can grab a card from 'deckOfCards' or 'cardsSeen'
            int position = infoReceived["Position"].GetInt(); //Position in 'myCards' vector where I should add the card
            
            if(From == "deckOfCards" and !game->deckOfCards.empty())
            {
                if(game->players[1]->addCard(game->deckOfCards.top(), position))
                {
                    game->deckOfCards.pop();
                    Label *aDeckCards = (Label*)game->scene->getChildByName("layoutUI")->getChildByName("amountDeckOfCards");
                    aDeckCards->setString(to_string(stoi(aDeckCards->getString()) - 1));
                }
            }
            else if(From == "cardsSeen" and !game->cardsSeen.empty())
            {
                if(game->players[1]->addCard(game->cardsSeen.top(), position))
                {
                    game->scene->removeChildByName(game->cardsSeen.top().imgName);
                    game->cardsSeen.pop();
                }
            }

            break;
        }
        case OPCODE_LEAVE_CARD:
        {
            //If I receive this opCode, it means that my opponent left a card in the stack
            /*
             Example of format that I should receive:
             {
                 "Card": "card-Clubs-2"
             }
            */
            
            Label *aDeckCards = (Label*)game->scene->getChildByName("layoutUI")->getChildByName("amountDeckOfCards");
            if(stoi(aDeckCards->getString()) == 2) //if there are 2 cards on deckOfCards, the game should end
            {
                if(game->scene->getChildByName("layoutUI")->getChildByName("btnContinue") == NULL) {
                    Node *btnContinueFake = Node::create();
                    btnContinueFake->setName("btnContinue");
                    game->scene->getChildByName("layoutUI")->addChild(btnContinueFake);
                }
                
                if(eventHandler->cardTouching != NULL)
                {
                    //If I'm touching a card I have to wait until I leave the card, and then I finish the game.
                    game->scene->scheduleOnce([this, data](float){
                        onReceiveDataFromMatch(data);
                    }, 0.25f, "scheduleOnceFinishGame");
                    
                    return;
                }
            }
            
            AudioEngine::play2d("sounds/cardTouch.wav");

            Rect safeArea = director->getSafeAreaRect();
            Size visibleSize = safeArea.size;
            Document infoReceived;
            infoReceived.Parse(data.data.c_str());
            if(infoReceived.HasParseError())
            {
                UserInterface *ui = UserInterface::getInstance(director->getRunningScene());
                ui->addWindow(WINDOW_ERROR, LanguageManager::getString("LABEL_ERROR_GENERIC"));
                return;
            }
            
            //Card to insert in cardsSeen:
            card cardToInsert;
            cardToInsert.imgName = infoReceived["Card"].GetString();

            vector<card>::iterator itCard;
            itCard = find_if(game->players[1]->myCards.begin(), game->players[1]->myCards.end(), [cardToInsert](const card &card){ return card.imgName == cardToInsert.imgName; });
            if(itCard == game->players[1]->myCards.end()) {
                UserInterface *ui = UserInterface::getInstance(Director::getInstance()->getRunningScene());
                ui->addWindow(WINDOW_ERROR, LanguageManager::getString("LABEL_ERROR_GENERIC"));
                return;
            }
            
            cardToInsert.suit = (*itCard).suit;
            cardToInsert.value = (*itCard).value;
            cardToInsert.points = (*itCard).points;
            
            //Push the card:
            Vec2 cardPosition = game->scene->getChildByName("layoutOpponentCards")->getChildByName(cardToInsert.imgName)->getPosition();
            game->cardsSeen.push(cardToInsert);
            
            Sprite *newLastSeenCard = Sprite::create("cards/" + game->cardsSeen.top().imgName + ".png");
            newLastSeenCard->setName(game->cardsSeen.top().imgName);
            newLastSeenCard->setPosition(game->scene->getChildByName("layoutOpponentCards")->getChildByName(cardToInsert.imgName)->getParent()->convertToWorldSpace(cardPosition));
            newLastSeenCard->setScale(visibleSize.width * CARD_SIZE / newLastSeenCard->getContentSize().width);
            game->scene->addChild(newLastSeenCard, (int32_t)game->cardsSeen.size());

            game->players[1]->removeCard(cardToInsert);
            
            Action *actionCardInsert = MoveTo::create(0.3, game->scene->getChildByName("cardBackground")->getPosition());
            actionCardInsert->setTag(1);
            newLastSeenCard->runAction(actionCardInsert);
            
            if(stoi(aDeckCards->getString()) == 2) //if there are 2 cards on deckOfCards, the game ends
            {
                if(game->scene->getChildByName("layoutUI")->getChildByName("btnContinue") != NULL)
                    game->scene->getChildByName("layoutUI")->removeChildByName("btnContinue");
                
                game->finishGameRound(NULL, "Draw");
            }
            else
            {
                game->players[0]->myTurn = true;
                game->players[1]->myTurn = false;
                game->scene->getChildByName("avatarMarked")->setPosition(game->scene->getChildByName("myAvatar")->getPosition());
            }
            
            break;
        }
        case OPCODE_ORDER_CARD:
        {
            //If I receive this opCode, it means that my opponent reordered a card
            /*
             Example of format that I should receive:
             {
                 "Card": "card-Clubs2",
                 "Position": 5
             }
            */
            
            Document infoReceived;
            infoReceived.Parse(data.data.c_str());
            if(infoReceived.HasParseError())
            {
                UserInterface *ui = UserInterface::getInstance(director->getRunningScene());
                ui->addWindow(WINDOW_ERROR, LanguageManager::getString("LABEL_ERROR_GENERIC"));
                return;
            }
            
            int position = infoReceived["Position"].GetInt();

            card cardToMove;
            cardToMove.imgName = infoReceived["Card"].GetString();
            
            vector<card>::iterator itCard;
            itCard = find_if(game->players[1]->myCards.begin(), game->players[1]->myCards.end(), [cardToMove](const card &card){ return card.imgName == cardToMove.imgName; });
            if(itCard == game->players[1]->myCards.end()) {
                UserInterface *ui = UserInterface::getInstance(Director::getInstance()->getRunningScene());
                ui->addWindow(WINDOW_ERROR, LanguageManager::getString("LABEL_ERROR_GENERIC"));
                return;
            }
            
            cardToMove.suit = (*itCard).suit;
            cardToMove.value = (*itCard).value;
            cardToMove.points = (*itCard).points;
            
            /*
             Note: We don't call 'reorderCard' because the method has a lot of movements (MoveTo, etc) and we have found a bug if the opponent moves many cards
             quickly. We can't sync and manage it correctly. So, we prefer just order the vector and NOT move the card sprites.
            */
            
            for(int i = 0; i < game->players[1]->myCards.size(); i++)
                game->scene->getChildByName("layoutOpponentCards")->getChildByName(game->players[1]->myCards[i].imgName)->setTag(i);
                
            //Re-order the vector (logic: https://coliru.stacked-crooked.com/a/5c31007000b9eeba)
            int oldIndex = (int)distance(game->players[1]->myCards.begin(), itCard);
            int newIndex = position;
            if (oldIndex > newIndex)
                rotate(game->players[1]->myCards.rend() - oldIndex - 1, game->players[1]->myCards.rend() - oldIndex, game->players[1]->myCards.rend() - newIndex);
            else
                rotate(game->players[1]->myCards.begin() + oldIndex, game->players[1]->myCards.begin() + oldIndex + 1, game->players[1]->myCards.begin() + newIndex + 1);
            
            for(int i = 0; i < game->players[1]->myCards.size(); i++)
                game->scene->getChildByName("layoutOpponentCards")->getChildByTag(i)->setName(game->players[1]->myCards[i].imgName); //update sprite names
            
            break;
        }
        case OPCODE_FINISH_GAME:
        {
            //If I receive this opCode, it means that my opponent has finished the game
            /*
             Example of format that I should receive:
             {
                 "Mode": "Gin", <-- mode can be 'Gin' or 'Knock'
                 "Card": "card-Clubs-2" <-- this is the card he used to break/finish the game
             }
            */
            
            if(game->scene->getChildByName("layoutUI")->getChildByName("btnContinue") == NULL) {
                Node *btnContinueFake = Node::create();
                btnContinueFake->setName("btnContinue");
                game->scene->getChildByName("layoutUI")->addChild(btnContinueFake);
            }
            
            if(eventHandler->cardTouching != NULL)
            {
                //If I'm touching a card I have to wait until I leave the card, and then I finish the game.
                game->scene->scheduleOnce([this, data](float){
                    onReceiveDataFromMatch(data);
                }, 0.25f, "scheduleOnceFinishGame");
                
                return;
            }
            
            Rect safeArea = director->getSafeAreaRect();
            Size visibleSize = safeArea.size;
            Document infoReceived;
            infoReceived.Parse(data.data.c_str());
            if(infoReceived.HasParseError())
            {
                UserInterface *ui = UserInterface::getInstance(director->getRunningScene());
                ui->addWindow(WINDOW_ERROR, LanguageManager::getString("LABEL_ERROR_GENERIC"));
                return;
            }
            
            //Card for finish the game:
            string mode = infoReceived["Mode"].GetString();
            card cardToFinish;
            cardToFinish.imgName = infoReceived["Card"].GetString();

            vector<card>::iterator itCard;
            itCard = find_if(game->players[1]->myCards.begin(), game->players[1]->myCards.end(), [cardToFinish](const card &card){ return card.imgName == cardToFinish.imgName; });
            if(itCard == game->players[1]->myCards.end()) {
                UserInterface *ui = UserInterface::getInstance(Director::getInstance()->getRunningScene());
                ui->addWindow(WINDOW_ERROR, LanguageManager::getString("LABEL_ERROR_GENERIC"));
                return;
            }
            
            cardToFinish.suit = (*itCard).suit;
            cardToFinish.value = (*itCard).value;
            cardToFinish.points = (*itCard).points;
            
            //Push the card:
            Vec2 cardPosition = game->scene->getChildByName("layoutOpponentCards")->getChildByName(cardToFinish.imgName)->getPosition();

            Sprite *cardBg = Sprite::create("cards/card" + mode + ".png");
            cardBg->setName("cardFinishBackground");
            cardBg->setScale(visibleSize.width * 0.105f / cardBg->getContentSize().width);
            cardBg->setPosition(game->scene->getChildByName("cardBackground")->getPositionX() - game->scene->getChildByName("cardBackground")->getBoundingBox().size.width * 1.05f, game->scene->getChildByName("cardBackground")->getPositionY());
            game->scene->addChild(cardBg);
            
            Sprite *cardFinishGame = Sprite::create("cards/cardBack.png");
            cardFinishGame->setName(cardToFinish.imgName);
            cardFinishGame->setPosition(game->scene->getChildByName("layoutOpponentCards")->getChildByName(cardToFinish.imgName)->getParent()->convertToWorldSpace(cardPosition));
            cardFinishGame->setScale(visibleSize.width * CARD_SIZE / cardFinishGame->getContentSize().width);
            game->scene->addChild(cardFinishGame);
            
            game->players[1]->removeCard(cardToFinish);
            
            Action *actionFinishGame = MoveTo::create(0.3, game->scene->getChildByName("cardFinishBackground")->getPosition());
            actionFinishGame->setTag(1);
            cardFinishGame->runAction(actionFinishGame);
            
            if(game->scene->getChildByName("layoutUI")->getChildByName("btnContinue") != NULL)
                game->scene->getChildByName("layoutUI")->removeChildByName("btnContinue");
            
            //Finish the game && show all cards
            game->finishGameRound(game->players[1], mode);

            break;
        }
        default:
        {
            break;
        }
    }
}


/**
 @brief  Callback when we detect joins / leaves in the match
 @param  event: event details
 @return -
*/

void NakamaServer::onMatchPresenceCallback(const NMatchPresenceEvent &event)
{
    Game *game = Game::getInstance();
    NakamaServer *nakama = NakamaServer::getInstance();
    auto scheduler = Director::getInstance()->getScheduler();

    if(event.matchId != game->matchId)
        return;
    
    //Joins:
    if(!event.joins.empty() and !game->players[0]->readyForPlay) //If there's joins and I'm not ready...
    {
        //I will schedule a method for check my status. When I'm ready I will let my opponent know:
        auto checkMyStatus = [](float) {
            Game *game = Game::getInstance();
            if(game->players[0]->readyForPlay)
            {
                auto scheduler = Director::getInstance()->getScheduler();
                NakamaServer *nakama = NakamaServer::getInstance();
                
                scheduler->unschedule("threadCheckMyStatus", Game::getInstance());
                nakama->sendMatchData(OPCODE_READY_FOR_PLAY, {});
            }
        };

        scheduler->unschedule("threadCheckMyStatus", Game::getInstance()); //for make sure that there's no similar threads already scheduled
        scheduler->schedule(checkMyStatus, Game::getInstance(), 0.1f, CC_REPEAT_FOREVER, 0, false, "threadCheckMyStatus");
    }
    else if(!event.joins.empty() and game->players[0]->readyForPlay) //If there's joins and I'm ready...
        nakama->sendMatchData(OPCODE_READY_FOR_PLAY, {});
  
    
    //Leaves:
    if(!event.leaves.empty()) //If someone left the match because of some reason (quit the game, connection problems, etc)
    {
        UserInterface *ui = UserInterface::getInstance(Director::getInstance()->getRunningScene());
        
        //bug fix: when we play the next round and touch "Continue" we leave the match and it shouldn't be announced to my opponent
        if(game->scene->getChildByName("layoutUI")->getChildByName("btnContinue") == NULL)
            ui->addWindow(WINDOW_FINISH);
    }
}


/**
 @brief  Send data to an existing match
 @param  opCode: kind of msg to send
         data: info to send
         presences: players to send the information. By default, the info is sent to all players in the match
 @return bool
*/

bool NakamaServer::sendMatchData(int64_t opCode, const NBytes &data, const vector<NUserPresence>& presences)
{
    Game *game = Game::getInstance();
    if(m_session == nullptr or game->matchId.empty())
        return false;
    
    m_rtClient->sendMatchData(game->matchId, opCode, data, presences);

    return true;
}


/**
 @brief  Destructor
 @param  -
 @return -
*/

NakamaServer::~NakamaServer()
{
}

