#ifndef __FACEBOOKHANDLER_H__
#define __FACEBOOKHANDLER_H__

#include "PluginFacebook/PluginFacebook.h"

using namespace std;

class FacebookHandler : public sdkbox::FacebookListener
{
public:
    FacebookHandler();
    ~FacebookHandler();
    static FacebookHandler* getInstance();

    virtual void onLogin(bool isLogin, const string& msg);
    virtual void onSharedSuccess(const string& message) {}
    virtual void onSharedFailed(const string& message) {}
    virtual void onSharedCancel() {}
    virtual void onAPI(const string& key, const string& jsonData);
    virtual void onPermission(bool isLogin, const string& msg) {}
    virtual void onFetchFriends(bool ok, const string& msg) {}
    virtual void onRequestInvitableFriends( const sdkbox::FBInvitableFriendsInfo& friends) {};
    virtual void onInviteFriendsWithInviteIdsResult( bool result, const string& msg) {}
    virtual void onInviteFriendsResult(bool result, const string& msg) {}
    virtual void onGetUserInfo( const sdkbox::FBGraphUser& userInfo ) {};
};

#endif
