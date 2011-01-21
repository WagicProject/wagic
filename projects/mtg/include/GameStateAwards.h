#ifndef _GAME_STATE_AWARDS_H_
#define _GAME_STATE_AWARDS_H_

#include <JGE.h>
#include "GameState.h"
#include "SimpleMenu.h"

class WGuiList;
class WGuiMenu;
class WSrcCards;

class GameStateAwards: public GameState, public JGuiListener
{
private:
    WGuiList * listview;
    WGuiMenu * detailview;
    WSrcCards * setSrc;
    SimpleMenu * menu;
    bool showMenu;
    bool showAlt;
    bool saveMe;
    int mState;
    int mDetailItem;

public:
    GameStateAwards(GameApp* parent);
    bool enterSet(int setid);
    bool enterStats(int option);
    virtual ~GameStateAwards();

    virtual void Start();
    virtual void End();
    virtual void Create();
    virtual void Destroy();
    virtual void Update(float dt);
    virtual void Render();
    virtual void ButtonPressed(int controllerId, int controlId);
};

#endif
