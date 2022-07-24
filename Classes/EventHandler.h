#ifndef __EVENTHANDLER_H__
#define __EVENTHANDLER_H__

#include "UserInterface.h"

using namespace cocos2d::experimental;

class EventHandler {
private:
    //UI Buttons
    void onTouchButtonPlay();
    void onTouchButtonRules();
    void onTouchButtonNoAds();
    
    void onTouchBuyItemNoAds();
    void onTouchRestoreItemNoAds();
    
    void onTouchButtonLoginWithEmail();
    bool onTouchButtonOkEmail(EditBox* inputText);
    
    void onTouchButtonLoginWithGameCenterOrGoogle();
    void onTouchButtonLoginWithFacebook();
    void onTouchButtonLoginWithApple();
    void onTouchButtonLogout();

    void onTouchButtonHome();
    
    void onTouchButtonContinue();
    
    bool onTouchCloseWindow();
    
    //Card events
    bool onTouchCardBegin(Touch *touch, Event *event);
    bool onTouchCardMoved(Touch *touch, Event *event);
    bool onTouchCardEnd(Touch *touch, Event *event);
    
public:
    EventHandler();
    ~EventHandler();
    static EventHandler* getInstance();
    EventListenerTouchOneByOne *cardsListener;
    
    Sprite *cardTouching;
    bool cardsMoving();
    
    //Friend classes that can access to the EventHandler private methods
    friend class HelloWorld;
    friend class UserInterface;
    friend class Game;
    friend class FacebookHandler;
    friend class PlayHandler;
    friend class AppleAuthHandler;
};

#endif
