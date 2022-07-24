#ifndef __IAPHANDLER_H__
#define __IAPHANDLER_H__

#include "PluginIAP/PluginIAP.h"

using namespace std;

class IAPHandler : public sdkbox::IAPListener
{
private:
    bool itemAdsPurchased;
    
    void onInitialized(bool ok);
    void onSuccess(sdkbox::Product const& p);
    void onFailure(sdkbox::Product const& p, const string &msg) {};
    void onCanceled(sdkbox::Product const& p) {};
    void onRestored(sdkbox::Product const& p);
    void onRestoreComplete ( bool ok , const string & msg ) {};
    void onProductRequestSuccess(vector<sdkbox::Product> const &products) {}
    void onProductRequestFailure(const string &msg) {}

public:
    IAPHandler();
    ~IAPHandler();
    static IAPHandler* getInstance();
    bool adsPurchased();
};

#endif
