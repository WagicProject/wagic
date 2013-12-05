#ifndef _GRID_DECK_VIEW_H
#define _GRID_DECK_VIEW_H

#include "DeckView.h"
#include "Easing.h"

class GridDeckView : public DeckView
{
private:
    static const float scroll_speed;
    static const float card_scale_small;
    static const float card_scale_big;
public:
    GridDeckView();
    virtual ~GridDeckView();
    void Reset();

    void UpdateViewState(float dt);
    void UpdateCardPosition(CardRep &rep, int index);

    void Render();
    MTGCard * Click(int x, int y);
    bool Button(Buttons button);
    MTGCard *getActiveCard();
private:
    int   mCols;
    int   mRows;
    InOutQuadEasing mSlide;           //[-1,1]. defines the y-offset of the cards
    InOutQuadEasing mScrollOffset;    //[-1,1]. defines the x-offset of the cards
    int   mCurrentSelection;          //0 <= mCurrentSelection < mCards.size(). defines the current selected and thus upscaled card
};

#endif //_GRID_DECK_VIEW_H
