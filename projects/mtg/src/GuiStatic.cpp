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
    GameObserver * game = player->getObserver();
    JRenderer * r = JRenderer::GetInstance();
    int life = player->life;
    int poisonCount = player->poisonCount;
    int energyCount = player->energyCount;
    int experienceCount = player->experienceCount;
    WFont * mFont = WResourceManager::Instance()->GetWFont(Fonts::MAIN_FONT);
    mFont->SetScale(DEFAULT_MAIN_FONT_SCALE);
    TargetChooser * tc = NULL;

    if (game)
        tc = game->getCurrentTargetChooser();

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
            x0 -= Width * actZ;
            y0 -= Height * actZ;
        }
        switch (corner)
        {
        case TOP_LEFT:
            player->getIcon()->SetHotSpot(0, 0);
            break;
        case BOTTOM_RIGHT:
            player->getIcon()->SetHotSpot(player->getIcon()->mWidth, player->getIcon()->mHeight);
            break;
        }
        player->getIcon()->SetColor(ARGB((int)actA, 255, avatarRed, avatarRed));
        if (tc && !tc->canTarget(player))
        {
            player->getIcon()->SetColor(ARGB((int)actA, 50, 50, 50));
        }
        r->RenderQuad(player->getIcon().get(), actX, actY, actT, Width/player->getIcon()->mWidth*actZ, Height/player->getIcon()->mHeight*actZ);
        if (mHasFocus)
        {
            r->FillRect(x0, x0, Width/player->getIcon()->mWidth * actZ, Height/player->getIcon()->mHeight * actZ, ARGB(abs(128 - wave),255,255,255));
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
    int lx = 255, ly = 255, lz = 255;
    if(life > 24) { lx = 127; ly = 255; lz = 212; }
    if(life > 16 && life < 24) { lx = 255; ly = 255; lz = 255; }
    if(life > 12 && life < 17) { lx = 255; ly = 255; lz = 105; }
    if(life > 8 && life < 13) { lx = 255; ly = 255; lz = 13; }
    if(life > 4 && life < 9) { lx = 255; ly = 166; lz = 0; }
    if(life < 5) { lx = 255; ly = 40; lz = 0; }
    sprintf(buffer, "%i", life);
    switch (corner)
    {
    case TOP_LEFT:
        mFont->SetColor(ARGB((int)actA / 4, 0, 0, 0));
        mFont->DrawString(buffer, actX + 2, actY - 2);
        mFont->SetScale(1.5f);
        mFont->SetColor(ARGB((int)actA, lx, ly, lz));
        mFont->DrawString(buffer, actX + 1, actY - 1);
        mFont->SetScale(1);
        break;
    case BOTTOM_RIGHT:
        mFont->SetScale(1.4f);
        mFont->SetColor(ARGB((int)actA, lx, ly, lz));
        mFont->DrawString(buffer, actX, actY - 14, JGETEXT_RIGHT);
        mFont->SetScale(1);
        break;
    }
    //poison
    char poison[10];
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
    //energy
    char energy[15];
    if (energyCount > 0)
    {
        sprintf(energy, "%i", energyCount);
        switch (corner)
        {
        case TOP_LEFT:
            mFont->SetColor(ARGB((int)actA / 1, 255, 255, 0));
            mFont->DrawString(energy, actX + 2, actY + 17);
            break;
        case BOTTOM_RIGHT:
            mFont->SetColor(ARGB((int)actA / 1 ,255, 255, 0));
            mFont->DrawString(energy, actX, actY - 27, JGETEXT_RIGHT);
            break;
        }
    }
    //experience
    char experience[15];
    if (experienceCount > 0)
    {
        sprintf(experience, "%i", experienceCount);
        switch (corner)
        {
        case TOP_LEFT:
            mFont->SetColor(ARGB((int)actA / 1, 255, 0, 255));
            mFont->DrawString(experience, actX + 2, actY + 24);
            break;
        case BOTTOM_RIGHT:
            mFont->SetColor(ARGB((int)actA / 1 ,255, 0, 255));
            mFont->DrawString(experience, actX - 10, actY - 27, JGETEXT_RIGHT);
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
    {
        cd->zone->owner->getObserver()->guiOpenDisplay = NULL;
        showCards = 0;
        cd->zone->owner->getObserver()->OpenedDisplay = NULL;
    }
    else if(!cd->zone->owner->getObserver()->OpenedDisplay)//one display at a time please.
    {
        cd->zone->owner->getObserver()->guiOpenDisplay = this;
        showCards = 1;
        cd->init(zone);
        cd->zone->owner->getObserver()->OpenedDisplay = cd;
    }
}

void GuiGameZone::Render()
{
    //Texture
    JQuadPtr quad = WResourceManager::Instance()->GetQuad(kGenericCardThumbnailID);
    JQuadPtr overlay;
    float scale = defaultHeight / quad->mHeight;
    float scale2 = scale;
    float modx = 0;
    float mody = 0;

    bool replaced = false;
    bool showtop = (zone && zone->owner->game->battlefield->nb_cards && zone->owner->game->battlefield->hasAbility(Constants::SHOWFROMTOPLIBRARY))?true:false;
    bool showopponenttop = (zone && zone->owner->opponent()->game->battlefield->nb_cards && zone->owner->opponent()->game->battlefield->hasAbility(Constants::SHOWOPPONENTTOPLIBRARY))?true:false;

    quad->SetColor(ARGB((int)(actA),255,255,255));
    if(type == GUI_EXILE || type == GUI_COMMANDZONE || type == GUI_SIDEBOARD)
    {
        quad->SetColor(ARGB((int)(actA),255,240,255));
    }
    
    //overlay
    JQuadPtr iconcard = WResourceManager::Instance()->RetrieveTempQuad("iconcard.png");
    JQuadPtr iconhand = WResourceManager::Instance()->RetrieveTempQuad("iconhand.png");
    JQuadPtr iconlibrary = WResourceManager::Instance()->RetrieveTempQuad("iconlibrary.png");
    JQuadPtr iconexile = WResourceManager::Instance()->RetrieveTempQuad("iconexile.png");
    JQuadPtr iconcommandzone = WResourceManager::Instance()->RetrieveTempQuad("iconcommandzone.png");
    JQuadPtr iconsideboard = WResourceManager::Instance()->RetrieveTempQuad("iconsideboard.png");

    if(iconlibrary && type == GUI_LIBRARY)
    {
        scale2 = defaultHeight / iconlibrary->mHeight;
        modx = -0.f;
        mody = -2.f;
        iconlibrary->SetColor(ARGB((int)(actA),255,255,255));
        quad = iconlibrary;
    }
    if(iconhand && type == GUI_OPPONENTHAND)
    {
        scale2 = defaultHeight / iconhand->mHeight;
        modx = -0.f;
        mody = -2.f;
        iconhand->SetColor(ARGB((int)(actA),255,255,255));
        quad = iconhand;       
    }
    if(iconcard && type == GUI_GRAVEYARD)
    {
        scale2 = defaultHeight / iconcard->mHeight;
        modx = -0.f;
        mody = -2.f;
        iconcard->SetColor(ARGB((int)(actA),255,255,255));
        quad = iconcard;
    }
    if(iconexile && type == GUI_EXILE)
    {
        scale2 = defaultHeight / iconexile->mHeight;
        modx = -0.f;
        mody = -2.f;
        iconexile->SetColor(ARGB((int)(actA),255,255,255));
        quad = iconexile;
    }
    if(iconcommandzone && type == GUI_COMMANDZONE)
    {
        scale2 = defaultHeight / iconcommandzone->mHeight;
        modx = -0.f;
        mody = -2.f;
        iconcommandzone->SetColor(ARGB((int)(actA),255,255,255));
        quad = iconcommandzone;
    }
    if(iconsideboard && type == GUI_SIDEBOARD)
    {
        scale2 = defaultHeight / iconsideboard->mHeight;
        modx = -0.f;
        mody = -2.f;
        iconsideboard->SetColor(ARGB((int)(actA),255,255,255));
        quad = iconsideboard;
    }

    if(type == GUI_LIBRARY && zone->nb_cards && !showCards)
    {
        int top = zone->nb_cards - 1;
        if(zone->cards[top] && (zone->cards[top]->canPlayFromLibrary()||showtop||showopponenttop))
        {
            MTGCardInstance * card = zone->cards[top];
            if(card && card->getObserver())
            {
                replaced = true;
                JQuadPtr kquad = WResourceManager::Instance()->RetrieveCard(card, CACHE_THUMB);
                if(kquad)
                {
                    kquad->SetColor(ARGB((int)(actA),255,255,255));
                    scale2 = defaultHeight / kquad->mHeight;
                    modx = (35/4)+1;
                    mody = (50/4)+1;
                    quad = kquad;
                }
                else
                {
                    quad = CardGui::AlternateThumbQuad(card);
                    if(quad)
                    {
                        quad->SetColor(ARGB((int)(actA),255,255,255));
                        scale2 = defaultHeight / quad->mHeight;
                        modx = (35/4)+1;
                        mody = (50/4)+1;
                    }
                }
            }
        }
    }

    //render small card quad
    if(quad)
        JRenderer::GetInstance()->RenderQuad(quad.get(), actX+modx, actY+mody, 0.0, scale2 * actZ, scale2 * actZ);
    /*if(overlay)
        JRenderer::GetInstance()->RenderQuad(overlay.get(), actX, actY, 0.0, scale2 * actZ, scale2 * actZ);*/

    float x0 = actX;
    if (x0 < SCREEN_WIDTH / 2)
    {
        x0 += 7;
    }

    if (mHasFocus)
    {
        if(!replaced)
            JRenderer::GetInstance()->FillRect(actX, actY, quad->mWidth * scale2 * actZ, quad->mHeight * scale2 * actZ,
                ARGB(abs(128 - wave),255,255,255));
    }

    //Number of cards
    WFont * mFont = WResourceManager::Instance()->GetWFont(Fonts::MAIN_FONT);
    mFont->SetScale(DEFAULT_MAIN_FONT_SCALE);
    char buffer[11];
    int mAlpha = (int) (actA);
    /*if(type == GUI_GRAVEYARD)
        sprintf(buffer, "%i\ng", zone->nb_cards);
    else if(type == GUI_LIBRARY)
        sprintf(buffer, "%i\nl", zone->nb_cards);
    else if(type == GUI_OPPONENTHAND)
        sprintf(buffer, "%i\nh", zone->nb_cards);
    else if(type == GUI_EXILE)
        sprintf(buffer, "%i\ne", zone->nb_cards);
    else*/
    sprintf(buffer, "%i", zone->nb_cards);
    mFont->SetColor(ARGB(mAlpha,0,0,0));
    mFont->DrawString(buffer, x0 + 1, actY + 1);
    if (actA > 120)
        mAlpha = 255;
    mFont->SetColor(ARGB(mAlpha,255,255,255));
    mFont->DrawString(buffer, x0, actY);
    
    //show top library - big card display
    if(type == GUI_LIBRARY && mHasFocus && zone->nb_cards && !showCards && replaced)
    {
        int top = zone->nb_cards - 1;
        if(zone->cards[top])
        {
            Pos pos = Pos(SCREEN_WIDTH - 35 - CardGui::BigWidth / 2, CardGui::BigHeight / 2 - 15, 0.80f, 0.0, 220);
            pos.actY = 165;
            if (x < (CardGui::BigWidth / 2)) pos.actX = CardGui::BigWidth / 2;
            CardGui::DrawCard(zone->cards[top], pos);
        }
    }

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
    else if(type == GUI_LIBRARY && zone->nb_cards && !showCards && key == JGE_BTN_OK && mHasFocus)
    {
        bool activateclick = true;
        
        int top = zone->nb_cards - 1;
        MTGCardInstance * card = zone->cards[top];
        GameObserver * game = card->getObserver();
        if(game)
        {
            TargetChooser * tc = game->getCurrentTargetChooser();
            if(tc && (tc->canTarget(card) || !tc->done || tc->Owner->isHuman()))
                activateclick = false;
        }

        if(card && activateclick)
        {
            card->getObserver()->cardClick(card);
            return true;
        }
    }
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

GuiExile::GuiExile(float x, float y, bool hasFocus, Player * player, GuiAvatars* parent) :
    GuiGameZone(x, y, hasFocus, player->game->exile, parent), player(player)
{
    type = GUI_EXILE;
}

int GuiExile::receiveEventPlus(WEvent* e)
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

int GuiExile::receiveEventMinus(WEvent* e)
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

ostream& GuiExile::toString(ostream& out) const
{
    return out << "GuiExile :::";
}

GuiCommandZone::GuiCommandZone(float x, float y, bool hasFocus, Player * player, GuiAvatars* parent) :
    GuiGameZone(x, y, hasFocus, player->game->commandzone, parent), player(player)
{
    type = GUI_COMMANDZONE;
}

int GuiCommandZone::receiveEventPlus(WEvent* e)
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

int GuiCommandZone::receiveEventMinus(WEvent* e)
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

ostream& GuiCommandZone::toString(ostream& out) const
{
    return out << "GuiCommandZone :::";
}

GuiSideboard::GuiSideboard(float x, float y, bool hasFocus, Player * player, GuiAvatars* parent) :
    GuiGameZone(x, y, hasFocus, player->game->sideboard, parent), player(player)
{
    type = GUI_SIDEBOARD;
}

int GuiSideboard::receiveEventPlus(WEvent* e)
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

int GuiSideboard::receiveEventMinus(WEvent* e)
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

ostream& GuiSideboard::toString(ostream& out) const
{
    return out << "GuiSideboard :::";
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
            //t->x = x + Width / 2;
            //t->y = y + Height / 2;
            //t->zoom = 0.6f;
            //I set to negative so we don't see the face when the cards move...
            t->x = -400.f;
            t->y = -400.f;
            t->mask = ARGB(0,0,0,0);
            t->zoom = -0.6f;
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
