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

    static const float scroll_speed = 5.0f;
    static const float card_scale_small = 0.48f;
    static const float card_scale_big = 0.6f;
public:
    GridDeckView(): DeckView(16), mCols(8), mRows(2), mSlide(0), mScrollOffset(0), mCurrentSelection(0), mColsToScroll(0), mStage(NONE)
    {}

    virtual ~GridDeckView()
    {}

    void Reset()
    {
        mSlide = 0;
        mScrollOffset = 0;
        mCurrentSelection = 0;
        mColsToScroll = 0;
        mStage = NONE;
    }

    void UpdateViewState(float dt)
    {
        switch(mStage)
        {
        case SCROLL_TO_SELECTED:
            if(mColsToScroll < 0){
                mScrollOffset -= dt * scroll_speed;
                if(mScrollOffset <= -1.0f)
                {
                    mScrollOffset += 1.0f;
                    deck()->next();
                    deck()->next();
                    mCurrentSelection = (mCurrentSelection >= 6) ? mCurrentSelection - 2 : -1;
                    reloadIndexes();
                    mColsToScroll += 1;
                }
            }else if(mColsToScroll > 0){
                mScrollOffset += dt * scroll_speed;
                if(mScrollOffset >= 1.0f)
                {
                    mScrollOffset -= 1.0f;
                    deck()->prev();
                    deck()->prev();
                    mCurrentSelection = (mCurrentSelection >= 0 && mCurrentSelection < 10) ? mCurrentSelection + 2 : -1;
                    reloadIndexes();
                    mColsToScroll -= 1;
                }
            }else if(mColsToScroll == 0){
                mScrollOffset = 0;
                mStage = NONE;
            }
            dirtyCardPos = true;
            break;
        case SLIDE_DOWN:
            mSlide -= 0.05f;
            if (mSlide < -1.0f)
            {
                dirtyFilters = true;
                mSlide = 1;
            }
            else if (mSlide > 0 && mSlide < 0.05)
            {
                mStage = NONE;
                mSlide = 0;
            }
            dirtyCardPos = true;
            break;
        case SLIDE_UP:
            mSlide += 0.05f;
            if (mSlide > 1.0f)
            {
                dirtyFilters = true;
                mSlide = -1;
            }
            else if (mSlide < 0 && mSlide > -0.05)
            {
                mStage = NONE;
                mSlide = 0;
            }
            dirtyCardPos = true;
            break;
        default:
            mStage = NONE;
            break;
        }
    }

    void UpdateCardPosition(CardRep &rep, int index)
    {
        int col = index / mRows;
        int row = index % mRows;
        float colWidth = SCREEN_WIDTH_F / (mCols - 3);
        float rowHeight = SCREEN_HEIGHT_F / mRows;

        rep.x = (col + mScrollOffset) * colWidth       - colWidth;
        rep.y = row * rowHeight + mSlide*SCREEN_HEIGHT + rowHeight/2;

        if(mCurrentSelection == index)
        {
            rep.scale = card_scale_big;
            if(row == 0){
                rep.y += rowHeight * (card_scale_big - card_scale_small);
            }else{
                rep.y -= rowHeight * (card_scale_big - card_scale_small);
            }
        }else{
            rep.scale = card_scale_small;
        }
    }

    void Render()
    {
        for(int i = 0; i < int(mCards.size()); ++i)
        {
            WResourceManager::Instance()->RetrieveCard(getCardRep(i).card);

            if(mCurrentSelection != i)
            {
                renderCard(i, 255);
            }
        }

        if(2 <= mCurrentSelection && mCurrentSelection < 12){
            renderCard(mCurrentSelection, 255);
        }
    }

    MTGCard * Click(int x, int y)
    {
        int n = getCardIndexNextTo(x, y);
        DebugTrace("Clicked: " << n);
        last_user_activity = 0;

        //clicked active card, and no animation is running
        if(n == mCurrentSelection && mStage == NONE)
        {
            return getActiveCard();
        }
        else if(n < 4 && mStage == NONE)
        {
            mColsToScroll = 1;
            mStage = SCROLL_TO_SELECTED;
        }
        else if(n >= 12 && mStage == NONE)
        {
            mColsToScroll = -1;
            mStage = SCROLL_TO_SELECTED;
        }
        else
        {
            mCurrentSelection = n;
            dirtyCardPos = true;
        }

        return NULL;
    }

    bool Button(Buttons button)
    {
        switch(button)
        {
        case JGE_BTN_LEFT:
            mColsToScroll -= 1;
            mStage = SCROLL_TO_SELECTED;
            last_user_activity = 0;
            break;
        case JGE_BTN_RIGHT:
            mColsToScroll += 1;
            mStage = SCROLL_TO_SELECTED;
            last_user_activity = 0;
            break;
        case JGE_BTN_UP:
            mStage = SLIDE_UP;
            SwitchFilter(1);
            last_user_activity = 0;
            break;
        case JGE_BTN_DOWN:
            mStage = SLIDE_DOWN;
            SwitchFilter(-1);
            last_user_activity = 0;
            break;
        default:
            return false;
        }
        return true;
    }

    MTGCard *getActiveCard()
    {
        if(mCurrentSelection >= 0 && mCurrentSelection < int(mCards.size()))
        {
            return mCards[mCurrentSelection].card;
        }
        else
        {
            return NULL;
        }
    }

    //maintains the current rotation for fluid animations
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
