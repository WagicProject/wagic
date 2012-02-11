/*
The Action Stack contains all information for Game Events that can be interrupted (Interruptible)
*/
#include "PrecompiledHeader.h"

#include "ActionStack.h"
#include "CardGui.h"
#include "Damage.h"
#include "GameObserver.h"
#include "ManaCost.h"
#include "MTGAbility.h"
#include "Subtypes.h"
#include "TargetChooser.h"
#include "Translate.h"
#include "WResourceManager.h"
#include "ModRules.h"
#include "AllAbilities.h"
#include "CardSelector.h"
#include <typeinfo>

namespace
{
    float kGamepadIconSize = 0.5f;

    std::string kInterruptMessageString("Interrupt?");
    std::string kInterruptString(": Interrupt");
    std::string kNoString(": No");
    std::string kNoToAllString(": No To All");
    static const float kIconVerticalOffset = 24;

}

/*
NextGamePhase requested by user
*/
int NextGamePhase::resolve()
{
    observer->userRequestNextGamePhase(false, false);
    return 1;
}

const string NextGamePhase::getDisplayName() const
{
    std::ostringstream stream;
    stream << "NextGamePhase.  (Current phase is: " << observer->getCurrentGamePhaseName() << ")";

    return stream.str();
}

void NextGamePhase::Render()
{
    WFont * mFont = observer->getResourceManager()->GetWFont(Fonts::MAIN_FONT);
    mFont->SetBase(0);
    mFont->SetScale(DEFAULT_MAIN_FONT_SCALE);
    char buffer[200];
    int playerId = 1;
    if (observer->currentActionPlayer == observer->players[1])
        playerId = 2;

    sprintf(buffer, "%s %i : -> %s", _("Player").c_str(), playerId, observer->getNextGamePhaseName());

    mFont->DrawString(buffer, x + 30, y, JGETEXT_LEFT);
}

NextGamePhase::NextGamePhase(GameObserver* observer, int id) :
Interruptible(observer, id)
{
    mHeight = 40;
    type = ACTION_NEXTGAMEPHASE;
}

ostream& NextGamePhase::toString(ostream& out) const
{
    out << "NextGamePhase ::: ";
    return out;
}

const string Interruptible::getDisplayName() const
{
    return typeid(*this).name();
}

float Interruptible::GetVerticalTextOffset() const
{
    static const float kTextVerticalOffset = (mHeight - observer->getResourceManager()->GetWFont(Fonts::MAIN_FONT)->GetHeight()) / 2;
    return kTextVerticalOffset;
}

void Interruptible::Render(MTGCardInstance * source, JQuad * targetQuad, string alt1, string alt2, string action,
    bool bigQuad)
{
    WFont * mFont = observer->getResourceManager()->GetWFont(Fonts::MAIN_FONT);
    mFont->SetColor(ARGB(255,255,255,255));
    mFont->SetScale(DEFAULT_MAIN_FONT_SCALE);

    mFont->DrawString(_(action).c_str(), x + 35, y + GetVerticalTextOffset(), JGETEXT_LEFT);
    JRenderer * renderer = JRenderer::GetInstance();
    JQuadPtr quad = observer->getResourceManager()->RetrieveCard(source, CACHE_THUMB);
    if (!quad.get())
        quad = CardGui::AlternateThumbQuad(source);
    if (quad.get())
    {
        quad->SetColor(ARGB(255,255,255,255));
        float scale = mHeight / quad->mHeight;
        renderer->RenderQuad(quad.get(), x + (quad->mWidth * scale / 2), y + (quad->mHeight * scale / 2), 0, scale, scale);
    }
    else if (alt1.size())
    {
        mFont->DrawString(_(alt1).c_str(), x, y + GetVerticalTextOffset());
    }

    if (bigQuad)
    {
        Pos pos = Pos(CardGui::BigWidth / 2, CardGui::BigHeight / 2 - 10, 1.0, 0.0, 220);
        CardGui::DrawCard(source, pos, observer->getCardSelector()->GetDrawMode());
    }

    if (targetQuad)
    {
        float backupX = targetQuad->mHotSpotX;
        float backupY = targetQuad->mHotSpotY;
        targetQuad->SetColor(ARGB(255,255,255,255));
        targetQuad->SetHotSpot(targetQuad->mWidth / 2, targetQuad->mHeight / 2);
        float scale = mHeight / targetQuad->mHeight;
        renderer->RenderQuad(targetQuad, x + 150, y + ((mHeight - targetQuad->mHeight) / 2) + targetQuad->mHotSpotY, 0, scale, scale);
        targetQuad->SetHotSpot(backupX, backupY);
    }
    else if (alt2.size())
    {
        mFont->DrawString(_(alt2).c_str(), x + 120, y + GetVerticalTextOffset());
    }
}

/* Ability */
int StackAbility::resolve()
{
    return (ability->resolve());
}
void StackAbility::Render()
{
    string action = ability->getMenuText();
    MTGCardInstance * source = ability->source;
    string alt1 = source->getName();

    Targetable * _target = ability->target;
    if (ability->getActionTc())
    {
        Targetable * t = ability->getActionTc()->getNextTarget();
        if (t)
            _target = t;
    }
    Damageable * target = NULL;
    if (_target != ability->source && (dynamic_cast<MTGCardInstance *>(_target) || dynamic_cast<Player *>(_target)))
    {
        target = (Damageable *) _target;
    }

    JQuadPtr quad;
    string alt2 = "";
    if (target)
    {
        quad = target->getIcon();
        if (target->type_as_damageable == DAMAGEABLE_MTGCARDINSTANCE)
        {
            alt2 = ((MTGCardInstance *) target)->name;
        }
    }

    Interruptible::Render(source, quad.get(), alt1, alt2, action);
}
StackAbility::StackAbility(GameObserver* observer, int id, MTGAbility * _ability) :
Interruptible(observer, id), ability(_ability)
{
    type = ACTION_ABILITY;
}

ostream& StackAbility::toString(ostream& out) const
{
    out << "StackAbility ::: ability : " << ability;
    return out;
}

const string StackAbility::getDisplayName() const
{
    std::ostringstream stream;
    if(ability->source)
    stream << "StackAbility.  (Source: " << ability->source->getDisplayName() << ")";
    else
    stream << "StackAbility.  (Source: " << ability->getMenuText() << ")";

    return stream.str();
}

/* Spell Cast */

Spell::Spell(GameObserver* observer, MTGCardInstance * _source) :
Interruptible(observer, 0)
{
    source = _source;
    mHeight = 40;
    type = ACTION_SPELL;
    cost = NEW ManaCost();
    cost->extraCosts = NULL;
    tc = NULL;
    from = _source->getCurrentZone();
    payResult = ManaCost::MANA_UNPAID;
    source->castMethod = Constants::NOT_CAST;
}

Spell::Spell(GameObserver* observer, int id, MTGCardInstance * _source, TargetChooser * tc, ManaCost * _cost, int payResult) :
Interruptible(observer, id), tc(tc), cost(_cost), payResult(payResult)
{
    if (!cost)
    {
        cost = NEW ManaCost();
        cost->extraCosts = NULL;
    }
    source = _source;
    mHeight = 40;
    type = ACTION_SPELL;
    from = _source->getCurrentZone();

    _source->backupTargets.clear();
    if (tc)
    {
        Targetable* t = NULL;
        for(size_t i = 0;i < tc->getNbTargets();i++)
        {
            t = tc->getNextTarget(t);
            _source->backupTargets.push_back(t);
        }
    }

    // fill information on how the card came into this zone. Right now the quickest way is to do it here, based on how the mana was paid...
    switch(payResult) {
        case ManaCost::MANA_UNPAID:
            source->castMethod = Constants::NOT_CAST;
            break;
        case ManaCost::MANA_PAID:
        case ManaCost::MANA_PAID_WITH_KICKER:
            source->castMethod = Constants::CAST_NORMALLY;
            break;
        default:
            source->castMethod = Constants::CAST_ALTERNATE;
            break;
    }
}

int Spell::computeX(MTGCardInstance * card)
{
    ManaCost * c = cost->Diff(card->getManaCost());
    int x = c->getCost(Constants::NB_Colors);
    delete c;
    return x;
}

bool Spell::FullfilledAlternateCost(const int &costType)
{
    bool hasFullfilledAlternateCost = false;

    switch (costType)
    {
    case ManaCost::MANA_PAID_WITH_KICKER:
        hasFullfilledAlternateCost = (payResult == ManaCost::MANA_PAID_WITH_KICKER);
        break;
    case ManaCost::MANA_PAID_WITH_ALTERNATIVE:
        hasFullfilledAlternateCost = (payResult == ManaCost::MANA_PAID_WITH_ALTERNATIVE);
        break;
    case ManaCost::MANA_PAID_WITH_BUYBACK:
        hasFullfilledAlternateCost = (payResult == ManaCost::MANA_PAID_WITH_BUYBACK);
        break;
    case ManaCost::MANA_PAID_WITH_FLASHBACK:
        hasFullfilledAlternateCost = (payResult == ManaCost::MANA_PAID_WITH_FLASHBACK);
        break;
    case ManaCost::MANA_PAID_WITH_RETRACE:
        hasFullfilledAlternateCost = (payResult == ManaCost::MANA_PAID_WITH_RETRACE);
        break;
    }

    return hasFullfilledAlternateCost;
}

const string Spell::getDisplayName() const
{
    return source->getName();
}
Spell::~Spell()
{
    SAFE_DELETE(cost);
    SAFE_DELETE(tc);
}

int Spell::resolve()
{
    if (!source->hasType(Subtypes::TYPE_INSTANT) && !source->hasType(Subtypes::TYPE_SORCERY))
    {
        Player * p = source->controller();
        int castMethod = source->castMethod;
        vector<Targetable*>backupTgt = source->backupTargets;
        source = p->game->putInZone(source, from, p->game->battlefield);
        
        // We need to get the information about the cast method on both the card in the stack AND the card in play,
        //so we copy it from the previous card (in the stack) to the new one (in play).
        source->castMethod = castMethod; 
        source->backupTargets = backupTgt;
        from = p->game->battlefield;
    }

    //Play SFX
    if (options[Options::SFXVOLUME].number > 0)
    {

		if(observer->getResourceManager())
			observer->getResourceManager()->PlaySample(source->getSample());
    }
    AbilityFactory af(observer);
    af.addAbilities(observer->mLayers->actionLayer()->getMaxId(), this);
    return 1;
}

MTGCardInstance * Spell::getNextCardTarget(MTGCardInstance * previous)
{
    if (!tc)
        return NULL;
    return tc->getNextCardTarget(previous);
}
Player * Spell::getNextPlayerTarget(Player * previous)
{
    if (!tc)
        return NULL;
    return tc->getNextPlayerTarget(previous);
}
Damageable * Spell::getNextDamageableTarget(Damageable * previous)
{
    if (!tc)
        return NULL;
    return tc->getNextDamageableTarget(previous);
}
Interruptible * Spell::getNextInterruptible(Interruptible * previous, int type)
{
    if (!tc)
        return NULL;
    return tc->getNextInterruptible(previous, type);
}
Spell * Spell::getNextSpellTarget(Spell * previous)
{
    if (!tc)
        return NULL;
    return tc->getNextSpellTarget(previous);
}
Damage * Spell::getNextDamageTarget(Damage * previous)
{
    if (!tc)
        return NULL;
    return tc->getNextDamageTarget(previous);
}
Targetable * Spell::getNextTarget(Targetable * previous)
{
    if (!tc)
        return NULL;
    return tc->getNextTarget(previous);
}

int Spell::getNbTargets()
{
    if (!tc)
        return 0;
    return (int) (tc->getNbTargets());
}

void Spell::Render()
{
    string action = source->getName();
    string alt1 = "";

    string alt2 = "";
    Damageable * target = getNextDamageableTarget();
    JQuadPtr quad;
    if (target)
    {
        quad = target->getIcon();
        if (target->type_as_damageable == DAMAGEABLE_MTGCARDINSTANCE)
        {
            alt2 = ((MTGCardInstance *) target)->name;
        }
    }
    Interruptible::Render(source, quad.get(), alt1, alt2, action, true);
}

ostream& Spell::toString(ostream& out) const
{
    out << "Spell ::: cost : " << cost;
    return out;
}

/* Put a card in graveyard */

PutInGraveyard::PutInGraveyard(GameObserver* observer, int id, MTGCardInstance * _card) :
Interruptible(observer, id)
{
    card = _card;
    removeFromGame = 0;
    type = ACTION_PUTINGRAVEYARD;
}

int PutInGraveyard::resolve()
{
    MTGGameZone * zone = card->getCurrentZone();
    if (zone == observer->players[0]->game->inPlay || zone == observer->players[1]->game->inPlay)
    {
        card->owner->game->putInZone(card, zone, card->owner->game->graveyard);
        return 1;
    }
    return 0;
}

void PutInGraveyard::Render()
{
    WFont * mFont = observer->getResourceManager()->GetWFont(Fonts::MAIN_FONT);
    mFont->SetBase(0);
    mFont->SetScale(DEFAULT_MAIN_FONT_SCALE);
    if (!removeFromGame)
    {
        mFont->DrawString(_("goes to graveyard").c_str(), x + 30, y, JGETEXT_LEFT);
    }
    else
    {
        mFont->DrawString(_("is exiled").c_str(), x + 30, y, JGETEXT_LEFT);
    }
    JRenderer * renderer = JRenderer::GetInstance();
    JQuadPtr quad = observer->getResourceManager()->RetrieveCard(card, CACHE_THUMB);
    if (quad.get())
    {
        quad->SetColor(ARGB(255,255,255,255));
        float scale = 30 / quad->mHeight;
        renderer->RenderQuad(quad.get(), x, y, 0, scale, scale);
    }
    else
    {
        mFont->DrawString(_(card->name).c_str(), x, y - 15);
    }
}

ostream& PutInGraveyard::toString(ostream& out) const
{
    out << "PutInGraveyard ::: removeFromGame : " << removeFromGame;
    return out;
}

/* Draw a Card */
DrawAction::DrawAction(GameObserver* observer, int id, Player * _player, int _nbcards) :
Interruptible(observer, id), nbcards(_nbcards), player(_player)
{
}

int DrawAction::resolve()
{
    for (int i = 0; i < nbcards; i++)
    {
        player->game->drawFromLibrary();
    }
    return 1;
}

void DrawAction::Render()
{
    WFont * mFont = observer->getResourceManager()->GetWFont(Fonts::MAIN_FONT);
    mFont->SetBase(0);
    mFont->SetScale(DEFAULT_MAIN_FONT_SCALE);
    char buffer[200];
    int playerId = 1;
    if (player == observer->players[1])
        playerId = 2;
    sprintf(buffer, _("Player %i draws %i card").c_str(), playerId, nbcards);
    mFont->DrawString(buffer, x + 35, y + GetVerticalTextOffset(), JGETEXT_LEFT);
}

ostream& DrawAction::toString(ostream& out) const
{
    out << "DrawAction ::: nbcards : " << nbcards << " ; player : " << player;
    return out;
}
//////
LifeAction::LifeAction(GameObserver* observer, int id, Damageable * _target, int amount) :
Interruptible(observer, id), amount(amount),target(_target)
{
}

int LifeAction::resolve()
{
target->life += amount;
    return 1;
}

void LifeAction::Render()
{
    WFont * mFont = observer->getResourceManager()->GetWFont(Fonts::MAIN_FONT);
    mFont->SetBase(0);
    mFont->SetScale(DEFAULT_MAIN_FONT_SCALE);
    char buffer[200];
    if(amount > 0)
        sprintf(buffer, _("Player gains %i life").c_str(), amount);
    else if(amount < 0)
        sprintf(buffer, _("Player loses %i life").c_str(), amount);
    else
        sprintf(buffer, _("Nothing happened").c_str(), amount);
    mFont->DrawString(buffer, x + 20, y, JGETEXT_LEFT);
}

ostream& LifeAction::toString(ostream& out) const
{
    out << "LifeAction ::: amount : " << amount << " ; target : " << target;
    return out;
}
/* The Action Stack itself */

int ActionStack::addPutInGraveyard(MTGCardInstance * card)
{
    PutInGraveyard * death = NEW PutInGraveyard(observer, mObjects.size(), card);
    addAction(death);
    return 1;
}

int ActionStack::addAbility(MTGAbility * ability)
{
    StackAbility * stackAbility = NEW StackAbility(observer, mObjects.size(), ability);
    int result = addAction(stackAbility);
    if (!observer->players[0]->isAI() && ability->source->controller() == observer->players[0] && 0
        == options[Options::INTERRUPTMYABILITIES].number)
        interruptDecision[0] = DONT_INTERRUPT;
    return result;
}

int ActionStack::addDraw(Player * player, int nb_cards)
{
    DrawAction * draw = NEW DrawAction(observer, mObjects.size(), player, nb_cards);
    addAction(draw);
    return 1;
}

int ActionStack::addLife(Damageable * _target, int amount)
{
    LifeAction * life = NEW LifeAction(observer, mObjects.size(), _target, amount);
    addAction(life);
    return 1;
}

int ActionStack::addDamage(MTGCardInstance * _source, Damageable * _target, int _damage)
{
    Damage * damage = NEW Damage(observer, _source, _target, _damage);
    addAction(damage);
    _source->thatmuch = _damage;
    _target->thatmuch = _damage;
    return 1;
}

int ActionStack::AddNextGamePhase()
{
    if (getNext(NULL, NOT_RESOLVED))
        return 0;

    NextGamePhase * next = NEW NextGamePhase(observer, mObjects.size());
    addAction(next);
    int playerId = (observer->currentActionPlayer == observer->players[1]) ? 1 : 0;
    interruptDecision[playerId] = DONT_INTERRUPT;
    return 1;
}

int ActionStack::AddNextCombatStep()
{
    if (getNext(NULL, NOT_RESOLVED))
        return 0;

    NextGamePhase * next = NEW NextGamePhase(observer, mObjects.size());
    addAction(next);
    return 1;
}

int ActionStack::setIsInterrupting(Player * player, bool log)
{
    askIfWishesToInterrupt = NULL;

    if (!gModRules.game.canInterrupt())
    {
        cancelInterruptOffer(DONT_INTERRUPT, log);
        return 0;
    }

     //Is it a valid interruption request, or is uninterruptible stuff going on in the game?
    if (observer->getCurrentTargetChooser())
    {
        DebugTrace("ActionStack: WARNING - We were asked to interrupt, During Targetchoosing" << endl
            << "source: " << (observer->getCurrentTargetChooser()->source ? observer->getCurrentTargetChooser()->source->name : "None" ) << endl );
        return 0;
    }

    int playerId = (player == observer->players[1]) ? 1 : 0;
    interruptDecision[playerId] = INTERRUPT;
    observer->isInterrupting = player;
    if(log)
        observer->logAction(player, "yes");
    return 1;
}

int ActionStack::addAction(Interruptible * action)
{
    for (int i = 0; i < 2; i++)
    {
        interruptDecision[i] = NOT_DECIDED;
    }
    Add(action);
    lastActionController = observer->currentlyActing();
    DebugTrace("Action added to stack: " << action->getDisplayName());

    return 1;
}

Spell * ActionStack::addSpell(MTGCardInstance * _source, TargetChooser * tc, ManaCost * mana, int payResult,
    int storm)
{
    DebugTrace("ACTIONSTACK Add spell");
    if (storm > 0)
    {
        mana = NULL;
    }
    Spell * spell = NEW Spell(observer, mObjects.size(), _source, tc, mana, payResult);
    addAction(spell);
    if (!observer->players[0]->isAI() && _source->controller() == observer->players[0] && 0 == options[Options::INTERRUPTMYSPELLS].number)
        interruptDecision[0] = DONT_INTERRUPT;
    return spell;
}

Interruptible * ActionStack::getAt(int id)
{
    if (id < 0)
        id = mObjects.size() + id;
    if (id > (int)(mObjects.size()) - 1 || id < 0)
        return NULL;
    return (Interruptible *) mObjects[id];
}

ActionStack::ActionStack(GameObserver* game)
    : GuiLayer(game), currentTutorial(0)
{
    for (int i = 0; i < 2; i++)
        interruptDecision[i] = NOT_DECIDED;
    askIfWishesToInterrupt = NULL;
    timer = -1;
    currentState = -1;
    mode = ACTIONSTACK_STANDARD;
    checked = 0;

    if(!observer->getResourceManager()) return;
    for (int i = 0; i < 8; ++i)
    {
        std::ostringstream stream;
        stream << "iconspsp" << i;
        pspIcons[i] = observer->getResourceManager()->RetrieveQuad("iconspsp.png", (float) i * 32, 0, 32, 32, stream.str(), RETRIEVE_MANAGE);
        pspIcons[i]->SetHotSpot(16, 16);
    }
}

int ActionStack::has(MTGAbility * ability)
{
    for (size_t i = 0; i < mObjects.size(); i++)
    {
        if (((Interruptible *) mObjects[i])->type == ACTION_ABILITY)
        {
            StackAbility * action = ((StackAbility *) mObjects[i]);
            if (action->state == NOT_RESOLVED && action->ability == ability)
                return 1;
        }
    }
    return 0;
}

int ActionStack::has(Interruptible * action)
{
    for (size_t i = 0; i < mObjects.size(); i++)
    {
        if (mObjects[i] == action)
            return 1;
    }
    return 0;
}

int ActionStack::resolve()
{
    Interruptible * action = getLatest(NOT_RESOLVED);

    if (!action)
        return 0;

    DebugTrace("Resolving Action on stack: " << action->getDisplayName());
    if (action->resolve())
    {
        action->state = RESOLVED_OK;
    }
    else
    {
        action->state = RESOLVED_NOK;
    }
    if (action->type == ACTION_DAMAGE)
        ((Damage *) action)->target->afterDamage();
    if (!getNext(NULL, NOT_RESOLVED))
    {
        for (int i = 0; i < 2; i++)
        {
					if (interruptDecision[i] != 2)
            interruptDecision[i] = NOT_DECIDED;
        }
    }
    else
    {
        for (int i = 0; i < 2; i++)
        {
            if (interruptDecision[i] != DONT_INTERRUPT_ALL)
                interruptDecision[i] = NOT_DECIDED;
        }
    }
    lastActionController = NULL;
    return 1;

}

Interruptible * ActionStack::getPrevious(Interruptible * next, int type, int state, int display)
{
    int n = getPreviousIndex(next, type, state, display);
    if (n == -1)
        return NULL;
    return ((Interruptible *) mObjects[n]);
}

int ActionStack::getPreviousIndex(Interruptible * next, int type, int state, int display)
{
    int found = 0;
    if (!next)
        found = 1;
    for (int i = (int)(mObjects.size()) - 1; i >= 0; i--)
    {
        Interruptible * current = (Interruptible *) mObjects[i];
        if (found && (type == 0 || current->type == type) && (state == 0 || current->state == state) && (display
            == -1 || current->display == display))
        {
            return i;
        }
        if (current == next)
            found = 1;
    }
    if (!found)
        return getPreviousIndex(NULL, type, state, display);
    return -1;
}

int ActionStack::count(int type, int state, int display)
{
    int result = 0;
    for (size_t i = 0; i < mObjects.size(); i++)
    {
        Interruptible * current = (Interruptible *) mObjects[i];
        if ((type == 0 || current->type == type) && (state == 0 || current->state == state) && (display == -1
            || current->display == display))
        {
            result++;
        }
    }
    return result;
}

Interruptible * ActionStack::getActionElementFromCard(MTGCardInstance * card)
{

	if(!card)
    return 0;
    for (size_t i = 0; i < mObjects.size(); i++)
    {
        Interruptible * current = (Interruptible *) mObjects[i];
        if (current->source == card)
        {
            return current;
        }
    }  
    return NULL;
}

Interruptible * ActionStack::getNext(Interruptible * previous, int type, int state, int display)
{
    int n = getNextIndex(previous, type, state, display);
    if (n == -1)
        return NULL;
    return ((Interruptible *) mObjects[n]);
}

int ActionStack::getNextIndex(Interruptible * previous, int type, int state, int display)
{
    int found = 0;
    if (!previous)
        found = 1;
    for (size_t i = 0; i < mObjects.size(); i++)
    {
        Interruptible * current = (Interruptible *) mObjects[i];
        if (found && (type == 0 || current->type == type) && (state == 0 || current->state == state) && (display
            == -1 || current->display == display))
        {
            return i;
        }
        if (current == previous)
            found = 1;
    }
    if (!found)
        return getNextIndex(NULL, type, state, display);
    return -1;
}

Interruptible * ActionStack::getLatest(int state)
{
    for (int i = (int)(mObjects.size()) - 1; i >= 0; i--)
    {
        Interruptible * action = ((Interruptible *) mObjects[i]);
        if (action->state == state)
            return action;
    }
    return NULL;
}

int ActionStack::receiveEventPlus(WEvent * event)
{
    int result = 0;
    for (size_t i = 0; i < mObjects.size(); ++i)
    {
        Interruptible * current = (Interruptible *) mObjects[i];
        result += current->receiveEvent(event);
    }
    return result;
}

void ActionStack::Update(float dt)
{
    //This is a hack to avoid updating the stack while tuto messages are being shown
    //Ideally, the tuto messages should be moved to a layer above this one
    //No need for Tuto when no human in game
    if (getCurrentTutorial() && (observer->players[0]->isHuman() || observer->players[1]->isHuman() ) )
        return;

    askIfWishesToInterrupt = NULL;
    //modal = 0;

    TargetChooser * tc = observer->getCurrentTargetChooser();
    int newState = observer->getCurrentGamePhase();
    currentState = newState;
    if (!tc)
        checked = 0;

    //Select Stack's display mode
    if (mode == ACTIONSTACK_STANDARD && tc && !checked)
    {
        checked = 1;

        for (size_t i = 0; i < mObjects.size(); i++)
        {
            Interruptible * current = (Interruptible *) mObjects[i];
            if (tc->canTarget(current))
            {
                if (mCurr < (int) mObjects.size() && mObjects[mCurr])
                    mObjects[mCurr]->Leaving(JGE_BTN_UP);
                current->display = 1;
                mCurr = i;
                mObjects[mCurr]->Entering();
                mode = ACTIONSTACK_TARGET;
                modal = 1;
            }
            else
            {
                current->display = 0;
            }
        }
        if (mode != ACTIONSTACK_TARGET)
        {
        }
    }
    else if (mode == ACTIONSTACK_TARGET && !tc)
    {
        mode = ACTIONSTACK_STANDARD;
        checked = 0;
    }

    if (mode == ACTIONSTACK_STANDARD)
    {
        modal = 0;
        if (getLatest(NOT_RESOLVED))
        {
            Interruptible * currentSpell = (Interruptible *)getLatest(NOT_RESOLVED);
            MTGCardInstance * card = currentSpell->source;
            if(card && card->has(Constants::SPLITSECOND))
            {
                resolve();
            }
            else
            {
                int currentPlayerId = 0;
                int otherPlayerId = 1;
                if (observer->currentlyActing() != observer->players[0])
                {
                    currentPlayerId = 1;
                    otherPlayerId = 0;
                }
                if (interruptDecision[currentPlayerId] == NOT_DECIDED)
                {
                    askIfWishesToInterrupt = observer->players[currentPlayerId];
                    observer->isInterrupting = observer->players[currentPlayerId];
                    modal = 1;
                }
                else if (interruptDecision[currentPlayerId] == INTERRUPT)
                {
                    observer->isInterrupting = observer->players[currentPlayerId];

                }
                else
                {
                    if (interruptDecision[otherPlayerId] == NOT_DECIDED)
                    {
                        askIfWishesToInterrupt = observer->players[otherPlayerId];
                        observer->isInterrupting = observer->players[otherPlayerId];
                        modal = 1;
                    }
                    else if (interruptDecision[otherPlayerId] == INTERRUPT)
                    {
                        observer->isInterrupting = observer->players[otherPlayerId];
                    }
                    else
                    {
                        resolve();
                    }
                }
            }
        }
    }
    else if (mode == ACTIONSTACK_TARGET)
    {
        GuiLayer::Update(dt);
    }
    if (askIfWishesToInterrupt)
    {
        // WALDORF - added code to use a game option setting to determine how
        // long the Interrupt timer should be. If it is set to zero (0), the
        // game will wait for ever for the user to make a selection.
        if (options[Options::INTERRUPT_SECONDS].number > 0)
        {
            int extraTime = 0;
            //extraTime is a multiplier, it counts the number of unresolved stack actions
            //then is used to extend the time you have to interupt.
            //this prevents you from "running out of time" while deciding.
            //before this int was added, it was possible to run out of time if you had 10 stack actions
            //and set the timer to 4 secs. BUG FIX //http://code.google.com/p/wagic/issues/detail?id=464
            extraTime = count(0, NOT_RESOLVED, 0);
            if (extraTime == 0)
	            extraTime = 1;//we never want this int to be 0.

            if (timer < 0)
                timer = static_cast<float>(options[Options::INTERRUPT_SECONDS].number * extraTime);
            timer -= dt;
            if (timer < 0)
                cancelInterruptOffer();
        }
    }
}

void ActionStack::cancelInterruptOffer(InterruptDecision cancelMode, bool log, Player* cancelMe)
{
    int playerId;
    if(cancelMe != NULL) {
        if(observer->players[1] == cancelMe)
            playerId = 1;
        else
            playerId = 0;
    } else {
        playerId = (observer->isInterrupting == observer->players[1]) ? 1 : 0;
    }
    interruptDecision[playerId] = cancelMode;
    askIfWishesToInterrupt = NULL;
    observer->isInterrupting = NULL;
    timer = -1;
    if(log) {
        stringstream stream;
        stream << "no " << cancelMode;
        observer->logAction(playerId, stream.str());
    }
}

void ActionStack::endOfInterruption(bool log)
{
    int playerId = (observer->isInterrupting == observer->players[1]) ? 1 : 0;
    interruptDecision[playerId] = NOT_DECIDED;
    observer->isInterrupting = NULL;
    if(log)
        observer->logAction(playerId, "endinterruption");
}

JButton ActionStack::handleInterruptRequest( JButton inputKey, int& x, int& y )
{
    if ( gModRules.game.canInterrupt() && y >= 10 && y < (kIconVerticalOffset + 16))
    {
        if (x >= interruptBtnXOffset && x < noBtnXOffset )
            return JGE_BTN_SEC;
        
        if (x >= noBtnXOffset && x < noToAllBtnXOffset)
            return JGE_BTN_OK;
        
        if (x >= noToAllBtnXOffset && x < interruptDialogWidth)
            return JGE_BTN_PRI;
    }
    
    return inputKey;
}


bool ActionStack::CheckUserInput(JButton inputKey)
{
    JButton key = inputKey;
    JButton trigger = (options[Options::REVERSETRIGGERS].number ? JGE_BTN_NEXT : JGE_BTN_PREV);
    if (mode == ACTIONSTACK_STANDARD)
    {        
        if (askIfWishesToInterrupt)
        {
            int x,y;
            if(observer->getInput()->GetLeftClickCoordinates(x, y))
            {
                key = handleInterruptRequest(inputKey, x, y);
            }
            
            if (JGE_BTN_SEC == key && gModRules.game.canInterrupt())
            {
                setIsInterrupting(askIfWishesToInterrupt);
                return true;
            }
            else if ((JGE_BTN_OK == key) || (trigger == key))
            {
                cancelInterruptOffer();
                return true;
            }
            else if ((JGE_BTN_PRI == key))
            {
                cancelInterruptOffer(DONT_INTERRUPT_ALL);
                return true;
            }
            return true;
        }
        else if (observer->isInterrupting)
        {
            if (JGE_BTN_SEC == key)
            {
                endOfInterruption();
                return true;
            }
        }
    }
    else if (mode == ACTIONSTACK_TARGET)
    {
        if (modal)
        {
            if (JGE_BTN_UP == key)
            {
                if (mObjects[mCurr])
                {
                    int n = getPreviousIndex(((Interruptible *) mObjects[mCurr]), 0, 0, 1);
                    if (n != -1 && n != mCurr && mObjects[mCurr]->Leaving(JGE_BTN_UP))
                    {
                        mCurr = n;
                        mObjects[mCurr]->Entering();
                        DebugTrace("ACTIONSTACK UP TO mCurr = " << mCurr);
                    }
                }
                return true;
            }
            else if (JGE_BTN_DOWN == key)
            {
                if( mObjects[mCurr])
                {
                    int n = getNextIndex(((Interruptible *) mObjects[mCurr]), 0, 0, 1);
                    if (n!= -1 && n != mCurr && mObjects[mCurr]->Leaving(JGE_BTN_DOWN))
                    {
                        mCurr = n;
                        mObjects[mCurr]->Entering();
                        DebugTrace("ACTIONSTACK DOWN TO mCurr " << mCurr);
                    }
                }
                return true;
            }
            else if (JGE_BTN_OK == key)
            {
                DebugTrace("ACTIONSTACK CLICKED mCurr = " << mCurr);

                observer->stackObjectClicked(((Interruptible *) mObjects[mCurr]));
                return true;
            }
            return true; //Steal the input to other layers if we're visible
        }
        if (JGE_BTN_CANCEL == key)
        {
            if (modal) modal = 0;
            else modal = 1;
            return true;
        }
    }
    return false;
}

//Cleans history of last turn
int ActionStack::garbageCollect()
{
    std::vector<JGuiObject *>::iterator iter = mObjects.begin();

    while (iter != mObjects.end())
    {
        Interruptible * current = ((Interruptible *) *iter);
        if (current->state != NOT_RESOLVED)
        {
            AManaProducer * amp = dynamic_cast<AManaProducer*>(current);
            if(amp)
            {
                manaObjects.erase(iter);
            }
            iter = mObjects.erase(iter);
            SAFE_DELETE(current);
        }
        else
            ++iter;
    }
    return 1;
}

void ActionStack::Fizzle(Interruptible * action)
{
    if (!action)
    {
        DebugTrace("ACTIONSTACK ==ERROR==: action is NULL in ActionStack::Fizzle");
        return;
    }
    if (action->type == ACTION_SPELL)
    {
        Spell * spell = (Spell *) action;
        spell->source->controller()->game->putInGraveyard(spell->source);
    }
    action->state = RESOLVED_NOK;
}

void ActionStack::Render()
{
    //This is a hack to avoid rendering the stack above the tuto messages
    //Ideally, the tuto messages should be moved to a layer above this one
    if (getCurrentTutorial())
        return;

    static const float kSpacer = 8;
    static const float x0 = 250;
    static const float y0 = 0;
    float width = 200;
    float height = 90;
    float currenty = y0 + 5;

    if (mode == ACTIONSTACK_STANDARD)
    {
        if (!askIfWishesToInterrupt || !askIfWishesToInterrupt->displayStack())
            return;

        for (size_t i = 0; i < mObjects.size(); i++)
        {
            Interruptible * current = (Interruptible *) mObjects[i];
            if (current->state == NOT_RESOLVED)
                height += current->mHeight;
        }

        WFont * mFont = observer->getResourceManager()->GetWFont(Fonts::MAIN_FONT);
        mFont->SetBase(0);
        mFont->SetScale(DEFAULT_MAIN_FONT_SCALE);
        mFont->SetColor(ARGB(255,255,255,255));
        JRenderer * renderer = JRenderer::GetInstance();

        renderer->FillRoundRect(x0 + 16, y0 + 16, width + 2, height + 2, 10, ARGB(128,0,0,0));
        renderer->FillRoundRect(x0 - 5, y0, width + 2, height + 2, 10, ARGB(200,0,0,0));
        renderer->DrawRoundRect(x0 - 5, y0, width + 2, height + 2, 10, ARGB(255,255,255,255));

        std::ostringstream stream;
        // WALDORF - changed "interrupt ?" to "Interrupt?". Don't display count down
        // seconds if the user disables auto progressing interrupts by setting the seconds
        // value to zero in Options.

        // Mootpoint 01/12/2011: draw the interrupt text first, at the top.  Offset the rest of the 
        // unresolved stack effects down so that they don't collide with the interrupt text.
        if (options[Options::INTERRUPT_SECONDS].number == 0)
            stream << _(kInterruptMessageString);
        else
            stream << _(kInterruptMessageString) << " " << static_cast<int>(timer);

        mFont->DrawString(stream.str(), x0 + 5, currenty);

//        static const float kIconVerticalOffset = 24;
        static const float kIconHorizontalOffset = 9;
        static const float kBeforeIconSpace = 10;
  
        //Render "interrupt?" text + possible actions
        {
            float currentx = x0 + 10;
            interruptBtnXOffset = static_cast<int>(currentx);
            
            if (gModRules.game.canInterrupt())
            {
                renderer->RenderQuad(pspIcons[7].get(), currentx, kIconVerticalOffset, 0, kGamepadIconSize, kGamepadIconSize);
                currentx+= kIconHorizontalOffset;
                mFont->DrawString(_(kInterruptString), currentx, kIconVerticalOffset - 6);
                currentx+= mFont->GetStringWidth(_(kInterruptString).c_str()) + kBeforeIconSpace;
            }

            noBtnXOffset = static_cast<int>(currentx);
            
            renderer->RenderQuad(pspIcons[4].get(), currentx, kIconVerticalOffset, 0, kGamepadIconSize, kGamepadIconSize);
            currentx+= kIconHorizontalOffset;
            mFont->DrawString(_(kNoString), currentx, kIconVerticalOffset - 6);
            currentx+= mFont->GetStringWidth(_(kNoString).c_str()) + kBeforeIconSpace;

            noToAllBtnXOffset = static_cast<int>(currentx);
            if (mObjects.size() > 1)
            {
                renderer->RenderQuad(pspIcons[6].get(), currentx, kIconVerticalOffset, 0, kGamepadIconSize, kGamepadIconSize);
                currentx+= kIconHorizontalOffset;
                mFont->DrawString(_(kNoToAllString), currentx, kIconVerticalOffset - 6);
                currentx+= mFont->GetStringWidth(_(kNoToAllString).c_str()) + kBeforeIconSpace;
            }
            
            interruptDialogWidth = static_cast<int>(currentx);
        }

        currenty += kIconVerticalOffset + kSpacer;

        for (size_t i = 0; i < mObjects.size(); i++)
        {
            Interruptible * current = (Interruptible *) mObjects[i];
            if (current && current->state == NOT_RESOLVED)
            {
                current->x = x0;
                current->y = currenty;
                current->Render();

                currenty += current->mHeight;
            }
        }
    }
    else if (mode == ACTIONSTACK_TARGET && modal)
    {
        for (size_t i = 0; i < mObjects.size(); i++)
        {
            Interruptible * current = (Interruptible *) mObjects[i];
            if (current->display)
                height += current->mHeight;
        }

        WFont * mFont = observer->getResourceManager()->GetWFont(Fonts::MAIN_FONT);
        mFont->SetScale(DEFAULT_MAIN_FONT_SCALE);
        mFont->SetColor(ARGB(255,255,255,255));

        JRenderer * renderer = JRenderer::GetInstance();
        renderer->FillRect(x0, y0, width, height, ARGB(200,0,0,0));
        renderer->DrawRect(x0 - 1, y0 - 1, width + 2, height + 2, ARGB(255,255,255,255));

        for (size_t i = 0; i < mObjects.size(); i++)
        {
            Interruptible * current = (Interruptible *) mObjects[i];
            if (mObjects[i] != NULL && current->display)
            {
                ((Interruptible *) mObjects[i])->x = x0 + 5;
                if (i != mObjects.size() - 1)
                {
                    ((Interruptible *) mObjects[i])->y = currenty;
                    currenty += ((Interruptible *) mObjects[i])->mHeight;
                }
                else
                {
                    ((Interruptible *) mObjects[i])->y = currenty + 40;
                    currenty += ((Interruptible *) mObjects[i])->mHeight + 40;
                }
                mObjects[i]->Render();
            }
        }
    }
}

#if defined (WIN32) || defined (LINUX)  || defined (IOS)

void Interruptible::Dump()
{
    string stype, sstate, sdisplay = "";
    switch (type)
    {
    case ACTION_SPELL:
        stype = "spell";
        break;
    case ACTION_DAMAGE:
        stype = "damage";
        break;
    case ACTION_DAMAGES:
        stype = "damages";
        break;
    case ACTION_NEXTGAMEPHASE:
        stype = "next phase";
        break;
    case ACTION_DRAW:
        stype = "draw";
        break;
    case ACTION_PUTINGRAVEYARD:
        stype = "put in graveyard";
        break;
    case ACTION_ABILITY:
        stype = "ability";
        break;
    default:
        stype = "unknown";
        break;
    }

    switch(state)
    {
    case NOT_RESOLVED:
        sstate = "not resolved";
        break;
    case RESOLVED_OK:
        sstate = "resolved";
        break;
    case RESOLVED_NOK:
        sstate = "fizzled";
        break;
    default:
        sstate = "unknown";
        break;
    }
    DebugTrace("type: " << stype << " " << type << " - state: " << sstate << " " << state << " - display: " << display);
}

void ActionStack::Dump()
{
    DebugTrace("=====\nDumping Action Stack=====");
    for (size_t i = 0; i < mObjects.size(); i++)
    {
        Interruptible * current = (Interruptible *)mObjects[i];
        current->Dump();
    }
}

#endif
