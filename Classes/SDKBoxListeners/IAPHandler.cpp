#include "IAPHandler.h"

/**
 @brief  Constructor
 @param  -
 @return -
*/

IAPHandler::IAPHandler()
{
    itemAdsPurchased = false;
}


/**
 @brief    Get always the same instance of the object (Singleton)
 @param    -
 @return   IAPHandler*
*/

IAPHandler* IAPHandler::getInstance()
{
   static IAPHandler sharedInstance;
   return &sharedInstance;
}


/**
 @brief  on Init plugin method
 @param  ok: true = ok / false = something wrong
 @return -
*/

void IAPHandler::onInitialized(bool ok)
{
    //We always call restore for get purchases not restored (if exist)
    sdkbox::IAP::restore();
}


/**
 @brief  Purchase success method
 @param  p: product purchased
 @return -
*/

void IAPHandler::onSuccess(sdkbox::Product const& p)
{
    if(p.id == "rummyfans.noads")
        itemAdsPurchased = true;
}


/**
 @brief    It's called when we call sdkbox::IAP::restore() and there are items to restore.
           It's called N times, where N = amount of items restored.
 @param    p: product restored
 @return   -
*/

void IAPHandler::onRestored(sdkbox::Product const& p)
{
    if(p.id == "rummyfans.noads")
        itemAdsPurchased = true;
}


/**
 @brief  Getter
 @param  -
 @return bool
*/

bool IAPHandler::adsPurchased()
{
    return itemAdsPurchased;
}


/**
 @brief  Destructor
 @param  -
 @return -
*/

IAPHandler::~IAPHandler()
{
}
