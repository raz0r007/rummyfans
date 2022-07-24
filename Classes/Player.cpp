#include "Player.h"
#include "Game.h"
#include "EventHandler.h"

/**
 @brief  Constructor
 @param  -
 @return -
*/

Player::Player()
{
    pid = "";
    name = "";
    avatarUrl = "";
    avatarImg = NULL;
    readyForPlay = false;
    myTurn = false;
    
    points = 0;
}


/**
 @brief  Fill default player info. Normally this method is used when we can't login to Nakama Server so we invent MY info in order to play with a bot
 @param  -
 @return -
*/

void Player::fillDefaultPlayerInfo()
{
    pid = "1";
    name = "Me";
    avatarUrl = "https://anstra.net/rummyfans/defaultAvatar.png";
    
    Image *m = new Image;
    m->initWithImageFile("others/defaultAvatar.png");
    avatarImg = m;
    
    points = 0;
    readyForPlay = true;
    myTurn = false;
}


/**
 @brief  Add card to the card list. This method is used in the gameplay when a player grab a new card.
 @param  c: Card to add
         posInVecToAddCard: Position in the vector where the card will be added (with this value we also can deduce the final newCard position in the layer)
 @return bool
*/

bool Player::addCard(card c, int posInVecToAddCard)
{
    Game *game = Game::getInstance();
    EventHandler *eventHandler = EventHandler::getInstance();
    Director *director = Director::getInstance();
    Rect safeArea = director->getSafeAreaRect();
    Size visibleSize = safeArea.size;
    Sprite *cardTouching;
    
    if(myCards.empty() or (myCards.size()+1) > MAX_CARDS_PER_PLAYER)
        return false;
    
    string layoutName;
    if(pid == game->players[0]->pid) //Me
    {
        layoutName = "layoutMyCards";
        cardTouching = eventHandler->cardTouching;
    }
    else //Opponent
    {
        layoutName = "layoutOpponentCards";
        
        if(c.imgName == game->deckOfCards.top().imgName) //deck of cards
            cardTouching = (Sprite*)game->scene->getChildByName("cardStack");
        else //last card seen
            cardTouching = (Sprite*)game->scene->getChildByName(game->cardsSeen.top().imgName);
    }
        
    auto layout = game->scene->getChildByName(layoutName);
    
    //Resize Layer
    Action *actionLayout = ResizeTo::create(0.3, Size(layout->getContentSize().width + (layout->getChildByName(myCards[0].imgName)->getBoundingBox().size.width * 0.70f), layout->getChildByName(myCards[0].imgName)->getBoundingBox().size.height));
    actionLayout->setTag((layoutName == "layoutMyCards") ? 0 : 1);
    layout->runAction(actionLayout);
    
    //Add the new card
    Vec2 newCardPosition;
    Sprite *newCard = Sprite::create((layoutName == "layoutMyCards")? "cards/" + c.imgName + ".png" : "cards/cardBack.png");
    newCard->setName(c.imgName);
    newCard->setAnchorPoint(Vec2(0, 0));
    newCard->setScale(visibleSize.width * CARD_SIZE / newCard->getContentSize().width);
    newCard->setPosition(layout->convertToNodeSpace(Vec2(cardTouching->getPositionX() - cardTouching->getBoundingBox().size.width/2, cardTouching->getPositionY() - cardTouching->getBoundingBox().size.height/2))); //position adapted to anchor point [0,0]

    if(posInVecToAddCard == myCards.size())
        newCardPosition = Vec2(layout->getChildByName(myCards[posInVecToAddCard-1].imgName)->getPositionX() + (newCard->getBoundingBox().size.width * 0.70f), layout->getChildByName(myCards[posInVecToAddCard-1].imgName)->getPositionY());
    else
        newCardPosition = layout->getChildByName(myCards[posInVecToAddCard].imgName)->getPosition();
    
    //Fix: GlobalOrder - opponent card overlap
    layout->setGlobalZOrder(99);
    auto layoutGlobalOff = CallFunc::create([layout]() { layout->setGlobalZOrder(0); });
    //
    
    layout->addChild(newCard, posInVecToAddCard);
    
    Action *actionNewCard = Sequence::create(MoveTo::create(0.5, newCardPosition), DelayTime::create(0.25), layoutGlobalOff, NULL);
    actionNewCard->setTag((layoutName == "layoutMyCards") ? 0 : 1);
    newCard->runAction(actionNewCard);
    
    //Re-order next cards
    for(int i = posInVecToAddCard; i < myCards.size(); i++)
    {
        Vec2 newPosition = Vec2(layout->getChildByName(myCards[i].imgName)->getPositionX() + (layout->getChildByName(myCards[i].imgName)->getBoundingBox().size.width * 0.70f), layout->getChildByName(myCards[i].imgName)->getPositionY());
        
        layout->getChildByName(myCards[i].imgName)->setLocalZOrder(layout->getChildByName(myCards[i].imgName)->getLocalZOrder() + 1);
        
        Action *actionCard = MoveTo::create(0.3, newPosition);
        actionCard->setTag((layoutName == "layoutMyCards") ? 0 : 1);
        layout->getChildByName(myCards[i].imgName)->runAction(actionCard);
    }
  
    //Update vector
    vector<card>::iterator itNewCard = myCards.begin();
    advance(itNewCard, posInVecToAddCard);
    myCards.insert(itNewCard, c);
    
    //Check new combinations
    if(layoutName == "layoutMyCards")
        markCombinationsIfExist();
    
    return true;
}


/**
 @brief  Re-order a card in my card list. This is used by the player for re-order their cards
 @param  c: Card to re-order
         posInVecToMoveCard: Position in the vector where the card will be moved (with this value we also can deduce the final Card position in the layer)
 @return bool
*/

bool Player::reorderCard(card c, int posInVecToMoveCard)
{
    Game *game = Game::getInstance();
    Director *director = Director::getInstance();
    Rect safeArea = director->getSafeAreaRect();
    Size visibleSize = safeArea.size;
    
    vector<card>::iterator itCardToMove = find_if(myCards.begin(), myCards.end(), [c](const card &card){ return card.imgName == c.imgName; });
    if(itCardToMove == myCards.end())
        return false;
    
    string layoutName;
    if(pid == game->players[0]->pid) //Me
        layoutName = "layoutMyCards";
    else //Opponent
        layoutName = "layoutOpponentCards";
        
    auto layout = game->scene->getChildByName(layoutName);
    
    //Move the card to the new position
    Action *actionCard = MoveTo::create(0.3, layout->getChildByName(myCards[(posInVecToMoveCard == myCards.size())? posInVecToMoveCard - 1 : posInVecToMoveCard].imgName)->getPosition());
    actionCard->setTag((layoutName == "layoutMyCards") ? 0 : 1);
    layout->getChildByName(c.imgName)->runAction(actionCard);
    layout->getChildByName(c.imgName)->setLocalZOrder(posInVecToMoveCard);
    
    //Re-order the other cards
    vector<card>::iterator itCardNewPos = myCards.begin();
    advance(itCardNewPos, posInVecToMoveCard);
    
    if(itCardNewPos < itCardToMove) //move a card from the right to the left
    {
        while(itCardNewPos != myCards.end() and itCardNewPos < itCardToMove)
        {
            Vec2 newPosition = Vec2(layout->getChildByName((*itCardNewPos).imgName)->getPositionX() + (layout->getChildByName((*itCardNewPos).imgName)->getBoundingBox().size.width * 0.70f), layout->getChildByName((*itCardNewPos).imgName)->getPositionY());
            
            layout->getChildByName((*itCardNewPos).imgName)->setLocalZOrder(layout->getChildByName((*itCardNewPos).imgName)->getLocalZOrder() + 1);
            
            Action *actionCardPos = MoveTo::create(0.3, newPosition);
            actionCardPos->setTag((layoutName == "layoutMyCards") ? 0 : 1);
            layout->getChildByName((*itCardNewPos).imgName)->runAction(actionCardPos);
            
            itCardNewPos++;
        }
    }
    else if(itCardNewPos > itCardToMove) //move a card from the left to the right
    {
        while(itCardNewPos != myCards.end() and itCardNewPos > itCardToMove)
        {
            Vec2 newPosition = Vec2(layout->getChildByName((*itCardNewPos).imgName)->getPositionX() - (layout->getChildByName((*itCardNewPos).imgName)->getBoundingBox().size.width * 0.70f), layout->getChildByName((*itCardNewPos).imgName)->getPositionY());
            
            layout->getChildByName((*itCardNewPos).imgName)->setLocalZOrder(layout->getChildByName((*itCardNewPos).imgName)->getLocalZOrder() - 1);
            
            Action *actionNewPos = MoveTo::create(0.3, newPosition);
            actionNewPos->setTag((layoutName == "layoutMyCards") ? 0 : 1);
            layout->getChildByName((*itCardNewPos).imgName)->runAction(actionNewPos);
            
            itCardNewPos--;
        }
    }
    //
  
    //Re-order the vector (logic: https://coliru.stacked-crooked.com/a/5c31007000b9eeba)
    int oldIndex = (int)distance(myCards.begin(), itCardToMove);
    int newIndex = posInVecToMoveCard;
    if (oldIndex > newIndex)
        rotate(myCards.rend() - oldIndex - 1, myCards.rend() - oldIndex, myCards.rend() - newIndex);
    else
        rotate(myCards.begin() + oldIndex, myCards.begin() + oldIndex + 1, myCards.begin() + newIndex + 1);
    
    //Check new combinations
    if(layoutName == "layoutMyCards")
        markCombinationsIfExist();
    
    return true;
}


/**
 @brief  Remove card from the card list. This method is used in the gameplay when a player drop a card.
 @param  c: Card to remove
 @return bool
*/

bool Player::removeCard(card c)
{
    Game *game = Game::getInstance();
    vector<card>::iterator itCardToRemove = find_if(myCards.begin(), myCards.end(), [c](const card &card){ return card.imgName == c.imgName; });
    if(itCardToRemove == myCards.end() or (myCards.size()-1) < MIN_CARDS_PER_PLAYER)
        return false;

    string layoutName;
    if(pid == game->players[0]->pid) //Me
        layoutName = "layoutMyCards";
    else //Opponent
        layoutName = "layoutOpponentCards";
        
    auto layout = game->scene->getChildByName(layoutName);
    
    //Resize Layer (the layer has always at least 1 card)
    
    Action *actionLayout = ResizeTo::create(0.3, Size(layout->getContentSize().width - (layout->getChildByName(c.imgName)->getBoundingBox().size.width * 0.70f), layout->getChildByName(c.imgName)->getBoundingBox().size.height));
    actionLayout->setTag((layoutName == "layoutMyCards") ? 0 : 1);
    layout->runAction(actionLayout);
    
    //Re-order next cards
    vector<card>::iterator itMyCards = itCardToRemove;
    itMyCards++;
    
    while(itMyCards != myCards.end())
    {
        Vec2 newPosition = Vec2(layout->getChildByName((*itMyCards).imgName)->getPositionX() - (layout->getChildByName((*itMyCards).imgName)->getBoundingBox().size.width * 0.70f), layout->getChildByName((*itMyCards).imgName)->getPositionY());

        layout->getChildByName((*itMyCards).imgName)->setLocalZOrder(layout->getChildByName((*itMyCards).imgName)->getLocalZOrder() - 1);
        
        Action *actionCard = MoveTo::create(0.3, newPosition);
        actionCard->setTag((layoutName == "layoutMyCards") ? 0 : 1);
        layout->getChildByName((*itMyCards).imgName)->runAction(actionCard);

        itMyCards++;
    }
    
    //Remove card
    layout->removeChildByName(c.imgName);
    myCards.erase(itCardToRemove);
    
    //Check new combinations
    if(layoutName == "layoutMyCards")
        markCombinationsIfExist();
    
    return true;
}


/**
 @brief  Check & mark on my cards if there are combinations that we should highlight in a  circle
 @param  circlesVisibles: if the circles should be visibles or not
         cardToAvoid: If I want to avoid a specific card for calculate combinations
 @return -
*/

void Player::markCombinationsIfExist(bool circlesVisibles, card *cardToAvoid)
{
    string layoutName;
    Game *game = Game::getInstance();
    Director *director = Director::getInstance();
    Rect safeArea = director->getSafeAreaRect();
    Size visibleSize = safeArea.size;
        
    if(pid == game->players[0]->pid) //Me
        layoutName = "layoutMyCards";
    else //Opponent
        layoutName = "layoutOpponentCards";
    
    auto layout = game->scene->getChildByName(layoutName);

    /*
    There're 2 possible combinations in Rummy (game mode: Gin Rummy):
        - byValue: it means 3 or more cards with the same value.
        - bySuit: it means 3 or more cards with the same suit AND these have to be staggered
    */
    
    //Detect combinations:
    string combSearching; //it could be 'byValue' or 'bySuit'
    vector<card> possibleCardsComb;
    vector<vector<card>> listCardsComb;
    int combStartIndex;
    bool groupedCard;
    vector<card> cardsToAnalyze;
    
    for(auto card : myCards)
    {
        if(cardToAvoid != NULL and cardToAvoid->imgName == card.imgName)
            continue;
        
        cardsToAnalyze.push_back(card);
    }
    
    for(int i = 0; i < cardsToAnalyze.size(); i++)
    {
        groupedCard = false;
        for(int j = 0; j < listCardsComb.size() and groupedCard == false; j++)
        {
            for(int k = 0; k < listCardsComb[j].size() and groupedCard == false; k++)
            {
                if(listCardsComb[j][k].imgName == cardsToAnalyze[i].imgName)
                    groupedCard = true;
            }
        }
        
        if(groupedCard == false)
            possibleCardsComb.push_back(cardsToAnalyze[i]);
        else
            continue;
        
        if(possibleCardsComb.size() == 1)
        {
            combStartIndex = i;
            continue;
        }
        else if(possibleCardsComb.size() == 2)
        {
            if(possibleCardsComb[0].value == possibleCardsComb[1].value)
                combSearching = "byValue";
            else if(possibleCardsComb[0].suit == possibleCardsComb[1].suit and (abs((int)possibleCardsComb[1].value - (int)possibleCardsComb[0].value) == 1))
                combSearching = "bySuit";
            else
            {
                i = combStartIndex;
                possibleCardsComb.clear();
            }
        }
        else if(possibleCardsComb.size() > 2)
        {
            if((combSearching == "byValue" and possibleCardsComb[possibleCardsComb.size()-1].value != possibleCardsComb[possibleCardsComb.size()-2].value) or
               (combSearching == "bySuit" and (possibleCardsComb[possibleCardsComb.size()-1].suit != possibleCardsComb[possibleCardsComb.size()-2].suit or (abs((int)possibleCardsComb[possibleCardsComb.size()-1].value - (int)possibleCardsComb[possibleCardsComb.size()-2].value) != 1))))
            {
                possibleCardsComb.pop_back();
                if(possibleCardsComb.size() >= 3)
                    listCardsComb.push_back(possibleCardsComb);
                
                i = combStartIndex;
                possibleCardsComb.clear();
            }
            else if(i == (cardsToAnalyze.size() - 1))
                listCardsComb.push_back(possibleCardsComb);
        }
    }

    
    //Clear circles
    for(auto card : myCards)
    {
       Sprite *spriteCard = (Sprite*)layout->getChildByName(card.imgName);

       if(spriteCard->getChildByName("circleCardComb") != NULL)
           spriteCard->removeChildByName("circleCardComb");
    }
    
    //Mark new combinations:
    for(int i = 0; i < listCardsComb.size(); i++)
    {
        string circleCardCombImg = "circleCardComb" + to_string(i+1); //don't forget that we can have 3 groups/combinations as max.
        for(auto card : listCardsComb[i])
        {
            Sprite *spriteCard = (Sprite*)layout->getChildByName(card.imgName);
            Sprite *circleCard = Sprite::create("others/" + circleCardCombImg + ".png");
      
            circleCard->setName("circleCardComb");
            circleCard->setVisible(circlesVisibles);
            circleCard->setScale(5.0f);
            circleCard->setPosition(spriteCard->getContentSize().width * 0.125f, spriteCard->getContentSize().height * 0.905f);
            spriteCard->addChild(circleCard);
        }
    }
}


/**
 @brief  Show my current cards. This method is used when the game is finished and both players should show their cards
 @param  -
 @return -
*/

void Player::showMyCards()
{
    Game *game = Game::getInstance();
    Layout *layout;
    Vec2 posLayoutEnd;
    
    Director *director = Director::getInstance();
    Rect safeArea = director->getSafeAreaRect();
    Size visibleSize = safeArea.size;
    Vec2 origin = safeArea.origin;

    if(pid == game->players[0]->pid) //Me
    {
        layout = (Layout*)game->scene->getChildByName("layoutMyCards");
        posLayoutEnd = Vec2(layout->getPositionX(), origin.y + layout->getChildByName(myCards[0].imgName)->getBoundingBox().size.height/2);
    }
    else
    {
        layout = (Layout*)game->scene->getChildByName("layoutOpponentCards");
        posLayoutEnd = Vec2(layout->getPositionX(), origin.y + visibleSize.height - layout->getChildByName(myCards[0].imgName)->getBoundingBox().size.height/2);
        markCombinationsIfExist(); //mark my opponent combinations
    }
    
    /*
     Re-order cards (we need to separate combinated cards and not combinated cards)
     Note: We don't use 'reorderCard' because of some sync problems
    */
        
    for(int i = 0; i < myCards.size(); i++)
        layout->getChildByName(myCards[i].imgName)->setTag(i);
        
    vector<card> auxMyCards = myCards; //we use aux var because we're modifying inside the 'for' the vector 'myCards'
    for(auto c : auxMyCards)
    {
        Sprite *spriteCard = (Sprite*)layout->getChildByName(c.imgName);
        if(spriteCard->getChildByName("circleCardComb") == NULL) //if the card is not combinated we need to move it at the end of the vector
        {
            vector<card>::iterator itCard;
            itCard = find_if(myCards.begin(), myCards.end(), [c](const card &card){ return card.imgName == c.imgName; });
                
            //Re-order the vector (logic: https://coliru.stacked-crooked.com/a/5c31007000b9eeba)
            int oldIndex = (int)distance(myCards.begin(), itCard);
            int newIndex = (int)myCards.size() - 1; //move card no combinated at the end of the vector
            if (oldIndex > newIndex)
                rotate(myCards.rend() - oldIndex - 1, myCards.rend() - oldIndex, myCards.rend() - newIndex);
            else
                rotate(myCards.begin() + oldIndex, myCards.begin() + oldIndex + 1, myCards.begin() + newIndex + 1);
        }
    }
        
    //Update sprite names && textures
    for(int i = 0; i < myCards.size(); i++)
    {
        Sprite *spriteCard = (Sprite*)layout->getChildByTag(i);
            
        spriteCard->setTexture("cards/" + myCards[i].imgName + ".png");
        spriteCard->setName(myCards[i].imgName);
    }
    
    //Because of we changed Textures, we need to re-calculate & re-mark combinations and mark no-combinations
    for(auto card : myCards)
    {
        Sprite *spriteCard = (Sprite*)layout->getChildByName(card.imgName);
        if(spriteCard->getChildByName("circleCardComb") != NULL)
            spriteCard->removeChildByName("circleCardComb");
    }
    
    markCombinationsIfExist();
    
    bool combinatedCards = false;
    bool noCombinatedCards = false;
    for(auto card : myCards)
    {
        Sprite *spriteCard = (Sprite*)layout->getChildByName(card.imgName);
        if(spriteCard->getChildByName("circleCardComb") == NULL)
        {
            Sprite *noCombSym = Sprite::create("others/noCombSymbol.png");
            noCombSym->setName("noCombSymbol");
            noCombSym->setScale(5.0f);
            noCombSym->setPosition(spriteCard->getContentSize().width * 0.125f, spriteCard->getContentSize().height/2);
            spriteCard->addChild(noCombSym);
            
            noCombinatedCards = true;
        }
        else
            combinatedCards = true;
    }
    
    //Finally, if we've combinated and no-combinated cards, we separate each group
    if(combinatedCards == true and noCombinatedCards == true)
    {
        for(int i = 0; i < myCards.size(); i++)
        {
            if(i != 0 and i != myCards.size()-1) //if the card isn't the first or last card..
            {
                Sprite *spriteCard = (Sprite*)layout->getChildByName(myCards[i].imgName);
                if(spriteCard->getChildByName("circleCardComb") != NULL)
                {
                    Action *actionCardComb = MoveTo::create(0.3, Vec2(spriteCard->getPositionX() - (spriteCard->getBoundingBox().size.width * 0.05f * i), spriteCard->getPositionY()));
                    actionCardComb->setTag((layout->getName() == "layoutMyCards") ? 0 : 1);
                    spriteCard->runAction(actionCardComb);
                }
                else
                {
                    Action *actionCardNoComb = MoveTo::create(0.3, Vec2(spriteCard->getPositionX() + (spriteCard->getBoundingBox().size.width * 0.05f * (myCards.size() - i - 1)), spriteCard->getPositionY()));
                    actionCardNoComb->setTag((layout->getName() == "layoutMyCards") ? 0 : 1);
                    spriteCard->runAction(actionCardNoComb);
                }
            }
        }
    }
    
    Action *actionLayout = MoveTo::create(0.5, posLayoutEnd);
    actionLayout->setTag((layout->getName() == "layoutMyCards") ? 0 : 1);
    layout->runAction(actionLayout);
}


/**
 @brief  Destructor
 @param  -
 @return -
*/

Player::~Player()
{
}
