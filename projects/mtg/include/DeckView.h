#ifndef _DECK_VIEW_H_
#define _DECK_VIEW_H_

#include <vector>

#include "MTGCard.h"
#include "DeckDataWrapper.h"
#include "WFont.h"
#include "WResourceManager.h"
#include "Pos.h"

#define NO_USER_ACTIVITY_HELP_DELAY 10
#define NO_USER_ACTIVITY_SHOWCARD_DELAY 0.1

class DeckView
{
protected:
    static const float max_scale;
    static const float x_center;
    static const float right_border;

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

    virtual void Render() = 0;
    virtual MTGCard * Click(int x, int y) = 0;
    virtual bool Button(Buttons button) = 0;
    virtual MTGCard *getActiveCard() = 0;

    virtual void SetDeck(DeckDataWrapper *toShow);
    DeckDataWrapper *deck();
    void SwitchFilter(int delta);
    int filter();
    void reloadIndexes();
    int getPosition();

protected:
    float last_user_activity;
    int mFilter;
    DeckDataWrapper *mCurrentDeck;

    CardRep& getCardRep(unsigned int index);
    void renderCard(int index, int alpha);
    int getCardIndexNextTo(int x, int y);

    vector<CardRep> mCards;
private:
    virtual void UpdateViewState(float dt) = 0;
    virtual void UpdateCardPosition(CardRep& rep, int index) = 0;
};

#endif // _DECK_VIEW_H_
