#ifndef __BOT_H__
#define __BOT_H__

#include "Card.h"

string getRandomBotName();

using namespace std;
using namespace cocos2d;
using namespace cocos2d::experimental;

class Bot : public Scene
{
private:
    static Bot* m_pInstance;
    
public:
    string botName;
    bool isPlaying;
    
    Bot();
    ~Bot();
    static Bot* getInstance();
    static void destroyInstance();
  
    bool create(string name, string avatarUrl);
    bool isMyTurn();
    bool tryToPlay();
    
    void grabCard(string from);
    void orderMyCardsByValue();
    void orderMyCardsBySuit();
    void leaveCard(card cardToLeave);
    void finishGameRound(card cardToFinish, string mode);
    
    void moveCardInVector(vector<card>::iterator itCardToMove, int newPosition);
};

#endif
