#ifndef __CARD_H__
#define __CARD_H__

#include "cocos2d.h" //for Android platform

using namespace std;

#define CARD_SIZE 0.095f //Card size based on screen width (the value is used for scale each card sprite)

enum class cardSuit
{
    Clubs      = 0,
    Diamonds   = 1,
    Hearts     = 2,
    Spades     = 3
};

enum class cardValue
{
    ValueA         = 0,
    Value2         = 1,
    Value3         = 2,
    Value4         = 3,
    Value5         = 4,
    Value6         = 5,
    Value7         = 6,
    Value8         = 7,
    Value9         = 8,
    Value10        = 9,
    ValueJ         = 10,
    ValueQ         = 11,
    ValueK         = 12
};

struct card {
    cardSuit suit;          //Categories: 'Clubs', 'Diamonds', 'Hearts', 'Spades'
    cardValue value;        //Values: 'A', '2', '3', '4', '5', '6', '7', '8', '9', '10', 'J', 'Q', 'K'
    string imgName;         //Png image name
    int points;             //Amount of points that it represents
};

#endif
