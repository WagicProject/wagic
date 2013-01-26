#ifndef _DUELLAYERS_H_
#define _DUELLAYERS_H_

#include "GuiLayers.h"

class MTGGuiHand;
class MTGGuiPlay;
class ActionLayer;
class ActionStack;
class DamageResolverLayer;
class GuiHandSelf;
class GuiHandOpponent;
class GuiCombat;
class GuiAvatars;
class CardSelectorBase;
struct Pos;
class MTGGamePhase;

class DuelLayers
{
protected:
    int nbitems;
    vector<GuiLayer*> objects;
    vector<Pos*> waiters;
    GuiCombat* combat;
    ActionLayer* action;
    ActionStack* stack;
    GuiHandSelf *hand;
    GuiAvatars * avatars;
    GameObserver* observer;
    MTGGamePhase* phaseHandler;
	int mPlayerViewIndex;
    CardSelectorBase* mCardSelector;

public:
    DuelLayers(GameObserver* go, int playerViewIndex = 0);
    ~DuelLayers();

    ActionLayer * actionLayer();
    ActionStack * stackLayer();
    GuiCombat * combatLayer();
    GuiAvatars * GetAvatars();
    MTGGamePhase* getPhaseHandler() {return phaseHandler;};
    virtual void Update(float dt, Player * player);
    void CheckUserInput(int isAI);
    void Render();
    void Add(GuiLayer * layer);
    void Remove();
    int receiveEvent(WEvent * e);
    float RightBoundary();
	int getPlayerViewIndex(){ return mPlayerViewIndex; };
	Player* getRenderedPlayer();
	Player* getRenderedPlayerOpponent();
	CardSelectorBase* getCardSelector() { return mCardSelector; };
	GameObserver* getObserver(){ return observer; };
};

#include "ActionLayer.h"
#include "GameObserver.h"
#include "MTGGamePhase.h"
#include "ActionStack.h"
#include "Damage.h"

#endif
