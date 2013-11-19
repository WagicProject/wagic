#include "PrecompiledHeader.h"

#include "AIPlayer.h"
#include "GameStateDuel.h"
#include "DeckManager.h"
#include "CardSelector.h"


// Instances for Factory
#include "AIPlayerBaka.h"

#ifdef AI_CHANGE_TESTING
#include "AIPlayerBakaB.h"
#endif

namespace AI {


bool Action::parseLine(const string& s)
{
  return true;
}

ostream& operator<<(ostream& out, const Action&)
{
  return out;
}

istream& operator>>(istream& in, Action& a)
{
    string s;

    while(std::getline(in, s))
    {
        if(!a.parseLine(s))
        {
            break;
        }
    }

    return in;
}

int AIPlayer::totalAIDecks = -1;

AIAction::AIAction(AIPlayer * owner, MTGCardInstance * c, MTGCardInstance * t)
    : owner(owner), ability(NULL), player(NULL), click(c), target(t)
{
    // useability tweak - assume that the user is probably going to want to see the full res card,
    // so prefetch it. The idea is that we do it here as we want to start the prefetch before it's time to render,
    // and waiting for it to actually go into play is too late, as we start drawing the card during the interrupt window.
    // This is a good intercept point, as the AI has committed to using this card.

    // if we're not in text mode, always get the thumb
    if (owner->getObserver()->getCardSelector()->GetDrawMode() != DrawMode::kText)
    {
        //DebugTrace("Prefetching AI card going into play: " << c->getImageName());
        if(owner->getObserver()->getResourceManager())
            owner->getObserver()->getResourceManager()->RetrieveCard(c, RETRIEVE_THUMB);
        
        // also cache the large image if we're using kNormal mode
        if (owner->getObserver()->getCardSelector()->GetDrawMode() == DrawMode::kNormal)
        {
            if(owner->getObserver()->getResourceManager())
                owner->getObserver()->getResourceManager()->RetrieveCard(c);
        }
    }
}

int AIAction::Act()
{
    GameObserver * g = owner->getObserver();
    if (player && !playerAbilityTarget)
    {
        g->cardClick(NULL, player);
        return 1;
    }
    if (ability)
    {
        g->cardClick(click, ability);
        if (target && !mAbilityTargets.size())
        {
            g->cardClick(target);
            return 1;
        }
        else if(playerAbilityTarget && !mAbilityTargets.size())
        {
            g->cardClick(NULL,(Player*)playerAbilityTarget);
            return 1;
        }
        if(mAbilityTargets.size())
        {
            return clickMultiAct(mAbilityTargets);
        }
    }
    else  if(mAbilityTargets.size())
    {
        return clickMultiAct(mAbilityTargets);
    }
    else if (click)
    { //Shouldn't be used, really...
        g->cardClick(click, click);
        if (target)
            g->cardClick(target);
        return 1;
    }
    return 0;
}

int AIAction::clickMultiAct(vector<Targetable*>& actionTargets)
{
    GameObserver * g = owner->getObserver();
    TargetChooser * tc = g->getCurrentTargetChooser();
    if(!tc) return 0;
    vector<Targetable*>::iterator ite = actionTargets.begin();
    while(ite != actionTargets.end())
    {
        MTGCardInstance * card = ((MTGCardInstance *) (*ite));
        if(card == (MTGCardInstance*)tc->source)//click source first.
        {
            g->cardClick(card);
            ite = actionTargets.erase(ite);
            continue;
        }
        ite++;
    }

    //shuffle to make it less predictable, otherwise ai will always seem to target from right to left. making it very obvious.
    owner->getRandomGenerator()->random_shuffle(actionTargets.begin(), actionTargets.end());

    for(int k = 0 ;k < int(actionTargets.size()) && k < tc->maxtargets; k++)
    {
        if (MTGCardInstance * card = dynamic_cast<MTGCardInstance *>(actionTargets[k]))
        {
            if(k+1 == int(actionTargets.size()))
                tc->done = true;
            g->cardClick(card);
        }
    }
    tc->attemptsToFill++;
    return 1;
}

AIPlayer::AIPlayer(GameObserver *observer, string file, string fileSmall, string avatarFile, MTGDeck * deck) :
    Player(observer, file, fileSmall, deck)
{
    agressivity = 50;
    forceBestAbilityUse = false;
    playMode = Player::MODE_AI;
    mFastTimerMode = false;

    if(avatarFile != "")
    {
        if(!loadAvatar(avatarFile, "bakaAvatar"))
        {
            avatarFile = "baka.jpg";
            loadAvatar(avatarFile, "bakaAvatar");
        }
        mAvatarName = avatarFile;
    }
    else //load a random avatar.
    {
        avatarFile = "avatar";
        char buffer[3];
        sprintf(buffer, "%i", int(observer->getRandomGenerator()->random()%100));
        avatarFile.append(buffer);
        avatarFile.append(".jpg");
        if(!loadAvatar(avatarFile, "bakaAvatar"))
        {
            avatarFile = "baka.jpg";
            loadAvatar(avatarFile, "bakaAvatar");
        }
        mAvatarName = avatarFile;
    }

    if (fileSmall == "ai_baka_eviltwin")
        mAvatar->SetHFlip(true);

}

AIPlayer::~AIPlayer()
{
    while (!clickstream.empty())
    {
        AIAction * action = clickstream.front();
        SAFE_DELETE(action);
        clickstream.pop();
    }

}


int AIPlayer::Act(float)
{
    if (observer->currentPlayer == this)
        observer->userRequestNextGamePhase();
    return 1;
}



int AIPlayer::clickMultiTarget(TargetChooser * tc, vector<Targetable*>& potentialTargets)
{
    vector<Targetable*>::iterator ite = potentialTargets.begin();
    while(ite != potentialTargets.end())
    {
        MTGCardInstance * card = dynamic_cast<MTGCardInstance *>(*ite);
        if(card && card == tc->source)//if the source is part of the targetting deal with it first. second click is "confirming click".
        {
            clickstream.push(NEW AIAction(this, card));
            DebugTrace("Ai clicked source as a target: " << (card ? card->name : "None" ) << endl );
            ite = potentialTargets.erase(ite);
            continue;
        }
        else if(Player * pTarget = dynamic_cast<Player *>(*ite))
        {
            clickstream.push(NEW AIAction(this, pTarget));
            DebugTrace("Ai clicked Player as a target");
            ite = potentialTargets.erase(ite);
            continue;
        }
        ite++;
    }

    randomGenerator.random_shuffle(potentialTargets.begin(), potentialTargets.end());
    if(potentialTargets.size())
        clickstream.push(NEW AIAction(this, NULL,tc->source,potentialTargets));
    while(clickstream.size())
    {
        AIAction * action = clickstream.front();
        action->Act();
        SAFE_DELETE(action);
        clickstream.pop();
    }
    return 1;
}

int AIPlayer::clickSingleTarget(TargetChooser *, vector<Targetable*>& potentialTargets, MTGCardInstance * chosenCard)
{
    int i = randomGenerator.random() % potentialTargets.size();

    if(MTGCardInstance * card = dynamic_cast<MTGCardInstance *>(potentialTargets[i]))
    {
        if (!chosenCard)
            clickstream.push(NEW AIAction(this, card));
    }
    else if(Player * player = dynamic_cast<Player *>(potentialTargets[i]))
    {
        clickstream.push(NEW AIAction(this, player));
    }

    return 1;
}


AIPlayer * AIPlayerFactory::createAIPlayer(GameObserver *observer, MTGAllCards * collection, Player * opponent, int deckid)
{
    char deckFile[512];
    string avatarFilename; // default imagename
    char deckFileSmall[512];
    
    if (deckid == GameStateDuel::MENUITEM_EVIL_TWIN)
    { //Evil twin
        sprintf(deckFile, "%s", opponent->deckFile.c_str());
        DebugTrace("Evil Twin => " << opponent->deckFile);
        avatarFilename = "avatar.jpg";
        sprintf(deckFileSmall, "%s", "ai_baka_eviltwin");
    }
    else
    {
        if (!deckid)
        {
            //random deck
            int nbdecks = MIN(AIPlayer::getTotalAIDecks(), options[Options::AIDECKS_UNLOCKED].number);
            if (!nbdecks)
                return NULL;
            deckid = 1 + WRand() % (nbdecks);
        }
        sprintf(deckFile, "ai/baka/deck%i.txt", deckid);
        DeckMetaData *aiMeta = observer->getDeckManager()->getDeckMetaDataByFilename( deckFile, true);
        avatarFilename = aiMeta->getAvatarFilename();
        sprintf(deckFileSmall, "ai_baka_deck%i", deckid);
    }

    int deckSetting = EASY;
    if ( opponent ) 
    {
        bool isOpponentAI = opponent->isAI() == 1;
        DeckMetaData *meta = observer->getDeckManager()->getDeckMetaDataByFilename( opponent->deckFile, isOpponentAI);
        if ( meta && meta->getVictoryPercentage() >= 65)
            deckSetting = HARD;
    }
    
    // AIPlayerBaka will delete MTGDeck when it's time
    AIPlayerBaka * baka = NEW AIPlayerBaka(observer, deckFile, deckFileSmall, avatarFilename, NEW MTGDeck(deckFile, collection,0, deckSetting));
    baka->deckId = deckid;
    baka->comboHint = NULL;
    if (baka->opponent() && baka->opponent()->isHuman())
        baka->setFastTimerMode(false);
    return baka;
}

int AIPlayer::receiveEvent(WEvent *)
{
    return 0;
}

void AIPlayer::Render()
{

}

bool AIPlayer::parseLine(const string& s)
{
    size_t limiter = s.find("=");
    if (limiter == string::npos) limiter = s.find(":");
    string areaS;
    if (limiter != string::npos)
    {
        areaS = s.substr(0, limiter);
        if (areaS.compare("rvalues") == 0)
        {
            randomGenerator.loadRandValues(s.substr(limiter + 1));
            return true;
        }
    }

    return Player::parseLine(s);
}


#ifdef AI_CHANGE_TESTING
AIPlayer * AIPlayerFactory::createAIPlayerTest(GameObserver *observer, MTGAllCards * collection, Player * opponent, string _folder)
{
    char deckFile[512];
    string avatarFilename; // default imagename
    char deckFileSmall[512];
  

    string folder = _folder.size() ? _folder : "ai/baka/";

    int deckid = 0;

    //random deck
    int nbdecks = 0;
    int found = 1;
    while (found && nbdecks < options[Options::AIDECKS_UNLOCKED].number)
    {
        found = 0;
        char buffer[512];
        sprintf(buffer, "%sdeck%i.txt", folder.c_str(), nbdecks + 1);
        if (FileExists(buffer))
        {
            found = 1;
            nbdecks++;
        }
    }
    if (!nbdecks)
    {
        if (_folder.size())
            return createAIPlayerTest(observer, collection, opponent, "");
        return NULL;
    }
    deckid = 1 + WRand() % (nbdecks);

    sprintf(deckFile, "%sdeck%i.txt", folder.c_str(), deckid);
    DeckMetaData *aiMeta = observer->getDeckManager()->getDeckMetaDataByFilename( deckFile, true);
    avatarFilename = aiMeta->getAvatarFilename();
    sprintf(deckFileSmall, "ai_baka_deck%i", deckid);


    int deckSetting = EASY;
    if ( opponent ) 
    {
        bool isOpponentAI = opponent->isAI() == 1;
        DeckMetaData *meta = observer->getDeckManager()->getDeckMetaDataByFilename( opponent->deckFile, isOpponentAI);
        if ( meta->getVictoryPercentage() >= 65)
            deckSetting = HARD;
    }
    
    // AIPlayerBaka will delete MTGDeck when it's time
    AIPlayerBaka * baka = opponent ? 
        NEW AIPlayerBakaB(observer, deckFile, deckFileSmall, avatarFilename, NEW MTGDeck(deckFile, collection,0, deckSetting)) :
        NEW AIPlayerBaka(observer, deckFile, deckFileSmall, avatarFilename, NEW MTGDeck(deckFile, collection,0, deckSetting));
    baka->deckId = deckid;
    baka->setObserver(observer);
    return baka;
}
#endif


int AIPlayer::getTotalAIDecks() 
{
    if (totalAIDecks == -1)
        totalAIDecks = countTotalDecks(0,0,1);
    return totalAIDecks;
}

int AIPlayer::countTotalDecks(int lower, int higher, int current) {

    char buffer[512];
    sprintf(buffer, "ai/baka/deck%i.txt", current);
    if (JFileSystem::GetInstance()->FileExists(buffer))
    {
        if (higher == current + 1)
            return current;
        int newlower = current;
        int newcurrent = higher ? (higher + newlower)/2 : current * 2;
        return countTotalDecks(newlower, higher, newcurrent);
    } else {
        if (!lower)
            return 0;
        if (lower == current - 1)
            return lower;

        int newhigher = current;
        int newcurrent = (lower + newhigher) / 2;
        return countTotalDecks(lower, newhigher, newcurrent);
    }
}

void AIPlayer::invalidateTotalAIDecks() 
{
    totalAIDecks = -1;
}

bool AIPlayer::canFirstStrikeKill(MTGCardInstance * card, MTGCardInstance *ennemy)
{
    if (ennemy->has(Constants::FIRSTSTRIKE) || ennemy->has(Constants::DOUBLESTRIKE))
        return false;
    if (!(card->has(Constants::FIRSTSTRIKE) || card->has(Constants::DOUBLESTRIKE)))
        return false;
    if (!(card->power >= ennemy->toughness))
        return false;
    if (!(card->power >= ennemy->toughness + 1) && ennemy->has(Constants::FLANKING))
        return false;
    return true;
}

bool AIPlayer::canPlay(MTGCardInstance * card)
{
    if (card->hasType(Subtypes::TYPE_LAND))
    {
        if (game->playRestrictions->canPutIntoZone(card, game->inPlay) == PlayRestriction::CANT_PLAY)
            return false;
    }
    else
    {
        if (game->playRestrictions->canPutIntoZone(card, game->stack) == PlayRestriction::CANT_PLAY)
            return false;
    }
    if (!manaPool->canAfford(card->getManaCost()))
        return false;

    return true;
}

int AIPlayer::getCreaturesInfo(Player * player, int neededInfo, int untapMode, int canAttack)
{
    int result = 0;
    CardDescriptor cd;
    cd.init();
    cd.setType("Creature");
    cd.unsecureSetTapped(untapMode);
    MTGCardInstance * card = NULL;
    while ((card = cd.nextmatch(player->game->inPlay, card)))
    {
        if (!canAttack || card->canAttack())
        {
            if (neededInfo == INFO_NBCREATURES)
            {
                result++;
            }
            else
            {
                result += card->power;
            }
        }
    }
    return result;
}

int AIPlayer::createAbilityPotentialsActions(MTGAbility * a, MTGCardInstance * c, vector<AIAction>& actions)
{
    if (!a->getActionTc())
    {
        AIAction aiAction(this, a, c, NULL);
        actions.push_back(aiAction);
        return 1;
    }

    vector<Targetable*>potentialTargets;
    for (int i = 0; i < 2; i++)
    {
        Player * p = observer->players[i];
        MTGGameZone * playerZones[] = { p->game->graveyard, p->game->library, p->game->hand, p->game->inPlay,p->game->stack };
        // try player first
        if(a->getActionTc()->canTarget((Targetable*)p))
        {
            if(a->getActionTc()->maxtargets == 1)
            {
                AIAction aiAction(this, a, p, c);
                actions.push_back(aiAction);
            }
            else
                potentialTargets.push_back(p);
        }
        for (int j = 0; j < 5; j++)
        {
            MTGGameZone * zone = playerZones[j];
            for (int k = 0; k < zone->nb_cards; k++)
            {
                MTGCardInstance * t = zone->cards[k];
                if (a->getActionTc()->canTarget(t))
                {
                    if(a->getActionTc()->maxtargets == 1)
                    {
                        AIAction aiAction(this, a, c, t);
                        actions.push_back(aiAction);
                    }
                    else
                    {
                        potentialTargets.push_back(t);
                    }
                }
            }
        }
    }
    vector<Targetable*>realTargets;
    if(a->getActionTc()->maxtargets != 1)
    {
        if(a->getActionTc()->getNbTargets() && a->getActionTc()->attemptsToFill > 4)
        {
            a->getActionTc()->done = true;
            return 0;
        }
        while(potentialTargets.size())
        {
            AIAction * check = NULL;

            Player * pTargeting = 0;
            MTGCardInstance * cTargeting = dynamic_cast<MTGCardInstance*>(potentialTargets[0]);
            if(cTargeting) 
            {
              check = NEW AIAction(this, a,c,cTargeting);
            } 
            else 
            {
              pTargeting = dynamic_cast<Player*>(potentialTargets[0]);
              if(pTargeting)
                check = NEW AIAction(this, a,pTargeting,c);
            }

            if(check && pTargeting)
            {
                AIAction aiAction(this, a,pTargeting,c);
                actions.push_back(aiAction);
            }
            if(check)
                realTargets.push_back(potentialTargets[0]);
            potentialTargets.erase(potentialTargets.begin());
            SAFE_DELETE(check);
        }
        if(!realTargets.size() || (int(realTargets.size()) < a->getActionTc()->maxtargets && a->getActionTc()->targetMin))
            return 0;
        AIAction aiAction(this, a, c,realTargets);
        aiAction.target = dynamic_cast<MTGCardInstance*>(realTargets[0]);
        aiAction.playerAbilityTarget = dynamic_cast<Player*>(realTargets[0]);
        actions.push_back(aiAction);
    }
    return 1;
}

}