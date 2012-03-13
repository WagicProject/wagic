#include "PrecompiledHeader.h"

#include "PhaseRing.h"
#include "MTGDefinitions.h"
#include "Player.h"
#include "WEvent.h"
//Parses a string and gives phase numer
GamePhase PhaseRing::phaseStrToInt(string s)
{
    if (s.compare("untap") == 0) return MTG_PHASE_UNTAP;
    if (s.compare("upkeep") == 0) return MTG_PHASE_UPKEEP;
    if (s.compare("draw") == 0) return MTG_PHASE_DRAW;
    if (s.compare("firstmain") == 0) return MTG_PHASE_FIRSTMAIN;
    if (s.compare("mainphase") == 0) return MTG_PHASE_FIRSTMAIN;
    if (s.compare("combatbegin") == 0) return MTG_PHASE_COMBATBEGIN;
    if (s.compare("combatbegins") == 0) return MTG_PHASE_COMBATBEGIN;
    if (s.compare("combatattackers") == 0) return MTG_PHASE_COMBATATTACKERS;
    if (s.compare("combatblockers") == 0) return MTG_PHASE_COMBATBLOCKERS;
    if (s.compare("combatdamage") == 0) return MTG_PHASE_COMBATDAMAGE;
    if (s.compare("combatend") == 0) return MTG_PHASE_COMBATEND;
    if (s.compare("combatends") == 0) return MTG_PHASE_COMBATEND;
    if (s.compare("secondmain") == 0) return MTG_PHASE_SECONDMAIN;
    if (s.compare("endofturn") == 0) return MTG_PHASE_ENDOFTURN;
    if (s.compare("end") == 0) return MTG_PHASE_ENDOFTURN;
    if (s.compare("cleanup") == 0) return MTG_PHASE_CLEANUP;
    DebugTrace("PHASERING: Unknown Phase name: " << s);
    return MTG_PHASE_INVALID;//was returning first main...why would we return something that is not == s?
}

string PhaseRing::phaseIntToStr(int id)
{
    if (id == MTG_PHASE_UNTAP) return "untap";
    if (id == MTG_PHASE_UPKEEP) return "upkeep";
    if (id == MTG_PHASE_DRAW) return "draw";
    if (id == MTG_PHASE_FIRSTMAIN) return "firstmain";
    if (id == MTG_PHASE_COMBATBEGIN) return "combatbegins";
    if (id == MTG_PHASE_COMBATATTACKERS) return "combatattackers";
    if (id == MTG_PHASE_COMBATBLOCKERS) return "combatblockers";
    if (id == MTG_PHASE_COMBATDAMAGE) return "combatdamage";
    if (id == MTG_PHASE_COMBATEND) return "combatends";
    if (id == MTG_PHASE_SECONDMAIN) return "secondmain";
    if (id == MTG_PHASE_ENDOFTURN) return "endofturn";
    if (id == MTG_PHASE_CLEANUP) return "cleanup";
    DebugTrace("PHASERING: Unknown Phase id:%i " << id);
    return "";
}

/* Creates a New phase ring with the default rules */
PhaseRing::PhaseRing(GameObserver* observer)
    :observer(observer)
{
    for (int i = 0; i < observer->getPlayersNumber(); i++)
    {
        list<Phase*>turnRing;//this is temporary;
        if(observer->players[i]->phaseRing.size())
        {
            addPhase(NEW Phase(MTG_PHASE_BEFORE_BEGIN, observer->players[i]));
            vector<string>customRing = split(observer->players[i]->phaseRing,',');
            for (unsigned int k = 0;k < customRing.size(); k++)
            {
                GamePhase customOrder = phaseStrToInt(customRing[k]);
                Phase * phase = NEW Phase(customOrder, observer->players[i]);
                addPhase(phase);
                turnRing.push_back(phase);
            }
            addPhase( NEW Phase(MTG_PHASE_AFTER_EOT, observer->players[i]));  
        }
        else
        {
            for (int j = 0; j < NB_MTG_PHASES; j++)
            {
                Phase * phase = NEW Phase((GamePhase)j, observer->players[i]);
                addPhase(phase);
                turnRing.push_back(phase);
            }
        }
        observer->gameTurn.push_back(turnRing);
        //push back the complete turn of a player for easy refference.
        //since they are pushed back per player you can denote that gameTurn[playerid] is the exact turn for that player.
    }
    turn = currentTurn();
    current = turn.begin();
}

list<Phase*> PhaseRing::currentTurn()
{
    list<Phase*> temp = observer->gameTurn[observer->currentPlayer->getId()];
    list<Phase *>::iterator tempiter;
    currentTurnList.clear();
    for (tempiter = temp.begin(); tempiter != temp.end(); tempiter++)
    {
        Phase * currentIter = *tempiter;
        currentTurnList.push_back(currentIter);
    }
    return currentTurnList;
}

list<Phase*> PhaseRing::nextTurn()
{
    list<Phase*> temp = observer->gameTurn[observer->nextTurnsPlayer()->getId()];
    list<Phase*>::iterator tempiter;
    Phase * currentIter = NULL;
    nextTurnList.clear();
    for (tempiter = temp.begin(); tempiter != temp.end(); tempiter++)
    {
        currentIter = *tempiter;
        nextTurnList.push_back(currentIter);
    }
    return nextTurnList;
}

PhaseRing::~PhaseRing()
{
    list<Phase *>::iterator it;
    for (it = ring.begin(); it != ring.end(); it++)
    {
        Phase * currentPhase = *it;
        delete (currentPhase);
    }
    for (it = extraPhases.begin();it != extraPhases.end();it++)
    {
        Phase * currentPhase = *it;
        SAFE_DELETE(currentPhase);
    }
}

//Tells if next phase will be another Damage phase rather than combat ends
bool PhaseRing::extraDamagePhase(int id)
{
    if (id != MTG_PHASE_COMBATEND) return false;
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
    if (current == turn.end())
    {
        turn = nextTurn();
        current = turn.begin();
    }
    return *current;
}

Phase * PhaseRing::forward(bool sendEvents)
{
    Phase * cPhaseOld = *current;
    //the following is a present event before change
    //if we need to remove a phase before it changes the game
    //needs to be aware of what phase we're coming from and going to
    //before we actually increment the phase iter.   {
    bool notEnd = false;
    size_t turnSteps = turn.size();
    if (current != turn.end())
    {
        current++;
        if(current == turn.end())
            current--;
        else
            notEnd = true;
    }
    //Warn the layers about the current phase before changing
    WEvent * e = NEW WEventPhasePreChange(cPhaseOld,*current);
    observer->receiveEvent(e);
    if(notEnd)
    {
        current--;
    }
    size_t turnStepsNow = turn.size();
    if(turnSteps != turnStepsNow)
        return forward(sendEvents);
    if (current != turn.end()) 
        current++;
    if (current == turn.end())
    {
        turn = nextTurn();
        current = turn.begin();
    }
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
    Phase * currentPhase = getCurrentPhase();
    while (currentPhase->id != id)
    { //Dangerous, risk for inifinte loop !
        DebugTrace("PhasingRing: goToPhase called, current phase is " << phaseName(currentPhase->id));
        currentPhase = forward(sendEvents);
        if(id == MTG_PHASE_INVALID)
        {
            DebugTrace("stopping on  this phase becuase goToPhase was told to go a phase that doesn't exist:" << phaseName(currentPhase->id));
            break;
        }
        if(currentPhase->player == player && currentPhase->id == id)
            break;
    }
    return currentPhase;
}

int PhaseRing::addPhase(Phase * phase)
{
    ring.push_back(phase);
    return 1;
}

int PhaseRing::addCombatAfter(Player* player, int after_id, bool withMain)
{
    list<Phase *>::iterator it;
    Phase * beforeLeaving;
    for (it = turn.begin(); it != turn.end(); it++)
    {
        Phase * currentPhase = *it;
        if (currentPhase->id == after_id)
        {
            beforeLeaving = currentPhase;
            it++;
            Phase * addPhase = NULL;
            list<Phase *>::iterator findP;
            for(findP = ring.begin();findP != ring.end();findP++)
            {
                addPhase = *findP;
                Phase * toAdd = NULL;
                bool add = false;
                if(addPhase->player == player)
                {
                    switch(addPhase->id)
                    {
                    case MTG_PHASE_COMBATEND:
                        {
                            toAdd = NEW Phase(*addPhase);
                            toAdd->isExtra = true;
                            turn.insert(it, toAdd);
                            extraPhases.push_back(toAdd);
                            observer->combatStep = BLOCKERS;
                            if(withMain)
                            {
                                toAdd = NEW Phase(*getPhase(MTG_PHASE_SECONDMAIN));
                                toAdd->isExtra = true;
                                extraPhases.push_back(toAdd);
                                turn.insert(it, toAdd);
                            }
                            Phase * check = getCurrentPhase();
                            bool checking = false;
                            if(check != beforeLeaving)
                                checking = true;
                            while(checking)
                            {
                                current--;//put us back to where we were.
                                check = getCurrentPhase();
                                if(check == beforeLeaving)
                                {
                                    checking = false;
                                    current++; //now move to the next phase, which will be the added phases.
                                }
                            }
                            return 1;
                        }
                    case MTG_PHASE_COMBATDAMAGE:
                        toAdd = NEW Phase(*addPhase);
                        add = true;
                        break;
                    case MTG_PHASE_COMBATBLOCKERS:
                        toAdd = NEW Phase(*addPhase);
                        add = true;
                        break;
                    case MTG_PHASE_COMBATATTACKERS:
                        toAdd = NEW Phase(*addPhase);
                        add = true;
                        break;
                    case MTG_PHASE_COMBATBEGIN:
                        toAdd = NEW Phase(*addPhase);
                        add = true;
                        break;
                    default:
                        break;
                    }
                    if(add)
                    {
                        toAdd->isExtra = true;
                        turn.insert(it, toAdd);
                        extraPhases.push_back(toAdd);
                    }
                }
            }
        }
    }
    return 0;
}

Phase * PhaseRing::getPhase(int _id)
{
    list<Phase *>::iterator it;
    for (it = turn.begin(); it != turn.end(); it++)
    {
        Phase * currentPhase = *it;
        if (currentPhase->id == _id)
        {
           return currentPhase;
        }
    }
    return NULL;
}

int PhaseRing::addPhaseAfter(GamePhase id, Player* player, int after_id)
{
    list<Phase *>::iterator it;
    for (it = turn.begin(); it != turn.end(); it++)
    {
        Phase * currentPhase = *it;
        if (currentPhase->id == after_id)
        {
            it++;
            Phase * addPhase = NULL;
            list<Phase *>::iterator findP;
            for(findP = ring.begin();findP != ring.end();findP++)
            {
                addPhase = *findP;
                if(addPhase->id == id && addPhase->player == player)
                {
                    Phase * toAdd = NEW Phase(*addPhase);
                    toAdd->isExtra = true;
                    turn.insert(it, toAdd);
                    extraPhases.push_back(toAdd);
                    return 1;
                }
            }
        }
    }
    return 0;
}

int PhaseRing::removePhase(int id)
{
    list<Phase *>::iterator it = turn.begin();
    while (it != turn.end())
    {
        Phase * currentPhase = *it;
        if (currentPhase->id == id)
        {
            if (current == it) 
                current++; //Avoid our cursor to get invalidated
            turn.erase(it);
            return 1;
        }
        else
        {
            it++;
        }
    }
    return 0;
}
