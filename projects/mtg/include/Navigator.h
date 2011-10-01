#ifndef NAVIGATOR_H
#define NAVIGATOR_H

#include "JTypes.h"

#include "CardSelector.h"
#include "DuelLayers.h"

#include <map>
#include <vector>

// private class only used by Navigator, see implementation file  
class CardZone;

class Navigator: public CardSelectorBase
{
public:

    Navigator(GameObserver *observer, DuelLayers* inDuelLayers);
    virtual ~Navigator();

    // Inherited functions from GuiLayer
    bool CheckUserInput(JButton inKey);
    bool CheckUserInput(int x, int y);
    void Update(float dt);
    void Render();

    //Limitor operations
    void PushLimitor();
    void PopLimitor();
    void Limit(LimitorFunctor<PlayGuiObject>* inLimitor, CardView::SelectorZone inZone);

    virtual void Add(PlayGuiObject*);
    virtual void Remove(PlayGuiObject*);
    virtual void Push() {}
    virtual void Pop() {}

protected:
    PlayGuiObject* GetCurrentCard();

    /**
     ** Helper function that translates a card type into an internal zone ID (used as the index for the card zone map)
     */
    int CardToCardZone(PlayGuiObject* card);

    void HandleKeyStroke(JButton inKey);

private:
    std::map<int, CardZone*> mCardZones;
    CardZone* mCurrentZone;
    Pos mDrawPosition;

    DuelLayers* mDuelLayers;

    bool mLimitorEnabled;
    std::stack<CardZone*> mCurrentZoneStack;
};

#endif //NAVIGATOR_H
