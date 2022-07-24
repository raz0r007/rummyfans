#ifndef __USERINTERFACE_H__
#define __USERINTERFACE_H__

#include "ui/CocosGUI.h"

using namespace std;
using namespace cocos2d;
using namespace cocos2d::ui;
using namespace cocos2d::experimental;

//Enum UI Window Types
enum wType {
    WINDOW_REMOVEADS     = 1,
    WINDOW_RESULTS       = 2,
    WINDOW_RULES         = 3,
    WINDOW_NEWMATCH      = 4,
    WINDOW_LOGIN         = 5,
    WINDOW_LOGIN_EMAIL   = 6,
    WINDOW_LOGOUT        = 7,
    WINDOW_ERROR         = 8,
    WINDOW_FINISH        = 9
};

//Enum UI Quick Errors
enum eType {
    ERROR_TURN          = 1,
    ERROR_TAKE_CARD     = 2,
    ERROR_LEAVE_CARD    = 3
};

class UserInterface {
private:
    Scene *scene; //UI Scene
    Layout *layoutUI;

    static UserInterface* m_pInstance;

public:
    UserInterface(Scene *s);
    ~UserInterface();
    static UserInterface* getInstance(Scene *s);
    static void destroyInstance();
    
    void addBackground(string imagePath);
    Button* addButton(string imagePath, Vec2 position, float scale, function<void(void)> onTouch, bool effect = false, Layout *layout = NULL);
    Label* addLabel(string text, Vec2 position, int size, float width, bool useTTF = true, Layout *layout = NULL);
    EditBox* addInput(string inputName, Vec2 position, int maxLength, bool isPasswordInput = false, Layout *layout = NULL);
    void addSprite(Node *n, Layout *layout = NULL);
    
    bool addWindow(unsigned int wType, string optionalText = "");
    bool removeCurrentWindow();
    int getCurrentWindowOpen();
    
    bool showQuickError(unsigned int eType);
};

#endif
