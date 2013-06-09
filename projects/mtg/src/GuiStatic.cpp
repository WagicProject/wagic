#include "PrecompiledHeader.h"

#include "Trash.h"
#include "GuiStatic.h"

GuiStatic::GuiStatic(float desiredHeight, float x, float y, bool hasFocus, GuiAvatars* parent) :
    PlayGuiObject(desiredHeight, x, y, 0, hasFocus), parent(parent)
{
}

void GuiStatic::Entering()
{
    parent->Activate(this);
}

bool GuiStatic::Leaving(JButton)
{
    parent->Deactivate(this);
    return false;
}

GuiAvatar::GuiAvatar(float x, float y, bool hasFocus, Player * player, Corner corner, GuiAvatars* parent) :
    GuiStatic(static_cast<float> (GuiAvatar::Height), x, y, hasFocus, parent), avatarRed(255), currentLife(player->life),
            currentpoisonCount(player->poisonCount), corner(corner), player(player)
{
    type = GUI_AVATAR;
}

void GuiAvatar::Render()
{
    JRenderer * r = JRenderer::GetInstance();
    int life = player->life;
    int poisonCount = player->poisonCount;
    WFont * mFont = WResourceManager::Instance()->GetWFont(Fonts::MAIN_FONT);
    mFont->SetScale(DEFAULT_MAIN_FONT_SCALE);
    //Avatar
    int lifeDiff = life - currentLife;
    if (lifeDiff < 0 && currentLife > 0)
    {
        avatarRed = 192 + (3 * 255 * lifeDiff) / currentLife / 4;
        if (avatarRed < 0)
            avatarRed = 0;
    }
    int poisonDiff = poisonCount - currentpoisonCount;
    if (poisonDiff < 0 && currentpoisonCount > 0)
    {
        avatarRed = 192 + (3 * 255 * poisonDiff) / currentpoisonCount / 4;
        if (avatarRed < 0)
            avatarRed = 0;
    }
    currentpoisonCount = poisonCount;
    currentLife = life;

    r->FillRect(actX + 2, actY + 2, Width * actZ, Height * actZ, ARGB((int)(actA / 2), 0, 0, 0));

    float x0 = actX;
    float y0 = actY;

    if (player->getIcon().get())
    {
        if (corner == BOTTOM_RIGHT)
        {
            x0 -= player->getIcon()->mWidth * actZ;
            y0 -= player->getIcon()->mHeight * actZ;
        }
        switch (corner)
        {
        case TOP_LEFT:
            player->getIcon()->SetHotSpot(0, 0);
            break;
        case BOTTOM_RIGHT:
            player->getIcon()->SetHotSpot(35, 50);
            break;
        }
        player->getIcon()->SetColor(ARGB((int)actA, 255, avatarRed, avatarRed));
        r->RenderQuad(player->getIcon().get(), actX, actY, actT, actZ, actZ);
        if (mHasFocus)
        {
            r->FillRect(x0, x0, player->getIcon()->mWidth * actZ, player->getIcon()->mHeight * actZ, ARGB(abs(128 - wave),255,255,255));
        }
    }

    if (avatarRed < 255)
    {
        avatarRed += 3;
        if (avatarRed > 255)
            avatarRed = 255;
    }

    if (player->getObserver()->currentPlayer == player)
        r->DrawRect(x0 - 1, y0 - 1, 36 * actZ, 51 * actZ, ARGB((int)actA, 0, 255, 0));
    else if (player->getObserver()->currentActionPlayer == player)
        r->DrawRect(x0, y0, 34 * actZ, 49 * actZ, ARGB((int)actA, 0, 0, 255));
    if (player->getObserver()->isInterrupting == player)
        r->DrawRect(x0, y0, 34 * actZ, 49 * actZ, ARGB((int)actA, 255, 0, 0));

    //Life
    char buffer[10];
    sprintf(buffer, "%i", life);
    switch (corner)
    {
    case TOP_LEFT:
        mFont->SetColor(ARGB((int)actA / 4, 0, 0, 0));
        mFont->DrawString(buffer, actX + 2, actY + 2);
        mFont->SetColor(ARGB((int)actA, 255, 255, 255));
        mFont->DrawString(buffer, actX + 1, actY + 1);
        break;
    case BOTTOM_RIGHT:
        mFont->SetColor(ARGB((int)actA, 255, 255, 255));
        mFont->DrawString(buffer, actX, actY - 10, JGETEXT_RIGHT);
        break;
    }
    //poison
    char poison[5];
    if (poisonCount > 0)
    {
        sprintf(poison, "%i", poisonCount);
        switch (corner)
        {
        case TOP_LEFT:
            mFont->SetColor(ARGB((int)actA / 1, 0, 255, 0));
            mFont->DrawString(poison, actX + 2, actY + 10);
            break;
        case BOTTOM_RIGHT:
            mFont->SetColor(ARGB((int)actA / 1 ,0, 255, 0));
            mFont->DrawString(poison, actX, actY - 20, JGETEXT_RIGHT);
            break;
        }
    }
    PlayGuiObject::Render();
}

ostream& GuiAvatar::toString(ostream& out) const
{
    return out << "GuiAvatar ::: avatarRed : " << avatarRed << " ; currentLife : " << currentLife << " ; currentpoisonCount : "
            << currentpoisonCount << " ; player : " << player;
}

void GuiGameZone::toggleDisplay()
{
    if (showCards)
        showCards = 0;
    else
    {
        showCards = 1;
        cd->init(zone);
    }
}

void GuiGameZone::Render()
{
    //Texture
    JQuadPtr quad = WResourceManager::Instance()->GetQuad(kGenericCardThumbnailID);
    float scale = defaultHeight / quad->mHeight;
    quad->SetColor(ARGB((int)(actA),255,255,255));

    JRenderer::GetInstance()->RenderQuad(quad.get(), actX, actY, 0.0, scale * actZ, scale * actZ);

    float x0 = actX;
    if (x0 < SCREEN_WIDTH / 2)
    {
        x0 += 7;
    }

    if (mHasFocus)
        JRenderer::GetInstance()->FillRect(actX, actY, quad->mWidth * scale * actZ, quad->mHeight * scale * actZ,
                ARGB(abs(128 - wave),255,255,255));

    //Number of cards
    WFont * mFont = WResourceManager::Instance()->GetWFont(Fonts::MAIN_FONT);
    mFont->SetScale(DEFAULT_MAIN_FONT_SCALE);
    char buffer[11];
    int mAlpha = (int) (actA);
    sprintf(buffer, "%i", zone->nb_cards);
    mFont->SetColor(ARGB(mAlpha,0,0,0));
    mFont->DrawString(buffer, x0 + 1, actY + 1);
    if (actA > 120)
        mAlpha = 255;
    mFont->SetColor(ARGB(mAlpha,255,255,255));
    mFont->DrawString(buffer, x0, actY);

    if (showCards)
        cd->Render();
    for (vector<CardView*>::iterator it = cards.begin(); it != cards.end(); ++it)
        (*it)->Render();
    PlayGuiObject::Render();
}

void GuiGameZone::ButtonPressed(int, int)
{
    zone->owner->getObserver()->ButtonPressed(this);
}

bool GuiGameZone::CheckUserInput(JButton key)
{
    if (showCards)
        return cd->CheckUserInput(key);
    return false;
}

void GuiGameZone::Update(float dt)
{
    if (showCards)
        cd->Update(dt);
    PlayGuiObject::Update(dt);

    for (vector<CardView*>::iterator it = cards.begin(); it != cards.end(); ++it)
    {
        CardView * c = (*it);
        c->Update(dt);

        //Dirty fix for http://code.google.com/p/wagic/issues/detail?id=113
        if (fabs(c->actX - c->x) < 0.01 && fabs(c->actY - c->y) < 0.01)
        {
            cards.erase(it);
            zone->owner->getObserver()->mTrash->trash(c);
            return;
        }
    }
}

GuiGameZone::GuiGameZone(float x, float y, bool hasFocus, MTGGameZone* zone, GuiAvatars* parent) :
    GuiStatic(static_cast<float> (GuiGameZone::Height), x, y, hasFocus, parent), zone(zone)
{
    
    cd = NEW CardDisplay(0, zone->owner->getObserver(),y > 150 ? static_cast<int> (x)-235:static_cast<int> (x)+23, static_cast<int> (y), this);
    cd->zone = zone;
    showCards = 0;
}

GuiGameZone::~GuiGameZone()
{
    if (cd)
        delete cd;
    for (vector<CardView*>::iterator it = cards.begin(); it != cards.end(); ++it)
        delete (*it);
}

ostream& GuiGameZone::toString(ostream& out) const
{
    return out << "GuiGameZone ::: zone : " << zone << " ; cd : " << cd << " ; showCards : " << showCards;
}

GuiGraveyard::GuiGraveyard(float x, float y, bool hasFocus, Player * player, GuiAvatars* parent) :
    GuiGameZone(x, y, hasFocus, player->game->graveyard, parent), player(player)
{
    type = GUI_GRAVEYARD;
}

int GuiGraveyard::receiveEventPlus(WEvent* e)
{
    if (WEventZoneChange* event = dynamic_cast<WEventZoneChange*>(e))
        if (event->to == zone)
        {
            CardView* t;
            if (event->card->view)
                t = NEW CardView(CardView::nullZone, event->card, *(event->card->view));
            else
                t = NEW CardView(CardView::nullZone, event->card, x, y);
            t->x = x + Width / 2;
            t->y = y + Height / 2;
            t->zoom = 0.6f;
            t->alpha = 0;
            cards.push_back(t);
            return 1;
        }
    return 0;
}

int GuiGraveyard::receiveEventMinus(WEvent* e)
{
    if (WEventZoneChange* event = dynamic_cast<WEventZoneChange*>(e))
        if (event->from == zone)
            for (vector<CardView*>::iterator it = cards.begin(); it != cards.end(); ++it)
                if (event->card->previous == (*it)->card)
                {
                    CardView* cv = *it;
                    cards.erase(it);
                    zone->owner->getObserver()->mTrash->trash(cv);
                    return 1;
                }
    return 0;
}

ostream& GuiGraveyard::toString(ostream& out) const
{
    return out << "GuiGraveyard :::";
}

//opponenthand begins
GuiOpponentHand::GuiOpponentHand(float x, float y, bool hasFocus, Player * player, GuiAvatars* parent) :
    GuiGameZone(x, y, hasFocus, player->game->hand, parent), player(player)
{
    type = GUI_OPPONENTHAND;
}

int GuiOpponentHand::receiveEventPlus(WEvent* e)
{
    if (WEventZoneChange* event = dynamic_cast<WEventZoneChange*>(e))
        if (event->to == zone)
        {
            CardView* t;
            if (event->card->view)
                t = NEW CardView(CardView::nullZone, event->card, *(event->card->view));
            else
                t = NEW CardView(CardView::nullZone, event->card, x, y);
            t->x = x + Width / 2;
            t->y = y + Height / 2;
            t->zoom = 0.6f;
            t->alpha = 0;
            cards.push_back(t);
            return 1;
        }
    return 0;
}

int GuiOpponentHand::receiveEventMinus(WEvent* e)
{
    if (WEventZoneChange* event = dynamic_cast<WEventZoneChange*>(e))
        if (event->from == zone)
            for (vector<CardView*>::iterator it = cards.begin(); it != cards.end(); ++it)
                if (event->card->previous == (*it)->card)
                {
                    CardView* cv = *it;
                    cards.erase(it);
                    zone->owner->getObserver()->mTrash->trash(cv);
                    return 1;
                }
    return 0;
}

ostream& GuiOpponentHand::toString(ostream& out) const
{
    return out << "GuiOpponentHand :::";
}

GuiLibrary::GuiLibrary(float x, float y, bool hasFocus, Player * player, GuiAvatars* parent) :
    GuiGameZone(x, y, hasFocus, player->game->library, parent), player(player)
{
    type = GUI_LIBRARY;
}

ostream& GuiLibrary::toString(ostream& out) const
{
    return out << "GuiLibrary :::";
}
