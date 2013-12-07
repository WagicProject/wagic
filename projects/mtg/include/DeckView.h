#ifndef _DECK_VIEW_H_
#define _DECK_VIEW_H_

#include <vector>

#include "MTGCard.h"
#include "DeckDataWrapper.h"
#include "WFont.h"
#include "WResourceManager.h"
#include "Pos.h"


class DeckView
{
protected:
    static const float no_user_activity_show_card_delay;

public:
    struct CardRep{
        float x;
        float y;
        float scale;
        MTGCard * card;
    };

    bool dirtyFilters;
    bool dirtyCardPos;

    DeckView(int numberOfCards);
    virtual ~DeckView();
    virtual void Reset();

    //advances the view and card representations
    void Update(float dt);
    virtual void SetDeck(DeckDataWrapper *toShow);
    DeckDataWrapper *deck();
    void SwitchFilter(int delta);
    void SwitchPosition(int delta);
    int filter();
    void reloadIndexes();
    int getPosition();

    virtual void Render() = 0;
    virtual MTGCard * Click(int x, int y) = 0;
    bool ButtonPressed(Buttons button);
    virtual MTGCard *getActiveCard() = 0;
    virtual void changePosition(int offset) = 0;
    virtual void changeFilter(int offset) = 0;
protected:
    float last_user_activity;
    int mFilter;
    DeckDataWrapper *mCurrentDeck;
    vector<CardRep> mCards;

    CardRep& getCardRep(unsigned int index);
    void renderCard(int index, int alpha, bool asThumbnail = false);
    int getCardIndexNextTo(int x, int y);
private:
    virtual void UpdateViewState(float dt) = 0;
    virtual void UpdateCardPosition(CardRep& rep, int index) = 0;
};

#endif // _DECK_VIEW_H_
