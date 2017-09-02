#include "PrecompiledHeader.h"
#include "GridDeckView.h"

const float GridDeckView::scroll_animation_duration = 0.3f;
const float GridDeckView::slide_animation_duration =  0.6f;
//const float GridDeckView::card_scale_small = 0.47f;
//const float GridDeckView::card_scale_big = 0.6f;
const float GridDeckView::card_scale_small = 0.42f;
const float GridDeckView::card_scale_big = 0.52f;

GridDeckView::GridDeckView()
    : DeckView(16), mCols(8), mRows(2), mScrollOffset(0), mSlideOffset(0),
      mScrollEasing(mScrollOffset), mSlideEasing(mSlideOffset), mCurrentSelection(-1),
      mButtonMode(false)
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
    mButtonMode = false;

    DeckView::Reset();
}

void GridDeckView::UpdateViewState(float dt)
{
    if(!mScrollEasing.finished())
    {
        mScrollEasing.update(dt);

        if(mScrollOffset <= -1.0f)
        {
            changePosition(2);
            moveSelection(-2, false);
            mScrollEasing.translate(1.0f);
        }
        else if(mScrollOffset >= 1.0f)
        {
            changePosition(-2);
            moveSelection(2, false);
            mScrollEasing.translate(-1.0f);
        }

        dirtyCardPos = true;
    }

    if(!mSlideEasing.finished())
    {
        mSlideEasing.update(dt);

        if(mSlideOffset < -1.0f)
        {
            mSlideEasing.translate(2.0f);
            changeFilter(1);
        }
        else if(mSlideOffset > 1.0f)
        {
            mSlideEasing.translate(-2.0f);
            changeFilter(-1);
        }

        dirtyCardPos = true;
    }
}

void GridDeckView::UpdateCardPosition(int index)
{
    CardRep &rep = mCards[index];

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
    bool mode = options[Options::GDVLARGEIMAGE].number?false:true;
    bool prefetch = options[Options::CARDPREFETCHING].number?true:false;

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
        if(prefetch && WResourceManager::Instance()->IsThreaded())
            WResourceManager::Instance()->RetrieveCard(mCards[i].card);

        renderCard(i, 255, mode,true);//the last value is to resize scale in drawcard so we don't have large borders on grid deck view
    }

    if(2 <= mCurrentSelection && mCurrentSelection < 12)
    {
        renderCard(mCurrentSelection, 255, false,true);
    }
}

bool GridDeckView::ButtonPressed(Buttons button)
{
    switch(button)
    {
    case JGE_BTN_LEFT:
        if(mButtonMode && mScrollEasing.finished()) moveSelection(-2, true);
        else if(!mButtonMode) changePositionAnimated(-1);
        last_user_activity = 0;
        break;
    case JGE_BTN_RIGHT:
        if(mButtonMode && mScrollEasing.finished()) moveSelection(2, true);
        else if(!mButtonMode) changePositionAnimated(1);
        last_user_activity = 0;
        break;
    case JGE_BTN_UP:
        if(mButtonMode && mScrollEasing.finished()) moveSelection(-1, true);
        else if(!mButtonMode) changeFilterAnimated(1);
        last_user_activity = 0;
        break;
    case JGE_BTN_DOWN:
        if(mButtonMode && mScrollEasing.finished()) moveSelection(1, true);
        else if(!mButtonMode) changeFilterAnimated(-1);
        last_user_activity = 0;
        break;
    case JGE_BTN_CTRL:
        if(mButtonMode)
        {
            mButtonMode = false;
            dirtyCardPos = true;
            mCurrentSelection = -1;
        }
        else return false;
        break;
    default:
        return false;
    }
    return true;
}

MTGCard * GridDeckView::Click(int x, int y)
{
    int n = getCardIndexNextTo(x, y);
    last_user_activity = 0;
    mButtonMode = false;

    if(mScrollEasing.finished() && mSlideEasing.finished())
    { //clicked and no animations running
        if(n == mCurrentSelection)
        {
            return getActiveCard();
        }
        else if(n < 4)
        {
            changePositionAnimated(-1);
        }
        else if(n >= 12)
        {
            changePositionAnimated(1);
        }
        else
        {
            mCurrentSelection = n;
            dirtyCardPos = true;
        }
    }

    return NULL;
}

MTGCard * GridDeckView::Click()
{
    if(mScrollEasing.finished() && mSlideEasing.finished())
    {
        MTGCard *active = getActiveCard();
        if(active != NULL)
        {
            return active;
        }
        else
        {
            mButtonMode = true;
            dirtyCardPos = true;
            mCurrentSelection = 4;
        }
    }

    return NULL;
}

void GridDeckView::changePositionAnimated(int offset)
{
    if(mScrollEasing.finished())
        mScrollEasing.start(-1.0f * offset, scroll_animation_duration * abs(offset));
    last_user_activity = 0;
}

void GridDeckView::changeFilterAnimated(int offset)
{
    if(mSlideEasing.finished())
        mSlideEasing.start(2.0f * offset, float(slide_animation_duration * abs(offset)));
    last_user_activity = 0;
}

MTGCard* GridDeckView::getActiveCard()
{
    if(mCurrentSelection >= 4 && mCurrentSelection < int(mCards.size())-4)
    {
        return mCards[mCurrentSelection].card;
    }
    else
    {
        return NULL;
    }
}

void GridDeckView::moveSelection(int offset, bool alignIfOutOfBounds)
{
    mCurrentSelection += offset;

    if(alignIfOutOfBounds)
    {
        if(mCurrentSelection < 4)
        {
            changePositionAnimated(-1);
        }
        else if(mCurrentSelection >= 12)
        {
            changePositionAnimated(1);
        }
    }
    else
    {
        if(mCurrentSelection < 4 || mCurrentSelection >= 12)
        {
            mCurrentSelection = -1;
        }
    }

    dirtyCardPos = true;
}
