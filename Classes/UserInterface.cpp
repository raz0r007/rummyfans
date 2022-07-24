#include "UserInterface.h"
#include "EventHandler.h"
#include "LanguageManager.h"
#include "NakamaServer.h"
#include "Game.h"
#include "Bot/Bot.h"
#include "audio/include/AudioEngine.h"

UserInterface* UserInterface::m_pInstance = 0;

/**
 @brief  Constructor
 @param  s: UI Scene
 @return -
*/

UserInterface::UserInterface(Scene *s)
{
    Director *director = Director::getInstance();
    
    //Scene
    scene = s;

    //Layout
    layoutUI = Layout::create();
    layoutUI->setName("layoutUI");
    layoutUI->setTouchEnabled(true);
    layoutUI->setSwallowTouches(true);
    
    if(s->getName() != "gameScene") //In gameScene, the EventListenerTouchOneByOne is the priority
        layoutUI->setContentSize(director->getWinSize());
    
    layoutUI->setAnchorPoint(Vec2(0, 0));
    layoutUI->setPosition(Vec2(0, 0));
    s->addChild(layoutUI, 2);
}


/**
 @brief    Get always the same instance of the object (Singleton)
 @param    s: Scene to obtain the UI Instance
 @return   UserInterface*
*/

UserInterface* UserInterface::getInstance(Scene *s)
{
    if(m_pInstance == NULL)
        m_pInstance = new UserInterface(s);
        
    return m_pInstance;
}


/**
 @brief    Destroy the UI Instance
 @param    -
 @return   -
*/

void UserInterface::destroyInstance()
{
    delete m_pInstance;
    m_pInstance = NULL;
}


/**
 @brief  Add background 100% X, Y
 @param  imagePath: Background image path
 @return -
*/

void UserInterface::addBackground(string imagePath)
{
    //We don't use safeArea for the background because it must be 100 % in x, y.
    Director *director = Director::getInstance();
    auto visibleSize = director->getVisibleSize();
    Vec2 origin = director->getVisibleOrigin();
    
    Sprite *bg = Sprite::create(imagePath);
    bg->setPosition(Vec2(origin.x + visibleSize.width/2, origin.y + visibleSize.height/2));
    bg->setScale(visibleSize.width / bg->getContentSize().width, visibleSize.height / bg->getContentSize().height); //100% x, y
    layoutUI->addChild(bg);
}


/**
 @brief  Add a new button to the UI
 @param  imagePath: Button image path
         position: Button position
         scale: Button scale
         onTouch: Function that must be called when a player touch the button
         effect: Button with effect (true/false)
         layout: Layout for add the button (by default is layoutUI)
 @return Button*
 */

Button* UserInterface::addButton(string imagePath, Vec2 position, float scale, function<void(void)> onTouch, bool effect, Layout *layout)
{
    Button *btn = Button::create();
    auto visibleSize = Director::getInstance()->getVisibleSize();

    btn->loadTextureNormal(imagePath);
    string fileName = imagePath.substr(imagePath.rfind("/")+1); //The buttons are in the folder ui/buttons/btnName...
    btn->setName(fileName.substr(0, fileName.find(".")));
    
    btn->setScale(visibleSize.width * scale / btn->getContentSize().width);
    btn->setPosition(position);
    btn->addTouchEventListener([=](Ref* sender, Widget::TouchEventType type) {
        switch(type)
        {
            case Widget::TouchEventType::BEGAN:
                onTouch();
                AudioEngine::play2d("sounds/uiSound.wav");
                    
                break;
            case Widget::TouchEventType::MOVED: break;
            case Widget::TouchEventType::ENDED: break;
            case Widget::TouchEventType::CANCELED: break;
        }
    });
    
    if(effect)
        btn->runAction(RepeatForever::create(Sequence::create(ScaleTo::create(btn->getScale(), btn->getScale()), ScaleBy::create(1, 1.05f), NULL)));
    
    (layout == NULL) ? layoutUI->addChild(btn) : layout->addChild(btn);
    
    return btn;
}


/**
 @brief  Add a new Label to the UI
 @param  text: Label text
         position: Label position
         size: Label size
         width: Width limit per line ( until a \r )
         useTTF: true = it uses the ttf font. False = it uses generic font
         layout: Layout for add the button (by default is layoutUI)
 @return Label*
*/

Label* UserInterface::addLabel(string text, Vec2 position, int size, float width, bool useTTF, Layout *layout)
{
    Label *l;
    
    if(useTTF)
        l = Label::createWithTTF(text, "fonts/Riffic-Bold.ttf", size);
    else
        l = Label::createWithSystemFont(text, "Arial", size);
    
    l->setWidth(width);
    l->setAlignment(TextHAlignment::CENTER);
    l->setColor(Color3B::WHITE);
    l->setPosition(position);
    
    (layout == NULL) ? layoutUI->addChild(l) : layout->addChild(l);
      
    return l;
}


/**
 @brief  Add a sprite in the layout UI
 @param  s: Node to add
         layout: Layout for add the node (by default is layoutUI)
 @return -
*/

void UserInterface::addSprite(Node *s, Layout *layout)
{
    if(layout == NULL)
        layoutUI->addChild(s);
    else
        layout->addChild(s);
}


/**
 @brief  Add a new input text
 @param  inputName: Name of the Node
         position: Position of the input
         maxLength: Max chars to type inside the input
         isPasswordInput: Bool that indicates if the chars should be hide in **** or not.
         layout: Layout for add the node (by default is layoutUI)
 @return EditBox*
*/

EditBox* UserInterface::addInput(string inputName, Vec2 position, int maxLength, bool isPasswordInput, Layout *layout)
{
    Director *director = Director::getInstance();
    auto visibleSize = director->getVisibleSize();
    
    EditBox* input = EditBox::create(Size(visibleSize.width * 0.25f, visibleSize.height * 0.09f), "ui/inputText.png");
    input->setName(inputName);
    input->setFontSize(int((visibleSize.height * 0.09f)/2));
    input->setPosition(position);
    input->setFontColor(Color3B::BLACK);
    input->setMaxLength(maxLength);
    input->setInputMode(EditBox::InputMode::SINGLE_LINE);
    input->setTextHorizontalAlignment(TextHAlignment::LEFT);
    input->setInputFlag(EditBox::InputFlag::INITIAL_CAPS_ALL_CHARACTERS);
    input->setReturnType(EditBox::KeyboardReturnType::DONE); //This line is VERY IMPORTANT! We are blocking \r using this.
    input->addTouchEventListener([=](Ref* sender, Widget::TouchEventType type) {
        switch(type)
        {
            case Widget::TouchEventType::BEGAN:
                if(layoutUI->getChildByName("layoutUIWindow") != NULL and layoutUI->getChildByName("layoutUIWindow")->getChildByName("labelInvalidEmail") != NULL)
                   layoutUI->getChildByName("layoutUIWindow")->getChildByName("labelInvalidEmail")->setVisible(false);
                
                break;
            case Widget::TouchEventType::MOVED: break;
            case Widget::TouchEventType::ENDED: break;
            case Widget::TouchEventType::CANCELED: break;
        }
    });
    
    if(isPasswordInput == true)
        input->setInputFlag(EditBox::InputFlag::PASSWORD);
    
    
    (layout == NULL) ? layoutUI->addChild(input) : layout->addChild(input);
 
    return input;
}


/**
 @brief  Add a new Window to the UI
 @param  wType: window type (enum wType)
         optionalText: Optional text for put in WINDOW_ERROR or WINDOW_RESULTS
 @return bool
*/

bool UserInterface::addWindow(unsigned int wType, string optionalText)
{
    string layoutWinName;
    if(layoutUI->getChildByName("layoutUIWindow") == NULL)
        layoutWinName = "layoutUIWindow";
    else if(layoutUI->getChildByName("layoutUIWindow2") == NULL)
    {
        layoutWinName = "layoutUIWindow2";
        
        //When we've a second window, we need to disable all buttons touch of the first window, because there're overlaping touch problems (cocos2d bug)
        //In the method 'removeCurrentWindow' we re-enable all buttons
        Vector<Node*> childs = layoutUI->getChildByName("layoutUIWindow")->getChildren();
        for(auto c : childs) {
            
            if(c->getName().find("btn") != string::npos)
            {
                Button *btn = (Button*)c;
                btn->setTouchEnabled(false);
            }
        }
        //--
    }
    else
        return false; //Max 2 windows at the same time
    
    //Vars
    Director *director = Director::getInstance();
    Rect safeArea = director->getSafeAreaRect();
    Size visibleSize = safeArea.size;
    Vec2 origin = safeArea.origin;
    auto winSize = director->getWinSize();
    
    //Window layout
    Layout *layoutWindow = Layout::create();
    layoutWindow->setName(layoutWinName);
    layoutWindow->setTouchEnabled(true);
    layoutWindow->setSwallowTouches(true);
    layoutWindow->setContentSize(winSize);
    layoutWindow->setAnchorPoint(Vec2(0, 0));
    layoutWindow->setPosition(Vec2(0, 0));
    
    if(wType != WINDOW_NEWMATCH) { //The window new match is a special window without black background
        layoutWindow->setBackGroundColor(Color3B::BLACK);
        layoutWindow->setBackGroundColorType(Layout::BackGroundColorType::SOLID);
        layoutWindow->setBackGroundColorOpacity(170);
    }
    
    layoutUI->addChild(layoutWindow, 99);
    
    //Window
    Sprite *w = Sprite::create("ui/window.png");
    w->setName("window");
    w->setTag(wType);
    float wScale = (wType == WINDOW_NEWMATCH) ? 0.75f : 0.65f;
    w->setScale(visibleSize.width * wScale / w->getContentSize().width);
    w->setPosition(Vec2(origin.x + visibleSize.width/2, origin.y + visibleSize.height + w->getBoundingBox().size.height/2));
    layoutWindow->addChild(w);
   
    //Content
    auto addContentWindow = CallFunc::create([=]() {
        
        string windowTitle;
        EventHandler *event = EventHandler::getInstance();
        
        switch(wType)
        {
            case WINDOW_NEWMATCH: {
                windowTitle = "titleNewMatch.png";
                
                //Home && logout buttons
                Button *btnHome = addButton("ui/btnHome.png", Vec2(origin.x, origin.y + visibleSize.height), 0.10f, [event]() { event->onTouchButtonHome(); }, false, layoutWindow);
                btnHome->setPosition(Vec2(origin.x + btnHome->getBoundingBox().size.width/2, origin.y + visibleSize.height - btnHome->getBoundingBox().size.height/2));
                
                Button *btnLogout = addButton("ui/btnLogout.png", Vec2(origin.x, origin.y + visibleSize.height), 0.10f, [event]() { event->onTouchButtonLogout(); }, false, layoutWindow);
                btnLogout->setPosition(Vec2(origin.x + visibleSize.width - btnLogout->getBoundingBox().size.width/2, origin.y + visibleSize.height - btnLogout->getBoundingBox().size.height/2));
                
                //Label finding..
                Label *findingOpp = addLabel(LanguageManager::getString("LABEL_FINDING_OPPONENT"), Vec2(w->getPositionX(), w->getPositionY() * 1.20f), 14, 0, true, layoutWindow);
                findingOpp->setName("labelFindingText");
              
                //Effect showing the last '...'
                auto hidePoints = CallFunc::create([findingOpp]() {
                
                    findingOpp->getLetter(findingOpp->getStringLength() - 1)->setVisible(false);
                    findingOpp->getLetter(findingOpp->getStringLength() - 2)->setVisible(false);
                    findingOpp->getLetter(findingOpp->getStringLength() - 3)->setVisible(false);
                });
                
                auto showPoint1 = CallFunc::create([findingOpp]() { findingOpp->getLetter(findingOpp->getStringLength() - 3)->setVisible(true); });
                auto showPoint2 = CallFunc::create([findingOpp]() { findingOpp->getLetter(findingOpp->getStringLength() - 2)->setVisible(true); });
                auto showPoint3 = CallFunc::create([findingOpp]() { findingOpp->getLetter(findingOpp->getStringLength() - 1)->setVisible(true); });

                findingOpp->runAction(RepeatForever::create(Sequence::create(hidePoints, DelayTime::create(0.5),
                                                                             showPoint1, DelayTime::create(0.5),
                                                                             showPoint2, DelayTime::create(0.5),
                                                                             showPoint3, DelayTime::create(0.5),
                                                                             NULL)));

                //Me
                Label *labelMe = addLabel("", Vec2(w->getPositionX() - w->getBoundingBox().size.width/4, findingOpp->getPositionY() - findingOpp->getBoundingBox().size.height * 1.3f), 12, 0, true, layoutWindow);
                labelMe->setName("labelMyName");
                
                Sprite *avatarBgMe = Sprite::create("others/avatarBackground.png");
                avatarBgMe->setName("avatarBgMe");
                avatarBgMe->setScale(visibleSize.width * 0.11f / avatarBgMe->getContentSize().width);
                avatarBgMe->setPosition(Vec2(labelMe->getPositionX(), labelMe->getPositionY() - avatarBgMe->getBoundingBox().size.height/1.3f));
                layoutWindow->addChild(avatarBgMe);
                
                //vs
                addLabel("vs", Vec2(w->getPositionX(), avatarBgMe->getPositionY()), 13, 0, true, layoutWindow);
                
                //Opponent
                Label *labelOpponent = addLabel("", Vec2(w->getPositionX() + w->getBoundingBox().size.width/4, findingOpp->getPositionY() - findingOpp->getBoundingBox().size.height * 1.3f), 12, 0, true, layoutWindow);
                labelOpponent->setName("labelOpponentName");
                
                Sprite *avatarBgOpp = Sprite::create("others/avatarBackground.png");
                avatarBgOpp->setName("avatarBgOpponent");
                avatarBgOpp->setScale(visibleSize.width * 0.11f / avatarBgOpp->getContentSize().width);
                avatarBgOpp->setPosition(Vec2(labelOpponent->getPositionX(), avatarBgMe->getPositionY()));
                layoutWindow->addChild(avatarBgOpp);
                
                //Loading avatar opponent...
                Sprite *loadingIcon = Sprite::createWithSpriteFrameName("loadingIcon01.png");
                loadingIcon->setName("loadingAvatar");
                loadingIcon->setPosition(avatarBgOpp->getPosition());
                loadingIcon->setScale(visibleSize.width * 0.03f / loadingIcon->getContentSize().width);
                loadingIcon->runAction(RepeatForever::create(Animate::create(AnimationCache::getInstance()->getAnimation("animationLoadingIcon"))));
                layoutWindow->addChild(loadingIcon);
                
                //Now I wait until I receive names/avatars:
                auto waitForPlayersInfo = [](float) {
                    
                    Director *director = Director::getInstance();
                    if(director->getRunningScene()->getName() != "matchScene")
                        return;
                    
                    static int timeWaitingForOpponent;
                    NakamaServer *nakama = NakamaServer::getInstance();
                    Game *game = Game::getInstance();
                    
                    Layout *layoutUI = (Layout*)director->getRunningScene()->getChildByName("layoutUI");
                    Layout *layoutWindow = (Layout*)layoutUI->getChildByName("layoutUIWindow");
                    Label *labelMe = (Label*)layoutWindow->getChildByName("labelMyName");
                    Label *labelOpponent = (Label*)layoutWindow->getChildByName("labelOpponentName");
                    Sprite *avatarBgMe = (Sprite*)layoutWindow->getChildByName("avatarBgMe");
                    Sprite *avatarBgOpp = (Sprite*)layoutWindow->getChildByName("avatarBgOpponent");

                    if(game->players[0]->name != "" and game->players[0]->avatarImg != NULL and layoutWindow->getChildByName("myAvatar") == NULL) //Me
                    {
                        //Label
                        labelMe->setString(game->players[0]->name);
                        
                        //Avatar
                        Texture2D *texture = new Texture2D();
                        texture->autorelease();
                        texture->initWithImage(game->players[0]->avatarImg);
                        
                        Sprite *myAvatar = Sprite::createWithTexture(texture);
                        myAvatar->setName("myAvatar");
                        myAvatar->setScale(avatarBgMe->getScale() - 0.075f);
                        myAvatar->setPosition(avatarBgMe->getPosition());
                        layoutWindow->addChild(myAvatar);
                        
                        //Ready for search opponents!
                        nakama->addMatchMaker();
                        timeWaitingForOpponent = 0;
                    }
                    
                    if(layoutWindow->getChildByName("myAvatar") != NULL) //If I already have my own info...
                    {
                        timeWaitingForOpponent++;
                        if((0.25 * timeWaitingForOpponent) >= 10.0f and nakama->matchMakerOpponentFound == false) //if I wait more than 10 seconds... I'm going to play with a Bot so I create a new Bot (0.25 is the time interval that the lanmda function waitForPlayersInfo is called
                        {
                            Bot *bot = Bot::getInstance();
                            bot->create(getRandomBotName(), "https://anstra.net/rummyfans/defaultAvatar.png");
                        }
                    }
                        
                    if(game->players[1]->name != "" and game->players[1]->avatarImg != NULL and layoutWindow->getChildByName("opponentAvatar") == NULL) //Opponent
                    {
                        //Label
                        labelOpponent->setString(game->players[1]->name);
                        
                        //Avatar
                        if(layoutWindow->getChildByName("loadingAvatar") != NULL)
                            layoutWindow->removeChildByName("loadingAvatar");
                        
                        Texture2D *texture = new Texture2D();
                        texture->autorelease();
                        texture->initWithImage(game->players[1]->avatarImg);
                        
                        Sprite *oppAvatar = Sprite::createWithTexture(texture);
                        oppAvatar->setName("opponentAvatar");
                        oppAvatar->setScale(avatarBgOpp->getScale() - 0.075f);
                        oppAvatar->setPosition(avatarBgOpp->getPosition());
                        layoutWindow->addChild(oppAvatar);
                        
                        if(Bot::getInstance()->isPlaying == true) //if I wait more than 10 seconds... I'm going to play with a Bot..
                        {
                            //First of all, I've to left from matchMaker (I'm not going to search opponents anymore)
                            nakama->removeCurrentMatchmaker();
                            nakama->disconnect();
                            nakama->logout();
                            
                            //Opponent found label:
                            UserInterface *ui = UserInterface::getInstance(director->getRunningScene());
                            Label *findingText = (Label*)layoutWindow->getChildByName("labelFindingText");
                            Vec2 textPos = findingText->getPosition();

                            findingText->stopAllActions();
                            layoutWindow->removeChild(findingText); //we remove it and we add a new label. We don't use setString() because there're bugs in this cocos method..
                            
                            Label *labelOppFound = ui->addLabel(LanguageManager::getString("LABEL_OPPONENT_FOUND"), textPos, 14, 0, true, layoutWindow);
                            labelOppFound->setName("labelOpponentFound");
                            
                            //Fill cards && start game
                            game->dealCards();
                            game->start();
                        }
                        else //else, i'm going to play with a human
                        {
                            nakama->joinMatchByToken(game->matchToken);
                        }
                    }
                    
                    if(game->players[0]->name != "" and game->players[0]->avatarImg != NULL and layoutWindow->getChildByName("myAvatar") != NULL and
                       game->players[1]->name != "" and game->players[1]->avatarImg != NULL and layoutWindow->getChildByName("opponentAvatar") != NULL)
                    {
                        auto scheduler = Director::getInstance()->getScheduler();
                        scheduler->unschedule("threadWaitingForPlayersInfo", Game::getInstance());
                    }
                };

                auto scheduler = Director::getInstance()->getScheduler();
                scheduler->unschedule("threadWaitingForPlayersInfo", Game::getInstance()); //for make sure that there's no similar threads already scheduled
                scheduler->schedule(waitForPlayersInfo, Game::getInstance(), 0.25f, CC_REPEAT_FOREVER, 0, false, "threadWaitingForPlayersInfo");
                //
                
                break;
            }
            case WINDOW_LOGIN: {
                windowTitle = "titleLogin.png";
                
                Label *loginMsg = addLabel(LanguageManager::getString("LABEL_LOGIN_MSG"), Vec2(w->getPositionX(), w->getPositionY() * 1.11f), 12, w->getBoundingBox().size.width/1.25f, true, layoutWindow);
                
                addButton("ui/btnFacebook.png", Vec2(loginMsg->getPositionX() - loginMsg->getBoundingBox().size.width/4, loginMsg->getPositionY() - loginMsg->getBoundingBox().size.height * 1.5f), 0.132f, [event]() { event->onTouchButtonLoginWithFacebook(); }, false, layoutWindow);

                string btnAccountName;
                #if !defined(__ANDROID__)
                    btnAccountName = "btnApple.png";
                #else
                    btnAccountName = "btnEmail.png"; //Google play coming soon...
                #endif
                
                addButton("ui/" + btnAccountName, Vec2(loginMsg->getPositionX() + loginMsg->getBoundingBox().size.width/4, loginMsg->getPositionY() - loginMsg->getBoundingBox().size.height * 1.5f), 0.13f, [event]() {
                    
                        #if !defined(__ANDROID__)
                            event->onTouchButtonLoginWithApple();
                        #else
                            event->onTouchButtonLoginWithEmail();
                            //event->onTouchButtonLoginWithGameCenterOrGoogle();
                        #endif
                    
                }, false, layoutWindow);
                
                break;
            }
            case WINDOW_LOGIN_EMAIL: {
                windowTitle = "titleLogin.png";
                
                Label *loginMsg = addLabel(LanguageManager::getString("LABEL_LOGIN_EMAIL_MSG"), Vec2(w->getPositionX() - w->getBoundingBox().size.width/4.5, w->getPositionY() * 1.11f), 12, 0, true, layoutWindow);
                
                EditBox *inputEmail = addInput("inputEmail", Vec2(loginMsg->getPositionX() + loginMsg->getBoundingBox().size.width/2, loginMsg->getPositionY()), 25, false, layoutWindow);
                inputEmail->setPositionX(inputEmail->getPositionX() + inputEmail->getBoundingBox().size.width/1.8f);
                
                Label *invalidEmail = addLabel("", Vec2(inputEmail->getPositionX() - inputEmail->getBoundingBox().size.width/2, inputEmail->getPositionY() - inputEmail->getBoundingBox().size.height), 10, 0, true, layoutWindow);
                invalidEmail->setName("labelInvalidEmail");
                invalidEmail->setAnchorPoint(Vec2(0, 0.5));
                invalidEmail->setColor(Color3B::RED);
                invalidEmail->setVisible(false);
                
                addButton("ui/btnOk.png", Vec2(w->getPositionX(), w->getPositionY() - w->getBoundingBox().size.height/4), 0.12f, [inputEmail, event]() {
                    event->onTouchButtonOkEmail(inputEmail);
                }, false, layoutWindow);
                
                break;
            }
            case WINDOW_LOGOUT: {
                windowTitle = "titleLogout.png";
                    
                addLabel(LanguageManager::getString("LABEL_LOGOUT_MSG"), Vec2(w->getPositionX(), w->getPositionY() * 1.11f), 12, w->getBoundingBox().size.width/1.5, true, layoutWindow);
                    
                addButton("ui/btnHome.png", Vec2(w->getPositionX(), w->getPositionY() - w->getBoundingBox().size.height/4.5f), 0.132f, [event]() { event->onTouchButtonHome(); /* home && logout nakama */ }, false, layoutWindow);
                    
                break;
            }
            case WINDOW_REMOVEADS: {
                windowTitle = "titleRemoveAds.png";
                
                Label *adsMsg = addLabel(LanguageManager::getString("LABEL_REMOVE_ADS"), Vec2(w->getPositionX(), w->getPositionY() * 1.11f), 12, w->getBoundingBox().size.width/1.5, true, layoutWindow);
                    
                addButton("ui/btnOk.png", Vec2(adsMsg->getPositionX() - adsMsg->getBoundingBox().size.width/4.0f, adsMsg->getPositionY() - adsMsg->getBoundingBox().size.height * 1.5f), 0.132f, [event]() { event->onTouchBuyItemNoAds(); }, false, layoutWindow);
                
                addButton("ui/btnRecover.png", Vec2(adsMsg->getPositionX() + adsMsg->getBoundingBox().size.width/4.0f, adsMsg->getPositionY() - adsMsg->getBoundingBox().size.height * 1.5f), 0.132f, [event]() { event->onTouchRestoreItemNoAds(); }, false, layoutWindow);
                
                break;
            }
            case WINDOW_RESULTS: {
                windowTitle = "titleResults.png";
                AudioEngine::play2d("sounds/uiSound.wav");

                Game *game = Game::getInstance();
                Label *lblWin = addLabel(optionalText + " " + LanguageManager::getString("LABEL_WIN_GAME"), Vec2(w->getPositionX(), w->getPositionY() * 1.20f), 12, 0, true, layoutWindow);
                
                Texture2D *myTexture = new Texture2D();
                myTexture->autorelease();
                myTexture->initWithImage(game->players[0]->avatarImg);
                Sprite *myAvatar = Sprite::createWithTexture(myTexture);
                myAvatar->setScale(visibleSize.width * 0.07f / myAvatar->getContentSize().width);
                myAvatar->setPosition(w->getPositionX() - w->getBoundingBox().size.width/4, w->getPositionY() - ((w->getBoundingBox().size.height/2) * 0.20f));
                addSprite(myAvatar, layoutWindow);
                
                Texture2D *oppTexture = new Texture2D();
                oppTexture->autorelease();
                oppTexture->initWithImage(game->players[1]->avatarImg);
                Sprite *oppAvatar = Sprite::createWithTexture(oppTexture);
                oppAvatar->setScale(visibleSize.width * 0.07f / oppAvatar->getContentSize().width);
                oppAvatar->setPosition(w->getPositionX() + w->getBoundingBox().size.width/4, w->getPositionY() - ((w->getBoundingBox().size.height/2) * 0.20f));
                addSprite(oppAvatar, layoutWindow);
                
                addLabel(game->players[0]->name, Vec2(myAvatar->getPositionX(), myAvatar->getPositionY() + myAvatar->getBoundingBox().size.height/1.2f), 10, 0, true, layoutWindow);
                addLabel(game->players[1]->name, Vec2(oppAvatar->getPositionX(), oppAvatar->getPositionY() + oppAvatar->getBoundingBox().size.height/1.2f), 10, 0, true, layoutWindow);
                Label *lp1points = addLabel(to_string(game->players[0]->points) + " points", Vec2(myAvatar->getPositionX(), myAvatar->getPositionY() - myAvatar->getBoundingBox().size.height/1.2f), 9, 0, true, layoutWindow);
                Label *lp2points = addLabel(to_string(game->players[1]->points) + " points", Vec2(oppAvatar->getPositionX(), oppAvatar->getPositionY() - oppAvatar->getBoundingBox().size.height/1.2f), 9, 0, true, layoutWindow);
                
                lp1points->setColor(Color3B::ORANGE);
                lp2points->setColor(Color3B::ORANGE);
                
                addButton("ui/btnHome.png", Vec2(lblWin->getPositionX(), lblWin->getPositionY() - ((w->getBoundingBox().size.height/2) * 0.90f)), 0.10f, [event]() { event->onTouchButtonHome(); }, false, layoutWindow);
                
                layoutUI->setGlobalZOrder(99); //bug fix: prioritized error window

                break;
            }
            case WINDOW_RULES: {
                windowTitle = "titleRules.png";
                
                auto scrollViewRules = ScrollView::create();
                Size scrollContentSize;
                scrollContentSize.setSize(w->getBoundingBox().size.width, w->getBoundingBox().size.height * 0.75f);
                scrollViewRules->setContentSize(scrollContentSize);
                scrollViewRules->setAnchorPoint(Vec2(0.5, 0.5));
                scrollViewRules->setPosition(w->getPosition());
                scrollViewRules->setInnerContainerSize(Size(scrollContentSize.width, scrollContentSize.height)); //we re-calculate the height below
                scrollViewRules->setScrollBarEnabled(true);
                scrollViewRules->setScrollBarColor(Color3B::ORANGE);
                scrollViewRules->setDirection(ScrollView::Direction::VERTICAL);
                //scrollViewRules->setBackGroundColor(Color3B::BLUE); //for debug
                //scrollViewRules->setBackGroundColorType(Layout::BackGroundColorType::SOLID); //for debug
                layoutWindow->addChild(scrollViewRules);
                
                //re-calculate innerCointainerSize height based on labels:
                Label *t1 = Label::createWithTTF(LanguageManager::getString("LABEL_RULES_TITLE_1"), "fonts/Riffic-Bold.ttf", 12);
                Label *t2 = Label::createWithTTF(LanguageManager::getString("LABEL_RULES_TITLE_2"), "fonts/Riffic-Bold.ttf", 12);
                Label *t3 = Label::createWithTTF(LanguageManager::getString("LABEL_RULES_TITLE_3"), "fonts/Riffic-Bold.ttf", 12);
                Label *t4 = Label::createWithTTF(LanguageManager::getString("LABEL_RULES_TITLE_4"), "fonts/Riffic-Bold.ttf", 12);
                Label *t5 = Label::createWithTTF(LanguageManager::getString("LABEL_RULES_TITLE_5"), "fonts/Riffic-Bold.ttf", 12);

                Label *d1 = Label::createWithSystemFont(LanguageManager::getString("LABEL_RULES_DESC_1"), "Arial", 10); d1->setWidth(scrollViewRules->getContentSize().width * 0.76f);
                Label *d2 = Label::createWithSystemFont(LanguageManager::getString("LABEL_RULES_DESC_2"), "Arial", 10); d2->setWidth(scrollViewRules->getContentSize().width * 0.76f);
                Label *d3 = Label::createWithSystemFont(LanguageManager::getString("LABEL_RULES_DESC_3"), "Arial", 10); d3->setWidth(scrollViewRules->getContentSize().width * 0.76f);
                Label *d4 = Label::createWithSystemFont(LanguageManager::getString("LABEL_RULES_DESC_4"), "Arial", 10); d4->setWidth(scrollViewRules->getContentSize().width * 0.76f);
                Label *d5 = Label::createWithSystemFont(LanguageManager::getString("LABEL_RULES_DESC_5"), "Arial", 10); d5->setWidth(scrollViewRules->getContentSize().width * 0.76f);
                
                scrollViewRules->setInnerContainerSize(Size(scrollContentSize.width, t1->getBoundingBox().size.height +
                                                                                     t2->getBoundingBox().size.height +
                                                                                     t3->getBoundingBox().size.height +
                                                                                     t4->getBoundingBox().size.height +
                                                                                     t5->getBoundingBox().size.height +
                                                                                     d1->getBoundingBox().size.height +
                                                                                     d2->getBoundingBox().size.height +
                                                                                     d3->getBoundingBox().size.height +
                                                                                     d4->getBoundingBox().size.height +
                                                                                     d5->getBoundingBox().size.height +
                                                                                     scrollContentSize.height * 0.95f //extra space
                                                        ));
                //
                
                //Deck
                Label *rulesTitle1 = addLabel(LanguageManager::getString("LABEL_RULES_TITLE_1"), Vec2(scrollViewRules->getInnerContainerSize().width * 0.10f, scrollViewRules->getInnerContainerSize().height * 0.98f), 12, 0, true, scrollViewRules);
                rulesTitle1->setAnchorPoint(Vec2(0, 0.5));
                rulesTitle1->setAlignment(TextHAlignment::LEFT);
                
                Label *rulesDesc1 = addLabel(LanguageManager::getString("LABEL_RULES_DESC_1"), Vec2(rulesTitle1->getPositionX() * 1.4f, rulesTitle1->getPositionY()), 10, scrollViewRules->getContentSize().width * 0.76f, false, scrollViewRules);
                rulesDesc1->setPositionY(rulesDesc1->getPositionY() - rulesDesc1->getBoundingBox().size.height/1.1f);
                rulesDesc1->setAnchorPoint(Vec2(0, 0.5));
                rulesDesc1->setAlignment(TextHAlignment::LEFT);
                
                //Objetive
                Label *rulesTitle2 = addLabel(LanguageManager::getString("LABEL_RULES_TITLE_2"), Vec2(rulesTitle1->getPositionX(), rulesDesc1->getPositionY() - rulesDesc1->getBoundingBox().size.height/2), 12, 0, true, scrollViewRules);
                rulesTitle2->setPositionY(rulesTitle2->getPositionY() - rulesTitle2->getBoundingBox().size.height);
                rulesTitle2->setAnchorPoint(Vec2(0, 0.5));
                rulesTitle2->setAlignment(TextHAlignment::LEFT);
                
                Label *rulesDesc2 = addLabel(LanguageManager::getString("LABEL_RULES_DESC_2"), Vec2(rulesDesc1->getPositionX(), rulesTitle2->getPositionY()), 10, scrollViewRules->getContentSize().width * 0.76f, false, scrollViewRules);
                rulesDesc2->setPositionY(rulesDesc2->getPositionY() - rulesDesc2->getBoundingBox().size.height/1.8f);
                rulesDesc2->setAnchorPoint(Vec2(0, 0.5));
                rulesDesc2->setAlignment(TextHAlignment::LEFT);
                
                //Dealing
                Label *rulesTitle3 = addLabel(LanguageManager::getString("LABEL_RULES_TITLE_3"), Vec2(rulesTitle1->getPositionX(), rulesDesc2->getPositionY() - rulesDesc2->getBoundingBox().size.height/2), 12, 0, true, scrollViewRules);
                rulesTitle3->setPositionY(rulesTitle3->getPositionY() - rulesTitle3->getBoundingBox().size.height);
                rulesTitle3->setAnchorPoint(Vec2(0, 0.5));
                rulesTitle3->setAlignment(TextHAlignment::LEFT);
                
                Label *rulesDesc3 = addLabel(LanguageManager::getString("LABEL_RULES_DESC_3"), Vec2(rulesDesc1->getPositionX(), rulesTitle3->getPositionY()), 10, scrollViewRules->getContentSize().width * 0.76f, false, scrollViewRules);
                rulesDesc3->setPositionY(rulesDesc3->getPositionY() - rulesDesc3->getBoundingBox().size.height/1.25f);
                rulesDesc3->setAnchorPoint(Vec2(0, 0.5));
                rulesDesc3->setAlignment(TextHAlignment::LEFT);
                
                //Playing
                Label *rulesTitle4 = addLabel(LanguageManager::getString("LABEL_RULES_TITLE_4"), Vec2(rulesTitle1->getPositionX(), rulesDesc3->getPositionY() - rulesDesc3->getBoundingBox().size.height/2), 12, 0, true, scrollViewRules);
                rulesTitle4->setPositionY(rulesTitle4->getPositionY() - rulesTitle4->getBoundingBox().size.height);
                rulesTitle4->setAnchorPoint(Vec2(0, 0.5));
                rulesTitle4->setAlignment(TextHAlignment::LEFT);
                
                Label *rulesDesc4 = addLabel(LanguageManager::getString("LABEL_RULES_DESC_4"), Vec2(rulesDesc1->getPositionX(), rulesTitle4->getPositionY()), 10, scrollViewRules->getContentSize().width * 0.76f, false, scrollViewRules);
                rulesDesc4->setPositionY(rulesDesc4->getPositionY() - rulesDesc4->getBoundingBox().size.height/1.75f);
                rulesDesc4->setAnchorPoint(Vec2(0, 0.5));
                rulesDesc4->setAlignment(TextHAlignment::LEFT);
                
                //Scoring
                Label *rulesTitle5 = addLabel(LanguageManager::getString("LABEL_RULES_TITLE_5"), Vec2(rulesTitle1->getPositionX(), rulesDesc4->getPositionY() - rulesDesc4->getBoundingBox().size.height/2), 12, 0, true, scrollViewRules);
                rulesTitle5->setPositionY(rulesTitle5->getPositionY() - rulesTitle5->getBoundingBox().size.height);
                rulesTitle5->setAnchorPoint(Vec2(0, 0.5));
                rulesTitle5->setAlignment(TextHAlignment::LEFT);
                
                Label *rulesDesc5 = addLabel(LanguageManager::getString("LABEL_RULES_DESC_5"), Vec2(rulesDesc1->getPositionX(), rulesTitle5->getPositionY()), 10, scrollViewRules->getContentSize().width * 0.76f, false, scrollViewRules);
                rulesDesc5->setPositionY(rulesDesc5->getPositionY() - rulesDesc5->getBoundingBox().size.height/1.75f);
                rulesDesc5->setAnchorPoint(Vec2(0, 0.5));
                rulesDesc5->setAlignment(TextHAlignment::LEFT);

                break;
            }
            case WINDOW_FINISH: {
                windowTitle = "titleFinish.png";
                AudioEngine::play2d("sounds/uiSound.wav");

                Label *loginMsg = addLabel(LanguageManager::getString("LABEL_ERROR_OPPONENT_LEAVE"), Vec2(w->getPositionX(), w->getPositionY() * 1.11f), 12, w->getBoundingBox().size.width/1.2f, true, layoutWindow);
                
                addButton("ui/btnHome.png", Vec2(loginMsg->getPositionX(), loginMsg->getPositionY() - loginMsg->getBoundingBox().size.height * 1.6f), 0.132f, [event]() { event->onTouchButtonHome(); /* home && logout nakama */ }, false, layoutWindow);
                
                layoutUI->setGlobalZOrder(99); //bug fix: prioritized error window
                
                break;
            }
            case WINDOW_ERROR: {
                windowTitle = "titleError.png";
                AudioEngine::play2d("sounds/uiSound.wav");

                Label *loginMsg = addLabel(optionalText, Vec2(w->getPositionX(), w->getPositionY() * 1.11f), 12, w->getBoundingBox().size.width/1.2f, true, layoutWindow);
                
                addButton("ui/btnHome.png", Vec2(loginMsg->getPositionX(), loginMsg->getPositionY() - loginMsg->getBoundingBox().size.height * 1.6f), 0.132f, [event]() { event->onTouchButtonHome(); /* home && logout nakama */ }, false, layoutWindow);
                
                layoutUI->setGlobalZOrder(99); //bug fix: prioritized error window
                
                break;
            }
            default:
                return false;
                break;
        }
        
        //Title
        Sprite *t = Sprite::create("ui/" + windowTitle);
        float tScale = (wType == WINDOW_NEWMATCH) ? 0.36f : 0.30f;
        t->setPosition(w->getPositionX(), w->getPositionY() + w->getBoundingBox().size.height/2.15);
        t->setName(windowTitle.substr(0, windowTitle.find(".")));
        t->setScale(visibleSize.width * tScale / t->getContentSize().width);
        layoutWindow->addChild(t);
            
        //Close button
        if(wType != WINDOW_NEWMATCH and wType != WINDOW_ERROR and wType != WINDOW_LOGOUT and wType != WINDOW_RESULTS and wType != WINDOW_FINISH) {
            
            addButton("ui/btnClose.png", Vec2(w->getPositionX() + w->getBoundingBox().size.width/2.1, w->getPositionY() + w->getBoundingBox().size.height/2), 0.05f, [event]() { event->onTouchCloseWindow(); }, false, layoutWindow);
        }
        
        return true;
    });
    

    //Effect
    auto actionWindow = CallFunc::create([=]() {
        
        w->runAction(Sequence::create(MoveTo::create(0.15, Vec2(origin.x + visibleSize.width/2, origin.y + visibleSize.height/2 - (visibleSize.height * 0.10f))), MoveBy::create(0.15, Vec2(0, visibleSize.height * 0.10f)), NULL));
    });
    
    if(wType != WINDOW_NEWMATCH)
    {
        layoutWindow->runAction(Sequence::create(actionWindow, DelayTime::create(0.35), addContentWindow, NULL));
    }
    else
    {
        auto setWindowPos = CallFunc::create([=]() { w->setPosition(origin.x + visibleSize.width/2, origin.y + visibleSize.height/2); });
        layoutWindow->runAction(Sequence::create(setWindowPos, addContentWindow, NULL));
    }
    
    return true;
}


/**
 @brief  Remove the current opened window
 @param  -
 @return bool
*/

bool UserInterface::removeCurrentWindow()
{
    if(layoutUI->getChildByName("layoutUIWindow") == NULL)
        return false;
   
    if(layoutUI->getChildByName("layoutUIWindow2") != NULL)
    {
        //First of all, We need to re-enable all buttons of the first window
        Vector<Node*> childs = layoutUI->getChildByName("layoutUIWindow")->getChildren();
        for(auto c : childs) {
            if(c->getName().find("btn") != string::npos)
            {
                Button *btn = (Button*)c;
                btn->setTouchEnabled(true);
            }
        }
        //--
        
        layoutUI->removeChildByName("layoutUIWindow2");
    }
    else
        layoutUI->removeChildByName("layoutUIWindow");
    
    return true;
}

/**
 @brief  Get the current window open. Note: this method could be improved adding 'layoutUIWindow2'
 @param  -
 @return int
*/

int UserInterface::getCurrentWindowOpen()
{
    auto layoutWindow = layoutUI->getChildByName("layoutUIWindow");
    if(layoutWindow == NULL)
        return -1;
    
    if(layoutWindow->getChildByName("titleError") != NULL)
        return WINDOW_ERROR;
    else if(layoutWindow->getChildByName("titleLogin") != NULL)
        return WINDOW_LOGIN;
    else if(layoutWindow->getChildByName("titleLogout") != NULL)
        return WINDOW_LOGOUT;
    else if(layoutWindow->getChildByName("titleNewMatch") != NULL)
        return WINDOW_NEWMATCH;
    else if(layoutWindow->getChildByName("titleRemoveAds") != NULL)
        return WINDOW_REMOVEADS;
    else if(layoutWindow->getChildByName("titleResults") != NULL)
        return WINDOW_RESULTS;
    else if(layoutWindow->getChildByName("titleRules") != NULL)
        return WINDOW_RULES;
    
    return -2;
}


/**
 @brief  Quick error messages on game play
 @param  eType: Type of error to show
 @return bool
*/

bool UserInterface::showQuickError(unsigned int eType)
{
    Director *director = Director::getInstance();
    string errorImage;
    
    if(director->getRunningScene()->getName() != "gameScene")
        return false;
    
    //Error image
    switch(eType)
    {
        case ERROR_TURN:
            errorImage = "errorTurn.png";
            break;
        case ERROR_TAKE_CARD:
            errorImage = "errorTakeCard.png";
            break;
        case ERROR_LEAVE_CARD:
            errorImage = "errorLeaveCard.png";
            break;
        default:
            return false;
            break;
    }
    
    //Show error
    Size visibleSize = director->getVisibleSize();
    Vec2 origin = director->getVisibleOrigin();
    
    Sprite *turnError = Sprite::create("others/" + errorImage);
    turnError->setName("errorMsg");
    turnError->setScale(visibleSize.width * 0.50f / turnError->getContentSize().width);
    turnError->setPosition(origin.x + visibleSize.width/2, origin.y + visibleSize.height + turnError->getBoundingBox().size.height/2);
    addSprite(turnError);
    
    auto vibrateCel = CallFunc::create([]() { Device::vibrate(0.5); });
    auto removeError = CallFunc::create([]() {
        Game *game = Game::getInstance();
        game->scene->getChildByName("layoutUI")->removeChildByName("errorMsg");
    });
    
    turnError->runAction(Sequence::create(MoveTo::create(0.5, Vec2(turnError->getPositionX(), turnError->getPositionY() - turnError->getBoundingBox().size.height)), vibrateCel, DelayTime::create(1), MoveTo::create(0.5, Vec2(turnError->getPositionX(), turnError->getPositionY() + turnError->getBoundingBox().size.height)), removeError, NULL));
    
    return true;
}


/**
 @brief  Destructor
 @param  -
 @return -
*/

UserInterface::~UserInterface()
{
}
