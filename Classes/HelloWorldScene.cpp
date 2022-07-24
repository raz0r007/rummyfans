/****************************************************************************
 Copyright (c) 2017-2018 Xiamen Yaji Software Co., Ltd.
 
 http://www.cocos2d-x.org
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ****************************************************************************/

#include "HelloWorldScene.h"
#include "EventHandler.h"
#include "Game.h"
#include "audio/include/AudioEngine.h"

USING_NS_CC;

Scene* HelloWorld::createScene()
{
    return HelloWorld::create();
}

bool HelloWorld::init()
{
    //////////////////////////////
    // 1. super init first
    if ( !Scene::init() )
    {
        return false;
    }

    SpriteFrameCache *spriteFrameCache = SpriteFrameCache::getInstance();
    AnimationCache *animationCache = AnimationCache::getInstance();
    
    //Animations
    spriteFrameCache->addSpriteFramesWithFile("animations/spritesRummyLogo.plist");
    animationCache->addAnimationsWithFile("animations/animationRummyLogo.plist");

    spriteFrameCache->addSpriteFramesWithFile("animations/spritesLoadingIcon.plist");
    animationCache->addAnimationsWithFile("animations/animationLoadingIcon.plist");
    
    //Sounds
    AudioEngine::preload("sounds/initGameMusic.wav");
    AudioEngine::preload("sounds/uiSound.wav");
    AudioEngine::preload("sounds/cardTouch.wav");

    return true;
}

/**
 @brief   Splash screen ( Anstra )
 @param   -
 @return  -
*/

void HelloWorld::drawSplashScreen()
{
    Director *director = Director::getInstance();
    Rect safeArea = director->getSafeAreaRect();
    Size visibleSize = safeArea.size;
    Vec2 origin = safeArea.origin;
    
    Sprite *bg = Sprite::create("Anstra.png");
    bg->setPosition(Vec2(origin.x + visibleSize.width/2, origin.y + visibleSize.height/2));
    
    float visSizeX = Director::getInstance()->getVisibleSize().width;
    float bgWidth = bg->getContentSize().width;
    float targetSmallSquareWidth = visSizeX * 0.40f; //40% x
    float scaleFactor = targetSmallSquareWidth / bgWidth;
    bg->setScale(scaleFactor);
    
    director->getRunningScene()->addChild(bg);
}


/**
 @brief  Load initial scene
 @param  -
 @return -
*/

void HelloWorld::loadInitialMenu()
{
    Director *director = Director::getInstance();
    Rect safeArea = Director::getInstance()->getSafeAreaRect();
    Size visibleSize = safeArea.size;
    Vec2 origin = safeArea.origin;
    EventHandler *event = EventHandler::getInstance();
 
    //Init scene
    Scene *initScene = Scene::create();
    initScene->setName("initScene");
    UserInterface::destroyInstance();
    director->replaceScene(TransitionFade::create(2, initScene));
   
    //Clean listeners
    director->getEventDispatcher()->removeAllEventListeners();
      
    //UI
    UserInterface *ui = UserInterface::getInstance(initScene);
    ui->addBackground("backgrounds/mainBackground.png");
    
    Sprite *rummyLogo = Sprite::createWithSpriteFrameName("rummyLogo1.png");
    rummyLogo->setScale(visibleSize.width * 0.70f / rummyLogo->getContentSize().width);
    rummyLogo->setPosition(origin.x + visibleSize.width/2, origin.y + visibleSize.height - rummyLogo->getBoundingBox().size.height/2 - (visibleSize.height * 0.05f));
    Animate *rummyAnim = Animate::create(AnimationCache::getInstance()->getAnimation("animationRummyLogo"));
    rummyLogo->runAction(RepeatForever::create(rummyAnim));
    ui->addSprite(rummyLogo);

    ui->addButton("ui/btnPlay.png", Vec2(origin.x + visibleSize.width/2, rummyLogo->getPositionY() - rummyLogo->getBoundingBox().size.height/2 - (visibleSize.height * 0.18f)), 0.175f, [event]() { event->onTouchButtonPlay(); }, true);

    ui->addButton("ui/btnHowToPlay.png", Vec2(origin.x + (visibleSize.width * 0.10f), origin.y + (visibleSize.height * 0.10f)), 0.15f, [event]() { event->onTouchButtonRules(); });
    
    ui->addButton("ui/btnNoAds.png", Vec2(origin.x + (visibleSize.width * 0.90f), origin.y + (visibleSize.height * 0.10f)), 0.15f, [event]() { event->onTouchButtonNoAds(); });
    
    //Destroy gameInstance if exists
    auto destroyGameInstance = CallFunc::create([this]() {
           Game *game = Game::getInstance();
           Director *director = Director::getInstance();
           auto scheduler = director->getScheduler();

           if(director->getRunningScene()->getName() == "initScene")
               scheduler->unscheduleAllForTarget(Game::getInstance());
        
           game->destroyInstance();
    });
    
    initScene->runAction(Sequence::create(DelayTime::create(2.5), destroyGameInstance, NULL));
    
    AudioEngine::stopAll();
    AudioEngine::play2d("sounds/initGameMusic.wav", true, 0.5f);
}
