#ifndef __PLAYHANDLER_H__
#define __PLAYHANDLER_H__

#include "PluginSdkboxPlay/PluginSdkboxPlay.h"

using namespace std;

class PlayHandler : public sdkbox::SdkboxPlayListener
{
public:
    #if !defined(__ANDROID__) //Special information for users logged in GameCenter
        string gcPublicKey;
        string gcSignature;
        string gcSalt;
        unsigned long long gcTimestamp;
    #endif

    PlayHandler();
    ~PlayHandler();
    static PlayHandler* getInstance();

    void onConnectionStatusChanged( int status );
    void onScoreSubmitted( const string& leaderboard_name, long score, bool maxScoreAllTime, bool maxScoreWeek, bool maxScoreToday ) {}
    void onIncrementalAchievementUnlocked( const string& achievement_name ) {}
    void onIncrementalAchievementStep( const string& achievement_name, double step ) {}
    void onAchievementUnlocked( const string& achievement_name, bool newlyUnlocked ) {}
    void onMyScore( const string& leaderboard_name, int time_span, int collection_type, long score ) {};
    void onMyScoreError ( const string & leaderboard_name, int time_span, int collection_type, int error_code, const string & error_description) {};
    void onGenerateIdentityVerificationSignature(const string& publicKey, const string& signature, const string& salt, unsigned long long timest, const string& error);
};


#endif
