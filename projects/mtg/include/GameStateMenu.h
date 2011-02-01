#ifndef _GAME_STATE_MENU_H_
#define _GAME_STATE_MENU_H_

#include <JGui.h>
#include <dirent.h>
#include "GameState.h"
#include "SimpleMenu.h"
#include "TextScroller.h"

class GameStateMenu: public GameState, public JGuiListener
{
private:
    TextScroller * scroller;
    int scrollerSet;
    JGuiController* mGuiController;
    SimpleMenu* subMenuController;
    SimpleMenu* gameTypeMenu;
    int hasChosenGameType;
    JQuadPtr mIcons[10];
    JTexture * bgTexture;
    JQuadPtr mBg;
    JTexture * splashTex;
    float mCreditsYPos;
    int currentState;
    //JMusic * bgMusic;
    int mVolume;
    char nbcardsStr[400];
    vector<string> langs;
    vector<string> primitives;
    string wallpaper;
    int primitivesLoadCounter;

    DIR *mDip;
    struct dirent *mDit;
    char mCurrentSetName[32];
    char mCurrentSetFileName[512];

    int mReadConf;
    float timeIndex;
    float angleMultiplier;
    float angleW;
    float yW;
    void fillScroller();

    void setLang(int id);
    string getLang(string s);
    void loadLangMenu();
    bool langChoices;
    void runTest(); //!!
    void listPrimitives();
    void genNbCardsStr(); //computes the contents of nbCardsStr
    void ensureMGuiController(); //creates the MGuiController if it doesn't exist
    string loadRandomWallpaper(); //loads a list of string of textures that can be randolmy shown on the loading screen
public:
    GameStateMenu(GameApp* parent);
    virtual ~GameStateMenu();
    virtual void Create();
    virtual void Destroy();
    virtual void Start();
    virtual void End();
    virtual void Update(float dt);
    virtual void Render();
    virtual void ButtonPressed(int controllerId, int controlId);

    int nextDirectory(const char * root, const char * file); // Retrieves the next directory to have matching file
    void resetDirectory();
    void createUsersFirstDeck(int setId);
    virtual ostream& toString(ostream& out) const;

    enum
    {
        MENU_CARD_PURCHASE = 2,
        MENU_DECK_SELECTION = 10,
        MENU_DECK_BUILDER = 11,
        MENU_FIRST_DUEL_SUBMENU = 102,
        MENU_LANGUAGE_SELECTION = 103,
    };
};

#endif
