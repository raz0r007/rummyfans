#ifndef __NAKAMASERVER_H__
#define __NAKAMASERVER_H__

#include "nakama-cpp/Nakama.h"
#include "network/HttpClient.h"

using namespace std;
using namespace Nakama;
using namespace cocos2d::network;

enum {
    //Init
    OPCODE_REQUEST_INIT_CARDS   = 0,
    OPCODE_SEND_INIT_CARDS      = 1,
    OPCODE_READY_FOR_PLAY       = 2,

    //Gameplay
    OPCODE_GRAB_CARD            = 3, //It could be a card from the stack or the last card seen
    OPCODE_LEAVE_CARD           = 4,
    OPCODE_ORDER_CARD           = 5,
    OPCODE_FINISH_GAME          = 6
};


class NakamaServer
{
private:
    
    //Callbacks
    void onLoginSucceeded(NSessionPtr);
    void onConnectSucceeded();
    void onConnectError(const NRtError&);
    void onDisconnect();
    
    void onMatchmakerMatchedCallback(NMatchmakerMatchedPtr);
    void onMatchPresenceCallback(const NMatchPresenceEvent&);
    void onReceiveDataFromMatch(NMatchData);

public:
    
    //Vars
    NClientParameters parameters;
    
    NSessionPtr m_session;
    NClientPtr m_client;
    NRtClientPtr m_rtClient;
    shared_ptr<NRtDefaultClientListener> m_rtListener;
   
    string matchMakerCurrentTicket;
    bool matchMakerOpponentFound;
    
    //Methods
    NakamaServer();
    ~NakamaServer();
    static NakamaServer* getInstance();
    
    void init();
    
    bool loginWithEmail(string email, string password);
    bool loginWithFacebook();
    bool loginWithGameCenter();
    bool loginWithGoogle();
    bool loginWithApple();
    void logout();
    
    bool connect();
    void disconnect();
    bool isConnected();
    
    bool getMyAccountInfo();
    bool updateMyAccountInfo(map<string, string> info, bool redirectToLoginOk = false);
    
    bool addMatchMaker(const string query = "+properties.playNewMatch:true");
    bool removeCurrentMatchmaker();
    
    bool joinMatchByToken(const string token);
    bool leaveMatch(const string matchId);
    
    bool sendMatchData(int64_t opCode, const NBytes &data, const vector<NUserPresence>& presences = {});
};

#endif
