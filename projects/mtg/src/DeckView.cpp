#include "DeckView.h"

#include "GameOptions.h"
#include "CardGui.h"

const float DeckView::max_scale = 0.96f;
const float DeckView::x_center = 180;
const float DeckView::right_border = SCREEN_WIDTH + 180;

DeckView::DeckView(int numberOfCards)
    :  dirtyFilters(true), dirtyCardPos(true), last_user_activity(0.0f), mFilter(0), mCurrentDeck(NULL)
{
    mCards.resize(numberOfCards);
}

DeckView::~DeckView()
{

}

void DeckView::Reset()
{
    dirtyFilters = true;
    dirtyCardPos = true;
    last_user_activity = 0;
    mFilter = 0;
    mCurrentDeck = NULL;
}

void DeckView::Update(float dt)
{
    last_user_activity += dt;

    UpdateViewState(dt);

    if(dirtyCardPos)
    {
        for(unsigned int i = 0; i < mCards.size(); ++i)
        {
            UpdateCardPosition(mCards[i], i);
        }
        dirtyCardPos = false;
    }
}

bool DeckView::ButtonPressed(Buttons button)
{
    switch(button)
    {
    case JGE_BTN_LEFT:
        changePosition(-1);
        last_user_activity = 0;
        break;
    case JGE_BTN_RIGHT:
        changePosition(1);
        last_user_activity = 0;
        break;
    case JGE_BTN_UP:
        changeFilter(1);
        last_user_activity = 0;
        break;
    case JGE_BTN_DOWN:
        changeFilter(-1);
        last_user_activity = 0;
        break;
    default:
        return false;
    }
    return true;
}

void DeckView::SetDeck(DeckDataWrapper *toShow)
{
    mCurrentDeck = toShow;
    reloadIndexes();
}

DeckDataWrapper* DeckView::deck()
{
    return mCurrentDeck;
}

void DeckView::SwitchFilter(int delta)
{
    unsigned int FilterCount = Constants::NB_Colors + 1;
    mFilter = (FilterCount + mFilter + delta) % FilterCount;
    dirtyFilters = true;
}

int DeckView::filter()
{
    return mFilter;
}

void DeckView::reloadIndexes() //fixme: feels ugly. check if we can remove this
{
    for (unsigned int i = 0; i < mCards.size(); i++)
    {
        mCards[i].card = deck()->getCard(i);
    }
}

DeckView::CardRep& DeckView::getCardRep(unsigned int index)
{
    return mCards[index];
}

void DeckView::renderCard(int index, int alpha)
{
    WFont * mFont = WResourceManager::Instance()->GetWFont(Fonts::MAIN_FONT);

    const CardRep& cardPosition = getCardRep(index);

    if (!cardPosition.card) return;

    if (!WResourceManager::Instance()->IsThreaded())
    {
        JQuadPtr backQuad = WResourceManager::Instance()->GetQuad(kGenericCardID);
        JQuadPtr quad;

        int cacheError = CACHE_ERROR_NONE;

        if (!options[Options::DISABLECARDS].number)
        {
            quad = WResourceManager::Instance()->RetrieveCard(cardPosition.card, RETRIEVE_EXISTING);
            cacheError = WResourceManager::Instance()->RetrieveError();
            if (!quad.get() && cacheError != CACHE_ERROR_404)
            {
                if (last_user_activity > (abs(2 - index) + 1) * NO_USER_ACTIVITY_SHOWCARD_DELAY)
                    quad = WResourceManager::Instance()->RetrieveCard(cardPosition.card);
                else
                {
                    quad = backQuad;
                }
            }
        }

        if (quad.get())
        {
            if (quad == backQuad)
            {
                quad->SetColor(ARGB(255,255,255,255));
                float _scale = cardPosition.scale * (285 / quad->mHeight);
                JRenderer::GetInstance()->RenderQuad(quad.get(), cardPosition.x, cardPosition.y, 0.0f, _scale, _scale);
            }
            else
            {
                Pos pos = Pos(cardPosition.x, cardPosition.y, cardPosition.scale * 285 / 250, 0.0, 255);
                CardGui::DrawCard(cardPosition.card, pos);
            }
        }
        else
        {
            Pos pos = Pos(cardPosition.x, cardPosition.y, cardPosition.scale * 285 / 250, 0.0, 255);
            CardGui::DrawCard(cardPosition.card, pos, DrawMode::kText);
        }
    }
    else
    {
        int mode = !options[Options::DISABLECARDS].number ? DrawMode::kNormal : DrawMode::kText;

        Pos pos = Pos(cardPosition.x, cardPosition.y, cardPosition.scale * 285 / 250, 0.0, 255);
        CardGui::DrawCard(cardPosition.card, pos, mode);
    }

    int quadAlpha = alpha;
    if (!deck()->count(cardPosition.card)) quadAlpha /= 2;
    quadAlpha = 255 - quadAlpha;
    if (quadAlpha > 0)
    {
        JRenderer::GetInstance()->FillRect(cardPosition.x - cardPosition.scale * 100.0f, cardPosition.y - cardPosition.scale * 142.5f, cardPosition.scale * 200.0f, cardPosition.scale * 285.0f,
                                           ARGB(quadAlpha,0,0,0));
    }
    if (last_user_activity < 3)
    {
        int fontAlpha = alpha;
        float qtY = cardPosition.y - 135 * cardPosition.scale;
        float qtX = cardPosition.x + 40 * cardPosition.scale;
        char buffer[4096];
        sprintf(buffer, "x%i", deck()->count(cardPosition.card));
        WFont * font = mFont;
        font->SetColor(ARGB(fontAlpha/2,0,0,0));
        JRenderer::GetInstance()->FillRect(qtX, qtY, font->GetStringWidth(buffer) + 6, 16, ARGB(fontAlpha/2,0,0,0));
        font->DrawString(buffer, qtX + 4, qtY + 4);
        font->SetColor(ARGB(fontAlpha,255,255,255));
        font->DrawString(buffer, qtX + 2, qtY + 2);
        font->SetColor(ARGB(255,255,255,255));
    }
}

int DeckView::getCardIndexNextTo(int x, int y)
{
    int bestCardIndex = -1;
    float bestDistance = 0;

    for(unsigned int i = 0; i < mCards.size(); i++)
    {
        const CardRep& cardPosition = getCardRep(i);

        float dx = (x - cardPosition.x);
        float dy = (y - cardPosition.y);
        float dist = dx*dx + dy*dy;

        if(dist < bestDistance || bestCardIndex == -1)
        {
            bestDistance = dist;
            bestCardIndex = i;
        }
    }

    return bestCardIndex;
}

int DeckView::getPosition()
{
    if(!mCurrentDeck)
    {
        return 0;
    }

    int total = mCurrentDeck->Size();
    int currentPos = (mCurrentDeck->getOffset() + 3) % total;

    while (currentPos <= 0) currentPos += total;
    return currentPos;
}
