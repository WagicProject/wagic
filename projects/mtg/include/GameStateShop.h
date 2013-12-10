#ifndef _GAME_STATE_SHOP_H_
#define _GAME_STATE_SHOP_H_

#include <JGE.h>
#include "GameState.h"
#include "SimpleMenu.h"
#include "OptionItem.h"
#include "PriceList.h"
#include "PlayerData.h"
#include "CardDisplay.h"
#include "DeckDataWrapper.h"
#include "Tasks.h"
#include "InteractiveButton.h"

#define STATE_BUY 1
#define STATE_SELL 2
#define STAGE_SHOP_MENU 3
#define STAGE_SHOP_SHOP 4
#define STAGE_SHOP_TASKS 5
#define STAGE_FADE_IN 6
#define STAGE_ASK_ABOUT 7
#define STAGE_SHOP_PURCHASE 8

#define BOOSTER_SLOTS 3
#define SHOP_SLOTS 11

#define SHOP_ITEMS SHOP_SLOTS+1
#define LIST_FADEIN 15

class MTGPack;
class MTGPacks;

class BoosterDisplay: public CardDisplay
{
public:
    BoosterDisplay(int id, GameObserver* game, int x, int y, JGuiListener * listener = NULL, TargetChooser * tc = NULL,
            int nb_displayed_items = 7);
    bool CheckUserInput(JButton key);
};

class ShopBooster
{
public:
    ShopBooster();
    string getName();
    void randomize(MTGPacks * packlist);
    int basePrice();
    int maxInventory();
    void addToDeck(MTGDeck * d, WSrcCards * srcCards);
    string getSort();
#ifdef TESTSUITE
    bool unitTest();
#endif
private:
    void randomCustom(MTGPacks * packlist);
    void randomStandard();
    MTGPack * pack;
    MTGSetInfo * mainSet;
    MTGSetInfo * altSet;
};

class GameStateShop: public GameState, public JGuiListener
{
private:
    JQuadPtr pspIcons[8];
    WSrcCards * srcCards;
    TaskList * taskList;
    WGuiMenu * shopMenu;
    WGuiFilters * filterMenu; //Filter menu slides in sideways from right, or up from bottom.
    WGuiCardImage * bigDisplay;
    BoosterDisplay * boosterDisplay;
    vector<MTGCardInstance*> subBooster;
    MTGDeck * booster;
    bool bListCards;
    InteractiveButton *cycleCardsButton, *showCardListButton;
    bool disablePurchase, clearInput;

    void beginFilters();
    void deleteDisplay();
    void enableButtons();
    void renderButtons();
    
    WSyncable bigSync;
    SimpleMenu * menu;
    PriceList * pricelist;
    PlayerData * playerdata;
    MTGPacks * packlist;
    bool mTouched;
    bool needLoad;
    int mPrices[SHOP_ITEMS];
    int mCounts[SHOP_ITEMS];
    int mInventory[SHOP_ITEMS];
    int lightAlpha;
    int alphaChange;
    int mBuying;

    DeckDataWrapper * myCollection;

    int mStage;
    ShopBooster mBooster[BOOSTER_SLOTS];

    void load();
    void save(bool force = false);
    void updateCounts();
    void beginPurchase(int controlId);
    void purchaseCard(int controlId);
    void purchaseBooster(int controlId);
    void cancelCard(int controlId);
    void cancelBooster(int controlId);
    int purchasePrice(int offset);
    string descPurchase(int controlId, bool tiny = false);
public:
    GameStateShop(GameApp* parent);
    virtual ~GameStateShop();
    virtual void Start();
    virtual void End();
    virtual void Create();
    virtual void Destroy();
    virtual void Update(float dt);
    virtual void Render();
    virtual void ButtonPressed(int controllerId, int controlId);
    virtual void OnScroll(int inXVelocity, int inYVelocity);
    static float _x1[], _y1[], _x2[], _y2[], _x3[], _y3[], _x4[], _y4[];
};

#endif

