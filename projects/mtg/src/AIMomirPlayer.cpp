#include "PrecompiledHeader.h"

#include "AIMomirPlayer.h"
#include "CardDescriptor.h"
#include "DamagerDamaged.h"
#include "AIStats.h"
#include "AllAbilities.h"

AIMomirPlayer::AIMomirPlayer(GameObserver *observer, string file, string fileSmall, string avatarFile, MTGDeck * deck) :
    AIPlayerBaka(observer, file, fileSmall, avatarFile, deck)
{
    momirAbility = NULL;
    agressivity = 100;
}

int AIMomirPlayer::getEfficiency(OrderedAIAction * action)
{
    MTGAbility * ability = action->ability;
    ManaCost * cost = ability->getCost();
    if (cost && !(cost->isExtraPaymentSet())) return 0; //Does not handle abilities with sacrifice yet
    int efficiency = AIPlayerBaka::getEfficiency(action);

    if (observer->getCurrentGamePhase() < MTG_PHASE_FIRSTMAIN) return 0;
    return efficiency;
}

MTGAbility * AIMomirPlayer::getMomirAbility()
{
    if (momirAbility) return momirAbility;

    momirAbility = observer->mLayers->actionLayer()->getAbility(MTGAbility::MOMIR);
    return momirAbility;
}

int AIMomirPlayer::momir()
{
    if (!game->hand->nb_cards) return 0; //nothing to discard :/
    int result = 0;
    int opponentCreatures = getCreaturesInfo(opponent(), INFO_NBCREATURES);
    int myCreatures = getCreaturesInfo(this, INFO_NBCREATURES );
    ManaCost * potentialMana = getPotentialMana();
    int converted = potentialMana->getConvertedCost();
    SAFE_DELETE(potentialMana);
    int efficiency = 100;
    int chance = 1 + (randomGenerator.random() % 100);
    if (converted == 5 && myCreatures > opponentCreatures && game->hand->nb_cards < 4) efficiency = 5; //Strategy: skip 5 drop
    if (converted == 7 && myCreatures > opponentCreatures && game->hand->nb_cards < 2) efficiency = 50; //Strategy: 7 drops have bad upkeep costs and the AI doesn't handle those right now...
    if (converted > 8) converted = 8;
    if (converted == 8) efficiency = 100 - (myCreatures - opponentCreatures);

    if (efficiency >= chance)
    {
        
        std::vector<int8_t> _cost;
        _cost.push_back(Constants::MTG_COLOR_ARTIFACT);
        _cost.push_back(converted);
        ManaCost * cost = NEW ManaCost(_cost);
        MTGAbility * ability = getMomirAbility();
        MTGCardInstance * card = game->hand->cards[0];
        if (ability->isReactingToClick(card, cost))
        {
            payTheManaCost(cost);
            AIAction * a = NEW AIAction(this, ability, card);
            clickstream.push(a);
            result = 1;
        }
        delete cost;
    }
    return result;
}

int AIMomirPlayer::computeActions()
{
    //Part of the strategy goes here. When should we put a land into play ?
    /*
     Another gift from Alex Majlaton on my first day playing Momir, and it has served me well ever since. It goes a little something like this: (a) if you are on the play, hit your Two through Four, skip your Five, and then hit all the way to Eight; (b) if you are on the draw and your opponent skips his One, you make Two through Eight; (c) if you are on the draw and your opponent hits a One, you match him drop-for-drop for the rest of the game.

     You skip your Five on the play because it is the weakest drop. There are plenty of serviceable guys there, but very few bombs compared to other drops
     the general rule is this: if you want to get to Eight, you have to skip two drops on the play and one drop on the draw.
     */

    Player * p = observer->currentPlayer;
    if (!(observer->currentlyActing() == this)) return 0;
    if (chooseTarget()) return 1;
    int currentGamePhase = observer->getCurrentGamePhase();
    if (observer->isInterrupting == this)
    { // interrupting
        selectAbility();
        return 1;
    }
    else if (p == this && observer->mLayers->stackLayer()->count(0, NOT_RESOLVED) == 0)
    { //standard actions
        CardDescriptor cd;
        MTGCardInstance * card = NULL;

        switch (currentGamePhase)
        {
        case MTG_PHASE_FIRSTMAIN:
        {
            ManaCost * potentialMana = getPotentialMana();
            int converted = potentialMana->getConvertedCost();
            SAFE_DELETE(potentialMana);
            
            if (converted < 8 || game->hand->nb_cards > 1)
            {
                //Attempt to put land into play
                cd.init();
                cd.setColor(Constants::MTG_COLOR_LAND);
                card = cd.match(game->hand);
                int canPutLandsIntoPlay = game->playRestrictions->canPutIntoZone(card, game->inPlay);
                if (card && (canPutLandsIntoPlay == PlayRestriction::CAN_PLAY))
                {
                    MTGAbility * putIntoPlay = observer->mLayers->actionLayer()->getAbility(MTGAbility::PUT_INTO_PLAY);
                    AIAction * a = NEW AIAction(this, putIntoPlay, card); //TODO putinplay action
                    clickstream.push(a);
                    return 1;
                }
            }
            momir();
            return 1;
            break;
        }
        case MTG_PHASE_SECONDMAIN:
            selectAbility();
            return 1;
            break;
        default:
            return AIPlayerBaka::computeActions();
            break;
        }
    }
    return AIPlayerBaka::computeActions();
}

