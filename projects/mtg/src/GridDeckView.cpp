#include "GridDeckView.h"

const float GridDeckView::scroll_animation_duration = 0.3f;
const float GridDeckView::slide_animation_duration =  0.6f;
const float GridDeckView::card_scale_small = 0.48f;
const float GridDeckView::card_scale_big = 0.7f;

GridDeckView::GridDeckView()
    : DeckView(16), mCols(8), mRows(2), mScrollOffset(0), mSlideOffset(0),
      mScrollEasing(mScrollOffset), mSlideEasing(mSlideOffset), mCurrentSelection(-1)
{

}

GridDeckView::~GridDeckView()
{

}

void GridDeckView::Reset()
{
    mSlideEasing.finish();
    mScrollEasing.finish();

    mCurrentSelection = 0;

    DeckView::Reset();
}

void GridDeckView::UpdateViewState(float dt)
{
    if(!mScrollEasing.finished())
    {
        mScrollEasing.update(dt);

        if(mScrollOffset <= -1.0f)
        {
            SwitchPosition(2);
            mScrollEasing.translate(1.0f);
            mCurrentSelection = (mCurrentSelection >= 6) ? mCurrentSelection - 2 : -1;
        }
        else if(mScrollOffset >= 1.0f)
        {
            SwitchPosition(-2);
            mScrollEasing.translate(-1.0f);
            mCurrentSelection = (mCurrentSelection >= 0 && mCurrentSelection < 10) ? mCurrentSelection + 2 : -1;
        }

        dirtyCardPos = true;
    }

    if(!mSlideEasing.finished())
    {
        mSlideEasing.update(dt);

        if(mSlideOffset < -1.0f)
        {
            mSlideEasing.translate(2.0f);
            SwitchFilter(1);
        }
        else if(mSlideOffset > 1.0f)
        {
            mSlideEasing.translate(-2.0f);
            SwitchFilter(-1);
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

    rep.x = (col + mScrollOffset) * colWidth       - colWidth;
    rep.y = row * rowHeight + mSlideOffset*SCREEN_HEIGHT + rowHeight/2;

    if(mCurrentSelection == index)
    {
        rep.scale = card_scale_big;
        if(row == 0)
        {
            rep.y += rowHeight * (card_scale_big - card_scale_small);
        }
        else
        {
            rep.y -= rowHeight * (card_scale_big - card_scale_small);
        }
    }
    else
    {
        rep.scale = card_scale_small;
    }
}

void GridDeckView::Render()
{
    int firstVisibleCard = 2;
    int lastVisibleCard = mCards.size() - 2;

    if(!mScrollEasing.finished())
    {
        if(mScrollEasing.delta_value > 0){
            firstVisibleCard = 0;
        }
        else
        {
            lastVisibleCard = mCards.size();
        }
    }

    for(int i = firstVisibleCard; i < lastVisibleCard; ++i)
    {

        if(mCurrentSelection != i)
        {
            if (WResourceManager::Instance()->IsThreaded())
            {
                WResourceManager::Instance()->RetrieveCard(getCardRep(i).card, RETRIEVE_THUMB);
            }
            renderCard(i, 255, true);
        }
        else
        {
            if (WResourceManager::Instance()->IsThreaded())
            {
                WResourceManager::Instance()->RetrieveCard(getCardRep(i).card);
            }
        }
    }

    if(2 <= mCurrentSelection && mCurrentSelection < 12)
    {
        renderCard(mCurrentSelection, 255, false);
    }
}

MTGCard * GridDeckView::Click(int x, int y)
{
    int n = getCardIndexNextTo(x, y);
    last_user_activity = 0;

    if(mScrollEasing.finished() && mSlideEasing.finished())
    { //clicked and no animations running
        if(n == mCurrentSelection)
        {
            return getActiveCard();
        }
        else if(n < 4)
        {
            changePosition(-1);
        }
        else if(n >= 12)
        {
            changePosition(1);
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
    mScrollEasing.start(-1.0f * offset, scroll_animation_duration * abs(offset));
    last_user_activity = 0;
}

void GridDeckView::changeFilter(int offset)
{
    if(offset < 0)
    {
        mSlideEasing.start(-2.0f, slide_animation_duration);
    }
    else if(offset > 0)
    {
        mSlideEasing.start(2.0f, slide_animation_duration);
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
