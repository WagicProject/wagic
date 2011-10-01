#include "PrecompiledHeader.h"

#include "PhaseRing.h"
#include "MTGDefinitions.h"
#include "Player.h"
#include "WEvent.h"
//Parses a string and gives phase numer
int PhaseRing::phaseStrToInt(string s)
{
    if (s.compare("untap") == 0) return Constants::MTG_PHASE_UNTAP;
    if (s.compare("upkeep") == 0) return Constants::MTG_PHASE_UPKEEP;
    if (s.compare("draw") == 0) return Constants::MTG_PHASE_DRAW;
    if (s.compare("firstmain") == 0) return Constants::MTG_PHASE_FIRSTMAIN;
    if (s.compare("combatbegin") == 0) return Constants::MTG_PHASE_COMBATBEGIN;
    if (s.compare("combatbegins") == 0) return Constants::MTG_PHASE_COMBATBEGIN;
    if (s.compare("combatattackers") == 0) return Constants::MTG_PHASE_COMBATATTACKERS;
    if (s.compare("combatblockers") == 0) return Constants::MTG_PHASE_COMBATBLOCKERS;
    if (s.compare("combatdamage") == 0) return Constants::MTG_PHASE_COMBATDAMAGE;
    if (s.compare("combatend") == 0) return Constants::MTG_PHASE_COMBATEND;
    if (s.compare("combatends") == 0) return Constants::MTG_PHASE_COMBATEND;
    if (s.compare("secondmain") == 0) return Constants::MTG_PHASE_SECONDMAIN;
    if (s.compare("endofturn") == 0) return Constants::MTG_PHASE_ENDOFTURN;
    if (s.compare("end") == 0) return Constants::MTG_PHASE_ENDOFTURN;
    if (s.compare("cleanup") == 0) return Constants::MTG_PHASE_CLEANUP;
    DebugTrace("PHASERING: Unknown Phase name: " << s);

    return Constants::MTG_PHASE_FIRSTMAIN;
}

/* Creates a New phase ring with the default rules */
PhaseRing::PhaseRing(GameObserver* observer)
    :observer(observer)
{
    for (int i = 0; i < observer->getPlayersNumber(); i++)
    {
        if(observer->players[i]->phaseRing.size())
        {
            addPhase(NEW Phase(Constants::MTG_PHASE_BEFORE_BEGIN, observer->players[i]));
            vector<string>customRing = split(observer->players[i]->phaseRing,',');
            for (unsigned int k = 0;k < customRing.size(); k++)
            {
                int customOrder = phaseStrToInt(customRing[k]);
                Phase * phase = NEW Phase(customOrder, observer->players[i]);
                addPhase(phase);
            }
            addPhase( NEW Phase(Constants::MTG_PHASE_AFTER_EOT, observer->players[i]));
        }
        else
        {
            for (int j = 0; j < Constants::NB_MTG_PHASES; j++)
            {
                Phase * phase = NEW Phase(j, observer->players[i]);
                addPhase(phase);
            }
        }
    }
    current = ring.begin();
}

PhaseRing::~PhaseRing()
{
    list<Phase *>::iterator it;
    for (it = ring.begin(); it != ring.end(); it++)
    {
        Phase * currentPhase = *it;
        delete (currentPhase);
    }
}

//Tells if next phase will be another Damage phase rather than combat ends
bool PhaseRing::extraDamagePhase(int id)
{
    if (id != Constants::MTG_PHASE_COMBATEND) return false;
    if (observer->combatStep != END_FIRST_STRIKE) return false;
    for (int j = 0; j < 2; ++j)
    {
        MTGGameZone * z = observer->players[j]->game->inPlay;
        for (int i = 0; i < z->nb_cards; ++i)
        {
            MTGCardInstance * card = z->cards[i];
            if ((card->isAttacker() || card->isDefenser()) && !(card->has(Constants::FIRSTSTRIKE) || card->has(
                            Constants::DOUBLESTRIKE))) return true;
        }
    }
    return false;
}

const char * PhaseRing::phaseName(int id)
{
    if (extraDamagePhase(id)) return "Combat Damage (2)";
    return Constants::MTGPhaseNames[id];
}

Phase * PhaseRing::getCurrentPhase()
{
    if (current == ring.end())
    {
        current = ring.begin();
    }
    return *current;
}

Phase * PhaseRing::forward(bool sendEvents)
{
    Phase * cPhaseOld = *current;
    if (current != ring.end()) current++;
    if (current == ring.end()) current = ring.begin();

    if (sendEvents)
    {
        //Warn the layers about the phase Change
        WEvent * e = NEW WEventPhaseChange(cPhaseOld, *current);
        observer->receiveEvent(e);
    }

    return *current;
}

Phase * PhaseRing::goToPhase(int id, Player * player, bool sendEvents)
{
    Phase * currentPhase = *current;
    while (currentPhase->id != id || currentPhase->player != player)
    { //Dangerous, risk for inifinte loop !

        DebugTrace("PhasingRing: goToPhase called, current phase is " << phaseName(currentPhase->id));

        currentPhase = forward(sendEvents);
    }
    return currentPhase;
}

int PhaseRing::addPhase(Phase * phase)
{
    ring.push_back(phase);
    return 1;
}

int PhaseRing::addPhaseBefore(int id, Player* player, int after_id, Player * after_player, int allOccurences)
{
    int result = 0;
    list<Phase *>::iterator it;
    for (it = ring.begin(); it != ring.end(); it++)
    {
        Phase * currentPhase = *it;
        if (currentPhase->id == after_id && currentPhase->player == after_player)
        {
            result++;
            ring.insert(it, NEW Phase(id, player));
            if (!allOccurences) return 1;
        }
    }
    return result;
}
int PhaseRing::removePhase(int id, Player * player, int allOccurences)
{
    int result = 0;
    list<Phase *>::iterator it = ring.begin();
    while (it != ring.end())
    {
        Phase * currentPhase = *it;
        if (currentPhase->id == id && currentPhase->player == player)
        {
            if (current == it) current++; //Avoid our cursor to get invalidated
            it = ring.erase(it);
            delete (currentPhase);
            result++;
            if (!allOccurences) return 1;
        }
        else
        {
            it++;
        }
    }
    return result;
}
