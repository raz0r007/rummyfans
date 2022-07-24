#include "AdMobHandler.h"

/**
 @brief  Constructor
 @param  -
 @return -
*/

AdMobHandler::AdMobHandler()
{
}


/**
 @brief    Get always the same instance of the object (Singleton)
 @param    -
 @return   AdMobHandler*
*/

AdMobHandler* AdMobHandler::getInstance()
{
    static AdMobHandler sharedInstance;
    return &sharedInstance;
}


/**
 @brief  Destructor
 @param  -
 @return -
*/

AdMobHandler::~AdMobHandler()
{
}
