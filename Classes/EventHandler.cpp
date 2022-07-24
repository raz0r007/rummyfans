#include "EventHandler.h"
#include "Game.h"
#include "NakamaServer.h"
#include "ThirdPartys/SQLite/SQLite.h"
#include "HelloWorldScene.h"
#include "LanguageManager.h"
#include "Bot/Bot.h"
#include <regex>
#include "audio/include/AudioEngine.h"

/**
 @brief  Constructor
 @param  -
 @return -
*/

EventHandler::EventHandler()
{
    cardTouching = NULL;
}


/**
 @brief    Get always the same instance of the object (Singleton)
 @param    -
 @return   EventHandler*
*/

EventHandler* EventHandler::getInstance() {
    static EventHandler sharedInstance;
    return &sharedInstance;
}


/**
 @brief  Button Play Action
 @param  -
 @return -
*/

void EventHandler::onTouchButtonPlay()
{
    Director *director = Director::getInstance();
    SQLite *sqlite = SQLite::getInstance();
    Game *game = Game::getInstance();
    
    if(sqlite->getLoginMethod().empty()) //If i'm not logged in into Nakama Server
    {
        //UI
        UserInterface *ui = UserInterface::getInstance(director->getRunningScene());
        ui->addWindow(WINDOW_LOGIN);
    }
    else
        game->findOpponent(); //Go to the window WINDOW_NEWMATCH (find opponent)
}


/**
 @brief  Button Continue, for continue playing the next game round
 @param  -
 @return -
*/

void EventHandler::onTouchButtonContinue()
{
    Game *game = Game::getInstance();
    UserInterface *ui = UserInterface::getInstance(game->scene);
    
    //The button should be touched only 1 time, after that we disable it:
    Button *btnContinue = (Button*)game->scene->getChildByName("layoutUI")->getChildByName("btnContinue");
    btnContinue->setEnabled(false);
    
    //Label waiting
    Label *lblWaiting = ui->addLabel(LanguageManager::getString("LABEL_WAITING_NEXT_ROUND"), Vec2(btnContinue->getPositionX(), game->scene->getChildByName("opponentAvatar")->getPositionY()), 9, 0);
    
    //Effect showing the last '...'
    auto hidePoints = CallFunc::create([lblWaiting]() {
    
        lblWaiting->getLetter(lblWaiting->getStringLength() - 1)->setVisible(false);
        lblWaiting->getLetter(lblWaiting->getStringLength() - 2)->setVisible(false);
        lblWaiting->getLetter(lblWaiting->getStringLength() - 3)->setVisible(false);
    });
    
    auto showPoint1 = CallFunc::create([lblWaiting]() { lblWaiting->getLetter(lblWaiting->getStringLength() - 3)->setVisible(true); });
    auto showPoint2 = CallFunc::create([lblWaiting]() { lblWaiting->getLetter(lblWaiting->getStringLength() - 2)->setVisible(true); });
    auto showPoint3 = CallFunc::create([lblWaiting]() { lblWaiting->getLetter(lblWaiting->getStringLength() - 1)->setVisible(true); });

    lblWaiting->runAction(RepeatForever::create(Sequence::create(hidePoints, DelayTime::create(0.5),
                                                                 showPoint1, DelayTime::create(0.5),
                                                                 showPoint2, DelayTime::create(0.5),
                                                                 showPoint3, DelayTime::create(0.5),
                                                                 NULL)));
    //
    
    //Reset info for the next round:
    Player *p1 = game->players[0];
    Player *p2 = game->players[1];
    
    p1->myCards.clear();
    p2->myCards.clear();
    p1->readyForPlay = false;
    p1->myTurn = false;
    p2->readyForPlay = false;
    p2->myTurn = false;
    
    while(!game->deckOfCards.empty())
        game->deckOfCards.pop();
    
    while(!game->cardsSeen.empty())
        game->cardsSeen.pop();
        
    game->players[0] = p1;
    game->players[1] = p2;
    //
    
    //Leave && re-join to a new match:
    if(!Bot::getInstance()->isPlaying)
    {
        NakamaServer *nakama = NakamaServer::getInstance();
        nakama->leaveMatch(game->matchId);
        nakama->addMatchMaker("+properties.nextRoundMatch:" + game->matchId);
    }
    else
    {
        auto startNewGame = CallFunc::create([]() {
            Game *game = Game::getInstance();
            game->players[1]->myTurn = true; //because of we're not joining to a nakama match, we can't determinate who joined first, so the bot start playing...
            game->players[0]->myTurn = false;
     
            game->dealCards();
            game->start();
        });
        
        auto scheduleBotPlay = CallFunc::create([]() {
            auto scheduler = Director::getInstance()->getScheduler();
            scheduler->unschedule(SEL_SCHEDULE(&Bot::tryToPlay), Bot::getInstance()); //for make sure that there's no similar threads already scheduled
            scheduler->schedule(SEL_SCHEDULE(&Bot::tryToPlay), Bot::getInstance(), 0.18f, false);
        });
        

        game->scene->runAction(Sequence::create(startNewGame, DelayTime::create(3), scheduleBotPlay, NULL));
    }
}


/**
 @brief  Button Login E-Mail
 @param  -
 @return -
*/

void EventHandler::onTouchButtonLoginWithEmail()
{
    Director *director = Director::getInstance();
    UserInterface *ui = UserInterface::getInstance(director->getRunningScene());
    
    ui->removeCurrentWindow();
    ui->addWindow(WINDOW_LOGIN_EMAIL);
}


/**
 @brief  Button OK E-Mail register
 @param  inputText: email entered
 @return bool
*/

bool EventHandler::onTouchButtonOkEmail(EditBox *inputText)
{
    Director *director = Director::getInstance();
    SQLite *sqlite = SQLite::getInstance();
    bool emailValid = std::regex_match(inputText->getText(), std::regex("(\\w+)(\\.|_)?(\\w*)@(\\w+)(\\.(\\w+))+"));
    Label *lblValidation = (Label*)director->getRunningScene()->getChildByName("layoutUI")->getChildByName("layoutUIWindow")->getChildByName("labelInvalidEmail");
    
    
    if(!emailValid) //Invalid e-mail
    {
        lblValidation->setString(LanguageManager::getString("LABEL_LOGIN_EMAIL_INVALID"));
        lblValidation->setVisible(true);
        Device::vibrate(0.5);
        return false;
    }
    else if(strlen(inputText->getText()) < 10) //Nakama allows emails between 10 and 255 chars
    {
        lblValidation->setString(LanguageManager::getString("LABEL_LOGIN_EMAIL_SHORT"));
        lblValidation->setVisible(true);
        Device::vibrate(0.5);
        return false;
    }
    else //Valid e-mail
    {
        string email = inputText->getText();
        
        //Save all data in sqlite:
        sqlite->setLoginMethod("Mail");
        sqlite->setNakamaUser(email);
        
        /*
        //Generate random password (length = 10)
        auto randchar = []() -> char
        {
            const char charset[] =
            "0123456789"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz";
            const size_t max_index = (sizeof(charset) - 1);
            return charset[ rand() % max_index ];
        };
        std::string randomPassword(10, 0);
        std::generate_n(randomPassword.begin(), 10, randchar);
        */
        
        string randomPassword = "aHy6N9kF4A"; //Bug fix: we've to use the same password for all e-mail users, due to problems related to re-login with the same email (invalid credentials)
        sqlite->setNakamaPassword(randomPassword);
        
        //Ready for play!
        onTouchButtonPlay();
    }
    
    return true;
}


/**
 @brief  Button Login GameCenter or Google
 @param  -
 @return -
*/

void EventHandler::onTouchButtonLoginWithGameCenterOrGoogle()
{
    if(sdkbox::PluginSdkboxPlay::isSignedIn())
        sdkbox::PluginSdkboxPlay::signout();
    
    sdkbox::PluginSdkboxPlay::signin();
}


/**
 @brief  Button Login Apple
 @param  -
 @return -
*/

void EventHandler::onTouchButtonLoginWithApple()
{
    sdkbox::PluginSignInWithApple::sign();
}


/**
 @brief  Button Login FB
 @param  -
 @return -
*/

void EventHandler::onTouchButtonLoginWithFacebook()
{
    if(sdkbox::PluginFacebook::isLoggedIn())
        sdkbox::PluginFacebook::logout();

    sdkbox::PluginFacebook::login();
}


/**
 @brief  Button Home
 @param  -
 @return -
*/

void EventHandler::onTouchButtonHome()
{
    Director *director = Director::getInstance();
    UserInterface *ui = UserInterface::getInstance(director->getRunningScene());
    
    if(director->getRunningScene()->getName() == "initScene")
    {
        ui->removeCurrentWindow();
        
        Button *btnPlay = (Button*)director->getRunningScene()->getChildByName("layoutUI")->getChildByName("btnPlay");
        btnPlay->setTouchEnabled(true); //we need to re-enable this button because I've disabled it on FacebookHandler::onLogin
    }
    else
    {
        HelloWorld h;
        h.loadInitialMenu(); //inside this method, we destroy gameInstance
        
        NakamaServer *nakama = NakamaServer::getInstance();
        nakama->disconnect();
        nakama->logout();
        
        if(Bot::getInstance()->isPlaying)
        {
            auto scheduler = director->getScheduler();
            scheduler->unscheduleAllForTarget(Bot::getInstance());
            Bot::destroyInstance();
        }
    }
}


/**
 @brief  Button Logout
 @param  -
 @return -
*/

void EventHandler::onTouchButtonLogout()
{
    Director *director = Director::getInstance();
    SQLite *sqlite = SQLite::getInstance();
    
    if(sqlite->getLoginMethod() == "Facebook")
        sdkbox::PluginFacebook::logout();
    else if(sqlite->getLoginMethod() == "GameCenter" or sqlite->getLoginMethod() == "Google")
        sdkbox::PluginSdkboxPlay::signout();
    
    //Clean my internal login data
    sqlite->setLoginMethod("");
    sqlite->setNakamaUser("");
    sqlite->setNakamaPassword("");
    
    //UI
    UserInterface *ui = UserInterface::getInstance(director->getRunningScene());
    ui->addWindow(WINDOW_LOGOUT);

    //Bug fix: if I logout I need to unschedule mainly 'waitForPlayersInfo'
    auto scheduler = Director::getInstance()->getScheduler();
    scheduler->unscheduleAllForTarget(Game::getInstance());
}


/**
 @brief  Button Rules
 @param  -
 @return -
*/

void EventHandler::onTouchButtonRules()
{
    Director *director = Director::getInstance();
    UserInterface *ui = UserInterface::getInstance(director->getRunningScene());
    
    ui->addWindow(WINDOW_RULES);
}


/**
 @brief  Button No Ads
 @param  -
 @return -
*/

void EventHandler::onTouchButtonNoAds()
{
    Director *director = Director::getInstance();
    UserInterface *ui = UserInterface::getInstance(director->getRunningScene());
    
    ui->addWindow(WINDOW_REMOVEADS);
}


/**
 @brief  Button Buy Item
 @param  -
 @return -
*/

void EventHandler::onTouchBuyItemNoAds()
{
    sdkbox::IAP::purchase("remove_ads");
}


/**
 @brief  Button Restore Item
 @param  -
 @return -
*/

void EventHandler::onTouchRestoreItemNoAds()
{
    sdkbox::IAP::restore();
    onTouchCloseWindow();
}


/**
 @brief  Button Close window
 @param  -
 @return bool
*/

bool EventHandler::onTouchCloseWindow()
{
    Director *director = Director::getInstance();
    UserInterface *ui = UserInterface::getInstance(director->getRunningScene());
    
    return ui->removeCurrentWindow();
}


/**
 @brief  Detect if any of the cards I can touch (this is: last cardSeen or my cards) are in movement (MoveTo in process)
         Action Tags => | 0: My Actions | 1: Opponent Actions |
 @param  -
 @return bool
*/

bool EventHandler::cardsMoving()
{
    Game *game = Game::getInstance();
    auto layoutMyCards = game->scene->getChildByName("layoutMyCards");
    vector<card> myCards = game->players[0]->myCards;
    
    if(game->cardsSeen.size() > 0 and game->scene->getChildByName(game->cardsSeen.top().imgName)->getNumberOfRunningActionsByTag(0) > 0)
        return true;

    bool cardMoving = false;
    for(int i = 0; i < myCards.size() and cardMoving == false; i++)
    {
        if(layoutMyCards->getChildByName(myCards[i].imgName)->getNumberOfRunningActionsByTag(0) > 0)
            cardMoving = true;
    }
    
    return cardMoving;
}


/**
 @brief  Callback when I touch a card (begin)
 @param  touch: touch Location
         event: event
 @return bool
*/

bool EventHandler::onTouchCardBegin(Touch *touch, Event *event)
{
    Game *game = Game::getInstance();
    Director *director = Director::getInstance();
    Rect safeArea = director->getSafeAreaRect();
    Size visibleSize = safeArea.size;

    if(cardTouching != NULL or cardsMoving())
        return false;
    
    //Possibles sprites where I could touched:
    Sprite *cardStack = (Sprite*)game->scene->getChildByName("cardStack");
    Sprite *cardSeen = (Sprite*)game->scene->getChildByName(game->cardsSeen.size() > 0 ? game->cardsSeen.top().imgName : "cardBackground");
    Sprite *myCards = (Sprite*)game->scene->getChildByName("layoutMyCards");
    
    Rect cardStackRect = Rect(0, 0, cardStack->getContentSize().width, cardStack->getContentSize().height);
    Rect cardSeenRect = Rect(0, 0, cardSeen->getContentSize().width, cardSeen->getContentSize().height);
    Rect myCardsRect = Rect(0, 0, myCards->getContentSize().width, myCards->getContentSize().height);
  
    //Now, we need to find out what sprite I touched
    if(cardStackRect.containsPoint(cardStack->convertToNodeSpace(touch->getLocation()))) //If I touch cardStack...
    {
        cardTouching = cardStack;
        return true;
    }
    else if(game->cardsSeen.size() > 0 and cardSeenRect.containsPoint(cardSeen->convertToNodeSpace(touch->getLocation()))) //If I touch cardSeen...
    {
        if(game->players[0]->myTurn == false)
        {
            UserInterface *ui = UserInterface::getInstance(game->scene);
            ui->showQuickError(ERROR_TURN);
            return false;
        }
        
        AudioEngine::play2d("sounds/cardTouch.wav");
        cardTouching = cardSeen;
        return true;
    }
    else if(myCardsRect.containsPoint(myCards->convertToNodeSpace(touch->getLocation()))) //If I touch one of my Cards...
    {
        AudioEngine::play2d("sounds/cardTouch.wav");

        //If I touched myCards Layer, we need to find out what's the card I touched:
        for(int i = 0; i < game->players[0]->myCards.size() and cardTouching == NULL; i++)
        {
            Sprite *myCard = (Sprite*)game->scene->getChildByName("layoutMyCards")->getChildByName(game->players[0]->myCards[i].imgName);
            Rect myCardRect = Rect(0, 0, myCard->getContentSize().width, myCard->getContentSize().height);
            
            if(myCardRect.containsPoint(myCard->convertToNodeSpace(touch->getLocation())))
                cardTouching = myCard;
        }
        
        /*
          Also, we need to detect if the card I touched is a card that I can use to finish the game. Modes to finish a game:
             - KNOCK: I've all cards combinated/in-groups except for a list of cards which sum of points are <= 10.
             - GIN: I've all cards combinated/in-groups
          
         Always we finish the game with the card number 11
        */
        
        if(game->players[0]->myCards.size() == 11)
        {
            vector<card> cardsCombinated;
            vector<card> cardsNoCombinated;
            card *cardToAvoid = NULL;
            
            //Avoid cardTouching (combinations)
            for(int i = 0; i < game->players[0]->myCards.size() and cardToAvoid == NULL; i++)
            {
                if (game->players[0]->myCards[i].imgName == cardTouching->getName()) {
                    cardToAvoid = &game->players[0]->myCards[i];
                }
            }
            
            game->players[0]->markCombinationsIfExist(true, cardToAvoid);
            //
            
            for(auto card : game->players[0]->myCards)
            {
                if(card.imgName == cardTouching->getName())
                    continue; //cardTouching doesn't matter because it's the card that I use to finish the game
                
                if(game->scene->getChildByName("layoutMyCards")->getChildByName(card.imgName)->getChildByName("circleCardComb") != NULL)
                    cardsCombinated.push_back(card);
                else
                    cardsNoCombinated.push_back(card);
            }
            
            if(cardsCombinated.size() == 10) //Gin
            {
                Sprite *cardBg = Sprite::create("cards/cardGin.png");
                cardBg->setName("cardFinishBackground");
                cardBg->setTag(0); //gin code
                cardBg->setScale(visibleSize.width * 0.105f / cardBg->getContentSize().width);
                cardBg->setPosition(game->scene->getChildByName("cardBackground")->getPositionX() - game->scene->getChildByName("cardBackground")->getBoundingBox().size.width * 1.05f, game->scene->getChildByName("cardBackground")->getPositionY());
                game->scene->addChild(cardBg);
            }
            else
            {
                int sumPointsNoComb = 0;
                for(auto card : cardsNoCombinated)
                    sumPointsNoComb += card.points;
                
                if(sumPointsNoComb <= 10) //Knock
                {
                    Sprite *cardBg = Sprite::create("cards/cardKnock.png");
                    cardBg->setName("cardFinishBackground");
                    cardBg->setTag(1); //knock code
                    cardBg->setScale(visibleSize.width * 0.105f / cardBg->getContentSize().width);
                    cardBg->setPosition(game->scene->getChildByName("cardBackground")->getPositionX() - game->scene->getChildByName("cardBackground")->getBoundingBox().size.width * 1.05f, game->scene->getChildByName("cardBackground")->getPositionY());
                    game->scene->addChild(cardBg);
                }
            }
        }
        
        return true;
    }

    return false;
}


/**
 @brief  Callback when I touch a card (moved)
 @param  touch: touch Location
         event: event
 @return bool
*/

bool EventHandler::onTouchCardMoved(Touch *touch, Event *event)
{
    Game *game = Game::getInstance();
    
    if(cardTouching == NULL or cardsMoving())
        return false;
    
    //drag & drop cards
    if(game->cardsSeen.size() > 0 and cardTouching->getName() == game->cardsSeen.top().imgName) //Last card seen
    {
        cardTouching->setGlobalZOrder(99); //prioritized card
        cardTouching->setPosition(touch->getLocation());
    }
    else if(cardTouching->getName() != "cardStack") //My cards
    {
        cardTouching->setGlobalZOrder(99); //prioritized card
        cardTouching->setPosition(cardTouching->getParent()->convertToNodeSpace(Vec2(touch->getLocation().x - cardTouching->getBoundingBox().size.width/2, touch->getLocation().y - cardTouching->getBoundingBox().size.height/2))); //position adapted to anchor point [0,0]
        
        //If cardTouching is intersecting with cardGin or cardKnock Backgrounds for finish the game... I iluminate the background
        if(game->scene->getChildByName("cardFinishBackground") != NULL)
        {
            Rect cardRect = cardTouching->getBoundingBox();
            Rect cardBgRect = game->scene->getChildByName("cardFinishBackground")->getBoundingBox();
            cardRect.origin = cardTouching->getParent()->convertToWorldSpace(cardRect.origin);
            cardBgRect.origin = game->scene->getChildByName("cardFinishBackground")->getParent()->convertToWorldSpace(cardBgRect.origin);
            
            if(cardRect.intersectsRect(cardBgRect))
                game->scene->getChildByName("cardFinishBackground")->setOpacity(170);
            else
                game->scene->getChildByName("cardFinishBackground")->setOpacity(255);
        }
    }
    
    return true;
}


/**
 @brief  Callback when I touch a card (ended)
 @param  touch: touch Location
         event: event
 @return bool
*/

bool EventHandler::onTouchCardEnd(Touch *touch, Event *event)
{
    Game *game = Game::getInstance();
    Bot *bot = Bot::getInstance();
    Director *director = Director::getInstance();
    auto scheduler = director->getScheduler();
  
    if(cardTouching == NULL or cardsMoving())
        return false;
    
    NakamaServer *nakama = NakamaServer::getInstance();
    Rect safeArea = director->getSafeAreaRect();
    Size visibleSize = safeArea.size;
    map<string, int> cards;
    
    if(game->cardsSeen.size() > 0)
        cards.insert(pair<string, int>(game->cardsSeen.top().imgName, -2));
    
    cards.insert(pair<string, int>("cardStack", -1));
    for(int i = 0; i < game->players[0]->myCards.size(); i++)
        cards.insert(pair<string, int>(game->players[0]->myCards[i].imgName, i));
    
    switch(cards.at(cardTouching->getName()))
    {
        case -2: //Top Card Seen
        {
            Rect cardSeenTopRect = cardTouching->getBoundingBox();
            Rect layoutMyCardsRect = game->scene->getChildByName("layoutMyCards")->getBoundingBox();
            cardSeenTopRect.origin = cardTouching->getParent()->convertToWorldSpace(cardSeenTopRect.origin);
            layoutMyCardsRect.origin = game->scene->getChildByName("layoutMyCards")->getParent()->convertToWorldSpace(layoutMyCardsRect.origin);
            
            if(cardSeenTopRect.intersectsRect(layoutMyCardsRect) //If the card is added to my list of cards..
               and (game->players[0]->myCards.size()+1) <= MAX_CARDS_PER_PLAYER and game->players[0]->myTurn == true)
            {
                //First of all, we need to find out where the card should be inserted. What's the card left and card right of this new card
                vector<int> neighborCardsPos; //It will have [1] or [2 -or- 3] elements (if it has 3 elements the first element should be ignored because is the card behind the first card). The vector will NEVER have 0 elements.
                
                for(int i = 0; i < game->players[0]->myCards.size(); i++)
                {
                    Sprite *card = (Sprite*)game->scene->getChildByName("layoutMyCards")->getChildByName(game->players[0]->myCards[i].imgName);
                    Rect cardRect = card->getBoundingBox();
                    cardRect.origin = card->getParent()->convertToWorldSpace(cardRect.origin);

                    if(cardSeenTopRect.intersectsRect(cardRect))
                        neighborCardsPos.push_back(i);
                }
                
                int cardVecPos = 0; //Position in the vector where the card should be added
                if(neighborCardsPos.size() > 1) //card in the middle
                    cardVecPos = neighborCardsPos[neighborCardsPos.size()-1];
                else if(neighborCardsPos.size() == 1 and neighborCardsPos[0] == 0) //first card
                    cardVecPos = neighborCardsPos[0];
                else if(neighborCardsPos.size() == 1 and neighborCardsPos[0] == game->players[0]->myCards.size()-1) //last card
                    cardVecPos = neighborCardsPos[0] + 1;
                else
                {
                    Action *actionMyCard = MoveTo::create(0.3, game->scene->getChildByName("cardBackground")->getPosition());
                    actionMyCard->setTag(0);
                    cardTouching->runAction(actionMyCard);
                    cardTouching->setGlobalZOrder(0);
                    break;
                }
                    
                //Add card && remove top card
                if(game->players[0]->addCard(game->cardsSeen.top(), cardVecPos))
                {
                    game->scene->removeChildByName(game->cardsSeen.top().imgName);
                    game->cardsSeen.pop();
                    
                    if(!bot->isPlaying)
                        nakama->sendMatchData(OPCODE_GRAB_CARD, "{\"From\":\"cardsSeen\",\"Position\":" + to_string(cardVecPos) + "}");
                }
            }
            else //else, it returns to the last card seen
            {
                UserInterface *ui = UserInterface::getInstance(game->scene);
                Action *actionReturnCard = MoveTo::create(0.3, game->scene->getChildByName("cardBackground")->getPosition());
                actionReturnCard->setTag(0);
                cardTouching->runAction(actionReturnCard);
                cardTouching->setGlobalZOrder(0);
                
                if(cardSeenTopRect.intersectsRect(layoutMyCardsRect)
                and (game->players[0]->myCards.size()+1) <= MAX_CARDS_PER_PLAYER and game->players[0]->myTurn == false)
                {
                    ui->showQuickError(ERROR_TURN);
                }
                else if(cardSeenTopRect.intersectsRect(layoutMyCardsRect)
                and (game->players[0]->myCards.size()+1) > MAX_CARDS_PER_PLAYER and game->players[0]->myTurn == true)
                {
                    ui->showQuickError(ERROR_LEAVE_CARD);
                }
            }
            
            break;
        }
        case -1: //Card from the Stack
        {
            const auto cardStackLocation = game->scene->getChildByName("cardStack")->convertToNodeSpace(touch->getLocation());
            const auto &cardStackSize = game->scene->getChildByName("cardStack")->getContentSize();
            const auto cardStackRect = Rect(0, 0, cardStackSize.width, cardStackSize.height);
            
            if(cardStackRect.containsPoint(cardStackLocation) and game->players[0]->myTurn == true) //If I grab a new card from the stack...
            {
                if(!game->deckOfCards.empty())
                {
                    if(game->players[0]->addCard(game->deckOfCards.top(), (int)game->players[0]->myCards.size()))
                    {
                        AudioEngine::play2d("sounds/cardTouch.wav");
                        game->deckOfCards.pop();
                        Label *aDeckCards = (Label*)game->scene->getChildByName("layoutUI")->getChildByName("amountDeckOfCards");
                        aDeckCards->setString(to_string(stoi(aDeckCards->getString()) - 1));
                        
                        if(!bot->isPlaying)
                            nakama->sendMatchData(OPCODE_GRAB_CARD, "{\"From\":\"deckOfCards\",\"Position\":" + to_string(game->players[0]->myCards.size()-1) + "}");
                    }
                    else if((game->players[0]->myCards.size()+1) > MAX_CARDS_PER_PLAYER)
                    {
                        UserInterface *ui = UserInterface::getInstance(game->scene);
                        ui->showQuickError(ERROR_LEAVE_CARD);
                    }
                }
            }
            else if(cardStackRect.containsPoint(cardStackLocation) and game->players[0]->myTurn == false)
            {
                UserInterface *ui = UserInterface::getInstance(game->scene);
                ui->showQuickError(ERROR_TURN);
            }
            
            cardTouching->setGlobalZOrder(0);
            break;
        }
        default: //My cards
        {
            Rect cardRect = cardTouching->getBoundingBox();
            Rect cardBackgroundRect = game->scene->getChildByName("cardBackground")->getBoundingBox();
            Rect layoutMyCardsRect = game->scene->getChildByName("layoutMyCards")->getBoundingBox();
            cardRect.origin = cardTouching->getParent()->convertToWorldSpace(cardRect.origin);
            cardBackgroundRect.origin = game->scene->getChildByName("cardBackground")->getParent()->convertToWorldSpace(cardBackgroundRect.origin);
            layoutMyCardsRect.origin = game->scene->getChildByName("layoutMyCards")->getParent()->convertToWorldSpace(layoutMyCardsRect.origin);
            
            Rect cardFinishBgRect;
            if(game->scene->getChildByName("cardFinishBackground") != NULL)
            {
                cardFinishBgRect = game->scene->getChildByName("cardFinishBackground")->getBoundingBox();
                cardFinishBgRect.origin = game->scene->getChildByName("cardFinishBackground")->getParent()->convertToWorldSpace(cardFinishBgRect.origin);
            }
            
            int i = 0;
            while(i < game->players[0]->myCards.size() and game->players[0]->myCards[i].imgName != cardTouching->getName())
                i++;
                      
            if(i < game->players[0]->myCards.size() and game->scene->getChildByName("cardFinishBackground") != NULL and cardRect.intersectsRect(cardFinishBgRect)) //If I finish the game...
            {
                card cardFinish = game->players[0]->myCards[i];
                
                Sprite *cardFinishGame = Sprite::create("cards/cardBack.png");
                cardFinishGame->setName(game->players[0]->myCards[i].imgName);
                cardFinishGame->setPosition(touch->getLocation());
                cardFinishGame->setScale(visibleSize.width * CARD_SIZE / cardFinishGame->getContentSize().width);
                game->scene->addChild(cardFinishGame);

                game->players[0]->removeCard(game->players[0]->myCards[i]);
                
                Action *actionFinishGame = MoveTo::create(0.3, game->scene->getChildByName("cardFinishBackground")->getPosition());
                actionFinishGame->setTag(0);
                cardFinishGame->runAction(actionFinishGame);
                
                string modeFinished = (game->scene->getChildByName("cardFinishBackground")->getTag() == 0) ? "Gin" : "Knock";
                
                if(!bot->isPlaying)
                    nakama->sendMatchData(OPCODE_FINISH_GAME, "{\"Mode\":\"" + modeFinished + "\",\"Card\":\"" + cardFinish.imgName + "\"}");
                else
                    scheduler->unschedule(SEL_SCHEDULE(&Bot::tryToPlay), Bot::getInstance()); //If the bot is playing, he should stop playing...
                    
                //Finish the game && show all cards
                game->finishGameRound(game->players[0], modeFinished);
            }
            else if(i < game->players[0]->myCards.size() and cardRect.intersectsRect(cardBackgroundRect) //If I leave my card in the stack...
               and (game->players[0]->myCards.size()-1) >= MIN_CARDS_PER_PLAYER and game->players[0]->myTurn == true)
            {
                if(game->scene->getChildByName("cardFinishBackground") != NULL)
                    game->scene->removeChildByName("cardFinishBackground");
                
                game->cardsSeen.push(game->players[0]->myCards[i]);
                
                Sprite *newLastSeenCard = Sprite::create("cards/" + game->cardsSeen.top().imgName + ".png");
                newLastSeenCard->setName(game->cardsSeen.top().imgName);
                newLastSeenCard->setPosition(touch->getLocation());
                newLastSeenCard->setScale(visibleSize.width * CARD_SIZE / newLastSeenCard->getContentSize().width);
                game->scene->addChild(newLastSeenCard, (int32_t)game->cardsSeen.size());

                game->players[0]->removeCard(game->players[0]->myCards[i]);
                
                Action *actionNewCard = MoveTo::create(0.3, game->scene->getChildByName("cardBackground")->getPosition());
                actionNewCard->setTag(0);
                newLastSeenCard->runAction(actionNewCard);
                
                if(!bot->isPlaying)
                    nakama->sendMatchData(OPCODE_LEAVE_CARD, "{\"Card\":\"" + game->cardsSeen.top().imgName + "\"}");
                
                Label *aDeckCards = (Label*)game->scene->getChildByName("layoutUI")->getChildByName("amountDeckOfCards");
                if(stoi(aDeckCards->getString()) == 2) //if there are 2 cards on deckOfCards, the game ends
                {
                    if(bot->isPlaying)
                        scheduler->unschedule(SEL_SCHEDULE(&Bot::tryToPlay), Bot::getInstance());
                    
                    game->finishGameRound(NULL, "Draw");
                }
                else
                {
                    game->players[0]->myTurn = false;
                    game->players[1]->myTurn = true;
                    game->scene->getChildByName("avatarMarked")->setPosition(game->scene->getChildByName("opponentAvatar")->getPosition());
                }
            }
            else if(i < game->players[0]->myCards.size() and cardRect.intersectsRect(layoutMyCardsRect)) //If I want to re-order a card
            {
                if(game->scene->getChildByName("cardFinishBackground") != NULL)
                    game->scene->removeChildByName("cardFinishBackground");
                
                //We need to find out where the card should be moved (what's the new card left and card right of this card)
                vector<int> neighborCardsPos;
                
                for(int k = 0; k < game->players[0]->myCards.size(); k++)
                {
                    Sprite *card = (Sprite*)game->scene->getChildByName("layoutMyCards")->getChildByName(game->players[0]->myCards[k].imgName);
               
                    if(card->getName() != cardTouching->getName() and cardTouching->getBoundingBox().intersectsRect(card->getBoundingBox()))
                        neighborCardsPos.push_back(k);
                }
                
                int cardVecPos = 0; //Position in the vector where the card should be moved
                
                //If I touch the card and I leave it in the same position, it should return to the same position (there's not reordering)
                if((neighborCardsPos.size() == 2 and neighborCardsPos[1] - neighborCardsPos[0] == 2) or
                   (neighborCardsPos.size() == 1 and neighborCardsPos[0] == 1) or
                   (neighborCardsPos.size() == 1 and neighborCardsPos[0] == game->players[0]->myCards.size()-2) or
                   neighborCardsPos.size() == 0)
                {
                    Sprite *nextCard = (Sprite*)game->scene->getChildByName("layoutMyCards")->getChildByName(game->players[0]->myCards[(i > 0)? i-1 : i+1].imgName);
                    
                    Action *actionNextCard = MoveTo::create(0.3, Vec2(nextCard->getPositionX() + (nextCard->getBoundingBox().size.width * 0.70f * ((i > 0)? 1 : -1)), nextCard->getPositionY()));
                    actionNextCard->setTag(0);
                    cardTouching->runAction(actionNextCard);
                    
                    cardTouching->setGlobalZOrder(0);
                    break;
                } //else... we need to do an exchange (there's reordering):
                else if(neighborCardsPos.size() == 1)
                    cardVecPos = neighborCardsPos[0];
                else //neighborCardsPos.size() > 1
                    cardVecPos = (cardTouching->getLocalZOrder() > neighborCardsPos[neighborCardsPos.size()-1])? neighborCardsPos[neighborCardsPos.size()-1] : neighborCardsPos[neighborCardsPos.size()-2];
                
              
                //Reorder cards
                vector<card>::iterator itCardToMove = find_if(game->players[0]->myCards.begin(), game->players[0]->myCards.end(), [=](const card &card){ return card.imgName == cardTouching->getName(); });
                
                string cardToMove = (*itCardToMove).imgName;
                if(itCardToMove == game->players[0]->myCards.end() or !game->players[0]->reorderCard(*itCardToMove, cardVecPos))
                {
                    UserInterface *ui = UserInterface::getInstance(Director::getInstance()->getRunningScene());
                    ui->addWindow(WINDOW_ERROR, LanguageManager::getString("LABEL_ERROR_GENERIC"));
                    
                    cardTouching->setGlobalZOrder(0);
                    break;
                }
                
                cardTouching->setGlobalZOrder(0);
                
                if(!bot->isPlaying)
                    nakama->sendMatchData(OPCODE_ORDER_CARD, "{\"Card\":\"" + cardToMove + "\",\"Position\":" + to_string(cardVecPos) + "}");
            }
            else if(i < game->players[0]->myCards.size())
            {
                if(game->scene->getChildByName("cardFinishBackground") != NULL)
                    game->scene->removeChildByName("cardFinishBackground");
                
                //Return to my list of cards
                Sprite *nextCard = (Sprite*)game->scene->getChildByName("layoutMyCards")->getChildByName(game->players[0]->myCards[(i > 0)? i-1 : i+1].imgName);
                
                Action *actionReturnCard = MoveTo::create(0.3, Vec2(nextCard->getPositionX() + (nextCard->getBoundingBox().size.width * 0.70f * ((i > 0)? 1 : -1)), nextCard->getPositionY()));
                actionReturnCard->setTag(0);
                cardTouching->runAction(actionReturnCard);
                
                cardTouching->setGlobalZOrder(0);
                
                UserInterface *ui = UserInterface::getInstance(game->scene);
                if(cardRect.intersectsRect(cardBackgroundRect) and game->players[0]->myTurn == false)
                {
                    ui->showQuickError(ERROR_TURN);
                }
                else if(cardRect.intersectsRect(cardBackgroundRect) and (game->players[0]->myCards.size()-1) < MIN_CARDS_PER_PLAYER and game->players[0]->myTurn == true)
                {
                    ui->showQuickError(ERROR_TAKE_CARD);
                }
            }
            
            break;
        }
    }
    
    game->players[0]->markCombinationsIfExist();
    cardTouching = NULL;
    return true;
}


/**
 @brief  Destructor
 @param  -
 @return -
*/

EventHandler::~EventHandler()
{
}
