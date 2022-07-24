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

#include "AppDelegate.h"
#include "HelloWorldScene.h"
#include "ThirdPartys/SQLite/SQLite.h"
#include "NakamaServer.h"
#include "NakamaCocos2d/NCocosHelper.h"

// #define USE_AUDIO_ENGINE 1
// #define USE_SIMPLE_AUDIO_ENGINE 1

#if USE_AUDIO_ENGINE && USE_SIMPLE_AUDIO_ENGINE
#error "Don't use AudioEngine and SimpleAudioEngine at the same time. Please just select one in your game!"
#endif

#if USE_AUDIO_ENGINE
#include "audio/include/AudioEngine.h"
using namespace cocos2d::experimental;
#elif USE_SIMPLE_AUDIO_ENGINE
#include "audio/include/SimpleAudioEngine.h"
using namespace CocosDenshion;
#endif

USING_NS_CC;

static cocos2d::Size designResolutionSize = cocos2d::Size(480, 320);
static cocos2d::Size smallResolutionSize = cocos2d::Size(480, 320);
static cocos2d::Size mediumResolutionSize = cocos2d::Size(1024, 768);
static cocos2d::Size largeResolutionSize = cocos2d::Size(2048, 1536);

AppDelegate::AppDelegate()
{
}

AppDelegate::~AppDelegate() 
{
#if USE_AUDIO_ENGINE
    AudioEngine::end();
#elif USE_SIMPLE_AUDIO_ENGINE
    SimpleAudioEngine::end();
#endif
}

// if you want a different context, modify the value of glContextAttrs
// it will affect all platforms
void AppDelegate::initGLContextAttrs()
{
    // set OpenGL context attributes: red,green,blue,alpha,depth,stencil,multisamplesCount
    GLContextAttrs glContextAttrs = {8, 8, 8, 8, 24, 8, 0};

    GLView::setGLContextAttrs(glContextAttrs);
}

// if you want to use the package manager to install more packages,  
// don't modify or remove this function
static int register_all_packages()
{
    return 0; //flag for packages manager
}

bool AppDelegate::applicationDidFinishLaunching() {

    //SDKBox Plugins
    #ifdef SDKBOX_ENABLED
        //Facebook SDK
        sdkbox::PluginFacebook::setListener(FacebookHandler::getInstance());
        sdkbox::PluginFacebook::init();
    
        //IAP SDK
        sdkbox::IAP::setListener(IAPHandler::getInstance());
        sdkbox::IAP::init();
    
        //AdMob SDK
        sdkbox::PluginAdMob::setListener(AdMobHandler::getInstance());
        sdkbox::PluginAdMob::init();
        sdkbox::PluginAdMob::cache("interstitial");
    
        //Play SDK (GameCenter or Google Play)
        sdkbox::PluginSdkboxPlay::setListener(PlayHandler::getInstance());
        sdkbox::PluginSdkboxPlay::init();
    
        #if !defined(__ANDROID__) //Only iOS
            //Apple Auth SDK
            sdkbox::PluginSignInWithApple::setListener(AppleAuthHandler::getInstance());
            sdkbox::PluginSignInWithApple::init();
        #endif
    #endif
   
    
    // initialize director
    auto director = Director::getInstance();
    auto glview = director->getOpenGLView();
    if(!glview) {
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32) || (CC_TARGET_PLATFORM == CC_PLATFORM_MAC) || (CC_TARGET_PLATFORM == CC_PLATFORM_LINUX)
        glview = GLViewImpl::createWithRect("RummyFans", cocos2d::Rect(0, 0, designResolutionSize.width, designResolutionSize.height));
#else
        glview = GLViewImpl::create("RummyFans");
#endif
        director->setOpenGLView(glview);
    }

    // turn on display FPS
    director->setDisplayStats(false);

    // set FPS. the default value is 1.0/60 if you don't call this
    director->setAnimationInterval(1.0f / 60);

    // Set the design resolution
    glview->setDesignResolutionSize(designResolutionSize.width, designResolutionSize.height, ResolutionPolicy::NO_BORDER);
    auto frameSize = glview->getFrameSize();
    // if the frame's height is larger than the height of medium size.
    if (frameSize.height > mediumResolutionSize.height)
    {        
        director->setContentScaleFactor(MIN(largeResolutionSize.height/designResolutionSize.height, largeResolutionSize.width/designResolutionSize.width));
    }
    // if the frame's height is larger than the height of small size.
    else if (frameSize.height > smallResolutionSize.height)
    {        
        director->setContentScaleFactor(MIN(mediumResolutionSize.height/designResolutionSize.height, mediumResolutionSize.width/designResolutionSize.width));
    }
    // if the frame's height is smaller than the height of medium size.
    else
    {        
        director->setContentScaleFactor(MIN(smallResolutionSize.height/designResolutionSize.height, smallResolutionSize.width/designResolutionSize.width));
    }

    register_all_packages();

    // create a scene. it's an autorelease object
    auto scene = HelloWorld::createScene();

    // run
    director->runWithScene(scene);

    //--- My code:
    auto loadSplashScreen = CallFunc::create([this]() {
        HelloWorld h;
        h.drawSplashScreen();
    });
    
    auto getServerIP = CallFunc::create([this]() {

        //Get Server IP & set parameters 
        HttpRequest* requestServer = new HttpRequest();
        requestServer->setUrl("https://anstra.net/rummyfans/srv.txt");
        requestServer->setRequestType(HttpRequest::Type::GET);
        requestServer->setResponseCallback([](HttpClient* client, HttpResponse* response) {
            HelloWorld h;
            NakamaServer *nakama = NakamaServer::getInstance();
            
            if(!response or response->getResponseCode() != 200)
            {
                nakama->parameters.serverKey = "defaultkey";
                nakama->parameters.host = "";
                nakama->parameters.port = DEFAULT_PORT;
                nakama->m_client = NCocosHelper::createDefaultClient(nakama->parameters);
                nakama->init();

                h.loadInitialMenu(); //We can't play but it doesn't matter, we redirect anyway to the init menu...
                return;
            }
            
            vector<char> *buffer = response->getResponseData();
            string ip(buffer->begin(), buffer->end());
            
            nakama->parameters.serverKey = "defaultkey"; //@todo: we should set a value here. Also, don't forget to consider to set parameters.ssl = true.
            nakama->parameters.host = ip;
            nakama->parameters.port = DEFAULT_PORT;
            nakama->m_client = NCocosHelper::createDefaultClient(nakama->parameters);
            nakama->init();

            h.loadInitialMenu(); //Once we've server ip, we can start playing!
        });
        
        HttpClient::getInstance()->send(requestServer);
        requestServer->release();
    });
    
    scene->runAction(Sequence::create(loadSplashScreen, DelayTime::create(2), getServerIP, NULL));
    SQLite::getInstance()->createTablesIfNotExists(); //First time we need to create the SQLite tables
    
    return true;
}

// This function will be called when the app is inactive. Note, when receiving a phone call it is invoked.
void AppDelegate::applicationDidEnterBackground() {
    Director::getInstance()->stopAnimation();

#if USE_AUDIO_ENGINE
    AudioEngine::pauseAll();
#elif USE_SIMPLE_AUDIO_ENGINE
    SimpleAudioEngine::getInstance()->pauseBackgroundMusic();
    SimpleAudioEngine::getInstance()->pauseAllEffects();
#endif
}

// this function will be called when the app is active again
void AppDelegate::applicationWillEnterForeground() {
    Director::getInstance()->startAnimation();

#if USE_AUDIO_ENGINE
    AudioEngine::resumeAll();
#elif USE_SIMPLE_AUDIO_ENGINE
    SimpleAudioEngine::getInstance()->resumeBackgroundMusic();
    SimpleAudioEngine::getInstance()->resumeAllEffects();
#endif
}
