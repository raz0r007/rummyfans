#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "Card.h"

#define MAX_CARDS_PER_PLAYER 11
#define MIN_CARDS_PER_PLAYER 10

using namespace std;
using namespace cocos2d;

class Player
{
public:
    string pid;
    string name;
    
    string avatarUrl;
    Image *avatarImg;
    
    int points; //current points in a specific match
    vector<card> myCards; //my cards in a specific match
    
    bool readyForPlay; //flag that indicates at the beginning of the match if i'm ready for start the match
    bool myTurn; //flag that indicates if currently is my turn or not
    
    Player();
    ~Player();

    void fillDefaultPlayerInfo();
    
    bool addCard(card c, int posInVecToAddCard);
    bool removeCard(card c);
    bool reorderCard(card c, int posInVecToMoveCard);
    
    void markCombinationsIfExist(bool circlesVisibles = true, card *cardToAvoid = NULL);
    
    void showMyCards();
};

#endif
