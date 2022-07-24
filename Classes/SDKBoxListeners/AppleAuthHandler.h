#ifndef __APPLEAUTHHANDLER_H__
#define __APPLEAUTHHANDLER_H__

#include "PluginSignInWithApple/PluginSignInWithApple.h"

using namespace std;

class AppleAuthHandler : public sdkbox::SignInWithAppleListener
{
private:
    void onAuthorizationDidComplete(const string& authInfo);
    void onAuthorizationCompleteWithError(const string& authInfo);
    void onAuthorizationStatus(const string& authState);

public:
    AppleAuthHandler();
    ~AppleAuthHandler();
    static AppleAuthHandler* getInstance();

    string userId;
    string userName;
};

#endif
