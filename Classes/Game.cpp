#include "Game.h"
#include "EventHandler.h"
#include "NakamaServer.h"
#include "ThirdPartys/SQLite/SQLite.h"
#include "audio/include/AudioEngine.h"

Game* Game::m_pInstance = 0;

/**
 @brief  Constructor
 @param  -
 @return -
*/

Game::Game()
{
    //Game Scene
    scene = new Scene;
    scene->setName("gameScene");
    
    //Players
    players[0] = new Player; //Me
    players[1] = new Player; //Opponent
}


/**
 @brief    Get always the same instance of the object (Singleton)
 @param    -
 @return   Game*
*/

Game* Game::getInstance()
{
    if(m_pInstance == NULL)
        m_pInstance = new Game;
        
    return m_pInstance;
}


/**
 @brief    Destroy the Game Instance.
 @param    -
 @return   -
*/

void Game::destroyInstance()
{
    delete m_pInstance;
    m_pInstance = NULL;
}


/**
 @brief    Get the full / original deck of cards
 @param    -
 @return   list<card>
*/

list<card> Game::getFullDeckOfCards()
{
    list<card> listCards;
    card cards[52];
  
    cards[0].suit = cardSuit::Clubs; cards[0].value = cardValue::Value2; cards[0].imgName = "card-Clubs-2"; cards[0].points = 2;
    cards[1].suit = cardSuit::Clubs; cards[1].value = cardValue::Value3; cards[1].imgName = "card-Clubs-3"; cards[1].points = 3;
    cards[2].suit = cardSuit::Clubs; cards[2].value = cardValue::Value4; cards[2].imgName = "card-Clubs-4"; cards[2].points = 4;
    cards[3].suit = cardSuit::Clubs; cards[3].value = cardValue::Value5; cards[3].imgName = "card-Clubs-5"; cards[3].points = 5;
    cards[4].suit = cardSuit::Clubs; cards[4].value = cardValue::Value6; cards[4].imgName = "card-Clubs-6"; cards[4].points = 6;
    cards[5].suit = cardSuit::Clubs; cards[5].value = cardValue::Value7; cards[5].imgName = "card-Clubs-7"; cards[5].points = 7;
    cards[6].suit = cardSuit::Clubs; cards[6].value = cardValue::Value8; cards[6].imgName = "card-Clubs-8"; cards[6].points = 8;
    cards[7].suit = cardSuit::Clubs; cards[7].value = cardValue::Value9; cards[7].imgName = "card-Clubs-9"; cards[7].points = 9;
    cards[8].suit = cardSuit::Clubs; cards[8].value = cardValue::Value10; cards[8].imgName = "card-Clubs-10"; cards[8].points = 10;
    cards[9].suit = cardSuit::Clubs; cards[9].value = cardValue::ValueA; cards[9].imgName = "card-Clubs-A"; cards[9].points = 1;
    cards[10].suit = cardSuit::Clubs; cards[10].value = cardValue::ValueJ; cards[10].imgName = "card-Clubs-J"; cards[10].points = 10;
    cards[11].suit = cardSuit::Clubs; cards[11].value = cardValue::ValueK; cards[11].imgName = "card-Clubs-K"; cards[11].points = 10;
    cards[12].suit = cardSuit::Clubs; cards[12].value = cardValue::ValueQ; cards[12].imgName = "card-Clubs-Q"; cards[12].points = 10;

    cards[13].suit = cardSuit::Diamonds; cards[13].value = cardValue::Value2; cards[13].imgName = "card-Diamonds-2"; cards[13].points = 2;
    cards[14].suit = cardSuit::Diamonds; cards[14].value = cardValue::Value3; cards[14].imgName = "card-Diamonds-3"; cards[14].points = 3;
    cards[15].suit = cardSuit::Diamonds; cards[15].value = cardValue::Value4; cards[15].imgName = "card-Diamonds-4"; cards[15].points = 4;
    cards[16].suit = cardSuit::Diamonds; cards[16].value = cardValue::Value5; cards[16].imgName = "card-Diamonds-5"; cards[16].points = 5;
    cards[17].suit = cardSuit::Diamonds; cards[17].value = cardValue::Value6; cards[17].imgName = "card-Diamonds-6"; cards[17].points = 6;
    cards[18].suit = cardSuit::Diamonds; cards[18].value = cardValue::Value7; cards[18].imgName = "card-Diamonds-7"; cards[18].points = 7;
    cards[19].suit = cardSuit::Diamonds; cards[19].value = cardValue::Value8; cards[19].imgName = "card-Diamonds-8"; cards[19].points = 8;
    cards[20].suit = cardSuit::Diamonds; cards[20].value = cardValue::Value9; cards[20].imgName = "card-Diamonds-9"; cards[20].points = 9;
    cards[21].suit = cardSuit::Diamonds; cards[21].value = cardValue::Value10; cards[21].imgName = "card-Diamonds-10"; cards[21].points = 10;
    cards[22].suit = cardSuit::Diamonds; cards[22].value = cardValue::ValueA; cards[22].imgName = "card-Diamonds-A"; cards[22].points = 1;
    cards[23].suit = cardSuit::Diamonds; cards[23].value = cardValue::ValueJ; cards[23].imgName = "card-Diamonds-J"; cards[23].points = 10;
    cards[24].suit = cardSuit::Diamonds; cards[24].value = cardValue::ValueK; cards[24].imgName = "card-Diamonds-K"; cards[24].points = 10;
    cards[25].suit = cardSuit::Diamonds; cards[25].value = cardValue::ValueQ; cards[25].imgName = "card-Diamonds-Q"; cards[25].points = 10;

    cards[26].suit = cardSuit::Hearts; cards[26].value = cardValue::Value2; cards[26].imgName = "card-Hearts-2"; cards[26].points = 2;
    cards[27].suit = cardSuit::Hearts; cards[27].value = cardValue::Value3; cards[27].imgName = "card-Hearts-3"; cards[27].points = 3;
    cards[28].suit = cardSuit::Hearts; cards[28].value = cardValue::Value4; cards[28].imgName = "card-Hearts-4"; cards[28].points = 4;
    cards[29].suit = cardSuit::Hearts; cards[29].value = cardValue::Value5; cards[29].imgName = "card-Hearts-5"; cards[29].points = 5;
    cards[30].suit = cardSuit::Hearts; cards[30].value = cardValue::Value6; cards[30].imgName = "card-Hearts-6"; cards[30].points = 6;
    cards[31].suit = cardSuit::Hearts; cards[31].value = cardValue::Value7; cards[31].imgName = "card-Hearts-7"; cards[31].points = 7;
    cards[32].suit = cardSuit::Hearts; cards[32].value = cardValue::Value8; cards[32].imgName = "card-Hearts-8"; cards[32].points = 8;
    cards[33].suit = cardSuit::Hearts; cards[33].value = cardValue::Value9; cards[33].imgName = "card-Hearts-9"; cards[33].points = 9;
    cards[34].suit = cardSuit::Hearts; cards[34].value = cardValue::Value10; cards[34].imgName = "card-Hearts-10"; cards[34].points = 10;
    cards[35].suit = cardSuit::Hearts; cards[35].value = cardValue::ValueA; cards[35].imgName = "card-Hearts-A"; cards[35].points = 1;
    cards[36].suit = cardSuit::Hearts; cards[36].value = cardValue::ValueJ; cards[36].imgName = "card-Hearts-J"; cards[36].points = 10;
    cards[37].suit = cardSuit::Hearts; cards[37].value = cardValue::ValueK; cards[37].imgName = "card-Hearts-K"; cards[37].points = 10;
    cards[38].suit = cardSuit::Hearts; cards[38].value = cardValue::ValueQ; cards[38].imgName = "card-Hearts-Q"; cards[38].points = 10;

    cards[39].suit = cardSuit::Spades; cards[39].value = cardValue::Value2; cards[39].imgName = "card-Spades-2"; cards[39].points = 2;
    cards[40].suit = cardSuit::Spades; cards[40].value = cardValue::Value3; cards[40].imgName = "card-Spades-3"; cards[40].points = 3;
    cards[41].suit = cardSuit::Spades; cards[41].value = cardValue::Value4; cards[41].imgName = "card-Spades-4"; cards[41].points = 4;
    cards[42].suit = cardSuit::Spades; cards[42].value = cardValue::Value5; cards[42].imgName = "card-Spades-5"; cards[42].points = 5;
    cards[43].suit = cardSuit::Spades; cards[43].value = cardValue::Value6; cards[43].imgName = "card-Spades-6"; cards[43].points = 6;
    cards[44].suit = cardSuit::Spades; cards[44].value = cardValue::Value7; cards[44].imgName = "card-Spades-7"; cards[44].points = 7;
    cards[45].suit = cardSuit::Spades; cards[45].value = cardValue::Value8; cards[45].imgName = "card-Spades-8"; cards[45].points = 8;
    cards[46].suit = cardSuit::Spades; cards[46].value = cardValue::Value9; cards[46].imgName = "card-Spades-9"; cards[46].points = 9;
    cards[47].suit = cardSuit::Spades; cards[47].value = cardValue::Value10; cards[47].imgName = "card-Spades-10"; cards[47].points = 10;
    cards[48].suit = cardSuit::Spades; cards[48].value = cardValue::ValueA; cards[48].imgName = "card-Spades-A"; cards[48].points = 1;
    cards[49].suit = cardSuit::Spades; cards[49].value = cardValue::ValueJ; cards[49].imgName = "card-Spades-J"; cards[49].points = 10;
    cards[50].suit = cardSuit::Spades; cards[50].value = cardValue::ValueK; cards[50].imgName = "card-Spades-K"; cards[50].points = 10;
    cards[51].suit = cardSuit::Spades; cards[51].value = cardValue::ValueQ; cards[51].imgName = "card-Spades-Q"; cards[51].points = 10;

    listCards.push_back(cards[0]);
    listCards.push_back(cards[1]);
    listCards.push_back(cards[2]);
    listCards.push_back(cards[3]);
    listCards.push_back(cards[4]);
    listCards.push_back(cards[5]);
    listCards.push_back(cards[6]);
    listCards.push_back(cards[7]);
    listCards.push_back(cards[8]);
    listCards.push_back(cards[9]);
    listCards.push_back(cards[10]);
    listCards.push_back(cards[11]);
    listCards.push_back(cards[12]);
    listCards.push_back(cards[13]);
    listCards.push_back(cards[14]);
    listCards.push_back(cards[15]);
    listCards.push_back(cards[16]);
    listCards.push_back(cards[17]);
    listCards.push_back(cards[18]);
    listCards.push_back(cards[19]);
    listCards.push_back(cards[20]);
    listCards.push_back(cards[21]);
    listCards.push_back(cards[22]);
    listCards.push_back(cards[23]);
    listCards.push_back(cards[24]);
    listCards.push_back(cards[25]);
    listCards.push_back(cards[26]);
    listCards.push_back(cards[27]);
    listCards.push_back(cards[28]);
    listCards.push_back(cards[29]);
    listCards.push_back(cards[30]);
    listCards.push_back(cards[31]);
    listCards.push_back(cards[32]);
    listCards.push_back(cards[33]);
    listCards.push_back(cards[34]);
    listCards.push_back(cards[35]);
    listCards.push_back(cards[36]);
    listCards.push_back(cards[37]);
    listCards.push_back(cards[38]);
    listCards.push_back(cards[39]);
    listCards.push_back(cards[40]);
    listCards.push_back(cards[41]);
    listCards.push_back(cards[42]);
    listCards.push_back(cards[43]);
    listCards.push_back(cards[44]);
    listCards.push_back(cards[45]);
    listCards.push_back(cards[46]);
    listCards.push_back(cards[47]);
    listCards.push_back(cards[48]);
    listCards.push_back(cards[49]);
    listCards.push_back(cards[50]);
    listCards.push_back(cards[51]);
    
    return listCards;
}


/**
 @brief  Deal the cards.  This method fill stacks and vectors to start a new game
 @param  -
 @return -
*/

void Game::dealCards()
{
    int cardsToAssign = 10 * 2; //10 cards for each Player. 2 players.
    int randomCard;
    list<card> listCards = getFullDeckOfCards();
    list<card>::iterator itCard;

    //Deal cards to both players:
    for(int i = 0; i < cardsToAssign; i++)
    {
        itCard = listCards.begin();
        randomCard = RandomHelper::random_int(0, (int)listCards.size() - 1);
        advance(itCard, randomCard); //the list pointer is moved
    
        (i % 2 == 0) ? players[0]->myCards.push_back(*itCard) : players[1]->myCards.push_back(*itCard);
        listCards.erase(itCard);
    }
    
    //First card to see in the Game/Match:
    itCard = listCards.begin();
    randomCard = RandomHelper::random_int(0, (int)listCards.size() - 1);
    advance(itCard, randomCard);
    
    cardsSeen.push(*itCard);
    listCards.erase(itCard);
    
    //Finally, this is the deck of cards (the other cards)
    while(listCards.size() != 0)
    {
        itCard = listCards.begin();
        randomCard = RandomHelper::random_int(0, (int)listCards.size() - 1); //we're simulating a mixed deck of cards
        advance(itCard, randomCard);
        
        deckOfCards.push(*itCard);
        listCards.erase(itCard);
    }
}


/**
 @brief  Initialization of a new game
 @param  -
 @return bool
*/

bool Game::start()
{
    EventHandler *event = EventHandler::getInstance();
    Director *director = Director::getInstance();
    Rect safeArea = director->getSafeAreaRect();
    Size visibleSize = safeArea.size;
    Vec2 origin = safeArea.origin;
    
    AudioEngine::stopAll();
    
    if(director->getRunningScene()->getName() == "gameScene") { //special for next game round
        scene->removeAllChildren();
        director->getEventDispatcher()->removeAllEventListeners();
    }
    
    //Game Scene Background
    Sprite *bg = Sprite::create("backgrounds/gameBackground.png");
    bg->setPosition(Vec2(director->getVisibleOrigin().x + director->getVisibleSize().width/2, director->getVisibleOrigin().y + director->getVisibleSize().height/2));
    bg->setScale(director->getVisibleSize().width / bg->getContentSize().width, director->getVisibleSize().height / bg->getContentSize().height); // 100% x, y
    scene->addChild(bg, 0);
    
    Sprite *rummyLogo = Sprite::create("backgrounds/rummyLogoTransparent.png");
    rummyLogo->setName("rummyLogo");
    rummyLogo->setPosition(bg->getPosition());
    rummyLogo->setScale(visibleSize.width * 0.25f / rummyLogo->getContentSize().width);
    scene->addChild(rummyLogo);
        
    //UI
    UserInterface::destroyInstance();
    UserInterface *ui = UserInterface::getInstance(scene);
    ui->addButton("ui/btnHome.png", Vec2((origin.x + visibleSize.width * 0.05), origin.y + (visibleSize.height * 0.93)), 0.10f, [event]() { event->onTouchButtonHome(); });
  
    // -------- Cards  --------
    //Cards stack
    Sprite *cardStack = Sprite::create("cards/cardStack.png");
    cardStack->setName("cardStack");
    cardStack->setScale(visibleSize.width * 0.115f / cardStack->getContentSize().width);
    cardStack->setPosition(Vec2(rummyLogo->getPositionX() - rummyLogo->getBoundingBox().size.width/2 - cardStack->getBoundingBox().size.width/2 - (visibleSize.width * 0.13f), rummyLogo->getPositionY()));
    scene->addChild(cardStack);

    Label *amountDeckOfCards = ui->addLabel(to_string((int)deckOfCards.size()) , Vec2(cardStack->getPositionX(), cardStack->getPositionY() - cardStack->getBoundingBox().size.height/1.7f), 9, 0, true);
    amountDeckOfCards->setName("amountDeckOfCards");
    
    //First card to see - card background:
    Sprite *cardBg = Sprite::create("cards/cardBackground.png");
    cardBg->setName("cardBackground");
    cardBg->setScale(visibleSize.width * 0.105f / cardBg->getContentSize().width);
    cardBg->setPosition(Vec2(rummyLogo->getPositionX() + rummyLogo->getBoundingBox().size.width/2 + cardBg->getBoundingBox().size.width/2 + (visibleSize.width * 0.13f), rummyLogo->getPositionY()));
    scene->addChild(cardBg);
    
    Sprite *cardToSee = Sprite::create("cards/" + cardsSeen.top().imgName + ".png");
    cardToSee->setName(cardsSeen.top().imgName);
    cardToSee->setScale(visibleSize.width * CARD_SIZE / cardToSee->getContentSize().width);
    cardToSee->setPosition(cardBg->getPosition());
    scene->addChild(cardToSee, (int32_t)cardsSeen.size());
    
    //My cards && my opponent cards
    auto layoutMyCards = Layout::create();
    layoutMyCards->setName("layoutMyCards");
    layoutMyCards->setPosition(Vec2(origin.x + visibleSize.width/2, origin.y - cardToSee->getBoundingBox().size.height/2));
    layoutMyCards->setContentSize(Size(0, 0));
    layoutMyCards->setAnchorPoint(Vec2(0.5, 0.5));
    scene->addChild(layoutMyCards);

    auto layoutOpponentCards = Layout::create();
    layoutOpponentCards->setName("layoutOpponentCards");
    layoutOpponentCards->setPosition(Vec2(origin.x + visibleSize.width/2, origin.y + visibleSize.height + cardToSee->getBoundingBox().size.height/2));
    layoutOpponentCards->setContentSize(Size(0, 0));
    layoutOpponentCards->setAnchorPoint(Vec2(0.5, 0.5));
    scene->addChild(layoutOpponentCards);
    
    vector<card> cardsP1 = players[0]->myCards; //Me
    vector<card> cardsP2 = players[1]->myCards; //Opponent
    
    //Debug:
    //layoutMyCards->setBackGroundColor(Color3B::BLUE);
    //layoutMyCards->setBackGroundColorType(Layout::BackGroundColorType::SOLID);
    //layoutOpponentCards->setBackGroundColor(Color3B::BLUE);
    //layoutOpponentCards->setBackGroundColorType(Layout::BackGroundColorType::SOLID);
    //--
    
    for(int i = 0; i < cardsP1.size(); i++)
    {
        Sprite *c = Sprite::create("cards/" + cardsP1[i].imgName + ".png");
        c->setName(cardsP1[i].imgName);
        c->setAnchorPoint(Vec2(0, 0));
        c->setScale(visibleSize.width * CARD_SIZE / c->getContentSize().width);
        
        if(i == 0) {
            c->setPosition(Vec2(0, 0));
            layoutMyCards->setContentSize(Size(c->getBoundingBox().size.width, c->getBoundingBox().size.height));
        }
        else {
            c->setPosition(Vec2(layoutMyCards->getChildByName(cardsP1[i-1].imgName)->getPositionX() + (c->getBoundingBox().size.width * 0.70f), 0));
            layoutMyCards->setContentSize(Size(layoutMyCards->getContentSize().width + (c->getBoundingBox().size.width * 0.70f), c->getBoundingBox().size.height));
        }
        
        layoutMyCards->addChild(c, i);
    }
    
    players[0]->markCombinationsIfExist(); //mark my combinations if exists
    
    for(int i = 0; i < cardsP2.size(); i++)
    {
        Sprite *c = Sprite::create("cards/cardBack.png"); //the card is hidden because it's opponent's card
        c->setName(cardsP2[i].imgName);
        c->setAnchorPoint(Vec2(0, 0));
        c->setScale(visibleSize.width * CARD_SIZE / c->getContentSize().width);
        
        if(i == 0) {
            c->setPosition(Vec2(0, 0));
            layoutOpponentCards->setContentSize(Size(c->getBoundingBox().size.width, c->getBoundingBox().size.height));
        }
        else {
            c->setPosition(Vec2(layoutOpponentCards->getChildByName(cardsP2[i-1].imgName)->getPositionX() + (c->getBoundingBox().size.width * 0.70f), 0));
            layoutOpponentCards->setContentSize(Size(layoutOpponentCards->getContentSize().width + (c->getBoundingBox().size.width * 0.70f),
                                                     c->getBoundingBox().size.height));
        }
        
        layoutOpponentCards->addChild(c, i);
    }
    // ------------------------

    //Avatars
    //Backgrounds:
    Vec2 layoutMyCardsFinalPos = Vec2(origin.x + visibleSize.width/2, (origin.y + amountDeckOfCards->getPositionY())/2 - layoutMyCards->getBoundingBox().size.height/2);
    Vec2 layoutoppCardsFinalPos = Vec2(origin.x + visibleSize.width/2, (origin.y + visibleSize.height + cardStack->getPositionY() + cardStack->getBoundingBox().size.height/1.7f)/2 + layoutOpponentCards->getBoundingBox().size.height/2);
    
    Sprite *myAvatarBg = Sprite::create("others/avatarBackground.png");
    myAvatarBg->setName("myAvatarBg");
    myAvatarBg->setPosition(Vec2(origin.x + visibleSize.width * 0.93f, layoutMyCardsFinalPos.y + layoutMyCards->getBoundingBox().size.height * 1.1f));
    myAvatarBg->setScale(visibleSize.width * 0.07f / myAvatarBg->getContentSize().width);
    scene->addChild(myAvatarBg);
    
    Sprite *oppAvatarBg = Sprite::create("others/avatarBackground.png");
    oppAvatarBg->setName("opponentAvatarBg");
    oppAvatarBg->setPosition(Vec2(origin.x + visibleSize.width * 0.93f, layoutoppCardsFinalPos.y - layoutOpponentCards->getBoundingBox().size.height * 1.1f));
    oppAvatarBg->setScale(visibleSize.width * 0.07f / oppAvatarBg->getContentSize().width);
    scene->addChild(oppAvatarBg);
    
    //Profile images:
    Texture2D *myTexture = new Texture2D();
    myTexture->autorelease();
    myTexture->initWithImage(players[0]->avatarImg);
    
    Sprite *myAvatar = Sprite::createWithTexture(myTexture);
    myAvatar->setName("myAvatar");
    myAvatar->setScale(myAvatarBg->getScale() - 0.03f);
    myAvatar->setPosition(myAvatarBg->getPosition());
    scene->addChild(myAvatar);
   
    Texture2D *oppTexture = new Texture2D();
    oppTexture->autorelease();
    oppTexture->initWithImage(players[1]->avatarImg);
    
    Sprite *oppAvatar = Sprite::createWithTexture(oppTexture);
    oppAvatar->setName("opponentAvatar");
    oppAvatar->setScale(oppAvatarBg->getScale() - 0.03f);
    oppAvatar->setPosition(oppAvatarBg->getPosition());
    scene->addChild(oppAvatar);
    
    //Turns
    Sprite *avatarMarked = Sprite::create("others/avatarMarked.png");
    avatarMarked->setName("avatarMarked");
    avatarMarked->setPosition((players[0]->myTurn == true) ? myAvatar->getPosition() : oppAvatar->getPosition());
    avatarMarked->setScale(visibleSize.width * 0.07f / avatarMarked->getContentSize().width);
    scene->addChild(avatarMarked);
    
    //Names && points:
    ui->addLabel(players[0]->name, Vec2(myAvatar->getPositionX(), myAvatar->getPositionY() - myAvatar->getBoundingBox().size.height/1.3f), 8, 0, true);
    ui->addLabel(players[1]->name, Vec2(oppAvatar->getPositionX(), oppAvatar->getPositionY() + oppAvatar->getBoundingBox().size.height/1.3f), 8, 0, true);

    Label *myPoints = ui->addLabel(to_string(players[0]->points) + "/100", Vec2(myAvatar->getPositionX(), myAvatar->getPositionY() + myAvatar->getBoundingBox().size.height/1.3), 8, 0, true);
    Label *oppPoints = ui->addLabel(to_string(players[1]->points) + "/100", Vec2(oppAvatar->getPositionX(), oppAvatar->getPositionY() - oppAvatar->getBoundingBox().size.height/1.3), 8, 0, true);
    
    myPoints->setName("labelMyPoints");
    oppPoints->setName("labelMyOpponentPoints");
    //----

    //Event Listener
    event->cardsListener = EventListenerTouchOneByOne::create();
    event->cardsListener->setSwallowTouches(true);
    event->cardsListener->onTouchBegan = CC_CALLBACK_2(EventHandler::onTouchCardBegin, event);
    event->cardsListener->onTouchMoved = CC_CALLBACK_2(EventHandler::onTouchCardMoved, event);
    event->cardsListener->onTouchEnded = CC_CALLBACK_2(EventHandler::onTouchCardEnd, event);
    director->getEventDispatcher()->addEventListenerWithSceneGraphPriority(event->cardsListener, scene);
   
    //Finally, the game starts!
    if(director->getRunningScene()->getName() != "gameScene") //special for next game round
        director->replaceScene(TransitionFade::create(0.5, scene));
    
    Action *actionMyCardsLayout = MoveTo::create(1, layoutMyCardsFinalPos);
    actionMyCardsLayout->setTag(0);
    layoutMyCards->runAction(actionMyCardsLayout);
    
    Action *actionOppCardsLayout = MoveTo::create(1, layoutoppCardsFinalPos);
    actionOppCardsLayout->setTag(1);
    layoutOpponentCards->runAction(actionOppCardsLayout);
    
    return true;
}


/**
 @brief  Find Opponent / Match Scene.
 @param  -
 @return -
*/

void Game::findOpponent()
{
    Director *director = Director::getInstance();
    SQLite *sqlite = SQLite::getInstance();
    
    //Clean the listeners
    director->getEventDispatcher()->removeAllEventListeners();
      
    //Match scene
    auto replaceScene = CallFunc::create([director]() {
        
        Scene *matchScene = Scene::create();
        matchScene->setName("matchScene");
        UserInterface::destroyInstance();
        director->replaceScene(TransitionFade::create(0.5, matchScene));
        
        //Scene UI
        UserInterface *ui = UserInterface::getInstance(matchScene);
        ui->addBackground("backgrounds/matchBackground.png");
        ui->addWindow(WINDOW_NEWMATCH);
    });
    
    auto findOpponent = CallFunc::create([sqlite]() {
        
        //Login to Nakama (when I log in, connect to the server, and I get my info... I search an opponent automatically)
        NakamaServer *nakama = NakamaServer::getInstance();
        if(sqlite->getLoginMethod() == "Facebook" and !nakama->isConnected())
        {
            if(sdkbox::PluginFacebook::isLoggedIn())
                nakama->loginWithFacebook();
            else
                sdkbox::PluginFacebook::login();
        }
        else if(sqlite->getLoginMethod() == "Mail" and !nakama->isConnected())
        {
            nakama->loginWithEmail(sqlite->getNakamaUser(), sqlite->getNakamaPassword());
        }
        else if(sqlite->getLoginMethod() == "GameCenter" and !nakama->isConnected())
        {
            if(sdkbox::PluginSdkboxPlay::isSignedIn())
                nakama->loginWithGameCenter();
            else
                sdkbox::PluginSdkboxPlay::signin();
        }
        else if(sqlite->getLoginMethod() == "Google" and !nakama->isConnected())
        {
            if(sdkbox::PluginSdkboxPlay::isSignedIn())
                nakama->loginWithGoogle();
            else
                sdkbox::PluginSdkboxPlay::signin();
        }
        else if(sqlite->getLoginMethod() == "Apple" and !nakama->isConnected())
        {
            #if !defined(__ANDROID__) //Only iOS
                //NOTE: PluginSignInWithApple is in testing phrase - it means: we don't have enough methods yet (example: isConnected, signout, etc)
                if(AppleAuthHandler::getInstance()->userId.empty()) //If i'm not connected (I don't have my Apple userId)
                    sdkbox::PluginSignInWithApple::sign();
                else
                    nakama->loginWithApple();
            #endif
        }
        else if((sqlite->getLoginMethod() == "Facebook" or sqlite->getLoginMethod() == "Mail" or sqlite->getLoginMethod() == "GameCenter"
                 or sqlite->getLoginMethod() == "Google" or sqlite->getLoginMethod() == "Apple")
                and nakama->isConnected()) //if i'm already connected, i just search an opponent..
            nakama->addMatchMaker();
    });
    
    director->getRunningScene()->runAction(Sequence::create(replaceScene, DelayTime::create(0.5), findOpponent, NULL));
}


/**
 @brief  Stop the game
 @param  -
 @return -
*/

void Game::stop()
{
    Game *game = Game::getInstance();
    Director *director = Director::getInstance();
  
    //Stop game
    director->getEventDispatcher()->removeAllEventListeners();
    director->getScheduler()->unscheduleAllForTarget(game);
}


/**
 @brief  This method is called at the end of a match / round, for show the cards & calculate / set the scores of each player
 @param  playerWinner: player who has finished the round
         gameModeFinished: game mode finished ( it could be "Gin", "Knock" or "Draw" )
 @return -
*/

void Game::finishGameRound(Player *playerWinner, string gameModeFinished)
{
    Director *director = Director::getInstance();
    Rect safeArea = director->getSafeAreaRect();
    Size visibleSize = safeArea.size;
    Vec2 origin = safeArea.origin;
    EventHandler *eventHandler = EventHandler::getInstance();
    UserInterface *ui = UserInterface::getInstance(scene);
    
    director->getEventDispatcher()->pauseEventListenersForTarget(scene); //stop touching cards
    
    //Show cards:
    players[0]->showMyCards();
    players[1]->showMyCards();
    
    if(playerWinner == NULL and gameModeFinished == "Draw")
    {
        ui->addButton("ui/btnContinue.png", scene->getChildByName("rummyLogo")->getPosition(), 0.20f, [eventHandler]() { eventHandler->onTouchButtonContinue(); });
        return;
    }
    
    //Calculate and set points:
    Layout *pWinnerLayout, *pLoserLayout;
    Player *playerLoser;
    Label *pWinnerLabelPoints, *pLoserLabelPoints;
    
    if(playerWinner->pid == players[0]->pid) //Me
    {
        pWinnerLayout = (Layout*)scene->getChildByName("layoutMyCards");
        pLoserLayout = (Layout*)scene->getChildByName("layoutOpponentCards");
        playerLoser = players[1];
        pWinnerLabelPoints = (Label*)scene->getChildByName("layoutUI")->getChildByName("labelMyPoints");
        pLoserLabelPoints = (Label*)scene->getChildByName("layoutUI")->getChildByName("labelMyOpponentPoints");
    }
    else
    {
        pWinnerLayout = (Layout*)scene->getChildByName("layoutOpponentCards");
        pLoserLayout = (Layout*)scene->getChildByName("layoutMyCards");
        playerLoser = players[0];
        pWinnerLabelPoints = (Label*)scene->getChildByName("layoutUI")->getChildByName("labelMyOpponentPoints");
        pLoserLabelPoints = (Label*)scene->getChildByName("layoutUI")->getChildByName("labelMyPoints");
    }

    /*
    Point Rules:
     - If the player who ended the match did GIN, he wins the sum of points of the no-combinated opponent's cards, plus 25 points (25 extra points is a gin bonus).
     - If the player who ended the match did KNOCK, he wins the DIFFERENCE of sum-points of the no-combinated opponent's cards and his own no-combinated cards.
     - Undercut: Occurs when the defending player has a deadwood count lower than or equal to that of the knocking player. In this case, the defender scores an undercut bonus of 25 points plus the difference in deadwood in the knocking player's hand.
     
     The first player who get 100 or more points, WIN the entire match.
    */
    
    vector<card> pWinnerCombCards;
    vector<card> pWinnerNoCombCards, pLoserNoCombCards;
    
    //We build sub-vectors for separate the info:
    for(auto card : playerWinner->myCards)
    {
        Sprite *spriteCard = (Sprite*)pWinnerLayout->getChildByName(card.imgName);
        if(spriteCard->getChildByName("circleCardComb") != NULL)
            pWinnerCombCards.push_back(card);
        else
            pWinnerNoCombCards.push_back(card);
    }
    
    for(auto card : playerLoser->myCards)
    {
        Sprite *spriteCard = (Sprite*)pLoserLayout->getChildByName(card.imgName);
        if(spriteCard->getChildByName("circleCardComb") == NULL)
            pLoserNoCombCards.push_back(card);
    }

    //Points of cards no combinated :
    int pWinnernoCombPoints = 0, pLosernoCombPoints = 0;
    for(auto card : pWinnerNoCombCards)
        pWinnernoCombPoints += card.points;
    
    for(auto card : pLoserNoCombCards)
        pLosernoCombPoints += card.points;
    
    //Deadwoods
    float cardStackPosX = scene->getChildByName("cardStack")->getPositionX();
    Vec2 posBoxDeadWoodWinner, posBoxDeadWoodLoser;
    scene->removeChildByName("cardStack");
    scene->getChildByName("layoutUI")->removeChildByName("amountDeckOfCards");
    
    Sprite *dwWinner = Sprite::create("others/points_deadWood.png");
    dwWinner->setName("points_deadWood_winner");
    dwWinner->setScale(visibleSize.width * 0.1f / dwWinner->getContentSize().width);

    Sprite *dwLoser = Sprite::create("others/points_deadWood.png");
    dwLoser->setName("points_deadWood_loser");
    dwLoser->setScale(visibleSize.width * 0.1f / dwLoser->getContentSize().width);
    
    if(pWinnerLayout->getPositionY() > pLoserLayout->getPositionY())
    {
        posBoxDeadWoodWinner = Vec2(cardStackPosX, (origin.y + visibleSize.height - pWinnerLayout->getBoundingBox().size.height/2) - pWinnerLayout->getBoundingBox().size.height/2 - dwWinner->getBoundingBox().size.height/1.8f);
        
        posBoxDeadWoodLoser = Vec2(cardStackPosX, (origin.y + pLoserLayout->getBoundingBox().size.height/2) + pLoserLayout->getBoundingBox().size.height/2 + dwLoser->getBoundingBox().size.height/1.6f);
    }
    else
    {
        posBoxDeadWoodLoser = Vec2(cardStackPosX, (origin.y + visibleSize.height - pLoserLayout->getBoundingBox().size.height/2) - pLoserLayout->getBoundingBox().size.height/2 - dwLoser->getBoundingBox().size.height/1.8f);
        
        posBoxDeadWoodWinner = Vec2(cardStackPosX, (origin.y + pWinnerLayout->getBoundingBox().size.height/2) + pWinnerLayout->getBoundingBox().size.height/2 + dwWinner->getBoundingBox().size.height/1.6f);
    }
    
    dwWinner->setPosition(posBoxDeadWoodWinner);
    dwLoser->setPosition(posBoxDeadWoodLoser);
    scene->addChild(dwWinner);
    scene->addChild(dwLoser);
    
    ui->addLabel(to_string(pWinnernoCombPoints), Vec2(dwWinner->getPositionX(), dwWinner->getPositionY() - dwWinner->getBoundingBox().size.height/3.7f), 10, 0, true);
    ui->addLabel(to_string(pLosernoCombPoints), Vec2(dwLoser->getPositionX(), dwLoser->getPositionY() - dwLoser->getBoundingBox().size.height/3.7f), 10, 0, true);

    //Now, we check gin / knock / undercut :
    int points = 0; //new points to be added to the winner
    Label *labelPoints;
    if(pWinnerCombCards.size() == 10) //Gin
    {
        for(auto card : pLoserNoCombCards)
            points += card.points;
        
        points += 25; //bonus 25+ points gin
        labelPoints = pWinnerLabelPoints;
        playerWinner->points += points;
        
        Sprite *ginBonusSprite = Sprite::create("others/points_ginBonus.png");
        ginBonusSprite->setName("points_ginBonus");
        ginBonusSprite->setPosition(dwWinner->getPositionX() + dwWinner->getBoundingBox().size.width * 1.2f, dwWinner->getPositionY());
        ginBonusSprite->setScale(visibleSize.width * 0.1f / ginBonusSprite->getContentSize().width);
        scene->addChild(ginBonusSprite);
        ui->addLabel("+25", Vec2(ginBonusSprite->getPositionX(), ginBonusSprite->getPositionY() - ginBonusSprite->getBoundingBox().size.height/3.7f), 10, 0, true);
    }
    else
    {
        if(pLosernoCombPoints <= pWinnernoCombPoints) //Undercut
        {
            points = 25 + abs(pWinnernoCombPoints - pLosernoCombPoints);
            labelPoints = pLoserLabelPoints;
            playerLoser->points += points;
            
            Sprite *underCutSprite = Sprite::create("others/points_underCut.png");
            underCutSprite->setName("points_underCut");
            underCutSprite->setPosition(dwLoser->getPositionX() + dwLoser->getBoundingBox().size.width * 1.2f, dwLoser->getPositionY());
            underCutSprite->setScale(visibleSize.width * 0.1f / underCutSprite->getContentSize().width);
            scene->addChild(underCutSprite);
            ui->addLabel("+25", Vec2(underCutSprite->getPositionX(), underCutSprite->getPositionY() - underCutSprite->getBoundingBox().size.height/3.7f), 10, 0, true);
        }
        else
        {
            points = abs(pWinnernoCombPoints - pLosernoCombPoints);
            labelPoints = pWinnerLabelPoints;
            playerWinner->points += points;
        }
    }
    
    string labelPointsStr = labelPoints->getString();
    int oldPoints = stoi(labelPointsStr.substr(0, labelPointsStr.find("/")));
    int currentPoints = oldPoints + points;
    
    auto actionScore = ActionFloat::create(1.0f, oldPoints, currentPoints, [labelPoints](int value) {
        labelPoints->setString(to_string(value) + "/100");
    });
    
    labelPoints->runAction(actionScore);
    Button *btnContinue = (Button*)ui->addButton("ui/btnContinue.png", scene->getChildByName("rummyLogo")->getPosition(), 0.16f, [eventHandler]() { eventHandler->onTouchButtonContinue(); });

    if(currentPoints >= 100) //game finished
    {
        btnContinue->setEnabled(false);
        ui->addWindow(WINDOW_RESULTS, (playerWinner->points >= 100) ? playerWinner->name : playerLoser->name);
    }
    
    if(IAPHandler::getInstance()->adsPurchased() == false)
    {
        sdkbox::PluginAdMob::show("interstitial");
        sdkbox::PluginAdMob::cache("interstitial");
    }
}


/**
 @brief  Destructor
 @param  -
 @return -
*/

Game::~Game()
{
    delete scene;
    delete players[0];
    delete players[1];
    
    m_pInstance = NULL;
}
