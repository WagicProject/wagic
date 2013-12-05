#include "GridDeckView.h"

const float GridDeckView::scroll_speed = 5.0f;
const float GridDeckView::card_scale_small = 0.48f;
const float GridDeckView::card_scale_big = 0.7f;

GridDeckView::GridDeckView()
    : DeckView(16), mCols(8), mRows(2), mSlide(0), mScrollOffset(0), mCurrentSelection(-1)
{

}

GridDeckView::~GridDeckView()
{

}

void GridDeckView::Reset()
{
    mSlide.finish();
    mScrollOffset.finish();

    mCurrentSelection = 0;
}

void GridDeckView::UpdateViewState(float dt)
{
    if(!mScrollOffset.finished())
    {
        mScrollOffset.update(dt);

        if(mScrollOffset.finished())
        {
            if(mScrollOffset.start_value > mScrollOffset.value)
            {
                deck()->next();
                deck()->next();
                mCurrentSelection = (mCurrentSelection >= 6) ? mCurrentSelection - 2 : -1;
            }
            else if(mScrollOffset.start_value < mScrollOffset.value)
            {
                deck()->prev();
                deck()->prev();
                mCurrentSelection = (mCurrentSelection >= 0 && mCurrentSelection < 10) ? mCurrentSelection + 2 : -1;
            }
            reloadIndexes();

            mScrollOffset.value = 0;
        }
        dirtyCardPos = true;
    }

    if(!mSlide.finished())
    {
        mSlide.update(dt);

        if(mSlide.value < mSlide.start_value){
            //going downwards
            if(mSlide.value < -1.0f){
                mSlide.translate(2.0f);
                SwitchFilter(1);
            }
        } else if(mSlide.value > mSlide.start_value){
            //upwards
            if(mSlide.value > 1.0f){
                mSlide.translate(-2.0f);
                SwitchFilter(-1);
            }
        }

        dirtyCardPos = true;
    }
}

void GridDeckView::UpdateCardPosition(CardRep &rep, int index)
{
    int col = index / mRows;
    int row = index % mRows;
    float colWidth = SCREEN_WIDTH_F / (mCols - 3);
    float rowHeight = SCREEN_HEIGHT_F / mRows;

    rep.x = (col + mScrollOffset.value) * colWidth       - colWidth;
    rep.y = row * rowHeight + mSlide.value*SCREEN_HEIGHT + rowHeight/2;

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

void GridDeckView::Render()
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

MTGCard * GridDeckView::Click(int x, int y)
{
    int n = getCardIndexNextTo(x, y);
    last_user_activity = 0;

    if(mScrollOffset.finished() && mSlide.finished())
    { //clicked and no animations running
        if(n == mCurrentSelection)
        {
            return getActiveCard();
        }
        else if(n < 4)
        {
            mScrollOffset.start(1.0f, 0.3f);
        }
        else if(n >= 12)
        {
            mScrollOffset.start(-1.0f, 0.3f);
        }
        else
        {
            mCurrentSelection = n;
            dirtyCardPos = true;
        }
    }

    return NULL;
}

void GridDeckView::changePosition(int offset)
{
    if(offset < 0){
        mScrollOffset.start( 1.0f, 0.3f);
    }else if(offset > 0){
        mScrollOffset.start(-1.0f, 0.3f);
    }
    last_user_activity = 0;
}

void GridDeckView::changeFilter(int offset)
{
    if(offset < 0){
        mSlide.start(-2.0f, 0.3f);
    }else if(offset > 0){
        mSlide.start(2.0f, 0.3f);
    }
    last_user_activity = 0;
}

MTGCard* GridDeckView::getActiveCard()
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
