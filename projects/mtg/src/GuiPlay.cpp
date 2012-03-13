#include "PrecompiledHeader.h"

#include "CardSelector.h"
#include "GameApp.h"
#include "GuiPlay.h"
#include "Subtypes.h"
#include "Trash.h"
#include "ModRules.h"

#define CARD_WIDTH (31)

const float GuiPlay::HORZWIDTH = 300.0f;
const float GuiPlay::VERTHEIGHT = 80.0f;


void GuiPlay::CardStack::reset(unsigned total, float x, float y)
{
    this->total = total;
    this->x = 0;
    baseX = x;
    this->y = 0;
    baseY = y;
}

void GuiPlay::CardStack::RenderSpell(MTGCardInstance* card, iterator begin, iterator end, float x, float y)
{
    while (begin != end)
    {
        if ((*begin)->card->target == card)
        {
            RenderSpell(card, begin + 1, end, x, y - 10);
            (*begin)->x = x;
            (*begin)->y = y;
            (*begin)->Render();
            return;
        }
        ++begin;
    }
}

GuiPlay::HorzStack::HorzStack()
{
}
GuiPlay::VertStack::VertStack()
{
}

void GuiPlay::VertStack::reset(unsigned total, float x, float y)
{
    GuiPlay::CardStack::reset(total, x - CARD_WIDTH, y);
    count = 0;
}

void GuiPlay::HorzStack::Render(CardView* card, iterator begin, iterator end)
{
    RenderSpell(card->card, begin, end, card->x, card->y - 10);
    card->Render();
}

void GuiPlay::HorzStack::Enstack(CardView* card)
{
    card->x = x + baseX;
    card->y = y + baseY;
    if (total < 8)
        x += CARD_WIDTH;
    else if (total < 16)
        x += (SCREEN_WIDTH - 200 - baseX) / total;
    else
        x += (SCREEN_WIDTH - 50 - baseX) / total;
}

void GuiPlay::VertStack::Enstack(CardView* card)
{
    int modulus = total < 10 ? 3 : 5;
    {
        if (0 == count % modulus)
        {
            x += CARD_WIDTH;
            y = 0;
        }
    }

    card->x = x + baseX;
    card->y = y + baseY;
    y += 12;
    if (++count == total - 1 && y == 12)
        y += 12;
}

void GuiPlay::VertStack::Render(CardView* card, iterator begin, iterator end)
{
    RenderSpell(card->card, begin, end, card->x + 5, card->y - 10);
    card->Render();
}

inline float GuiPlay::VertStack::nextX()
{
    if (0 == count)
        return x + CARD_WIDTH;
    else
        return x;
}

GuiPlay::BattleField::BattleField() :
    attackers(0), blockers(0), height(0.0), red(0), colorFlow(0)
{
}
const float GuiPlay::BattleField::HEIGHT = 80.0f;
void GuiPlay::BattleField::addAttacker(MTGCardInstance*)
{
    ++attackers;
    colorFlow = 1;
}
void GuiPlay::BattleField::removeAttacker(MTGCardInstance*)
{
    --attackers;
}
void GuiPlay::BattleField::reset(float x, float y)
{
    HorzStack::reset(0, x, y);
    currentAttacker = 1;
}
void GuiPlay::BattleField::EnstackAttacker(CardView* card)
{
    card->x = currentAttacker * (HORZWIDTH - 20) / (attackers + 1);
    card->y = baseY + (card->card->getObserver()->players[0] == card->card->controller() ? 20 + y : -20 - y);
    ++currentAttacker;
    //  JRenderer::GetInstance()->RenderQuad(WResourceManager::Instance()->GetQuad("BattleIcon"), card->actX, card->actY, 0, 0.5 + 0.1 * sinf(JGE::GetInstance()->GetTime()), 0.5 + 0.1 * sinf(JGE::GetInstance()->GetTime()));
}
void GuiPlay::BattleField::EnstackBlocker(CardView* card)
{
    MTGCardInstance * c = card->card;
    if (!c)
        return;
    int offset = 0;
    if (c->defenser && c->defenser->view)
    {
        offset = c->defenser->getDefenserRank(c);
        card->x = c->defenser->view->x + 5 * offset;
    }
    card->y = baseY + (card->card->getObserver()->players[0] == card->card->controller() ? 20 + y + 6 * offset : -20 - y + 6 * offset);
}
void GuiPlay::BattleField::Update(float dt)
{
    if (0 == attackers)
        height -= 10 * dt * height;
    else
        height += 10 * dt * (HEIGHT - height);

    if (colorFlow)
    {
        red += static_cast<int> (colorFlow * 300 * dt);
        if (red < 0)
            red = 0;
        if (red > 70)
            red = 70;
    }
}
void GuiPlay::BattleField::Render()
{
    if (height > 3)
        JRenderer::GetInstance()->FillRect(22, SCREEN_HEIGHT / 2 + 10 - height / 2, 250, height, ARGB(127, red, 0, 0));
}

GuiPlay::GuiPlay(GameObserver* game) :
    GuiLayer(game)
{
    end_spells = cards.end();
}

GuiPlay::~GuiPlay()
{
    for (iterator it = cards.begin(); it != cards.end(); ++it)
    {
        delete (*it);
    }
}

bool isSpell(CardView* c)
{
    return c->card->isSpell() && !c->card->isCreature() && !c->card->hasType(Subtypes::TYPE_PLANESWALKER);
}
void GuiPlay::Replace()
{
    unsigned opponentSpellsN = 0, selfSpellsN = 0, opponentLandsN = 0, opponentCreaturesN = 0, 
            battleFieldAttackersN = 0, battleFieldBlockersN = 0, selfCreaturesN = 0, selfLandsN = 0;

    end_spells = stable_partition(cards.begin(), cards.end(), &isSpell);

    for (iterator it = cards.begin(); it != end_spells; ++it)
        if (!(*it)->card->target)
        {
            if((!(*it)->card->hasSubtype(Subtypes::TYPE_AURA)|| ((*it)->card->hasSubtype(Subtypes::TYPE_AURA) && (*it)->card->playerTarget)) && !(*it)->card->hasType(Subtypes::TYPE_PLANESWALKER))
            {
                if (observer->players[0] == (*it)->card->controller())
                    ++selfSpellsN;
                else
                    ++opponentSpellsN;
            }
        }
    for (iterator it = end_spells; it != cards.end(); ++it)
    {
        if ((*it)->card->isCreature())
        {
            if ((*it)->card->isAttacker())
                ++battleFieldAttackersN;
            else if ((*it)->card->isDefenser())
                ++battleFieldBlockersN;
            else if (observer->players[0] == (*it)->card->controller())
                ++selfCreaturesN;
            else
                ++opponentCreaturesN;
        }
        else if ((*it)->card->isLand() || (*it)->card->hasType(Subtypes::TYPE_PLANESWALKER))
        {
            if (observer->players[0] == (*it)->card->controller())
                ++selfLandsN;
            else
                ++opponentLandsN;
        }
    }

    opponentSpells.reset(opponentSpellsN, 18, 60);
    selfSpells.reset(selfSpellsN, 18, 215);

    for (iterator it = cards.begin(); it != end_spells; ++it)
        if (!(*it)->card->target)
        {
            if((!(*it)->card->hasSubtype(Subtypes::TYPE_AURA)|| ((*it)->card->hasSubtype(Subtypes::TYPE_AURA) && (*it)->card->playerTarget)) && !(*it)->card->hasType(Subtypes::TYPE_PLANESWALKER))
            {
                if (observer->players[0] == (*it)->card->controller())
                    selfSpells.Enstack(*it);
                else
                    opponentSpells.Enstack(*it);
            }
        }
    float x = 24 + opponentSpells.nextX();
    //seperated the variable X into 2 different variables. There are 2 players here!!
    //we should not be using a single variable to determine the positioning of cards!!
    float myx = 24 + selfSpells.nextX();
    opponentLands.reset(opponentLandsN,x, 50);
    opponentCreatures.reset(opponentCreaturesN, x, 95);
    battleField.reset(x, 145);//what does this variable do? I can comment it out with no repercussions...is this being double handled?
    selfCreatures.reset(selfCreaturesN, myx, 195);
    selfLands.reset(selfLandsN, myx, 240);

    for (iterator it = end_spells; it != cards.end(); ++it)
    {
        if ((*it)->card->isCreature())
        {
            if ((*it)->card->isAttacker())
                battleField.EnstackAttacker(*it);
            else if ((*it)->card->isDefenser())
                battleField.EnstackBlocker(*it);
            else if (observer->players[0] == (*it)->card->controller())
                selfCreatures.Enstack(*it);
            else
                opponentCreatures.Enstack(*it);
        }
        else if ((*it)->card->isLand())
        {
            if (observer->players[0] == (*it)->card->controller())
                selfLands.Enstack(*it);
            else
                opponentLands.Enstack(*it);
        }

    }
    //rerun the iter reattaching planes walkers to the back of the lands.
    for (iterator it = end_spells; it != cards.end(); ++it)
    {
        if ((*it)->card->hasType(Subtypes::TYPE_PLANESWALKER))
        {
            if (observer->players[0] == (*it)->card->controller())
                selfLands.Enstack(*it);
            else
                opponentLands.Enstack(*it);
        }
    }
}

void GuiPlay::Render()
{
    battleField.Render();

    for (iterator it = cards.begin(); it != cards.end(); ++it)
        if ((*it)->card->isLand())
        {
            if (observer->players[0] == (*it)->card->controller())
                selfLands.Render(*it, cards.begin(), end_spells);
            else
                opponentLands.Render(*it, cards.begin(), end_spells);
        }
        else if ((*it)->card->isCreature())
        {
            if (observer->players[0] == (*it)->card->controller())
                selfCreatures.Render(*it, cards.begin(), end_spells);
            else
                opponentCreatures.Render(*it, cards.begin(), end_spells);
        }
        else if(!(*it)->card->hasType(Subtypes::TYPE_PLANESWALKER))
        {
            if (!(*it)->card->target)
            {
                if (observer->players[0] == (*it)->card->controller())
                    selfSpells.Render(*it, cards.begin(), end_spells);
                else
                    opponentSpells.Render(*it, cards.begin(), end_spells);
            }
        }
        else
        {
            if (!(*it)->card->target)
            {
                if (observer->players[0] == (*it)->card->controller())
                    selfPlaneswalker.Render(*it, cards.begin(), end_spells);
                else
                    opponentPlaneswalker.Render(*it, cards.begin(), end_spells);
            }
        }

}
void GuiPlay::Update(float dt)
{
    battleField.Update(dt);
    for (iterator it = cards.begin(); it != cards.end(); ++it)
        (*it)->Update(dt);
}

int GuiPlay::receiveEventPlus(WEvent * e)
{
    if (WEventZoneChange *event = dynamic_cast<WEventZoneChange*>(e))
    {
        if ((observer->players[0]->inPlay() == event->to) || (observer->players[1]->inPlay() == event->to))
        {
            CardView * card;
            if (event->card->view)
            {
                //fix for http://code.google.com/p/wagic/issues/detail?id=462.
                // We don't want a card in the hand to have an alpha of 0
                event->card->view->alpha = 255;

                card = NEW CardView(CardView::playZone, event->card, *(event->card->view));
            }
            else
                card = NEW CardView(CardView::playZone, event->card, 0, 0);
            cards.push_back(card);

            if (event->card->isTapped())
                gModRules.cards.activateEffect->doEffect(card);
            else
                gModRules.cards.activateEffect->undoEffect(card);

            card->alpha = 255;

            // Make sure that the card is repositioned before adding it to the CardSelector, as
            // the card's position is a cue for certain CardSelector variants as to what zone the card is placed in
            Replace();
            observer->getCardSelector()->Add(card);
            return 1;
        }
    }
    else if (WEventCreatureAttacker* event = dynamic_cast<WEventCreatureAttacker*>(e))
    {
        if (NULL != event->after)
            battleField.addAttacker(event->card);
        else if (NULL != event->before)
            battleField.removeAttacker(event->card);
        Replace();
    }
    else if (dynamic_cast<WEventCreatureBlocker*> (e))
    {
        Replace();
    }
    else if (WEventCardTap* event = dynamic_cast<WEventCardTap*>(e))
    {
        if (CardView* cv = dynamic_cast<CardView*>(event->card->view))
        {
            if (event->after)
                gModRules.cards.activateEffect->doEffect(cv);
            else
                gModRules.cards.activateEffect->undoEffect(cv);
            //cv->t = event->after ? M_PI / 2 : 0;
        }
        else if (event->card->view != NULL)
        {
            if (event->after)
                gModRules.cards.activateEffect->doEffect(event->card->view);
            else
                gModRules.cards.activateEffect->undoEffect(event->card->view);
            //event->card->view->actT = event->after ? M_PI / 2 : 0;
        }
        else
        {
			// this should never happen, if you have a consistent repro case, ping Wil please
            assert(false);
        }
        return 1;
    }
    else if (WEventPhaseChange *event = dynamic_cast<WEventPhaseChange*>(e))
    {
        if (MTG_PHASE_COMBATEND == event->to->id)
            battleField.colorFlow = -1;
    }
    else if (dynamic_cast<WEventCardChangeType*> (e))
        Replace();
    return 0;
}

int GuiPlay::receiveEventMinus(WEvent * e)
{
    if (WEventZoneChange *event = dynamic_cast<WEventZoneChange*>(e))
    {
        if ((observer->players[0]->inPlay() == event->from) || (observer->players[1]->inPlay() == event->from))
            for (iterator it = cards.begin(); it != cards.end(); ++it)
                if (event->card->previous == (*it)->card || event->card == (*it)->card)
                {
                    if (event->card->previous && event->card->previous->attacker)
                        battleField.removeAttacker(event->card->previous);
                    else if (event->card->attacker)
                        battleField.removeAttacker(event->card);
                    CardView* cv = *it;
                    observer->getCardSelector()->Remove(cv);
                    cards.erase(it);
                    observer->mTrash->trash(cv);
                    Replace();
                    return 1;
                }
    }
    return 0;
}
