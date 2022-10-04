#include "DeckView.h"

#include "GameOptions.h"
#include "CardGui.h"
#include "CardDescriptor.h"

const float DeckView::no_user_activity_show_card_delay = 0.1f;

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
            UpdateCardPosition(i);
        }
        dirtyCardPos = false;
    }
}

void DeckView::SetDeck(DeckDataWrapper *toShow)
{
    mCurrentDeck = toShow;
    dirtyCardPos = true;
    dirtyFilters = true;
    reloadIndexes();
}

DeckDataWrapper* DeckView::deck()
{
    return mCurrentDeck;
}

void DeckView::changeFilter(int delta)
{
    unsigned int FilterCount = Constants::NB_Colors + 1;
    mFilter = (FilterCount + mFilter + delta) % FilterCount;
    dirtyFilters = true;
}

void DeckView::changePosition(int delta)
{
    for(int i = 0; i < delta; ++i)
    {
        mCurrentDeck->next();
    }

    for(int i = 0; i > delta; --i)
    {
        mCurrentDeck->prev();
    }

    reloadIndexes();
}

int DeckView::filter()
{
    return mFilter;
}

void DeckView::reloadIndexes()
{
    if(mCurrentDeck != NULL)
    {
        for (unsigned int i = 0; i < mCards.size(); i++)
        {
            mCards[i].card = deck()->getCard(i);
        }
    }
}

void DeckView::renderCard(int index, int alpha, bool asThumbnail, bool griddeckview)
{
    WFont * mFont = WResourceManager::Instance()->GetWFont(Fonts::MAIN_FONT);

    const CardRep& cardPosition = mCards[index];

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
                if (last_user_activity > (abs(2 - index) + 1) * no_user_activity_show_card_delay)
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
                int mode = !options[Options::DISABLECARDS].number ? DrawMode::kNormal : DrawMode::kText;
                Pos pos = Pos(cardPosition.x, cardPosition.y, cardPosition.scale * 285 / 250, 0.0, 255);
                CardGui::DrawCard(cardPosition.card, pos, mode, asThumbnail, true);
            }
        }
        else
        {
            Pos pos = Pos(cardPosition.x, cardPosition.y, cardPosition.scale * 285 / 250, 0.0, 255);
            CardGui::DrawCard(cardPosition.card, pos, DrawMode::kText, asThumbnail, true);
        }
    }
    else
    {//NORMAL VIEW WITH IMAGES
        int mode = !options[Options::DISABLECARDS].number ? DrawMode::kNormal : DrawMode::kText;
        Pos pos = Pos(cardPosition.x, cardPosition.y, cardPosition.scale * 285 / 250, 0.0, 255);
        CardGui::DrawCard(cardPosition.card, pos, mode, asThumbnail, true, griddeckview);
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
        float qtY = cardPosition.y - 115 * cardPosition.scale;
        float qtX = cardPosition.x + 62 * cardPosition.scale;
        char buffer[4096];
        sprintf(buffer, "x%i", deck()->count(cardPosition.card));
        WFont * font = mFont;
        font->SetScale(1.4f);
        font->SetColor(ARGB(fontAlpha/2,0,0,0));
        JRenderer::GetInstance()->FillRect(qtX, qtY, font->GetStringWidth(buffer) + 6, 15, ARGB(fontAlpha/2,0,0,0));
        JRenderer::GetInstance()->DrawRect(qtX, qtY, font->GetStringWidth(buffer) + 6, 15, ARGB(fontAlpha/2,240,240,240));
        font->DrawString(buffer, qtX + 5, qtY + 0);
        font->SetColor(ARGB(fontAlpha,255,255,255));
        font->DrawString(buffer, qtX + 4, qtY - 1);
        font->SetColor(ARGB(255,255,255,255));
        font->SetScale(1.0f);
    }
}

int DeckView::getCardIndexNextTo(int x, int y)
{
    int bestCardIndex = -1;
    float bestDistance = 0;

    for(unsigned int i = 0; i < mCards.size(); i++)
    {
        const CardRep& cardPosition = mCards[i];

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
