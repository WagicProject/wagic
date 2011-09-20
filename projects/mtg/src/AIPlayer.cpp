#include "PrecompiledHeader.h"

#include "AIPlayer.h"
#include "GameStateDuel.h"
#include "DeckManager.h"
#include "CardSelectorSingleton.h"


// Instances for Factory
#include "AIPlayerBaka.h"

#ifdef AI_CHANGE_TESTING
#include "AIPlayerBakaB.h"
#endif



const char * const MTG_LAND_TEXTS[] = { "artifact", "forest", "island", "mountain", "swamp", "plains", "other lands" };

int AIAction::currentId = 0;

AIAction::AIAction(MTGCardInstance * c, MTGCardInstance * t)
    : ability(NULL), player(NULL), click(c), target(t)
{
    id = currentId++;

    // useability tweak - assume that the user is probably going to want to see the full res card,
    // so prefetch it. The idea is that we do it here as we want to start the prefetch before it's time to render,
    // and waiting for it to actually go into play is too late, as we start drawing the card during the interrupt window.
    // This is a good intercept point, as the AI has committed to using this card.

    // if we're not in text mode, always get the thumb
    if (CardSelectorSingleton::Instance()->GetDrawMode() != DrawMode::kText)
    {
        //DebugTrace("Prefetching AI card going into play: " << c->getImageName());
        WResourceManager::Instance()->RetrieveCard(c, RETRIEVE_THUMB);
        
        // also cache the large image if we're using kNormal mode
        if (CardSelectorSingleton::Instance()->GetDrawMode() == DrawMode::kNormal)
        {
            WResourceManager::Instance()->RetrieveCard(c);
        }
    }
}

int AIAction::Act()
{
    GameObserver * g = GameObserver::GetInstance();
    if (player && !playerAbilityTarget)
    {
        g->cardClick(NULL, player);
        return 1;
    }
    if (ability)
    {
        g->mLayers->actionLayer()->reactToClick(ability, click);
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
    GameObserver * g = GameObserver::GetInstance();
    TargetChooser * tc = g->getCurrentTargetChooser();
    if(!tc) return 0;
    bool sourceIncluded = false;
    for(size_t f = 0;f < actionTargets.size();f++)
    {
        MTGCardInstance * card = ((MTGCardInstance *) actionTargets[f]);
        if(card == (MTGCardInstance*)tc->source)//click source first.
        {
            g->cardClick(card);
            actionTargets.erase(actionTargets.begin() + f);
            sourceIncluded = true;
        }
    }
    std::random_shuffle(actionTargets.begin(), actionTargets.end());
    //shuffle to make it less predictable, otherwise ai will always seem to target from right to left. making it very obvious.
    for(int k = 0;k < int(actionTargets.size());k++)
    {
        int type = actionTargets[k]->typeAsTarget();
        switch (type)
        {
        case TARGET_CARD:
            {
                if(k < tc->maxtargets)
                {
                    MTGCardInstance * card = ((MTGCardInstance *) actionTargets[k]);
                    if(k+1 == int(actionTargets.size()))
                        tc->done = true;
                    g->cardClick(card);
                }
            }
            break;
        }
    }
    tc->attemptsToFill++;
    return 1;
}

AIPlayer::AIPlayer(string file, string fileSmall, MTGDeck * deck) :
    Player(file, fileSmall, deck)
{
    agressivity = 50;
    forceBestAbilityUse = false;
    playMode = Player::MODE_AI;
    mFastTimerMode = false;

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


int AIPlayer::Act(float dt)
{
    GameObserver * gameObs = GameObserver::GetInstance();
    if (gameObs->currentPlayer == this)
        gameObs->userRequestNextGamePhase();
    return 1;
}




int AIPlayer::clickMultiTarget(TargetChooser * tc, vector<Targetable*>& potentialTargets)
{
    bool sourceIncluded = false;
    for(int f = 0;f < int(potentialTargets.size());f++)
    {
        MTGCardInstance * card = ((MTGCardInstance *) potentialTargets[f]);
        Player * pTarget = (Player*)potentialTargets[f];
        if(card && card == (MTGCardInstance*)tc->source)//if the source is part of the targetting deal with it first. second click is "confirming click".
        {
            clickstream.push(NEW AIAction(card));
            DebugTrace("Ai clicked source as a target: " << (card ? card->name : "None" ) << endl );
            potentialTargets.erase(potentialTargets.begin() + f);
            sourceIncluded = true;
        }
        if(pTarget && pTarget->typeAsTarget() == TARGET_PLAYER)
        {
            clickstream.push(NEW AIAction(pTarget));
            DebugTrace("Ai clicked Player as a target");
            potentialTargets.erase(potentialTargets.begin() + f);
        }
    }
    std::random_shuffle(potentialTargets.begin(), potentialTargets.end());
    if(potentialTargets.size())
        clickstream.push(NEW AIAction(NULL,tc->source,potentialTargets));
    while(clickstream.size())
    {
        AIAction * action = clickstream.front();
        action->Act();
        SAFE_DELETE(action);
        clickstream.pop();
    }
    return 1;
}

int AIPlayer::clickSingleTarget(TargetChooser * tc, vector<Targetable*>& potentialTargets, MTGCardInstance * chosenCard)
{
        int i = WRand() % potentialTargets.size();
        int type = potentialTargets[i]->typeAsTarget();
        switch (type)
        {
        case TARGET_CARD:
            {
                if(!chosenCard)
                {
                    MTGCardInstance * card = ((MTGCardInstance *) potentialTargets[i]);
                    clickstream.push(NEW AIAction(card));
                    chosenCard = card;
                }
                break;
            }
        case TARGET_PLAYER:
            {
                Player * player = ((Player *) potentialTargets[i]);
                clickstream.push(NEW AIAction(player));
                break;
            }
        }
    return 1;
}


AIPlayer * AIPlayerFactory::createAIPlayer(MTGAllCards * collection, Player * opponent, int deckid)
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
            int nbdecks = 0;
            int found = 1;
            while (found && nbdecks < options[Options::AIDECKS_UNLOCKED].number)
            {
                found = 0;
                char buffer[512];
                sprintf(buffer, "ai/baka/deck%i.txt", nbdecks + 1);
                if (FileExists(buffer))
                {
                    found = 1;
                    nbdecks++;
                }
            }
            if (!nbdecks)
                return NULL;
            deckid = 1 + WRand() % (nbdecks);
        }
        sprintf(deckFile, "ai/baka/deck%i.txt", deckid);
        DeckMetaData *aiMeta = DeckManager::GetInstance()->getDeckMetaDataByFilename( deckFile, true);
        avatarFilename = aiMeta->getAvatarFilename();
        sprintf(deckFileSmall, "ai_baka_deck%i", deckid);
    }

    int deckSetting = EASY;
    if ( opponent ) 
    {
        bool isOpponentAI = opponent->isAI() == 1;
        DeckMetaData *meta = DeckManager::GetInstance()->getDeckMetaDataByFilename( opponent->deckFile, isOpponentAI);
        if ( meta->getVictoryPercentage() >= 65)
            deckSetting = HARD;
    }
    
    // AIPlayerBaka will delete MTGDeck when it's time
    AIPlayerBaka * baka = NEW AIPlayerBaka(deckFile, deckFileSmall, avatarFilename, NEW MTGDeck(deckFile, collection,0, deckSetting));
    baka->deckId = deckid;
    return baka;
}

int AIPlayer::receiveEvent(WEvent * event)
{
    return 0;
}

void AIPlayer::Render()
{
#ifdef RENDER_AI_STATS
    if (getStats()) getStats()->Render();
#endif
}

#ifdef AI_CHANGE_TESTING
AIPlayer * AIPlayerFactory::createAIPlayerTest(MTGAllCards * collection, Player * opponent, int deckid)
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
            int nbdecks = 0;
            int found = 1;
            while (found && nbdecks < options[Options::AIDECKS_UNLOCKED].number)
            {
                found = 0;
                char buffer[512];
                sprintf(buffer, "ai/baka/deck%i.txt", nbdecks + 1);
                if (FileExists(buffer))
                {
                    found = 1;
                    nbdecks++;
                }
            }
            if (!nbdecks)
                return NULL;
            deckid = 1 + WRand() % (nbdecks);
        }
        sprintf(deckFile, "ai/baka/deck%i.txt", deckid);
        DeckMetaData *aiMeta = DeckManager::GetInstance()->getDeckMetaDataByFilename( deckFile, true);
        avatarFilename = aiMeta->getAvatarFilename();
        sprintf(deckFileSmall, "ai_baka_deck%i", deckid);
    }

    int deckSetting = EASY;
    if ( opponent ) 
    {
        bool isOpponentAI = opponent->isAI() == 1;
        DeckMetaData *meta = DeckManager::GetInstance()->getDeckMetaDataByFilename( opponent->deckFile, isOpponentAI);
        if ( meta->getVictoryPercentage() >= 65)
            deckSetting = HARD;
    }
    
    // AIPlayerBaka will delete MTGDeck when it's time
    AIPlayerBakaB * baka = NEW AIPlayerBakaB(deckFile, deckFileSmall, avatarFilename, NEW MTGDeck(deckFile, collection,0, deckSetting));
    baka->deckId = deckid;
    return baka;
}
#endif
