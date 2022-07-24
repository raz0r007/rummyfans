#include "Bot.h"
#include "Game.h"
#include "EventHandler.h"
#include "audio/include/AudioEngine.h"

Bot* Bot::m_pInstance = 0;

/**
 @brief  Constructor
 @param  -
 @return -
*/

Bot::Bot()
{
    isPlaying = false;
}


/**
 @brief    Get always the same instance of the object (Singleton)
 @param    -
 @return   Bot*
*/

Bot* Bot::getInstance()
{
    if(m_pInstance == NULL)
        m_pInstance = new Bot;
        
    return m_pInstance;
}


/**
 @brief    Create a new bot
 @param    name: bot's name
           avatarUrl: avatar url
 @return   bool
*/

bool Bot::create(string name, string avatarUrl)
{
    Director *director = Director::getInstance();
    if(director->getRunningScene()->getName() != "matchScene") //the bot should be created always on matchScene
        return false;

    //Creating a bot:
    botName = name;
    isPlaying = true; //if we create a bot, it means that we're going to play with a bot
    
    Game *game = Game::getInstance();
    game->players[1]->pid = "0";
    game->players[1]->name = name;
    game->players[1]->avatarUrl = avatarUrl;
    
    Image *m = new Image;
    m->initWithImageFile("others/defaultAvatar.png");
    game->players[1]->avatarImg = m;
    
    game->players[1]->points = 0;
    game->players[1]->readyForPlay = true;
    game->players[1]->myTurn = true; //because of we're not joining to a nakama match, we can't determinate who joined first, so the bot start playing...
    game->players[0]->myTurn = false;
    
    //Finally, we need to configure a Scheduler for check whenever is my turn and act in consecuence:
    auto scheduler = Director::getInstance()->getScheduler();
    
    scheduler->unschedule(SEL_SCHEDULE(&Bot::tryToPlay), Bot::getInstance()); //for make sure that there's no similar threads already scheduled
    scheduler->schedule(SEL_SCHEDULE(&Bot::tryToPlay), Bot::getInstance(), 0.18f, false);
    
    return true;
}


/**
 @brief    Try to play (if myTurn is true)
 @param    -
 @return   bool
*/

bool Bot::tryToPlay()
{
    Director *director = Director::getInstance();
    Game *game = Game::getInstance();
    Bot *bot = Bot::getInstance();
    EventHandler *eventHandler = EventHandler::getInstance();
    
    if(eventHandler->cardTouching != NULL or game->scene->getActionManager()->getNumberOfRunningActions() > 0)
        return false;
    
    if(director->getRunningScene()->getName() == "gameScene" and bot->isMyTurn() == true) //if is my turn..
    {
        //If is my turn, the first decision that I need to take is if I grab a card from the stack or the last card seen, so we're going to take this decision:
        auto grabCard = CallFunc::create([]() {
            Game *game = Game::getInstance();
            Bot *bot = Bot::getInstance();
            vector<card>::iterator itCard;
            card lastCardSeen = game->cardsSeen.top();
            
            itCard = find_if(game->players[1]->myCards.begin(), game->players[1]->myCards.end(), [lastCardSeen](const card &card){
                return (card.suit == lastCardSeen.suit and abs((int)card.value - (int)lastCardSeen.value) == 1); //if this condition is true, this card could be useful for me
            });
            if(itCard == game->players[1]->myCards.end())
            {
                //if I didn't have lucky, I'm going to look if it's useful for group with same values
                itCard = find_if(game->players[1]->myCards.begin(), game->players[1]->myCards.end(), [lastCardSeen](const card &card){ return card.value == lastCardSeen.value; });
                if(itCard == game->players[1]->myCards.end()) {
                    bot->grabCard("deckOfCards"); //if I didn't have lucky, I'm going to grab a card from the stack
                }
                else
                    bot->grabCard("cardsSeen");
            }
            else
                bot->grabCard("cardsSeen");
        });
        
        //Now I've 11 cards, so I'm going to recalculate the groups that I can make and re-order:
        auto orderCards = CallFunc::create([]() {
            Bot *bot = Bot::getInstance();

            bot->orderMyCardsBySuit();
            bot->orderMyCardsByValue();
        });
            
        //Now I should leave a card or finish the round:
        auto leaveCardOrFinishRound = CallFunc::create([]() {
            Game *game = Game::getInstance();
            Bot *bot = Bot::getInstance();
            auto scheduler = Director::getInstance()->getScheduler();

            vector<card> cardsNoGrouped;
            card cardNoGroupedMaxValue; //I want to know this card because I'm going to use it for finish a round or for leave the card into the stack (is my strategy.. leave the higher card in order to have few points no grouped if I lose or win the round)
            int sumValues = 0; //sum of values of cards NOT grouped (10 cards)
            int sumValuesTotal = 0; //sum of values of cards NOT grouped (11 cards)
            for(auto card : game->players[1]->myCards)
            {
                if(game->scene->getChildByName("layoutOpponentCards")->getChildByName(card.imgName)->getChildByName("circleCardComb") != NULL)
                    continue;
                
                cardsNoGrouped.push_back(card);
                sumValuesTotal += (int)card.points;
            }
            
            if(cardsNoGrouped.size() == 1)
            {
                sumValues = 0; //Gin
                cardNoGroupedMaxValue = cardsNoGrouped[0];
            }
            else if(cardsNoGrouped.size() > 1) //Knock... or nothing..
            {
                cardNoGroupedMaxValue = cardsNoGrouped[0]; //cardNoGroupedMaxValue is the card that i'm going to use for finish a round or leave the card into the stack
                for(auto card : cardsNoGrouped)
                {
                   if(card.value > cardNoGroupedMaxValue.value)
                       cardNoGroupedMaxValue = card;
                }
                
                sumValues = sumValuesTotal - (int)cardNoGroupedMaxValue.points;
            }
            else if(cardsNoGrouped.empty()) //in this case, i'm obligated to destroy a group of cards
            {
                cardNoGroupedMaxValue = game->players[1]->myCards[game->players[1]->myCards.size()-1]; //we could be more intelligent and destroy a specific group, but it doesn't matter!. Also, in this case I will continue playing despite of I can finish the round (maybe). But, again, it doesn't matter, we're not so intelligent...
            }
            
            if(sumValues <= 10 and !cardsNoGrouped.empty()) //I can finish the round! (we're not so intelligent to do something like: wait until we've GIN instead of KNOCK....), but it doesn't matter!
            {
                scheduler->unschedule(SEL_SCHEDULE(&Bot::tryToPlay), Bot::getInstance());
                bot->finishGameRound(cardNoGroupedMaxValue, (sumValues == 0) ? "Gin" : "Knock");
            }
            else //Continue playing...
            {
                Label *aDeckCards = (Label*)game->scene->getChildByName("layoutUI")->getChildByName("amountDeckOfCards");
                if(stoi(aDeckCards->getString()) == 2) //if there are 2 cards on deckOfCards, the game ends
                    scheduler->unschedule(SEL_SCHEDULE(&Bot::tryToPlay), Bot::getInstance());
                    
                bot->leaveCard(cardNoGroupedMaxValue); //Leave && finish game round if neccesary (2 cards on the pile)
            }
        });
        
        Action *actionBot = Sequence::create(grabCard, DelayTime::create(8), orderCards, DelayTime::create(2), leaveCardOrFinishRound, NULL);
        actionBot->setTag(1);
        game->scene->runAction(actionBot);
        
        return true;
    }
    
    return false;
}


/**
 @brief    Grab a new card to my list of cards
 @param    from: "deckOfCards" or "cardsSeen"
 @return   -
*/

void Bot::grabCard(string from)
{
    Game *game = Game::getInstance();
    
    AudioEngine::play2d("sounds/cardTouch.wav");
    
    if(from == "deckOfCards" and !game->deckOfCards.empty())
    {
        if(game->players[1]->addCard(game->deckOfCards.top(), (int)game->players[1]->myCards.size()))
        {
            game->deckOfCards.pop();
            Label *aDeckCards = (Label*)game->scene->getChildByName("layoutUI")->getChildByName("amountDeckOfCards");
            aDeckCards->setString(to_string(stoi(aDeckCards->getString()) - 1));
        }
    }
    else if(from == "cardsSeen" and !game->cardsSeen.empty())
    {
        if(game->players[1]->addCard(game->cardsSeen.top(), (int)game->players[1]->myCards.size()))
        {
            game->scene->removeChildByName(game->cardsSeen.top().imgName);
            game->cardsSeen.pop();
        }
    }
}


/**
 @brief    Leave a card in the stack or if I want to finish the round
 @param    cardToLeave: card to leave in the stack
 @return   -
*/

void Bot::leaveCard(card cardToLeave)
{
    Director *director = Director::getInstance();
    Rect safeArea = director->getSafeAreaRect();
    Size visibleSize = safeArea.size;
    Game *game = Game::getInstance();
    Vec2 cardPosition = game->scene->getChildByName("layoutOpponentCards")->getChildByName(cardToLeave.imgName)->getPosition();
    Label *aDeckCards = (Label*)game->scene->getChildByName("layoutUI")->getChildByName("amountDeckOfCards");
    EventHandler *eventHandler = EventHandler::getInstance();
    
    if(stoi(aDeckCards->getString()) == 2) //if there are 2 cards on deckOfCards, the game should end
    {
        if(game->scene->getChildByName("layoutUI")->getChildByName("btnContinue") == NULL) {
            Node *btnContinueFake = Node::create();
            btnContinueFake->setName("btnContinue");
            game->scene->getChildByName("layoutUI")->addChild(btnContinueFake);
        }
        
        if(eventHandler->cardTouching != NULL)
        {
            //If I'm touching a card I have to wait until I leave the card, and then I finish the game.
            game->scene->scheduleOnce([this, cardToLeave](float){
                leaveCard(cardToLeave);
            }, 0.25f, "scheduleOnceFinishGame");
            
            return;
        }
    }
    
    game->cardsSeen.push(cardToLeave);
    
    AudioEngine::play2d("sounds/cardTouch.wav");
    
    Sprite *newLastSeenCard = Sprite::create("cards/" + game->cardsSeen.top().imgName + ".png");
    newLastSeenCard->setName(game->cardsSeen.top().imgName);
    newLastSeenCard->setPosition(game->scene->getChildByName("layoutOpponentCards")->getChildByName(cardToLeave.imgName)->getParent()->convertToWorldSpace(cardPosition));
    newLastSeenCard->setScale(visibleSize.width * CARD_SIZE / newLastSeenCard->getContentSize().width);
    game->scene->addChild(newLastSeenCard, (int32_t)game->cardsSeen.size());

    game->players[1]->removeCard(cardToLeave);
    
    Action *actionLeaveCard = MoveTo::create(0.3, game->scene->getChildByName("cardBackground")->getPosition());
    actionLeaveCard->setTag(1);
    newLastSeenCard->runAction(actionLeaveCard);
    
    if(stoi(aDeckCards->getString()) == 2) //if there are 2 cards on deckOfCards, the game ends
    {
        if(game->scene->getChildByName("layoutUI")->getChildByName("btnContinue") != NULL)
            game->scene->getChildByName("layoutUI")->removeChildByName("btnContinue");
        
        game->finishGameRound(NULL, "Draw");
    }
    else
    {
        //Finally, is my opponent's turn:
        game->players[0]->myTurn = true;
        game->players[1]->myTurn = false;
        game->scene->getChildByName("avatarMarked")->setPosition(game->scene->getChildByName("myAvatar")->getPosition());
    }
}


/**
 @brief    Finish game round
 @param    cardToFinish: card to finish the round
           mode: mode to finish could be Gin or Knock
 @return   -
*/

void Bot::finishGameRound(card cardToFinish, string mode)
{
    Director *director = Director::getInstance();
    Rect safeArea = director->getSafeAreaRect();
    Size visibleSize = safeArea.size;
    Game *game = Game::getInstance();
    Vec2 cardPosition = game->scene->getChildByName("layoutOpponentCards")->getChildByName(cardToFinish.imgName)->getPosition();
    EventHandler *eventHandler = EventHandler::getInstance();
    
    if(game->scene->getChildByName("layoutUI")->getChildByName("btnContinue") == NULL) {
        Node *btnContinueFake = Node::create();
        btnContinueFake->setName("btnContinue");
        game->scene->getChildByName("layoutUI")->addChild(btnContinueFake);
    }
    
    if(eventHandler->cardTouching != NULL)
    {
        //If I'm touching a card I have to wait until I leave the card, and then we finish the game.
        game->scene->scheduleOnce([this, cardToFinish, mode](float){
            finishGameRound(cardToFinish, mode);
        }, 0.25f, "scheduleOnceFinishGame");
        
        return;
    }
    
    Sprite *cardBg = Sprite::create("cards/card" + mode + ".png");
    cardBg->setName("cardFinishBackground");
    cardBg->setScale(visibleSize.width * 0.105f / cardBg->getContentSize().width);
    cardBg->setPosition(game->scene->getChildByName("cardBackground")->getPositionX() - game->scene->getChildByName("cardBackground")->getBoundingBox().size.width * 1.05f, game->scene->getChildByName("cardBackground")->getPositionY());
    game->scene->addChild(cardBg);
    
    Sprite *cardFinishGame = Sprite::create("cards/cardBack.png");
    cardFinishGame->setName(cardToFinish.imgName);
    cardFinishGame->setPosition(game->scene->getChildByName("layoutOpponentCards")->getChildByName(cardToFinish.imgName)->getParent()->convertToWorldSpace(cardPosition));
    cardFinishGame->setScale(visibleSize.width * CARD_SIZE / cardFinishGame->getContentSize().width);
    game->scene->addChild(cardFinishGame);
    
    game->players[1]->removeCard(cardToFinish);
    
    Action *actionFinishGame = MoveTo::create(0.3, game->scene->getChildByName("cardFinishBackground")->getPosition());
    actionFinishGame->setTag(1);
    cardFinishGame->runAction(actionFinishGame);
    
    if(game->scene->getChildByName("layoutUI")->getChildByName("btnContinue") != NULL)
        game->scene->getChildByName("layoutUI")->removeChildByName("btnContinue");
    
    //Finish the game && show all cards
    game->finishGameRound(game->players[1], mode);
}



/**
 @brief    Check if is my turn to play
 @param    -
 @return   bool
*/

bool Bot::isMyTurn()
{
    Game *game = Game::getInstance();
    return game->players[1]->myTurn;
}


/**
 @brief    Order my cards by value. Example: 8 - 8 - 8.
 @param    -
 @return   -
*/

void Bot::orderMyCardsByValue()
{
    Game *game = Game::getInstance();
    vector<card> myCards = game->players[1]->myCards;
    
    for(int i = 0; i < myCards.size(); i++)
    {
        if(game->scene->getChildByName("layoutOpponentCards")->getChildByName(myCards[i].imgName)->getChildByName("circleCardComb") != NULL)
            continue;
        
        card cardSelected = myCards[i];
        int j = i + 1;
        
        while(j < myCards.size())
        {
            if(myCards[j].value == cardSelected.value)
            {
                vector<card>::iterator itCard = game->players[1]->myCards.begin();
                advance(itCard, j);
                moveCardInVector(itCard, i+1);
            }
            
            j++;
        }
        
        game->players[1]->markCombinationsIfExist(false);
    }
}


/**
 @brief    Order my cards by suit. Example: 7 - 8 - 9
 @param    -
 @return   -
*/

void Bot::orderMyCardsBySuit()
{
    Game *game = Game::getInstance();
    vector<card> myCards = game->players[1]->myCards;
    
    for(int i = 0; i < myCards.size(); i++)
    {
        if(game->scene->getChildByName("layoutOpponentCards")->getChildByName(myCards[i].imgName)->getChildByName("circleCardComb") != NULL)
            continue;
        
        card cardSelected = myCards[i];
        int j = i + 1;
        
        while(j < myCards.size())
        {
            if(myCards[j].suit == cardSelected.suit and abs((int)myCards[j].value - (int)cardSelected.value) == 1)
            {
                vector<card>::iterator itCard = game->players[1]->myCards.begin();
                advance(itCard, j);
                moveCardInVector(itCard, i+1);
                
                cardSelected = myCards[j];
            }
            
            j++;
        }
        
        game->players[1]->markCombinationsIfExist(false);
    }
}


/**
 @brief    Move a card in my vector and rename sprites
 @param    itCardToMove: card to move
           newPosition: new position in the vector
 @return   -
*/

void Bot::moveCardInVector(vector<card>::iterator itCardToMove, int newPosition)
{
    Game *game = Game::getInstance();
    
    for(int i = 0; i < game->players[1]->myCards.size(); i++)
        game->scene->getChildByName("layoutOpponentCards")->getChildByName(game->players[1]->myCards[i].imgName)->setTag(i);
        
    //Re-order the vector (logic: https://coliru.stacked-crooked.com/a/5c31007000b9eeba)
    int oldIndex = (int)distance(game->players[1]->myCards.begin(), itCardToMove);
    int newIndex = newPosition;
    if (oldIndex > newIndex)
        rotate(game->players[1]->myCards.rend() - oldIndex - 1, game->players[1]->myCards.rend() - oldIndex, game->players[1]->myCards.rend() - newIndex);
    else
        rotate(game->players[1]->myCards.begin() + oldIndex, game->players[1]->myCards.begin() + oldIndex + 1, game->players[1]->myCards.begin() + newIndex + 1);
    
    for(int i = 0; i < game->players[1]->myCards.size(); i++)
        game->scene->getChildByName("layoutOpponentCards")->getChildByTag(i)->setName(game->players[1]->myCards[i].imgName); //update sprite names
}


/**
 @brief    Destroy the Bot Instance.
 @param    -
 @return   -
*/

void Bot::destroyInstance()
{
    delete m_pInstance;
    m_pInstance = NULL;
}


/**
 @brief  Destructor
 @param  -
 @return -
*/

Bot::~Bot()
{
    m_pInstance = NULL;
}


// -------

string getRandomBotName()
{
    vector<string> randomNames;
    randomNames.push_back("Armstrong");
    randomNames.push_back("Bandit");
    randomNames.push_back("Beast");
    randomNames.push_back("Boomer");
    randomNames.push_back("Buzz");
    randomNames.push_back("Casper");
    randomNames.push_back("Caveman");
    randomNames.push_back("C-Block");
    randomNames.push_back("Centice");
    randomNames.push_back("Chipper");
    randomNames.push_back("Cougar");
    randomNames.push_back("Dude");
    randomNames.push_back("Foamer");
    randomNames.push_back("Fury");
    randomNames.push_back("Gerwin");
    randomNames.push_back("Goose");
    randomNames.push_back("Heater");
    randomNames.push_back("Hollywood");
    randomNames.push_back("Hound");
    randomNames.push_back("Iceman");
    randomNames.push_back("Imp");
    randomNames.push_back("Jester");
    randomNames.push_back("JM");
    randomNames.push_back("Junker");
    randomNames.push_back("Khan");
    randomNames.push_back("Maverick");
    randomNames.push_back("Middy");
    randomNames.push_back("Merlin");
    randomNames.push_back("Mountain");
    randomNames.push_back("Myrtle");
    randomNames.push_back("Outlaw");
    randomNames.push_back("Poncho");
    randomNames.push_back("Rainmaker");
    randomNames.push_back("Raja");
    randomNames.push_back("Rex");
    randomNames.push_back("Roundhouse");
    randomNames.push_back("Sabretooth");
    randomNames.push_back("Saltie");
    randomNames.push_back("Samara");
    randomNames.push_back("Scout");
    randomNames.push_back("Shepard");
    randomNames.push_back("Slider");
    randomNames.push_back("Squall");
    randomNames.push_back("Sticks");
    randomNames.push_back("Stinger");
    randomNames.push_back("Storm");
    randomNames.push_back("Sundown");
    randomNames.push_back("Sultan");
    randomNames.push_back("Swabbie");
    randomNames.push_back("Tusk");
    randomNames.push_back("Tex");
    randomNames.push_back("Viper");
    randomNames.push_back("Wolfman");
    randomNames.push_back("Yuri");
    
    return randomNames[RandomHelper::random_int(0, (int)randomNames.size()-1)];
}
