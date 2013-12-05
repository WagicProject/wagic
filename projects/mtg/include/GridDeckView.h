#ifndef _GRID_DECK_VIEW_H
#define _GRID_DECK_VIEW_H

#include "DeckView.h"

class GridDeckView : public DeckView
{
private:
    enum AnimationStage{
        NONE = 0,
        SLIDE_UP,
        SLIDE_DOWN,
        SCROLL_TO_SELECTED
    };

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
    float mSlide;            //[-1,1]. defines, the y-offset of the cards
    float mScrollOffset;
    int   mCurrentSelection; //0 <= mCurrentSelection < mCards.size(). defines the current selected and thus upscaled card
    int   mColsToScroll;  //the number of cols we need to scroll
    AnimationStage mStage; // state machine state. for animation purposes
};

#endif //_GRID_DECK_VIEW_H
