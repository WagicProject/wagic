#ifndef _GRID_DECK_VIEW_H
#define _GRID_DECK_VIEW_H

#include "DeckView.h"
#include "Easing.h"

class GridDeckView : public DeckView
{
private:
    static const float scroll_animation_duration;
    static const float slide_animation_duration;
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

    void changePosition(int offset);
    void changeFilter(int offset);

    MTGCard *getActiveCard();
private:
    int mCols;
    int mRows;
    float mScrollOffset, mSlideOffset;
    InOutQuadEasing mScrollEasing;
    InOutQuadEasing mSlideEasing;
    int mCurrentSelection;
};

#endif //_GRID_DECK_VIEW_H
