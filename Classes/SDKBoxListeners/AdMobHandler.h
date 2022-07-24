#ifndef __ADMOBHANDLER_H__
#define __ADMOBHANDLER_H__

#include "PluginAdMob/PluginAdMob.h"

using namespace std;

class AdMobHandler : public sdkbox::AdMobListener
{
private:
    void adViewDidReceiveAd(const string &name) {}
    void adViewDidFailToReceiveAdWithError(const string &name, const string &msg) {}
    void adViewWillPresentScreen(const string &name) {}
    void adViewDidDismissScreen(const string &name) {}
    void adViewWillDismissScreen(const string &name) {}
    void adViewWillLeaveApplication(const string &name) {}
    void reward(const string &name, const string &currency, double amount) {}
    
public:
    AdMobHandler();
    ~AdMobHandler();
    static AdMobHandler* getInstance();
};

#endif
