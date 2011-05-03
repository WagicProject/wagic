#include "PrecompiledHeader.h"

#include "MTGAbility.h"
#include "ManaCost.h"
#include "MTGGameZones.h"
#include "AllAbilities.h"
#include "Damage.h"
#include "TargetChooser.h"
#include "CardGui.h"
#include "MTGDeck.h"
#include "Translate.h"
#include "ThisDescriptor.h"
#include "ExtraCost.h"


//Used for Lord/This parsing
const string kLordKeywords[] = { "lord(", "foreach(", "aslongas(", "teach(", "all(" };
const size_t kLordKeywordsCount = 5;

const string kThisKeywords[] = { "this(", "thisforeach(" };
const size_t kThisKeywordsCount = 2;


// Used for the maxCast/maxPlay ability parsing
const string kMaxCastKeywords[] = { "maxplay(", "maxcast("};
const int kMaxCastZones[] = { MTGGameZone::BATTLEFIELD, MTGGameZone::STACK};
const size_t kMaxCastKeywordsCount = 2;

//Used for alternate Costs parsing
const string kAlternateCostKeywords[] = { "kicker", "retrace", "alternative", "buyback", "flashback", "facedown"}; 
const int kAlternateCostIds[] = {
    ManaCost::MANA_PAID_WITH_KICKER, ManaCost::MANA_PAID_WITH_RETRACE, ManaCost::MANA_PAID_WITH_ALTERNATIVE,
    ManaCost::MANA_PAID_WITH_BUYBACK, ManaCost::MANA_PAID_WITH_FLASHBACK, ManaCost::MANA_PAID_WITH_MORPH
};

//Used for "dynamic ability" parsing
const string kDynamicSourceKeywords[] = {"source", "mytgt", "myself", "myfoe"};
const int kDynamicSourceIds[] = { AADynamic::DYNAMIC_SOURCE_AMOUNT, AADynamic::DYNAMIC_MYTGT_AMOUNT, AADynamic::DYNAMIC_MYSELF_AMOUNT, AADynamic::DYNAMIC_MYFOE_AMOUNT};

const string kDynamicTypeKeywords[] = {"power", "toughness", "manacost", "colors", "age", "charge", "oneonecounters", "thatmuch"};
const int kDynamicTypeIds[] = {
    AADynamic::DYNAMIC_ABILITY_TYPE_POWER, AADynamic::DYNAMIC_ABILITY_TYPE_TOUGHNESS, AADynamic::DYNAMIC_ABILITY_TYPE_MANACOST, 
    AADynamic::DYNAMIC_ABILITY_TYPE_COLORS,  AADynamic::DYNAMIC_ABILITY_TYPE_AGE, AADynamic::DYNAMIC_ABILITY_TYPE_CHARGE, AADynamic::DYNAMIC_ABILITY_TYPE_ONEONECOUNTERS,
    AADynamic::DYNAMIC_ABILITY_TYPE_THATMUCH
};

const string kDynamicEffectKeywords[] = {"strike", "draw", "lifeloss", "lifegain", "pumppow", "pumptough", "pumpboth", "deplete", "countersoneone" };
const int kDynamicEffectIds[] = { 
    AADynamic::DYNAMIC_ABILITY_EFFECT_STRIKE, AADynamic::DYNAMIC_ABILITY_EFFECT_DRAW, AADynamic::DYNAMIC_ABILITY_EFFECT_LIFELOSS, AADynamic::DYNAMIC_ABILITY_EFFECT_LIFEGAIN,
    AADynamic::DYNAMIC_ABILITY_EFFECT_PUMPPOWER, AADynamic::DYNAMIC_ABILITY_EFFECT_PUMPTOUGHNESS,  AADynamic::DYNAMIC_ABILITY_EFFECT_PUMPBOTH, AADynamic::DYNAMIC_ABILITY_EFFECT_DEPLETE,
    AADynamic::DYNAMIC_ABILITY_EFFECT_COUNTERSONEONE
};


const string kDynamicWhoKeywords[] = {"eachother", "itself", "targetcontroller", "targetopponent", "tosrc", "srccontroller", "srcopponent" };
const int kDynamicWhoIds[] = { 
     AADynamic::DYNAMIC_ABILITY_WHO_EACHOTHER, AADynamic::DYNAMIC_ABILITY_WHO_ITSELF, AADynamic::DYNAMIC_ABILITY_WHO_TARGETCONTROLLER, AADynamic::DYNAMIC_ABILITY_WHO_TARGETOPPONENT,
     AADynamic::DYNAMIC_ABILITY_WHO_TOSOURCE,  AADynamic::DYNAMIC_ABILITY_WHO_SOURCECONTROLLER,  AADynamic::DYNAMIC_ABILITY_WHO_SOURCEOPPONENT
};


int MTGAbility::parseCastRestrictions(MTGCardInstance * card,Player * player,string restrictions,string otherRestrictions)
{
    restrictions.append(otherRestrictions);
    //we can do this becuase the function calls send them seperately, so one will always be empty
    vector <string> restriction = split(restrictions, ',');
    AbilityFactory af;
    int cPhase = game->getCurrentGamePhase();
    for(unsigned int i = 0;i < restriction.size();i++)
    {
        int checkPhaseBased = af.parseRestriction(restriction[i]);
        switch (checkPhaseBased)
        {
        case PLAYER_TURN_ONLY:
            if (player != game->currentPlayer)
                return 0;
            break;
        case OPPONENT_TURN_ONLY:
            if (player == game->currentPlayer)
                return 0;
            break;
        case AS_SORCERY:
            if (player != game->currentPlayer)
                return 0;
            if (cPhase != Constants::MTG_PHASE_FIRSTMAIN && cPhase != Constants::MTG_PHASE_SECONDMAIN)
                return 0;
            break;
        }
        if (checkPhaseBased >= MY_BEFORE_BEGIN && checkPhaseBased <= MY_AFTER_EOT)
        {
            if (player != game->currentPlayer)
                return 0;
            if (cPhase != checkPhaseBased - MY_BEFORE_BEGIN + Constants::MTG_PHASE_BEFORE_BEGIN)
                return 0;
        }
        if (checkPhaseBased >= OPPONENT_BEFORE_BEGIN && checkPhaseBased <= OPPONENT_AFTER_EOT)
        {
            if (player == game->currentPlayer)
                return 0;
            if (cPhase != checkPhaseBased - OPPONENT_BEFORE_BEGIN + Constants::MTG_PHASE_BEFORE_BEGIN)
                return 0;
        }
        if (checkPhaseBased >= BEFORE_BEGIN && checkPhaseBased <= AFTER_EOT)
        {
            if (cPhase != checkPhaseBased - BEFORE_BEGIN + Constants::MTG_PHASE_BEFORE_BEGIN)
                return 0;
        }
        size_t typeRelated = restriction[i].find("typemin:");
        size_t check = NULL;
        if(typeRelated != string::npos)
        {
            bool less = false;
            bool more = false;
            int mytypemin = 0;
            int opponenttypemin = 0;
            int min = 0;
            int opponentmin = 0;
            string type = "*";
            string opponenttype = "*";

            check = restriction[i].find("mytypemin:");
            if( check != string::npos)
            {
                size_t start = restriction[i].find(":", check);
                size_t end = restriction[i].find(" ", check);
                size_t lesser = restriction[i].find(":less",check);
                size_t morer = restriction[i].find(":more",check);
                if(lesser != string::npos)
                {
                    less = true;
                }
                else if(morer != string::npos)
                {
                    more = true;
                }
                else
                {
                    min = atoi(restriction[i].substr(start + 1, end - start - 1).c_str());
                }
                size_t found = restriction[i].find("type(");
                if (found != string::npos)
                {
                    end = restriction[i].find(")", found);
                    type = restriction[i].substr(found + 5, end - found - 5).c_str();
                }
                mytypemin = card->controller()->game->inPlay->countByType(type.c_str());
                if(mytypemin < min && less == false && more == false)
                    return 0;
            }
            check = restriction[i].find("opponenttypemin:");
            if( check != string::npos)
            {
                size_t start = restriction[i].find(":", check);
                size_t end = restriction[i].find(" ", check);
                opponentmin = atoi(restriction[i].substr(start + 1, end - start - 1).c_str());

                size_t found = restriction[i].find("opponenttype(");
                if (found != string::npos)
                {
                    end = restriction[i].find(")", found);
                    opponenttype = restriction[i].substr(found + 13, end - found - 13).c_str();
                }
                opponenttypemin = card->controller()->opponent()->game->inPlay->countByType(opponenttype.c_str());
                if(opponenttypemin < opponentmin && less == false && more == false)
                    return 0;
            }
            if(less  && more == false && opponenttypemin <= mytypemin)
                return 0;
            if(less == false && more  && opponenttypemin >= mytypemin)
                return 0;
        }
        check = restriction[i].find("turn:");
        if(check != string::npos)
        {
            int Turn = 0;
            size_t start = restriction[i].find(":", check);
            size_t end = restriction[i].find(" ", check);
            Turn = atoi(restriction[i].substr(start + 1, end - start - 1).c_str());
            if(game->turn < Turn-1)
                return 0;
        }
        check = restriction[i].find("casted a spell");
        if(check != string::npos)
        {
            if(player->game->stack->seenThisTurn("*", Constants::CAST_ALL) < 1)
                return 0;
        }
        check = restriction[i].find("casted(");
        if(check != string::npos)
        {
            size_t end = restriction[i].find(")",check);
            string tc = restriction[i].substr(check + 7,end - check - 7);
            if(tc.find("|mystack") != string::npos)
            {
                if(player->game->stack->seenThisTurn(tc, Constants::CAST_ALL) < 1)
                    return 0;
            }
            if(tc.find("|opponentstack") != string::npos)
            {
                if(player->opponent()->game->stack->seenThisTurn(tc, Constants::CAST_ALL) < 1)
                    return 0;
            }
        }
        check = restriction[i].find("one of a kind");
        if(check != string::npos)
        {
            if(player->game->inPlay->hasName(card->name))
                return 0;
        }
        check = restriction[i].find("before battle damage");
        if(check != string::npos)
        {
            if(cPhase > Constants::MTG_PHASE_COMBATBLOCKERS)
                return 0;
        }
        check = restriction[i].find("after battle");
        if(check != string::npos)
        {
            if(cPhase < Constants::MTG_PHASE_COMBATBLOCKERS)
                return 0;
        }
        check = restriction[i].find("during battle");
        if(check != string::npos)
        {
            if(cPhase < Constants::MTG_PHASE_COMBATBEGIN ||cPhase > Constants::MTG_PHASE_COMBATEND )
                return 0;
        }
        check = restriction[i].find("control snow land");
        if(check != string::npos)
        {
            if(!player->game->inPlay->hasPrimaryType("snow","land"))
                return 0;
        }
        check = restriction[i].find("control two or more vampires");
        if(check != string::npos)
        {
            restriction.push_back("mytypemin:2 type(vampire)");
        }
        check = restriction[i].find("control less creatures");
        if(check != string::npos)
        {
            restriction.push_back("mytypemin:less type(creature)");
        }
        check = restriction[i].find("fourth turn");
        if(check != string::npos)
        {
            restriction.push_back("turn:4");
        }
    }
    return 1;
}

int MTGAbility::allowedToCast(MTGCardInstance * card,Player * player)
{
    return parseCastRestrictions(card,player,card->restriction,"");
}

int MTGAbility::allowedToAltCast(MTGCardInstance * card,Player * player)
{
       return parseCastRestrictions(card,player,"",card->otherrestriction);
}

int AbilityFactory::countCards(TargetChooser * tc, Player * player, int option)
{
    int result = 0;
    GameObserver * game = GameObserver::GetInstance();
    for (int i = 0; i < 2; i++)
    {
        if (player && player != game->players[i])
            continue;
        MTGGameZone * zones[] = { game->players[i]->game->inPlay, game->players[i]->game->graveyard, game->players[i]->game->hand };
        for (int k = 0; k < 3; k++)
        {
            for (int j = zones[k]->nb_cards - 1; j >= 0; j--)
            {
                MTGCardInstance * current = zones[k]->cards[j];
                if (tc->canTarget(current))
                {
                    switch (option)
                    {
                    case COUNT_POWER:
                        result += current->power;
                        break;
                    default:
                        result++;
                        break;
                    }
                }
            }
        }
    }
    return result;
}

Counter * AbilityFactory::parseCounter(string s, MTGCardInstance * target, Spell * spell)
{
    int nb = 1;
    int maxNb = 0;
    string name = "";
    size_t start = 0;
    size_t end = s.length();
    size_t separator = s.find(",", start);
    if (separator == string::npos)
        separator = s.find(".", start);
    if (separator != string::npos)
    {
        size_t separator2 = s.find(",", separator + 1);
        if (separator2 == string::npos)
            separator2 = s.find(".", separator + 1);
        size_t separator3 = string::npos;
        if (separator2 != string::npos)
        {
            name = s.substr(separator2 + 1, end - separator2 - 1);
            separator3 = s.find(",", separator2 + 1);
            if (separator3 != string::npos)
            {
                name = s.substr(separator2 + 1,separator3 - separator2 - 1);
            }
        }
        string nbstr = s.substr(separator + 1, separator2 - separator - 1);
        WParsedInt * wpi;
        if (target)
        {
            wpi = NEW WParsedInt(nbstr, spell, target);
        }
        else
        {
            wpi = NEW WParsedInt(atoi(nbstr.c_str()));
        }
        nb = wpi->getValue();
        delete (wpi);
        string maxNbstr;
        if (separator3 != string::npos)
        {
            maxNbstr = s.substr(separator3 + 1, end - separator3 - 1);
            WParsedInt * wpinb;
            if (target)
            {
                wpinb = NEW WParsedInt(maxNbstr, spell, target);
            }
            else
            {
                wpinb = NEW WParsedInt(atoi(maxNbstr.c_str()));
            }
            maxNb = wpinb->getValue();
            delete(wpinb);
        }
        end = separator;
    }

    string spt = s.substr(start, end - start);
    int power, toughness;
    if (parsePowerToughness(spt, &power, &toughness))
    {
        Counter * counter = NEW Counter(target, name.c_str(), power, toughness);
        counter->nb = nb;
        counter->maxNb = maxNb;
        return counter;
    }
    return NULL;
}

int AbilityFactory::parsePowerToughness(string s, int *power, int *toughness)
{
    size_t found = s.find("/");
    if (found != string::npos)
    {
        size_t end = s.find(" ", found);
        if (end == string::npos)
            end = s.size();
        size_t start = s.find_last_of(" ", found);
        if (start == string::npos)
            start = -1;

        *power = atoi(s.substr(start + 1, s.size() - found).c_str());
        *toughness = atoi(s.substr(found + 1, end - found - 1).c_str());

        return 1;
    }
    return 0;
}

TargetChooser * AbilityFactory::parseSimpleTC(const std::string& s, const std::string& _starter, MTGCardInstance * card, bool forceNoTarget)
{
    string starter = _starter;
    starter.append("(");

    size_t found = s.find(starter);
    if (found == string::npos)
        return NULL;

    size_t start = found + starter.size(); 

    size_t end = s.find(")", start);
    if (end == string::npos)
    {
        DebugTrace("malformed syntax " << s);
        return NULL;
    }

    string starget = s.substr(start , end - start);
    TargetChooserFactory tcf;
    TargetChooser * tc =  tcf.createTargetChooser(starget, card);

    if (tc && forceNoTarget)
        tc->targetter = NULL;

    return tc;
}

// evaluate trigger ability
// ie auto=@attacking(mytgt):destroy target(*)
// eval only the text between the @ and the first :
TriggeredAbility * AbilityFactory::parseTrigger(string s, string magicText, int id, Spell * spell, MTGCardInstance *card,
                                                Targetable * target)
{
    size_t found = string::npos;

    //restrictions on triggers  
    bool once = (s.find("once") != string::npos);
    bool sourceUntapped = (s.find("sourcenottap") != string::npos);
    bool sourceTap = (s.find("sourcetap") != string::npos);
    bool limitOnceATurn = (s.find("turnlimited") != string::npos);
    bool isSuspended = (s.find("suspended") != string::npos);
    bool opponentPoisoned = (s.find("opponentpoisoned") != string::npos);

    bool lifelost = (s.find("foelost(") != string::npos); 
    int lifeamount = lifelost ? atoi(s.substr(found + 8,')').c_str()) : 0 ;

    //Card Changed Zone
    found = s.find("movedto(");
    if (found != string::npos)
    {
        size_t end = s.find(")");
        string starget = s.substr(found + 8, end - found - 8);
        TargetChooserFactory tcf;

        TargetChooser *toTc = NULL;
        TargetChooser *toTcCard = NULL;
        end = starget.find("|");
        if (end == string::npos)
        {
            toTcCard = tcf.createTargetChooser("*", card);
            found = 0;
        }
        else
        {
            toTcCard = tcf.createTargetChooser(starget.substr(0, end).append("|*"), card);
            found = end + 1;
        }
        toTcCard->setAllZones();
        toTcCard->targetter = NULL; //avoid protection from
        starget = starget.substr(found, end - found).insert(0, "*|");
        toTc = tcf.createTargetChooser(starget, card);
        toTc->targetter = NULL; //avoid protection from

        TargetChooser *fromTc = NULL;
        TargetChooser * fromTcCard = NULL;
        found = s.find("from(");
        if (found != string::npos)
        {
            end = s.find("|", found);
            if (end == string::npos)
            {
                fromTcCard = tcf.createTargetChooser("*", card);
                found = found + 5;
            }
            else
            {
                fromTcCard = tcf.createTargetChooser(s.substr(found + 5, end - found - 5).append("|*"), card);
                found = end + 1;
            }
            fromTcCard->setAllZones();
            fromTcCard->targetter=NULL; //avoid protection from
            end = s.find(")", found);
            starget = s.substr(found, end - found).insert(0, "*|");
            fromTc = tcf.createTargetChooser(starget, card);
            fromTc->targetter = NULL; //avoid protection from
        }
        return NEW TrCardAddedToZone(id, card, (TargetZoneChooser *) toTc, toTcCard, (TargetZoneChooser *) fromTc, fromTcCard,once,sourceUntapped,isSuspended);
    }

    //Card unTapped
    if (TargetChooser *tc = parseSimpleTC(s,"untapped", card))
        return NEW TrCardTapped(id, card, tc, false);

    //Card Tapped
    if (TargetChooser *tc = parseSimpleTC(s,"tapped", card))
        return NEW TrCardTapped(id, card, tc, true);

    //Card Tapped for mana
    if (TargetChooser *tc = parseSimpleTC(s,"tappedformana", card))
        return NEW TrCardTappedformana(id, card, tc, true);
    
//CombatTrigger
    //Card card attacked and is blocked
    found = s.find("combat(");
    if (found != string::npos)
    {
        size_t end = s.find(")",found);
        string combatTrigger = s.substr(found + 7, end - found - 7);
        //find combat traits, only trigger types, the general restrictions are found earlier.
        bool attackingTrigger = false;
        bool attackedAloneTrigger = false;
        bool notBlockedTrigger = false;
        bool attackBlockedTrigger = false;
        bool blockingTrigger = false;
        vector <string> combatTriggerVector = split(combatTrigger, ',');
        for (unsigned int i = 0 ; i < combatTriggerVector.size() ; i++)
        { 
            if(combatTriggerVector[i] == "attacking")
                attackingTrigger = true;
            if(combatTriggerVector[i] == "attackedalone")
                attackedAloneTrigger = true;
            if(combatTriggerVector[i] == "notblocked")
                notBlockedTrigger = true;
            if(combatTriggerVector[i] == "blocked")
                attackBlockedTrigger = true;
            if(combatTriggerVector[i] == "blocking")
                blockingTrigger = true;
        }  
        //build triggers TCs
        TargetChooser *tc = parseSimpleTC(s, "source", card);
        if(!tc)//a source( is required, from( is optional.
            return NULL;

        TargetChooser *fromTc = parseSimpleTC(s, "from", card);
        
        return NEW TrCombatTrigger(id, card, tc, fromTc,once,limitOnceATurn,sourceUntapped,opponentPoisoned,
            attackingTrigger,attackedAloneTrigger,notBlockedTrigger,attackBlockedTrigger,blockingTrigger);
    }

    //Card card is drawn
    if (TargetChooser * tc = parseSimpleTC(s, "drawn", card))
        return NEW TrcardDrawn(id, card, tc);

    //Card is sacrificed
    if (TargetChooser * tc = parseSimpleTC(s, "sacrificed", card))
        return NEW TrCardSacrificed(id, card, tc);

    //Card is Discarded
    if (TargetChooser * tc = parseSimpleTC(s, "discarded", card))
        return NEW TrCardDiscarded(id, card, tc);

    //Card Damaging non combat
    if (TargetChooser * tc = parseSimpleTC(s, "noncombatdamaged", card))
    {
        TargetChooser *fromTc = parseSimpleTC(s, "from", card);
        return NEW TrDamaged(id, card, tc, fromTc, 2);
    }

    //Card Damaging combat
    if (TargetChooser * tc = parseSimpleTC(s, "combatdamaged", card))
    {
        TargetChooser *fromTc = parseSimpleTC(s, "from", card);
        return NEW TrDamaged(id, card, tc, fromTc, 1,sourceUntapped,limitOnceATurn);
    }

    //Card Damaging
    if (TargetChooser * tc = parseSimpleTC(s, "damaged", card))
    {
        TargetChooser *fromTc = parseSimpleTC(s, "from", card);
        return NEW TrDamaged(id, card, tc, fromTc, 0,sourceUntapped,limitOnceATurn);
    }

    //Lifed
    if (TargetChooser * tc = parseSimpleTC(s, "lifed", card))
    {
        TargetChooser *fromTc = parseSimpleTC(s, "from", card);
        return NEW TrLifeGained(id, card, tc, fromTc, 0,sourceUntapped);
    }

    //Life Loss
    if (TargetChooser * tc = parseSimpleTC(s, "lifeloss", card))
    {
        TargetChooser *fromTc = parseSimpleTC(s, "from", card);
        return NEW TrLifeGained(id, card, tc, fromTc,1,sourceUntapped);
    }

    //Card Damaged and killed by a creature this turn
    if (TargetChooser * tc = parseSimpleTC(s, "vampired", card))
    {
        TargetChooser *fromTc = parseSimpleTC(s, "from", card);
        return NEW TrVampired(id, card, tc, fromTc);
    }

    //when card becomes the target of a spell or ability
    if (TargetChooser * tc = parseSimpleTC(s, "targeted", card))
    {
        TargetChooser *fromTc = parseSimpleTC(s, "from", card);
        return NEW TrTargeted(id, card, tc, fromTc, 0);
    }
    
    int who = 0;
    if (s.find("my") != string::npos)
        who = 1;
    if (s.find("opponent") != string::npos)
        who = -1;
    if (s.find("targetcontroller") != string::npos)
        who = -2;

    //Next Time...
    found = s.find("next");
    if (found != string::npos)
    {
        for (int i = 0; i < Constants::NB_MTG_PHASES; i++)
        {
            found = s.find(Constants::MTGPhaseCodeNames[i]);
            if (found != string::npos)
            {
                return NEW TriggerNextPhase(id, card, target, i, who,sourceUntapped);
            }
        }
    }

    //Each Time...
    found = s.find("each");
    if (found != string::npos)
    {
        for (int i = 0; i < Constants::NB_MTG_PHASES; i++)
        {
            found = s.find(Constants::MTGPhaseCodeNames[i]);
            if (found != string::npos)
            {
                return NEW TriggerAtPhase(id, card, target, i, who,sourceUntapped,sourceTap,lifelost,lifeamount);
            }
        }
    }

    return NULL;
}

int AbilityFactory::parseRestriction(string s)
{
    if (s.find("myturnonly") != string::npos)
        return ActivatedAbility::PLAYER_TURN_ONLY;
    if (s.find("opponentturnonly") != string::npos)
        return ActivatedAbility::OPPONENT_TURN_ONLY;
    if (s.find("assorcery") != string::npos)
        return ActivatedAbility::AS_SORCERY;

    string types[] = { "my", "opponent", "" };
    int starts[] = { ActivatedAbility::MY_BEFORE_BEGIN, ActivatedAbility::OPPONENT_BEFORE_BEGIN, ActivatedAbility::BEFORE_BEGIN };
    for (int j = 0; j < 3; ++j)
    {
        size_t found = s.find(types[j]);
        if (found != string::npos)
        {
            for (int i = 0; i < Constants::NB_MTG_PHASES; i++)
            {
                string toFind = types[j];
                toFind.append(Constants::MTGPhaseCodeNames[i]).append("only");
                found = s.find(toFind);
                if (found != string::npos)
                {
                    return starts[j] + i;
                }
            }
        }
    }

    return ActivatedAbility::NO_RESTRICTION;
}

// When abilities encapsulate each other, gets the deepest one (it is the one likely to have the most relevant information)
MTGAbility * AbilityFactory::getCoreAbility(MTGAbility * a)
{
    AForeach * fea = dynamic_cast<AForeach*>(a);
    if(fea)
        return getCoreAbility(fea->ability);
        
    AAsLongAs * aea = dynamic_cast<AAsLongAs*>(a);
    if(aea)
        return getCoreAbility(aea->ability);
        
    GenericTargetAbility * gta = dynamic_cast<GenericTargetAbility*> (a);
    if (gta)
        return getCoreAbility(gta->ability);

    GenericActivatedAbility * gaa = dynamic_cast<GenericActivatedAbility*> (a);
    if (gaa)
        return getCoreAbility(gaa->ability);

    if (MultiAbility * abi = dynamic_cast<MultiAbility*>(a))
        return getCoreAbility(abi->abilities[0]);

    return a;
}

std::vector<std::string>&  AbilityFactory::parseBetween(const std::string& s, string start, string stop, bool stopRequired, std::vector<std::string>& elems)
{
    size_t found = s.find(start);
    if (found == string::npos)
        return elems;
    
    size_t offset = found + start.size();
    size_t end = s.find(stop, offset);
    if (end == string::npos && stopRequired)
        return elems;

    elems.push_back(s.substr(0,found));
    if (end != string::npos)
    {
        elems.push_back(s.substr(offset, end - offset));
        elems.push_back(s.substr(end + 1));
    }
    else
    {
        elems.push_back(s.substr(offset));
        elems.push_back("");
    }

    return elems;
}

std::vector<std::string> AbilityFactory::parseBetween(const std::string& s, string start, string stop, bool stopRequired)
{
    std::vector<std::string> elems;
    return parseBetween(s, start, stop, stopRequired, elems);
}

//Parses a string and returns the corresponding MTGAbility object
//Returns NULL if parsing failed
//Beware, Spell CAN be null when the function is called by the AI trying to analyze the effects of a given card
MTGAbility * AbilityFactory::parseMagicLine(string s, int id, Spell * spell, MTGCardInstance *card, int activated, int forceUEOT,
                int oneShot, int forceFOREVER, MTGGameZone * dest)
{
    size_t found;
    trim(s);

    //TODO This block redundant with calling function
    if (!card && spell)
        card = spell->source;
    if (!card)
        return NULL;
    MTGCardInstance * target = card->target;
    if (!target)
        target = card;

    //need to remove the section inside the transforms ability from the string before parsing
    //TODO: store string values of "&&" so we can remove the classes added just to add support
    //the current parser finds other abilities inside what should be nested abilities, and converts them into
    //actual abilities, this is a limitation.
    found = s.find("transforms((");
    if (found != string::npos && storedString.empty())
    {
        size_t real_end = s.find("))", found);
        size_t end = s.find(",", found);
        if (end == string::npos)
            end = real_end;
        size_t stypesStartIndex = found + 12;
        storedString.append(s.substr(stypesStartIndex, real_end - stypesStartIndex).c_str());
        s.erase(stypesStartIndex, real_end - stypesStartIndex);
    }

    vector<string> splitTrigger = parseBetween(s, "@", ":");
    if (splitTrigger.size())
    {
        TriggeredAbility * trigger = parseTrigger(splitTrigger[1], s, id, spell, card, target);
        //Dirty way to remove the trigger text (could get in the way)
        if (trigger)
        {
            MTGAbility * a = parseMagicLine(splitTrigger[2], id, spell, card, activated);
            if (!a)
            {
                delete trigger;
                return NULL;
            }
            return NEW GenericTriggeredAbility(id, card, trigger, a, NULL, target);
        }
    }

    int restrictions = parseRestriction(s);

    string newName = "";
    vector<string> splitName = parseBetween(s, "name(", ")");
    if (splitName.size())
    {
        newName = splitName[1];
        s = splitName[0];
        s.append(splitName[2]);
        //we erase the name section from the string to avoid 
        //accidently building an mtg ability with the text meant for menuText.
    }

    TargetChooser * tc = NULL;
    string sWithoutTc = s;

    //Target Abilities - We also handle the case "notatarget" here, for things such as copy effects
    bool isTarget = true;
    vector<string> splitTarget = parseBetween(s, "notatarget(", ")");
    if (splitTarget.size())
        isTarget = false;
    else
        splitTarget = parseBetween(s, "target(", ")");

    if (splitTarget.size())
    {
        TargetChooserFactory tcf;
        tc = tcf.createTargetChooser(splitTarget[1], card);
        if (!isTarget)
            tc->targetter = NULL;
        sWithoutTc = splitTarget[0];
        sWithoutTc.append(splitTarget[2]);
    }

    size_t delimiter = sWithoutTc.find("}:");
    size_t firstNonSpace = sWithoutTc.find_first_not_of(" ");
    if (delimiter != string::npos && firstNonSpace != string::npos && sWithoutTc[firstNonSpace] == '{')
    {
        ManaCost * cost = ManaCost::parseManaCost(sWithoutTc.substr(0, delimiter + 1), NULL, card);
        int doTap = 0; //Tap in the cost ?
        if(cost && cost->extraCosts)
        {
            for(unsigned int i = 0; i < cost->extraCosts->costs.size();i++)
            {
                ExtraCost * tapper = dynamic_cast<TapCost*>(cost->extraCosts->costs[i]);
                if(tapper)
                    doTap = 1;

            }
        }
        if (doTap || cost)
        {
            string s1 = sWithoutTc.substr(delimiter + 2);
            //grabbing the sideffect string and amount before parsing abilities.
            //side effect ei:dragond whelp.
            MTGAbility * sideEffect = NULL;
            string usesBeforeSideEffect = "";
            size_t limiteffect_str = s1.find("limit^");
            if (limiteffect_str != string::npos)
            {
            size_t end = s1.rfind("^");
            string sideEffectStr = s1.substr(limiteffect_str + 6,end - limiteffect_str - 6);
            s1.erase(limiteffect_str,end - limiteffect_str);
            end = s1.find("^");
            usesBeforeSideEffect = s1.substr(end+1);
            s1.erase(end-1);
            sideEffect = parseMagicLine(sideEffectStr, id, spell, card, 1);
            }
            
            
            MTGAbility * a = parseMagicLine(s1, id, spell, card, 1);
            if (!a)
            {
                DebugTrace("ABILITYFACTORY Error parsing: " << sWithoutTc);
                return NULL;
            }
            string limit = "";
            size_t limit_str = sWithoutTc.find("limit:");
            if (limit_str != string::npos)
            {
                limit = sWithoutTc.substr(limit_str + 6);
            }
            ////A stupid Special case for ManaProducers, becuase Ai only understands manaabilities that are not nested.
            AManaProducer * amp = dynamic_cast<AManaProducer*> (a);
            if (amp)
            {
                amp->cost = cost;
                if (cost && card->typeAsTarget() == TARGET_CARD)
                    cost->setExtraCostsAction(a, card);
                amp->oneShot = 0;
                amp->tap = doTap;
                amp->limit = limit;
                amp->sideEffect = sideEffect;
                amp->usesBeforeSideEffects = usesBeforeSideEffect;
                amp->restrictions = restrictions;
                return amp;
            }
            
            AEquip *ae = dynamic_cast<AEquip*> (a);
            if (ae)
            {
                ae->cost = cost;
                if (!tc)
                {
                    TargetChooserFactory tcf;
                    tc = tcf.createTargetChooser("creature|myBattlefield", card);
                }
                ae->tc = tc;
                return ae;
            }
            if (tc)
                return NEW GenericTargetAbility(newName,id, card, tc, a, cost, limit,sideEffect,usesBeforeSideEffect, restrictions, dest);
            return NEW GenericActivatedAbility(newName,id, card, a, cost, limit,sideEffect,usesBeforeSideEffect,restrictions, dest);
        }
        SAFE_DELETE(cost);
    }

    // figure out alternative cost effects
    string keyword;
    int costType = -1;

    for (size_t i = 0; i < sizeof(kAlternateCostIds)/sizeof(kAlternateCostIds[0]); ++i)
    {
        if (s.find(kAlternateCostKeywords[i]) == 0)
        {
            if (!(spell && spell->FullfilledAlternateCost(kAlternateCostIds[i])))
            {
                DebugTrace("INFO parseMagicLine: Alternative Cost was not fulfilled for " << s);
                SAFE_DELETE(tc);
                return NULL;
            }
            return parseMagicLine(s.substr(kAlternateCostKeywords[i].length()), id, spell, card);
        }
    }

    //When...comes into play, you may...
    //When...comes into play, choose one...
    const string mayKeywords[] = {"may ", "choice "};
    const bool mayMust[] = { false, true };
    for (size_t i =0; i < sizeof(mayMust)/sizeof(mayMust[0]); ++i)
    {
        if (sWithoutTc.find(mayKeywords[i]) == 0)
        {
            string s1 = sWithoutTc.substr(mayKeywords[i].length());
            MTGAbility * a1 = parseMagicLine(s1, id, spell, card);
            if (!a1)
                return NULL;

            if (tc)
                a1 = NEW GenericTargetAbility(newName,id, card, tc, a1);
            else
                a1 = NEW GenericActivatedAbility(newName,id, card, a1, NULL);
            return NEW MayAbility(id, a1, card,mayMust[i]);
        }
    }
    
    //Upkeep Cost
    found = s.find("upcostmulti");
    if (found != string::npos)
    {
        bool Cumulative = false;
        size_t cumulative = s.find("cumulativeupcost");
        if(cumulative != string::npos)
            Cumulative = true;
        size_t start = s.find("[");
        size_t end = s.find("]", start);
        string s1 = s.substr(start + 1, end - start - 1);
        size_t seperator = s1.find(";");
        int phase = Constants::MTG_PHASE_UPKEEP;
        int once = 0;
        if (seperator != string::npos)
        {
            for (int i = 0; i < Constants::NB_MTG_PHASES; i++)
            {
                if (s1.find("next") != string::npos)
                    once = 1;
                string phaseStr = Constants::MTGPhaseCodeNames[i];
                if (s1.find(phaseStr) != string::npos)
                {
                    phase = PhaseRing::phaseStrToInt(phaseStr);
                    break;
                }
            }
            s1 = s1.substr(0, seperator - 1);
        }
        ManaCost * cost = ManaCost::parseManaCost(s1);

        if (!cost)
        {
            DebugTrace("MTGABILITY: Parsing Error: " << s);
            return NULL;
        }

        string sAbility = s.substr(end + 1);
        MTGAbility * a = parseMagicLine(sAbility, id, spell, card);

        if (!a)
        {
            DebugTrace("MTGABILITY: Parsing Error: " << s);
            delete (cost);
            return NULL;
        }

        return NEW AUpkeep(id, card, a, cost, restrictions, phase, once,Cumulative);
    }
    
    //Phase based actions
    found = s.find("phaseactionmulti");
    if (found != string::npos)
    {
        vector<string> splitActions = parseBetween(s, "[", "]");
        if (!splitActions.size())
        {
            DebugTrace("MTGABILITY:Parsing Error " << s);
            return NULL;
        }
        string s1 = splitActions[1];
        int phase = Constants::MTG_PHASE_UPKEEP;
        for (int i = 0; i < Constants::NB_MTG_PHASES; i++)
        {
            string phaseStr = Constants::MTGPhaseCodeNames[i];
            if (s1.find(phaseStr) != string::npos)
            {
                phase = PhaseRing::phaseStrToInt(phaseStr);
                break;
            }
        }
        
        bool opponentturn = (s1.find("my") == string::npos);
        bool myturn = (s1.find("opponent") == string::npos);

        bool sourceinPlay = (s1.find("sourceinplay") != string::npos);
        bool next = (s1.find("next") == string::npos); //Why is this one the opposite of the two others? That's completely inconsistent
        bool once = (s1.find("once") != string::npos);

        MTGCardInstance * _target = NULL;
        if (spell)
            _target = spell->getNextCardTarget();
        if(!_target)
            _target = target;
          return NEW APhaseActionGeneric(id, card,_target, splitActions[2], restrictions, phase,sourceinPlay,next,myturn,opponentturn,once);
    }
    
    //Multiple abilities for ONE cost
    found = s.find("&&");
    if (found != string::npos)
    {
        SAFE_DELETE(tc);
        vector<string> multiEffects = split(s,'&');
        MultiAbility * multi = NEW MultiAbility(id, card, target, NULL);
        for(unsigned int i = 0;i < multiEffects.size();i++)
        {
            if(!multiEffects[i].empty())
            {
                MTGAbility * addAbility = parseMagicLine(multiEffects[i], id, spell, card, activated);
                multi->Add(addAbility);
            }
        }
        multi->oneShot = 1;
        return multi;
    }

    //rather dirty way to stop thises and lords from conflicting with each other.
    size_t lord = string::npos;
    for (size_t j = 0; j < kLordKeywordsCount; ++j)
    {
        size_t found2 = s.find(kLordKeywords[j]);
        if (found2 != string::npos && ((found == string::npos) || found2 < found))
        {
            lord = found2;
        }
    }

    //This, ThisForEach;
    found = string::npos;
    int i = -1;
    for (size_t j = 0; j < kThisKeywordsCount; ++j)
    {
        size_t found2 = s.find(kThisKeywords[j]);
        if (found2 != string::npos && ((found == string::npos) || found2 < found))
        {
            found = found2;
            i = j;
        }
    }
    if (found != string::npos && found < lord)
    {
        //why does tc even exist here? This shouldn't happen...
        SAFE_DELETE(tc); //http://code.google.com/p/wagic/issues/detail?id=424

        size_t header = kThisKeywords[i].size();
        size_t end = s.find(")", found + header);
        string s1;
        if (found == 0 || end != s.size() - 1)
        {
            s1 = s.substr(end + 1);
        }
        else
        {
            s1 = s.substr(0, found);
        }
        if (end != string::npos)
        {
            string thisDescriptorString = s.substr(found + header, end - found - header);
            ThisDescriptorFactory tdf;
            ThisDescriptor * td = tdf.createThisDescriptor(thisDescriptorString);

            if (!td)
            {
                DebugTrace("MTGABILITY: Parsing Error:" << s);
                return NULL;
            }

            MTGAbility * a = parseMagicLine(s1, id, spell, card, 0, activated);
            if (!a)
            {
                SAFE_DELETE(td);
                return NULL;
            }
            MTGAbility * result = NULL;
            int oneShot = 0;
            found = s.find(" oneshot");
            if (found != string::npos)
                oneShot = 1;
            if (activated)
                oneShot = 1;
            if (card->hasType(Subtypes::TYPE_SORCERY) || card->hasType(Subtypes::TYPE_INSTANT))
                oneShot = 1;
            if (a->oneShot)
                oneShot = 1;
            Damageable * _target = NULL;
            if (spell)
                _target = spell->getNextDamageableTarget();
            if (!_target)
                _target = target;

            switch (i)
            {
            case 0:
                result = NEW AThis(id, card, _target, td, a);
                break;
            case 1:
                result = NEW AThisForEach(id, card, _target, td, a);
                break;
            default:
                result = NULL;
            }
            if (result)
            {
                result->oneShot = oneShot;
            }
            return result;
        }
        return NULL;
    }

    //Lord, foreach, aslongas

    found = string::npos;
    i = -1;
	for (size_t j = 0; j < kLordKeywordsCount; ++j)
    {
        size_t found2 = s.find(kLordKeywords[j]);
        if (found2 != string::npos && ((found == string::npos) || found2 < found))
        {
            found = found2;
            i = j;
        }
    }
    if (found != string::npos)
    {
        SAFE_DELETE(tc);
        size_t header = kLordKeywords[i].size();
        size_t end = s.find(")", found + header);
        string s1;
        if (found == 0 || end != s.size() - 1)
        {
            s1 = s.substr(end + 1);
        }
        else
        {
            s1 = s.substr(0, found);
        }
        if (end != string::npos)
        {
            int lordIncludeSelf = 1;
            size_t other = s1.find(" other");
            if (other != string::npos)
            {
                lordIncludeSelf = 0;
                s1.replace(other, 6, "");
            }
            string lordTargetsString = s.substr(found + header, end - found - header);
            TargetChooserFactory tcf;
            TargetChooser * lordTargets = tcf.createTargetChooser(lordTargetsString, card);

            if (!lordTargets)
            {
                DebugTrace("MTGABILITY: Parsing Error: " << s);
                return NULL;
            }

            MTGAbility * a = parseMagicLine(s1, id, spell, card, 0, activated); //activated lords usually force an end of turn ability
            if (!a)
            {
                SAFE_DELETE(lordTargets);
                return NULL;
            }
            MTGAbility * result = NULL;
            int oneShot = 0;
            if (activated)
                oneShot = 1;
            if (i == 4)
                oneShot = 1;
            if (a->oneShot)
                oneShot = 1;
            if (card->hasType(Subtypes::TYPE_SORCERY) || card->hasType(Subtypes::TYPE_INSTANT))
                oneShot = 1;
            found = s.find("while ");
            if (found != string::npos)
                oneShot = 0;
            found = s.find(" oneshot");
            if (found != string::npos)
                oneShot = 1;
            Damageable * _target = NULL;
            if (spell)
                _target = spell->getNextDamageableTarget();
            if (!_target)
                _target = target;

            int mini = 0;
            int maxi = 0;
            bool miniFound = false;
            bool maxiFound = false;
            bool compareZone = false;

            found = s.find(" >");
            if (found != string::npos)
            {
                mini = atoi(s.substr(found + 2, 3).c_str());
                miniFound = true;
            }

            found = s.find(" <");
            if (found != string::npos)
            {
                maxi = atoi(s.substr(found + 2, 3).c_str());
                maxiFound = true;
            }
            
            found = s.find("compare");
            if (found != string::npos)
            {
            compareZone = true;
            }
            
            switch (i)
            {
            case 0:
                result = NEW ALord(id, card, lordTargets, lordIncludeSelf, a);
                break;
            case 1:
                result = NEW AForeach(id, card, _target, lordTargets, lordIncludeSelf, a, mini, maxi);
                break;
            case 2:
                {
                    if (!miniFound && !maxiFound)//for code without an operator treat as a mini.
                    {
                        miniFound = true;
                    }
                    result = NEW AAsLongAs(id, card, _target, lordTargets, lordIncludeSelf, a, mini, maxi,miniFound,maxiFound,compareZone);
                }
                break;
            case 3:
                result = NEW ATeach(id, card, lordTargets, lordIncludeSelf, a);
                break;
            case 4:
                result = NEW ALord(id, card, lordTargets, lordIncludeSelf, a);
                break;
            default:
                result = NULL;
            }
            if (result)
                result->oneShot = oneShot;
            return result;
        }
        return NULL;
    }

    if (!activated && tc)
    {
        MTGAbility * a = parseMagicLine(sWithoutTc, id, spell, card);
        if (!a)
        {
            DebugTrace("ABILITYFACTORY Error parsing: " << s);
            return NULL;
        }
        a = NEW GenericTargetAbility(newName,id, card, tc, a);
        return NEW MayAbility(id, a, card, true);
    }

    SAFE_DELETE(tc);
    
    //dynamic ability builder
    vector<string> splitDynamic = parseBetween(s, "dynamicability<!", "!>");
    if (splitDynamic.size())
    {
        string s1 = splitDynamic[1];
        int type = 0;
        int effect = 0;
        int who = 0;
        int amountsource = 0;

        //source
        for (size_t i = 0; i < sizeof(kDynamicSourceIds)/sizeof(kDynamicSourceIds[0]); ++i)
        {
            size_t abilityamountsource = s1.find(kDynamicSourceKeywords[i]);
            if (abilityamountsource != string::npos)
            {
                amountsource = kDynamicSourceIds[i];
                break;
            }
        }

        //main variable or type
        for (size_t i = 0; i < sizeof(kDynamicTypeIds)/sizeof(kDynamicTypeIds[0]); ++i)
        {
            size_t abilitytype = s1.find(kDynamicTypeKeywords[i]);
            if (abilitytype != string::npos)
            {
                type = kDynamicTypeIds[i];
                break;
            }
        }

        //effect
        for (size_t i = 0; i < sizeof(kDynamicEffectIds)/sizeof(kDynamicEffectIds[0]); ++i)
        {
            size_t abilityeffect = s1.find(kDynamicEffectKeywords[i]);
            if (abilityeffect != string::npos)
            {
                effect = kDynamicEffectIds[i];
                break;
            }
        }

        //target
        for (size_t i = 0; i < sizeof(kDynamicWhoIds)/sizeof(kDynamicWhoIds[0]); ++i)
        {
            size_t abilitywho = s1.find(kDynamicWhoKeywords[i]);
            if (abilitywho != string::npos)
            {
                who = kDynamicWhoIds[i];
                break;
            }
        }

        string sAbility = splitDynamic[2];
        MTGAbility * stored = NULL;
        if(!sAbility.empty())
            stored = parseMagicLine(sAbility, id, spell, card);

        MTGAbility * a = NEW AADynamic(id, card, target,type,effect,who,amountsource,stored);
        a->oneShot = 1;
        return a;
   }

    //Phase based actions
    //TODO: This is 100% the same code as phaseActionMulti, can't these 2 be merged somehow?   
    found = s.find("phaseaction");
    if (found != string::npos)
    {
        vector<string> splitActions = parseBetween(s, "[", "]");
        if (!splitActions.size())
        {
            DebugTrace("MTGABILITY:Parsing Error " << s);
            return NULL;
        }
        string s1 = splitActions[1];
        int phase = Constants::MTG_PHASE_UPKEEP;
        for (int i = 0; i < Constants::NB_MTG_PHASES; i++)
        {
            string phaseStr = Constants::MTGPhaseCodeNames[i];
            if (s1.find(phaseStr) != string::npos)
            {
                phase = PhaseRing::phaseStrToInt(phaseStr);
                break;
            }
        }

        bool opponentturn = (s1.find("my") == string::npos);
        bool myturn = (s1.find("opponent") == string::npos);

        bool sourceinPlay = (s1.find("sourceinplay") != string::npos);
        bool next = (s1.find("next") == string::npos); //Why is this one the opposite of the two others? That's completely inconsistent
        bool once = (s1.find("once") != string::npos);

        MTGCardInstance * _target = NULL;
        if (spell)
            _target = spell->getNextCardTarget();
        if(!_target)
            _target = target;
          return NEW APhaseActionGeneric(id, card,_target, trim(splitActions[2]), restrictions, phase,sourceinPlay,next,myturn,opponentturn,once);
    }

    //Upkeep Cost
    //TODO: This is 100% the same code as upkeepCostMulti, can't these 2 be merged somehow?
    //also see phaseActionMulti
    found = s.find("upcost");
    if (found != string::npos)
    {
    bool Cumulative = false;
    size_t cumulative = s.find("cumulativeupcost");
    if(cumulative != string::npos)
    Cumulative = true;
        size_t start = s.find("[");
        size_t end = s.find("]", start);
        string s1 = s.substr(start + 1, end - start - 1);
        size_t seperator = s1.find(";");
        int phase = Constants::MTG_PHASE_UPKEEP;
        int once = 0;
        if (seperator != string::npos)
        {
            for (int i = 0; i < Constants::NB_MTG_PHASES; i++)
            {
                if (s1.find("next") != string::npos)
                    once = 1;

                string phaseStr = Constants::MTGPhaseCodeNames[i];
                if (s1.find(phaseStr) != string::npos)
                {
                    phase = PhaseRing::phaseStrToInt(phaseStr);
                    break;
                }

            }
            s1 = s1.substr(0, seperator - 1);
        }
        ManaCost * cost = ManaCost::parseManaCost(s1);

        if (!cost)
        {
            DebugTrace("MTGABILITY: Parsing Error: " << s);
            return NULL;
        }

        string sAbility = s.substr(end + 1);
        MTGAbility * a = parseMagicLine(sAbility, id, spell, card);

        if (!a)
        {
            DebugTrace("MTGABILITY: Parsing Error: " << s);
            delete (cost);
            return NULL;
        }

        return NEW AUpkeep(id, card, a, cost, restrictions, phase, once,Cumulative);
    }

    //Cycling
    found = s.find("cycling");
    if (found != string::npos)
    {
        MTGAbility * a = NEW ACycle(id, card, target);
        a->oneShot = 1;
        return a;
    }

    //ninjutsu
    found = s.find("ninjutsu");
    if (found != string::npos)
    {
        MTGAbility * a = NEW ANinja(id, card, target);
        a->oneShot = 1;
        return a;
    }

    //combat removal
    found = s.find("removefromcombat");
    if (found != string::npos)
    {
        MTGAbility * a = NEW ACombatRemoval(id, card, target);
        a->oneShot = 1;
        return a;
    }

   //momentary blink
    found = s.find("(blink)");
    if (found != string::npos)
    {
        bool ueoteffect = (s.find("(blink)ueot") != string::npos);
        bool forsource = (s.find("(blink)forsrc") != string::npos);
        bool blinkhand = (s.find("hand(blink)") != string::npos);
        size_t returnAbility = s.find("return(");
        string sAbility = s.substr(returnAbility + 7);
        MTGAbility * stored = NULL;
        if(!sAbility.empty())
        {
            stored = parseMagicLine(sAbility, id, spell, card);
        }
        MTGAbility * a = NEW ABlinkGeneric(id, card, target,ueoteffect,forsource,blinkhand,stored);
        a->oneShot = 1;
        return a;
    }

    //Fizzle (counterspell...)
    found = s.find("fizzle");
    if (found != string::npos)
    {
        Spell * starget = NULL;
        if (spell)
            starget = spell->getNextSpellTarget();
        MTGAbility * a = NEW AAFizzler(id, card, starget);
        a->oneShot = 1;
        return a;
    }

    //Describes a player target in many abilities
    int who = TargetChooser::UNSET;
    if (s.find(" controller") != string::npos)
        who = TargetChooser::CONTROLLER;
    if (s.find(" opponent") != string::npos)
        who = TargetChooser::OPPONENT;
    if (s.find(" targetcontroller") != string::npos)
        who = TargetChooser::TARGET_CONTROLLER;
    if (s.find(" owner") != string::npos)
        who = TargetChooser::OWNER;

    //livingweapon (used for token below)
    bool aLivingWeapon = (s.find("livingweapon") != string::npos);

    //Token creator. Name, type, p/t, abilities
    vector<string> splitToken = parseBetween(s, "token(", ")");
    if (splitToken.size())
    {
        WParsedInt * multiplier = NULL;
        size_t star = s.find("*");
        string starfound = "";
        if (star != string::npos)
        {
            starfound = s.substr(star + 1);
            size_t starEnd= starfound.find_first_of(" ");
            starfound = starfound.substr(0,starEnd);
            multiplier = NEW WParsedInt(starfound, spell, card);
        }
        
        int tokenId = atoi(splitToken[1].c_str());
        if (tokenId)
        {
            MTGCard * safetycard = MTGCollection()->getCardById(tokenId);
            if (!safetycard) //Error, card not foudn in DB
                return NEW ATokenCreator(id, card, target, NULL, "ID NOT FOUND", "ERROR ID",0, 0, "",0, NULL,0);

            ATokenCreator * tok = NEW ATokenCreator(id, card,target, NULL, tokenId, starfound, multiplier, who);
            tok->oneShot = 1;
            return tok;
        }

        string tokenDesc = splitToken[1];
        vector<string> tokenParameters = split(tokenDesc, ',');
        if (tokenParameters.size() < 3)
        {
            DebugTrace("incorrect Parameters for Token" << tokenDesc);
            return NULL;
        }
        string sname = tokenParameters[0];
        string stypes = tokenParameters[1];
        string spt = tokenParameters[2];

        //reconstructing string abilities from the split version,
        // then we re-split it again in the token constructor,
        // this needs to be improved
        string sabilities = (tokenParameters.size() > 3)? tokenParameters[3] : "";
        for (size_t i = 4; i < tokenParameters.size(); ++i)
        {
            sabilities.append(",");
            sabilities.append(tokenParameters[i]);
        }
        int value = 0;
        if (spt.find("xx/xx") != string::npos)
            value = card->X / 2;
        else if (spt.find("x/x") != string::npos)
            value = card->X;

        int power, toughness;
        parsePowerToughness(spt, &power, &toughness);
        
        ATokenCreator * tok = NEW ATokenCreator(
            id, card,target, NULL, sname, stypes, power + value, toughness + value, 
            sabilities, starfound, multiplier, who, aLivingWeapon, spt);
        tok->oneShot = 1;
        if(aLivingWeapon)
            tok->forceDestroy = 1;
        return tok;  
    }

    //Equipment
    found = s.find("equip");
    if (found != string::npos)
    {
        return NEW AEquip(id, card);
    }
    
    //Equipment (attach)
    found = s.find("attach");
    if (found != string::npos)
    {
        return NEW AEquip(id, card, 0, ActivatedAbility::NO_RESTRICTION);
    }

    //MoveTo Move a card from a zone to another
    vector<string> splitMove = parseBetween(s, "moveto(", ")");
    if (splitMove.size())
    {
        //hack for http://code.google.com/p/wagic/issues/detail?id=120
        //We assume that auras don't move their own target...
        if (card->hasType(Subtypes::TYPE_AURA))
            target = card;

        MTGAbility * a = NEW AAMover(id, card, target, splitMove[1]);
        a->oneShot = 1;
        return a;
    }

    //Copy a target
    found = s.find("copy");
    if (found != string::npos)
    {
        MTGAbility * a = NEW AACopier(id, card, target);
        a->oneShot = 1;
        return a;
    }

    //imprint
    found = s.find("phaseout");
    if (found != string::npos)
    {
        MTGAbility * a = NEW AAPhaseOut(id, card, target);
        a->oneShot = 1;
        return a;
    }

    //clone
    found = s.find("clone");
    if (found != string::npos)
    {
        string with = "";
        vector<string> splitWith = parseBetween(s, "with(", ")");
        if (splitWith.size())
        {
            with = splitWith[1];
        }
        MTGAbility * a = NEW AACloner(id, card, target, 0, who, with);
        a->oneShot = 1;
        return a;
    }

    //Bury, destroy, sacrifice, reject(discard)
    if (s.find("bury") != string::npos)
    {
        MTGAbility *a = NEW AABuryCard(id, card, target, AABanishCard::BURY);
        a->oneShot = 1;
        return a;
    }

    if (s.find("destroy") != string::npos)
    {
        MTGAbility * a = NEW AADestroyCard(id, card, target, AABanishCard::DESTROY);
        a->oneShot = 1;
        return a;
    }

    if (s.find("sacrifice") != string::npos)
    {
        MTGAbility *a = NEW AASacrificeCard(id, card, target, AABanishCard::SACRIFICE);
        a->oneShot = 1;
        return a;
    }

    if (s.find("reject") != string::npos)
    {
        MTGAbility *a = NEW AADiscardCard(id, card, target, AABanishCard::DISCARD);
        a->oneShot = 1;
        return a;
    }

    found = s.find("ueot");
    if (found != string::npos)
        forceUEOT = 1;
    found = s.find("oneshot");
    if (found != string::npos)
        oneShot = 1;
    found = s.find("forever");
    if (found != string::npos)
        forceFOREVER = 1;

    //Prevent Damage
    const string preventDamageKeywords[] = { "preventallcombatdamage", "preventallnoncombatdamage", "preventalldamage", "fog" };
    const int preventDamageTypes[] = {0, 2, 1, 0}; //TODO enum ?
    const bool preventDamageForceOneShot[] = { false, false, false, true };

    for (size_t i = 0; i < sizeof(preventDamageTypes)/sizeof(preventDamageTypes[0]); ++i)
    {
        found = s.find(preventDamageKeywords[i]);
        if (found != string::npos)
        {
            string to = "";
            string from = "";

            vector<string> splitTo = parseBetween(s, "to(", ")");
            if (splitTo.size())
                to = splitTo[1];

            vector<string> splitFrom = parseBetween(s, "from(", ")");
            if (splitFrom.size())
                from = splitFrom[1];

            MTGAbility * ab;
            if (forceUEOT || preventDamageForceOneShot[i])
                ab = NEW APreventDamageTypesUEOT(id, card, to, from, preventDamageTypes[i]);
            else
                ab = NEW APreventDamageTypes(id, card, to, from, preventDamageTypes[i]);

            if (preventDamageForceOneShot[i])
                ab->oneShot = 1;

            return ab;
        }
    }
 
    //Reset damages on cards
    found = s.find("resetdamage");
    if (found != string::npos)
    {
        MTGAbility * a = NEW AAResetDamage(id, card, target);
        a->oneShot = 1;
        return a;
    }

    //Damage
    vector<string> splitDamage = parseBetween(s, "damage:", " ", false);
    if (splitDamage.size())
    {
        Targetable * t = spell ? spell->getNextTarget() : NULL;
        MTGAbility * a = NEW AADamager(id, card, t, splitDamage[1], NULL, who);
        a->oneShot = 1;
        return a;
    }

    //remove poison
    vector<string> splitPoison = parseBetween(s, "alterpoison:", " ", false);
    if (splitPoison.size())
    {
        int poison = atoi(splitPoison[1].c_str());
        Targetable * t = spell ? spell->getNextTarget() : NULL;
        MTGAbility * a = NEW AAAlterPoison(id, card, t, poison, NULL, who);
        a->oneShot = 1;
        return a;
    }

    //prevent next damage
    vector<string> splitPrevent = parseBetween(s, "prevent:", " ", false);
    if (splitPrevent.size())
    {
        int preventing = atoi(splitPrevent[1].c_str());
        Targetable * t = spell ? spell->getNextTarget() : NULL;
        MTGAbility * a = NEW AADamagePrevent(id, card, t, preventing, NULL, who);
        a->oneShot = 1;
        return a;
    }

    //set life total
    vector<string> splitLifeset = parseBetween(s, "lifeset:", " ", false);
    if (splitLifeset.size())
    {
        WParsedInt * life = NEW WParsedInt(splitLifeset[1], spell, card);
        Damageable * t = spell ? spell->getNextDamageableTarget() : NULL;
        MTGAbility * a = NEW AALifeSet(id, card, t, life, NULL, who);
        a->oneShot = 1;
        return a;
    }

    //gain/lose life
    vector<string> splitLife = parseBetween(s, "life:", " ", false);
    if (splitLife.size())
    {
        Targetable * t = spell ? spell->getNextTarget() : NULL;
        MTGAbility * a = NEW AALifer(id, card, t, splitLife[1], NULL, who);
        a->oneShot = 1;
        return a;
    }

    // Win the game
    found = s.find("wingame");
    if (found != string::npos)
    {
        Damageable * d = NULL;
        if (spell)
            d = spell->getNextDamageableTarget();
        MTGAbility * a = NEW AAWinGame(id, card, d, NULL, who);
        a->oneShot = 1;
        return a;
    }

    //Draw
    vector<string> splitDraw = parseBetween(s, "draw:", " ", false);
    if (splitDraw.size())
    {
        Targetable * t = spell ? spell->getNextTarget() : NULL;
        MTGAbility * a = NEW AADrawer(id, card, t, NULL,splitDraw[1], who);
        a->oneShot = 1;
        return a;
    }

    //Deplete
    vector<string> splitDeplete = parseBetween(s, "deplete:", " ", false);
    if (splitDeplete.size())
    {
        Targetable * t = spell ? spell->getNextTarget() : NULL;
        MTGAbility * a = NEW AADepleter(id, card, t , splitDeplete[1], NULL, who);
        a->oneShot = 1;
        return a;
    }

    //Shuffle
    found = s.find("shuffle");
    if (found != string::npos)
    {
        Targetable * t = spell? spell->getNextTarget() : NULL;
        MTGAbility * a = NEW AAShuffle(id, card, t, NULL, who);
        a->oneShot = 1;
        return a;
    }

    //Remove Mana from ManaPool
    vector<string> splitRemove = parseBetween(s, "removemana(", ")");
    if (splitRemove.size())
    {
        Targetable * t = spell? spell->getNextTarget() : NULL;
        MTGAbility *a = NEW AARemoveMana(id, card, t, splitRemove[1], who);
        a->oneShot = 1;
        return a;
    }

    //Cast/Play Restrictions
	for (size_t i = 0; i < kMaxCastKeywordsCount; ++i)
    {
        vector<string> splitCast = parseBetween(s, kMaxCastKeywords[i], ")");
        if (splitCast.size())
        {
            TargetChooserFactory tcf;
            TargetChooser * castTargets = tcf.createTargetChooser(splitCast[1], card);

            size_t space = s.find(" ");
            vector<string> splitValue = parseBetween(splitCast[2], "", " ", false);
            if (!splitValue.size())
            {
                DebugTrace ("MTGABILITY: Error parsing Cast/Play Restriction" << s);
                return NULL;
            }

            string valueStr = splitValue[1];
            bool modifyExisting = (valueStr.find("+") != string::npos || valueStr.find("-") != string::npos);

            WParsedInt * value = NEW WParsedInt(valueStr, spell, card);
            Targetable * t = spell? spell->getNextTarget() : NULL;
            if (card->hasType(Subtypes::TYPE_INSTANT) || card->hasType(Subtypes::TYPE_SORCERY) || forceUEOT)
            {
                return NEW AInstantCastRestrictionUEOT(id, card, t, castTargets, value, modifyExisting, kMaxCastZones[i], who);
            }
            return NEW ACastRestriction(id, card, t, castTargets, value, modifyExisting, kMaxCastZones[i], who);
        }
    }

    //Discard
    vector<string> splitDiscard = parseBetween(s, "discard:", " ", false);
    if (splitDiscard.size())
    {
        Targetable * t = spell ? spell->getNextTarget() : NULL;
        MTGAbility * a = NEW AARandomDiscarder(id, card, t, splitDiscard[1], NULL, who);
        a->oneShot = 1;
        return a;
    }

    //rampage
    vector<string> splitRampage = parseBetween(s, "rampage(", ")");
    if (splitRampage.size())
    {
        vector<string> rampageParameters = split(splitRampage[1], ',');
        int power, toughness;
        if (!parsePowerToughness(rampageParameters[0], &power, &toughness))
        {
            DebugTrace("MTGAbility Parse error in rampage" << s);
            return NULL;
        }
        int MaxOpponent = atoi(rampageParameters[1].c_str());
        return NEW ARampageAbility(id, card, power, toughness, MaxOpponent);
    }
    
    //flanking
    if (s.find("flanker") != string::npos)
    {
        return NEW AFlankerAbility(id, card);
    }

    //spirit link
    //combat damage spirit link
    if (s.find("spiritlink") != string::npos)
    {
        bool combatOnly = (s.find("combatspiritlink") != string::npos);
        return NEW ASpiritLinkAbility(id, card, combatOnly);
    }

    //bushido
    vector<string> splitBushido = parseBetween(s, "bushido(", ")");
    if (splitBushido.size())
    {
        int power, toughness;
        if (!parsePowerToughness(splitBushido[1], &power, &toughness))
        {
            DebugTrace("MTGAbility Parse error in bushido" << s);
            return NULL;
        }
        return NEW ABushidoAbility(id, card, power, toughness);
    }

    //counter
    vector<string> splitCounter = parseBetween(s, "counter(", ")");
    if (splitCounter.size())
    {
        string counterString = splitCounter[1];
        Counter * counter = parseCounter(counterString, target, spell);
        if (!counter)
        {
            DebugTrace("MTGAbility: can't parse counter:" << s);
            return NULL;
        }

        MTGAbility * a =
            NEW AACounter(id, card, target,counterString, counter->name.c_str(), counter->power, counter->toughness, counter->nb,counter->maxNb);
        delete (counter);
        a->oneShot = 1;
        return a;
    }
    
    //removes all counters of the specifified type.
    vector<string> splitRemoveCounter = parseBetween(s, "removeallcounters(", ")");
    if (splitRemoveCounter.size())
    {
        string counterString = splitRemoveCounter[1];
        if(counterString.find("all") != string::npos)
        {
            MTGAbility * a = NEW AARemoveAllCounter(id, card, target, "All", 0, 0, 1, true);
            a->oneShot = 1;
            return a;
        }

        Counter * counter = parseCounter(counterString, target, spell);
        if (!counter)
        {
            DebugTrace("MTGAbility: can't parse counter:" << s);
            return NULL;
        }

        MTGAbility * a =
            NEW AARemoveAllCounter(id, card, target, counter->name.c_str(), counter->power, counter->toughness, counter->nb,false);
        delete (counter);
        a->oneShot = 1;
        return a;
    }
    
    //Becomes... (animate artifact...: becomes(Creature, manacost/manacost)
    vector<string> splitBecomes = parseBetween(s, "becomes(", ")");
    if (splitBecomes.size())
    {
        vector<string> becomesParameters = split(splitBecomes[1], ',');
        string stypes = becomesParameters[0];
        vector<string> pt = split(becomesParameters[1], '/');
        string newPower = pt[0];
        string newToughness = pt[1];
        string sabilities = (becomesParameters.size() > 2) ? becomesParameters[2] : "";

        if (oneShot || forceUEOT)
            return NEW ATransformerInstant(id, card, target, stypes, sabilities,newPower,true,newToughness,true,vector<string>(),false,forceFOREVER);

        if(forceFOREVER)
            return NEW ATransformerInstant(id, card, target, stypes, sabilities,newPower,true,newToughness,true,vector<string>(),false,forceFOREVER);

        return  NEW ATransformer(id, card, target, stypes, sabilities,newPower,true,newToughness,true,vector<string>(),false,forceFOREVER);
    }

    //bloodthirst
    vector<string> splitBloodthirst = parseBetween(s, "bloodthirst:", " ", false);
    if (splitBloodthirst.size())
    {
        return NEW ABloodThirst(id, card, target, atoi(splitBloodthirst[1].c_str()));
    }

    //Vanishing
    vector<string> splitVanishing = parseBetween(s, "vanishing:", " ", false);
    if (splitVanishing.size())
    {
        return NEW AVanishing(id, card, NULL, restrictions, atoi(splitVanishing[1].c_str()), "time");
    }

    //Fading
    vector<string> splitFading = parseBetween(s, "fading:", " ", false);
    if (splitFading.size())
    {
        return NEW AVanishing(id, card, NULL, restrictions, atoi(splitFading[1].c_str()), "fade");
    }

    //Alter cost
    if (s.find("altercost(") != string::npos)
        return getManaReduxAbility(s.substr(s.find("altercost(") + 10), id, spell, card, target);

    //transform....(hivestone,living enchantment)
    //TODO: cleanup this block, it's a rats nest
    found = s.find("transforms((");
    if (found != string::npos)
    {
    string extraTransforms = "";
        size_t stypesStartIndex = found + 12;
        string transformsParamsString = "";
        transformsParamsString.append(storedString);//the string between found and real end is removed at start.
        
        found = transformsParamsString.find("transforms((");
        if (found != string::npos && extraTransforms.empty())
        {
            size_t real_end = transformsParamsString.find("))", found);
            size_t end = transformsParamsString.find(",", found);
            if (end == string::npos)
                end = real_end;
            size_t stypesStartIndex = found + 12;
            extraTransforms.append(transformsParamsString.substr(stypesStartIndex, real_end - stypesStartIndex).c_str());
            transformsParamsString.erase(stypesStartIndex, real_end - stypesStartIndex);
        }
        vector<string> effectParameters = split( transformsParamsString, ',');
        string stypes = effectParameters[0];

        string sabilities = transformsParamsString.substr(stypes.length());
        bool newpowerfound = false;
        string newpower = "";
        bool newtoughnessfound = false;
        string newtoughness = "";
        vector <string> abilities = split(sabilities, ',');
        bool newAbilityFound = false;
        vector<string> newAbilitiesList;
        storedString.erase();
        storedString.append(extraTransforms);
        extraTransforms.erase();
        for (unsigned int i = 0 ; i < abilities.size() ; i++)
        {
            if(abilities[i].empty())
                abilities.erase(abilities.begin()+i);
        }            
        for(unsigned int j = 0;j < abilities.size();j++)
        {
            if(abilities[j].find("setpower=") != string::npos)
            {
                newpowerfound = true;
                int powerstart = abilities[j].find("setpower=");
                newpower = abilities[j].substr(powerstart + 9).c_str();
            }
            if(abilities[j].find("settoughness=") != string::npos)
            {
                newtoughnessfound = true;
                int toughnessstart = abilities[j].find("settoughness=");
                newtoughness = abilities[j].substr(toughnessstart + 13).c_str();
            }
            if(abilities[j].find("newability[") != string::npos)
            {
                newAbilityFound = true;
                size_t NewSkill = abilities[j].find("newability[");
                size_t NewSkillEnd = abilities[j].find_last_of("]");
                string newAbilities = abilities[j].substr(NewSkill + 11,NewSkillEnd - NewSkill - 11);
                newAbilitiesList.push_back(newAbilities);
            }
        }
        MTGAbility * a;
        bool foreverEffect = false;
        if (forceFOREVER)
        {
        foreverEffect = true;
        }
        if (oneShot || forceUEOT)
        {
            a = NEW ATransformerInstant(id, card, target, stypes, sabilities,newpower,newpowerfound,newtoughness,newtoughnessfound,newAbilitiesList,newAbilityFound,foreverEffect);
        }
        else if(foreverEffect)
        {
        a = NEW ATransformerInstant(id, card, target, stypes, sabilities,newpower,newpowerfound,newtoughness,newtoughnessfound,newAbilitiesList,newAbilityFound,foreverEffect);
        }
        else
        {
            a = NEW ATransformer(id, card, target, stypes, sabilities,newpower,newpowerfound,newtoughness,newtoughnessfound,newAbilitiesList,newAbilityFound);
        }
        return a;
    }
    
    //Change Power/Toughness
    WParsedPT * wppt = NEW WParsedPT(s, spell, card);
    bool nonstatic = false;
    if (wppt->ok)
    {
        if(s.find("nonstatic") != string::npos)
            nonstatic = true;
        if (!activated)
        {
            if (card->hasType(Subtypes::TYPE_INSTANT) || card->hasType(Subtypes::TYPE_SORCERY) || forceUEOT)
            {
                return NEW PTInstant(id, card, target, wppt,s,nonstatic);
            }
            return NEW APowerToughnessModifier(id, card, target, wppt,s,nonstatic);
        }
        return NEW PTInstant(id, card, target, wppt,s,nonstatic);
    }
    else
    {
        delete wppt;
    }

    //Mana Producer
    found = s.find("add");
    if (found != string::npos)
    {
        ManaCost * output = ManaCost::parseManaCost(s.substr(found));
        Targetable * t = spell ? spell->getNextTarget() : NULL;
        MTGAbility * a = NEW AManaProducer(id, card, t, output, NULL, who);
        a->oneShot = 1;
        if(newName.size())
            ((AManaProducer*)a)->menutext = newName;
        return a;
    }

    //Protection from...
    vector<string> splitProtection = parseBetween(s, "protection from(", ")");
    if (splitProtection.size())
    {
        TargetChooserFactory tcf;
        TargetChooser * fromTc = tcf.createTargetChooser(splitProtection[1], card);
        if (!fromTc)
            return NULL;
        fromTc->setAllZones();
        if (!activated)
        {
            if (card->hasType(Subtypes::TYPE_INSTANT) || card->hasType(Subtypes::TYPE_SORCERY) || forceUEOT)
            {
                return NULL; //TODO
            }
            return NEW AProtectionFrom(id, card, target, fromTc, splitProtection[1]);
        }
        return NULL; //TODO
    }

    //targetter can not target...
    vector<string> splitCantTarget = parseBetween(s, "cantbetargetof(", ")");
    if (splitCantTarget.size())
    {
        TargetChooserFactory tcf;
        TargetChooser * fromTc = tcf.createTargetChooser(splitCantTarget[1], card);
        if (!fromTc)
            return NULL;
        fromTc->setAllZones();
        if (!activated)
        {
            if (card->hasType(Subtypes::TYPE_INSTANT) || card->hasType(Subtypes::TYPE_SORCERY) || forceUEOT)
            {
                return NULL; //TODO
            }
            return NEW ACantBeTargetFrom(id, card, target, fromTc);
        }
        return NULL; //TODO
    }
    
    //Can't be blocked by...
    vector<string> splitCantBlock = parseBetween(s, "cantbeblockedby(", ")");
    if (splitCantBlock.size())
    {
        TargetChooserFactory tcf;
        TargetChooser * fromTc = tcf.createTargetChooser(splitCantBlock[1], card);
        if (!fromTc)
            return NULL;
        //default target zone to opponentbattlefield here?
        if (!activated)
        {
            if (card->hasType(Subtypes::TYPE_INSTANT) || card->hasType(Subtypes::TYPE_SORCERY) || forceUEOT)
            {
                return NULL; //TODO
            }
            return NEW ACantBeBlockedBy(id, card, target, fromTc);
        }
        return NULL; //TODO
    }

    //frozen, next untap this does not untap.
    found = s.find("frozen");
    if (found != string::npos)
    {
        MTGAbility * a = NEW AAFrozen(id, card, target);
        a->oneShot = 1;
        return a;
    }

    //get a new target
    if ((s.find("retarget") != string::npos) || s.find("newtarget") != string::npos)
    {
        MTGAbility * a = NEW AANewTarget(id, card,target, (s.find("retarget") != string::npos));
        a->oneShot = 1;
        return a;
    }

    //morph
    found = s.find("morph");
    if (found != string::npos)
    {
        MTGAbility * a = NEW AAMorph(id, card, target);
        a->oneShot = 1;
        return a;
    }
    
    //identify what a leveler creature will max out at.
    vector<string> splitMaxlevel = parseBetween(s, "maxlevel:", " ", false);
    if (splitMaxlevel.size())
    {
        MTGAbility * a = NEW AAWhatsMax(id, card, card, NULL, atoi(splitMaxlevel[1].c_str()));
        a->oneShot = 1;
        return a;
    }

    //switch targest power with toughness
    found = s.find("swap");
    if (found != string::npos)
    {
        MTGAbility * a = NEW ASwapPTUEOT(id, card, target);
        a->oneShot = 1;
        return a;
    }
    
    //Regeneration
    found = s.find("regenerate");
    if (found != string::npos)
    {
        MTGAbility * a = NEW AStandardRegenerate(id, card, target);
        a->oneShot = 1;
        return a;
    }
    
    //Gain/loose simple Ability
    for (int j = 0; j < Constants::NB_BASIC_ABILITIES; j++)
    {
        found = s.find(Constants::MTGBasicAbilities[j]);
        if (found == 0 || found == 1)
        {
            int modifier = 0;
            if(s.find("absorb") || s.find("flanking"))
                modifier += 1;
            else
                modifier = 1;

            if (found > 0 && s[found - 1] == '-')
                modifier = 0;
            else if(found > 0  && s[found - 1] == '-' && (s.find("absorb") || s.find("flanking")))
                modifier -= 1;

            if (!activated)
            {
                if (card->hasType(Subtypes::TYPE_INSTANT) || card->hasType(Subtypes::TYPE_SORCERY) || forceUEOT)
                {
                    return NEW AInstantBasicAbilityModifierUntilEOT(id, card, target, j, modifier);
                }
                return NEW ABasicAbilityModifier(id, card, target, j, modifier);
            }
            return NEW ABasicAbilityAuraModifierUntilEOT(id, card, target, NULL, j, modifier);
        }
    }

    //Untapper (Ley Druid...)
    found = s.find("untap");
    if (found != string::npos)
    {
        MTGAbility * a = NEW AAUntapper(id, card, target);
        a->oneShot = 1;
        return a;
    }

    //Tapper (icy manipulator)
    found = s.find("tap");
    if (found != string::npos)
    {
        MTGAbility * a = NEW AATapper(id, card, target);
        a->oneShot = 1;
        return a;
    }

    DebugTrace(" no matching ability found. " << s);
    return NULL;
}

//Tells the AI if the ability should target itself or an ennemy
int AbilityFactory::abilityEfficiency(MTGAbility * a, Player * p, int mode, TargetChooser * tc,Targetable * target)
{
    if (!a)
        return BAKA_EFFECT_DONTKNOW;

    if (GenericTargetAbility * abi = dynamic_cast<GenericTargetAbility*>(a))
    {
        if (mode == MODE_PUTINTOPLAY)
            return BAKA_EFFECT_GOOD;
        return abilityEfficiency(abi->ability, p, mode, abi->tc);
    }
    if (GenericActivatedAbility * abi = dynamic_cast<GenericActivatedAbility*>(a))
    {
        if (mode == MODE_PUTINTOPLAY)
            return BAKA_EFFECT_GOOD;
        return abilityEfficiency(abi->ability, p, mode, tc);
    }
    if (MultiAbility * abi = dynamic_cast<MultiAbility*>(a))
        return abilityEfficiency(abi->abilities[0], p, mode, tc);
    if (MayAbility * abi = dynamic_cast<MayAbility*>(a))
        return abilityEfficiency(abi->ability, p, mode, tc);
    if (ALord * abi = dynamic_cast<ALord *>(a))
    {
        int myCards = countCards(abi->tc, p);
        int theirCards = countCards(abi->tc, p->opponent());
        int efficiency = abilityEfficiency(abi->ability, p, mode, tc);
        if ( ((myCards < theirCards) && efficiency == BAKA_EFFECT_GOOD) || ((myCards > theirCards) && efficiency == BAKA_EFFECT_BAD)   )
            return efficiency;
        return -efficiency;
    }
    if (AAsLongAs * abi = dynamic_cast<AAsLongAs *>(a))
        return abilityEfficiency(abi->ability, p, mode, tc);
    if (AForeach * abi = dynamic_cast<AForeach *>(a))
        return abilityEfficiency(abi->ability, p, mode, tc);
    if (ATeach * abi = dynamic_cast<ATeach *>(a))
        return abilityEfficiency(abi->ability, p, mode, tc);
    if (dynamic_cast<AAFizzler *> (a))
        return BAKA_EFFECT_BAD;
    if (dynamic_cast<AADamagePrevent *> (a))
        return BAKA_EFFECT_GOOD;
    if (dynamic_cast<AACloner *> (a))
        return BAKA_EFFECT_GOOD;
    if (dynamic_cast<ASwapPTUEOT *> (a))
        return BAKA_EFFECT_BAD;
    if (dynamic_cast<AAUntapper *> (a))
        return BAKA_EFFECT_GOOD;
    if (dynamic_cast<AATapper *> (a))
        return BAKA_EFFECT_BAD;
    if (AACounter * ac = dynamic_cast<AACounter *>(a))
    {
        bool negative_effect = ac->power < 0 || ac->toughness < 0;
        if ((ac->nb > 0 && negative_effect) || (ac->nb < 0 && !negative_effect))
            return BAKA_EFFECT_BAD;
        return BAKA_EFFECT_GOOD;
    }
    if (dynamic_cast<ATokenCreator *> (a))
        return BAKA_EFFECT_GOOD;

    if (AAMover * aam = dynamic_cast<AAMover *>(a))
    {
        MTGGameZone * z = aam->destinationZone(target);
        if (tc && tc->targetsZone(p->game->library))
        {
            if (z == p->game->hand || z == p->game->inPlay)
                return BAKA_EFFECT_GOOD;
        }
        return BAKA_EFFECT_BAD; //TODO
    }

    if (dynamic_cast<AACopier *> (a))
        return BAKA_EFFECT_GOOD;
    if (dynamic_cast<AABuryCard *> (a))
        return BAKA_EFFECT_BAD;
    if (dynamic_cast<AADestroyCard *> (a))
        return BAKA_EFFECT_BAD;
    if (dynamic_cast<AStandardRegenerate *> (a))
        return BAKA_EFFECT_GOOD;
    if (AALifer * abi = dynamic_cast<AALifer *>(a))
        return abi->getLife() > 0 ? BAKA_EFFECT_GOOD : BAKA_EFFECT_BAD;
    if (AAAlterPoison * abi = dynamic_cast<AAAlterPoison *>(a))
        return abi->poison > 0 ? BAKA_EFFECT_GOOD : BAKA_EFFECT_BAD;
    if (dynamic_cast<AADepleter *> (a))
        return BAKA_EFFECT_BAD;
    if (dynamic_cast<AADrawer *> (a))
        return BAKA_EFFECT_GOOD;
    if (dynamic_cast<AARandomDiscarder *> (a))
        return BAKA_EFFECT_BAD;
    if (dynamic_cast<ARampageAbility *> (a))
        return BAKA_EFFECT_GOOD;
    if (dynamic_cast<ABushidoAbility *> (a))
        return BAKA_EFFECT_GOOD;
    if (PTInstant * abi = dynamic_cast<PTInstant *>(a))
        return (abi->wppt->power.getValue() >= 0 && abi->wppt->toughness.getValue() >= 0) ? BAKA_EFFECT_GOOD : BAKA_EFFECT_BAD;
    if (APowerToughnessModifier * abi = dynamic_cast<APowerToughnessModifier *>(a))
        return (abi->wppt->power.getValue() >= 0 && abi->wppt->toughness.getValue() >= 0) ? BAKA_EFFECT_GOOD : BAKA_EFFECT_BAD;
    if (PTInstant * abi = dynamic_cast<PTInstant *>(a))
        return abilityEfficiency(abi->ability, p, mode, tc);

    if (dynamic_cast<ACantBeBlockedBy *> (a))
        return BAKA_EFFECT_GOOD;
    if (dynamic_cast<AProtectionFrom *> (a))
        return BAKA_EFFECT_GOOD;

    map<int, bool> badAbilities;
    badAbilities[(int)Constants::CANTATTACK] = true;
    badAbilities[(int)Constants::CANTBLOCK] = true;
    badAbilities[(int)Constants::CLOUD] = true;
    badAbilities[(int)Constants::DEFENDER] = true;
    badAbilities[(int)Constants::DOESNOTUNTAP] = true;
    badAbilities[(int)Constants::MUSTATTACK] = true;
    badAbilities[(int)Constants::CANTREGEN] = true;

    if (AInstantBasicAbilityModifierUntilEOT * abi = dynamic_cast<AInstantBasicAbilityModifierUntilEOT *>(a))
    {
        int result = badAbilities[abi->ability] ? BAKA_EFFECT_BAD : BAKA_EFFECT_GOOD;
        return (abi->value > 0) ? result : -result;
    }
    if (ABasicAbilityModifier * abi = dynamic_cast<ABasicAbilityModifier *>(a))
    {
        int result = (badAbilities[abi->ability]) ? BAKA_EFFECT_BAD : BAKA_EFFECT_GOOD;
        return (abi->modifier > 0) ? result : -result;
    }
    if (ABasicAbilityAuraModifierUntilEOT * abi = dynamic_cast<ABasicAbilityAuraModifierUntilEOT *>(a))
        return abilityEfficiency(abi->ability, p, mode);
    if (dynamic_cast<AManaProducer*> (a))
        return BAKA_EFFECT_GOOD;
    return BAKA_EFFECT_DONTKNOW;
}

//Returns the "X" cost that was paid for a spell
int AbilityFactory::computeX(Spell * spell, MTGCardInstance * card)
{
    if (spell)
        return spell->computeX(card);
    if(card) return card->X;
    return 0;
}

int AbilityFactory::getAbilities(vector<MTGAbility *> * v, Spell * spell, MTGCardInstance * card, int id, MTGGameZone * dest)
{
    if (!card && spell)
        card = spell->source;
    if (!card)
        return 0;
    MTGCardInstance * target = card->target;
    if (!target)
        target = card;
    //card->getManaCost()->copy(card->model->data->getManaCost());
    //zeth:i added this originally for no reason really, however
    //i didn't realize ai runs this function about a million times during a match.
    //it litterally distroys any altering cost effects.
    //if for some reason i added this becuase of some bug...we need to think of a more clever way
    //to do this.
    string magicText;
    if (dest)
    {
        card->graveEffects = false;
        card->exileEffects = false;
        GameObserver * g = GameObserver::GetInstance();
        for (int i = 0; i < 2; ++i)
        {
            MTGPlayerCards * zones = g->players[i]->game;
            if (dest == zones->hand)
            {
                magicText = card->magicTexts["hand"];
                break;
            }
            if (dest == zones->graveyard)
            {
                magicText = card->magicTexts["graveyard"];
                card->graveEffects = true;
                break;
            }
            if (dest == zones->stack)
            {
                magicText = card->magicTexts["stack"];
                break;
            }
            if (dest == zones->exile)
            {
                magicText = card->magicTexts["exile"];
                 card->exileEffects = true;
                break;
            }
            if (dest == zones->library)
            {
                magicText = card->magicTexts["library"];
                break;
            }
            //Other zones needed ?
            return 0;
        }
    }
    else
    {
        if(card->previous && card->previous->morphed && !card->turningOver)
        {
            magicText = card->magicTexts["facedown"];
            card->power = 2;
            card->life = 2;
            card->toughness = 2;
            card->setColor(0,1);
            card->name = "";
            card->types.clear();
            string cre = "Creature";
            card->setType(cre.c_str());
            card->basicAbilities.reset();
            card->getManaCost()->reinit();
        }
        else if(card && !card->morphed && card->turningOver)
        {
            card->power += card->origpower-2;
            card->life += card->origtoughness-2;
            card->toughness += card->origtoughness-2;
            card->setColor(0,1);
            card->name = card->model->data->name;
            card->types = card->model->data->types;
            card->colors = card->model->data->colors;

            card->basicAbilities |= card->model->data->basicAbilities;

            ManaCost * copyCost = card->model->data->getManaCost();
            card->getManaCost()->copy(copyCost);
            magicText = card->model->data->magicText;
            string faceupC= card->magicTexts["faceup"];
            magicText.append("\n");
            magicText.append(faceupC);

        }
        else if(card && card->hasType(Subtypes::TYPE_EQUIPMENT) && card->target)
        {
            magicText = card->model->data->magicText;
            string equipText = card->magicTexts["skill"];
            magicText.append("\n");
            magicText.append(equipText);
        }
        else
        {
            magicText = card->magicText;
        }
    }
    if (card->alias && magicText.size() == 0 && !dest)
    {
        MTGCard * c = MTGCollection()->getCardById(card->alias);
        if (!c)
            return 0;
        magicText = c->data->magicText;
    }
    string line;
    int size = magicText.size();
    if (size == 0)
        return 0;
    size_t found;
    int result = id;

    while (magicText.size())
    {
        found = magicText.find("\n");
        if (found != string::npos)
        {
            line = magicText.substr(0, found);
            magicText = magicText.substr(found + 1);
        }
        else
        {
            line = magicText;
            magicText = "";
        }
        MTGAbility * a = parseMagicLine(line, result, spell, card, 0, 0, 0, 0, dest);
        if (a)
        {
            v->push_back(a);
            result++;
        }
        else
        {
            DebugTrace("ABILITYFACTORY ERROR: Parser returned NULL");
        }
    }
    return result;
}

//Some basic functionalities that can be added automatically in the text file
/*
 * Several objects are computed from the text string, and have a direct influence on what action we should take
 * (direct impact on the game such as draw a card immediately, or create a New GameObserver and add it to the Abilities,etc..)
 * These objects are:
 *   - trigger (if there is an "@" in the string, this is a triggered ability)
 *   - target (if there ie a "target(" in the string, then this is a TargetAbility)
 *   - doTap (a dirty way to know if tapping is included in the cost...
 */
int AbilityFactory::magicText(int id, Spell * spell, MTGCardInstance * card, int mode, TargetChooser * tc, MTGGameZone * dest)
{
    int dryMode = 0;
    if (!spell && !dest)
        dryMode = 1;

    vector<MTGAbility *> v;
    int result = getAbilities(&v, spell, card, id, dest);

    for (size_t i = 0; i < v.size(); ++i)
    {
        MTGAbility * a = v[i];
        if (dryMode)
        {
            result = abilityEfficiency(a, card->controller(), mode, tc);
            for (size_t i = 0; i < v.size(); ++i)
                SAFE_DELETE(v[i]);
            return result;
        }

        if (a)
        {
            if (a->oneShot)
            {
                a->resolve();
                delete (a);
            }
            else
            {
                a->addToGame();
            }
        }
        else
        {
            DebugTrace("ABILITYFACTORY ERROR: Parser returned NULL");
        }
    }

    return result;
}

void AbilityFactory::addAbilities(int _id, Spell * spell)
{
    MTGCardInstance * card = spell->source;

    if (spell->getNbTargets() == 1)
    {
        card->target = spell->getNextCardTarget();
        if (card->target && (!spell->tc->canTarget(card->target) || card->target->isPhased))
        {
            MTGPlayerCards * zones = card->controller()->game;
            zones->putInZone(card, spell->from, card->owner->game->graveyard);
            return; //fizzle
        }
    } 
    _id = magicText(_id, spell);

    GameObserver * game = GameObserver::GetInstance();
    MTGPlayerCards * zones = card->controller()->game;

    int id = card->alias;
    switch (id)
    {
    case 1092: //Aladdin's lamp
    {
        AAladdinsLamp * ability = NEW AAladdinsLamp(_id, card);
        game->addObserver(ability);
        break;
    }
    case 1095: //Armageddon clock
    {
        AArmageddonClock * ability = NEW AArmageddonClock(_id, card);
        game->addObserver(ability);
        break;
    }

    case 1191: //Blue Elemental Blast
    {
        if (card->target)
        {
            card->target->controller()->game->putInGraveyard(card->target);
        }
        else
        {
            Spell * starget = spell->getNextSpellTarget();
            game->mLayers->stackLayer()->Fizzle(starget);
        }
        break;
    }
    case 1282: //Chaoslace
    {
        if (card->target)
        {
            card->target->setColor(Constants::MTG_COLOR_RED, 1);
        }
        else
        {
            Spell * starget = spell->getNextSpellTarget();
            starget->source->setColor(Constants::MTG_COLOR_RED, 1);
        }
        break;
    }
    case 1335: //Circle of protection : black
    {
        game->addObserver(NEW ACircleOfProtection(_id, card, Constants::MTG_COLOR_BLACK));
        break;
    }
    case 1336: //Circle of protection : blue
    {
        game->addObserver(NEW ACircleOfProtection(_id, card, Constants::MTG_COLOR_BLUE));
        break;
    }
    case 1337: //Circle of protection : green
    {
        game->addObserver(NEW ACircleOfProtection(_id, card, Constants::MTG_COLOR_GREEN));
        break;
    }
    case 1338: //Circle of protection : red
    {
        game->addObserver(NEW ACircleOfProtection(_id, card, Constants::MTG_COLOR_RED));
        break;
    }
    case 1339: //Circle of protection : white
    {
        game->addObserver(NEW ACircleOfProtection(_id, card, Constants::MTG_COLOR_WHITE));
        break;
    }
    case 1102: //Conservator
    {
        game->addObserver(NEW AConservator(_id, card));
        break;
    }

    case 1103: //Crystal Rod
    {
        int cost[] = { Constants::MTG_COLOR_ARTIFACT, 1 };
        ASpellCastLife* ability = NEW ASpellCastLife(_id, card, Constants::MTG_COLOR_BLUE, NEW ManaCost(cost, 1), 1);
        game->addObserver(ability);
        break;
    }
    case 1152: //Deathlace
    {
        if (card->target)
        {
            card->target->setColor(Constants::MTG_COLOR_BLACK, 1);
        }
        else
        {
            Spell * starget = spell->getNextSpellTarget();
            starget->source->setColor(Constants::MTG_COLOR_BLACK, 1);
        }
        break;
    }

    case 1345: //Farmstead
    {
        game->addObserver(NEW AFarmstead(_id, card, card->target));
        break;
    }
    case 1291: //Fireball
    {
        int x = computeX(spell, card);
        game->addObserver(NEW AFireball(_id, card, spell, x));
        break;
    }
    case 1113: //Iron Star
    {
        int cost[] = { Constants::MTG_COLOR_ARTIFACT, 1 };
        ASpellCastLife* ability = NEW ASpellCastLife(_id, card, Constants::MTG_COLOR_RED, NEW ManaCost(cost, 1), 1);
        game->addObserver(ability);
        break;
    }
    case 1351: // Island Sanctuary
    {
        game->addObserver(NEW AIslandSanctuary(_id, card));
        break;
    }
    case 1114: //Ivory cup
    {
        int cost[] = { Constants::MTG_COLOR_ARTIFACT, 1 };
        ASpellCastLife* ability = NEW ASpellCastLife(_id, card, Constants::MTG_COLOR_WHITE, NEW ManaCost(cost, 1), 1);
        game->addObserver(ability);
        break;
    }
    case 1117: //Jandors Ring
    {
        game->addObserver(NEW AJandorsRing(_id, card));
        break;
    }
    case 1254: //Kudzu
    {
        game->addObserver(NEW AKudzu(id, card, card->target));
        break;
    }
    case 1257: //Lifelace
    {
        if (card->target)
        {
            card->target->setColor(Constants::MTG_COLOR_GREEN, 1);
        }
        else
        {
            Spell * starget = spell->getNextSpellTarget();
            starget->source->setColor(Constants::MTG_COLOR_GREEN, 1);
        }
        break;
    }
    case 1124: //Mana Vault (the rest is softcoded!)
    {
        game->addObserver(NEW ARegularLifeModifierAura(_id + 2, card, card, Constants::MTG_PHASE_DRAW, -1, 1));
        break;
    }
    case 1215: //Power Leak
    {
        game->addObserver(NEW APowerLeak(_id, card, card->target));
        break;
    }
    case 1358: //Purelace
    {
        if (card->target)
        {
            card->target->setColor(Constants::MTG_COLOR_WHITE, 1);
        }
        else
        {
            Spell * starget = spell->getNextSpellTarget();
            starget->source->setColor(Constants::MTG_COLOR_WHITE, 1);
        }
        break;
    }
    case 1312: //Red Elemental Blast
    {
        if (card->target)
        {
            card->target->controller()->game->putInGraveyard(card->target);
        }
        else
        {
            Spell * starget = spell->getNextSpellTarget();
            game->mLayers->stackLayer()->Fizzle(starget);
        }
        break;
    }

    case 1139: //The Rack
    {
        game->addObserver(NEW ALifeZoneLink(_id, card, Constants::MTG_PHASE_UPKEEP, -3));
        break;
    }

    case 1140: //Throne of Bone
    {
        int cost[] = { Constants::MTG_COLOR_ARTIFACT, 1 };
        ASpellCastLife* ability = NEW ASpellCastLife(_id, card, Constants::MTG_COLOR_BLACK, NEW ManaCost(cost, 1), 1);
        game->addObserver(ability);
        break;
    }

    case 1142: //Wooden Sphere
    {
        int cost[] = { Constants::MTG_COLOR_ARTIFACT, 1 };
        ASpellCastLife* ability = NEW ASpellCastLife(_id, card, Constants::MTG_COLOR_GREEN, NEW ManaCost(cost, 1), 1);
        game->addObserver(ability);
        break;
    }
    case 1143: //Animate Dead
    {
        AAnimateDead * a = NEW AAnimateDead(_id, card, card->target);
        game->addObserver(a);
        card->target = ((MTGCardInstance *) a->target);
        break;
    }
    case 1156: //Drain Life
    {
        Damageable * target = spell->getNextDamageableTarget();
        int x = spell->cost->getConvertedCost() - 2; //TODO Fix that !!! + X should be only black mana, that needs to be checked !
        game->mLayers->stackLayer()->addDamage(card, target, x);
        if (target->life < x)
            x = target->life;
        game->currentlyActing()->gainLife(x);
        break;
    }
    case 1159: //Erg Raiders
    {
        AErgRaiders* ability = NEW AErgRaiders(_id, card);
        game->addObserver(ability);
        break;
    }
    case 1202: //Hurkyl's Recall
    {
        Player * player = spell->getNextPlayerTarget();
        if (player)
        {
            for (int i = 0; i < 2; i++)
            {
                MTGInPlay * inplay = game->players[i]->game->inPlay;
                for (int j = inplay->nb_cards - 1; j >= 0; j--)
                {
                    MTGCardInstance * card = inplay->cards[j];
                    if (card->owner == player && card->hasType(Subtypes::TYPE_ARTIFACT))
                    {
                        player->game->putInZone(card, inplay, player->game->hand);
                    }
                }
            }
        }
        break;
    }
    case 1209: //Mana Short
    {
        Player * player = spell->getNextPlayerTarget();
        if (player)
        {
            MTGInPlay * inplay = player->game->inPlay;
            for (int i = 0; i < inplay->nb_cards; i++)
            {
                MTGCardInstance * current = inplay->cards[i];
                if (current->hasType(Subtypes::TYPE_LAND))
                    current->tap();
            }
            player->getManaPool()->init();
        }
        break;
    }
    case 1167: //Mind Twist
    {
        int xCost = computeX(spell, card);
        for (int i = 0; i < xCost; i++)
        {
            game->opponent()->game->discardRandom(game->opponent()->game->hand, card);
        }
        break;
    }
    case 1176: //Sacrifice
    {
        ASacrifice * ability = NEW ASacrifice(_id, card, card->target);
        game->addObserver(ability);
        break;
    }
    case 1224: //Spell Blast
    {
        int x = computeX(spell, card);
        Spell * starget = spell->getNextSpellTarget();
        if (starget)
        {
            if (starget->cost->getConvertedCost() <= x)
                game->mLayers->stackLayer()->Fizzle(starget);
        }
        break;
    }
    case 1194: //Control Magic
    {
        game->addObserver(NEW AControlStealAura(_id, card, card->target));
        break;
    }
    case 1231: //Volcanic Eruption
    {
        int x = computeX(spell, card);
        int _x = x;
        MTGCardInstance * target = spell->getNextCardTarget();
        while (target && _x)
        {
            target->destroy();
            _x--;
            target = spell->getNextCardTarget(target);
        }
        x -= _x;
        for (int i = 0; i < 2; i++)
        {
            game->mLayers->stackLayer()->addDamage(card, game->players[i], x);
            for (int j = 0; j < game->players[i]->game->inPlay->nb_cards; j++)
            {
                MTGCardInstance * current = game->players[i]->game->inPlay->cards[j];
                if (current->isCreature())
                {
                    game->mLayers->stackLayer()->addDamage(card, current, x);
                }
            }
        }
        break;
    }
    case 1288: //EarthBind
    {
        game->addObserver(NEW AEarthbind(_id, card, card->target));
        break;
    }
    case 1344: //Eye for an Eye
    {
        Damage * damage = spell->getNextDamageTarget();
        if (damage)
        {
            game->mLayers->stackLayer()->addDamage(card, damage->source->controller(), damage->damage);
        }
        break;
    }
    case 1243: //Fastbond
    {
        game->addObserver(NEW AFastbond(_id, card));
        break;
    }
    case 1225: //Stasis
    {
        game->addObserver(NEW AStasis(_id, card));
        break;
    }
    case 1227: //Toughtlace
    {
        if (card->target)
        {
            card->target->setColor(Constants::MTG_COLOR_BLUE, 1);
        }
        else
        {
            Spell * starget = spell->getNextSpellTarget();
            starget->source->setColor(Constants::MTG_COLOR_BLUE, 1);
        }
        break;
    }
    case 1480: //Energy Tap
    {
        card->target->tap();
        int mana = card->target->getManaCost()->getConvertedCost();
        game->currentlyActing()->getManaPool()->add(Constants::MTG_COLOR_ARTIFACT, mana);
    }

        //Addons ICE-AGE Cards

    case 2474: //Minion of Leshrac
    {
        game->addObserver(NEW AMinionofLeshrac(_id, card));
        break;
    }

    case 2732: //Kjeldoran Frostbeast
    {
        game->addObserver(NEW AKjeldoranFrostbeast(_id, card));
        break;
    }

        // --- addon Mirage ---

    case 3410: //Seed of Innocence
    {
        GameObserver * game = GameObserver::GetInstance();
        for (int i = 0; i < 2; i++)
        {
            for (int j = 0; j < game->players[i]->game->inPlay->nb_cards; j++)
            {
                MTGCardInstance * current = game->players[i]->game->inPlay->cards[j];
                if (current->hasType("Artifact"))
                {
                    game->players[i]->game->putInGraveyard(current);
                    current->controller()->gainLife(current->getManaCost()->getConvertedCost());
                }
            }
        }
        break;
    }

        //-- addon 10E---

    case 129767: //Threaten
    {
        game->addObserver(NEW AInstantControlSteal(_id, card, card->target));
        break;
    }
  
    case 129774: // Traumatize
    {
        int nbcards;
        Player * player = spell->getNextPlayerTarget();
        MTGLibrary * library = player->game->library;
        nbcards = (library->nb_cards) / 2;
        for (int i = 0; i < nbcards; i++)
        {
            if (library->nb_cards)
                player->game->putInZone(library->cards[library->nb_cards - 1], library, player->game->graveyard);
        }
        break;
    }

    case 135215: //Sylvan Basilisk
    {
        game->addObserver(NEW ABasilik(_id, card));
        break;
    }
    case 130553:// Beacon of Immortality
    {
        Player * player = spell->getNextPlayerTarget();
        if (player->life < (INT_MAX / 4))
            player->life += player->life;
        zones->putInZone(card, spell->from, zones->library);
        zones->library->shuffle();
        break;
    }
    case 135262:// Beacon of Destruction & unrest
    {
        zones->putInZone(card, spell->from, zones->library);
        zones->library->shuffle();
        break;
    }
    case 129750: //Sudden Impact
    {
        Damageable * target = spell->getNextDamageableTarget();
        Player * p = spell->getNextPlayerTarget();
        MTGHand * hand = p->game->hand;
        int damage = hand->nb_cards;
        game->mLayers->stackLayer()->addDamage(card, target, damage);
        break;
    }
    case 130369: // Soulblast
    {
        int damage = 0;
        Damageable * target = spell->getNextDamageableTarget();
        for (int j = card->controller()->game->inPlay->nb_cards - 1; j >= 0; --j)
        {
            MTGCardInstance * current = card->controller()->game->inPlay->cards[j];
            if (current->hasType(Subtypes::TYPE_CREATURE))
            {
                card->controller()->game->putInGraveyard(current);
                damage += current->power;
            }
        }
        game->mLayers->stackLayer()->addDamage(card, target, damage);
        break;
    }

    case 129698: // Reminisce
    {
        int nbcards;
        Player * player = spell->getNextPlayerTarget();
        MTGLibrary * library = player->game->library;
        MTGGraveyard * graveyard = player->game->graveyard;
        nbcards = (graveyard->nb_cards);
        for (int i = 0; i < nbcards; i++)
        {
            if (graveyard->nb_cards)
                player->game->putInZone(graveyard->cards[graveyard->nb_cards - 1], graveyard, library);
        }
        library->shuffle();
        break;
    }

        // --- addon Ravnica---

    case 89114: //Psychic Drain
    {
        Player * player = spell->getNextPlayerTarget();
        MTGLibrary * library = player->game->library;
        int x = computeX(spell, card);
        for (int i = 0; i < x; i++)
        {
            if (library->nb_cards)
                player->game->putInZone(library->cards[library->nb_cards - 1], library, player->game->graveyard);
        }
        game->currentlyActing()->gainLife(x);
        break;
    }

    default:
        break;
    }

    /* We want to get rid of these basicAbility things.
     * basicAbilities themselves are alright, but creating New object depending on them is dangerous
     * The main reason is that classes that add an ability to a card do NOT create these objects, and therefore do NOT
     * Work.
     * For example, setting EXALTED for a creature is not enough right now...
     * It shouldn't be necessary to add an object. State based abilities could do the trick
     */

    if (card->basicAbilities[(int)Constants::EXALTED])
    {
        game->addObserver(NEW AExalted(_id, card));
    }

    if (card->basicAbilities[(int)Constants::FLANKING])
    {
        game->addObserver(NEW AFlankerAbility(_id, card));
    }

    // Tested works the first r10 did not function because of the mistake in the array of the definition
    if (card->basicAbilities[(int)Constants::FORESTHOME])
    {
        game->addObserver(NEW AStrongLandLinkCreature(_id, card, "forest"));
    }
    if (card->basicAbilities[(int)Constants::ISLANDHOME])
    {
        game->addObserver(NEW AStrongLandLinkCreature(_id, card, "island"));
    }
    if (card->basicAbilities[(int)Constants::MOUNTAINHOME])
    {
        game->addObserver(NEW AStrongLandLinkCreature(_id, card, "mountain"));
    }
    if (card->basicAbilities[(int)Constants::SWAMPHOME])
    {
        game->addObserver(NEW AStrongLandLinkCreature(_id, card, "swamp"));
    }
    if (card->basicAbilities[(int)Constants::PLAINSHOME])
    {
        game->addObserver(NEW AStrongLandLinkCreature(_id, card, "plains"));
    }

    if(card->previous && card->previous->previous && card->previous->previous->suspended)
        card->basicAbilities[(int)Constants::HASTE] = 1;

    if (card->hasType(Subtypes::TYPE_INSTANT) || card->hasType(Subtypes::TYPE_SORCERY))
    {
        MTGPlayerCards * zones = card->controller()->game;
        if (card->alternateCostPaid[ManaCost::MANA_PAID_WITH_BUYBACK] > 0)
        {
            zones->putInZone(card, zones->stack, zones->hand);
        }
        else if (card->alternateCostPaid[ManaCost::MANA_PAID_WITH_FLASHBACK] > 0)
        {
            zones->putInZone(card, zones->stack, zones->exile);
        }
        else
        {
            zones->putInZone(card, zones->stack, zones->graveyard);
        }
    }

}

//mehods used in parseMagicLine()

//ManaRedux -> manaredux(colorless,+2)
//          -> manaredux(green,-2)
MTGAbility * AbilityFactory::getManaReduxAbility(string s, int id, Spell *spell, MTGCardInstance *card, MTGCardInstance *target)
{
    int color = -1;
    string manaCost;
    size_t endIndex = manaCost.find(")");
    if (s.find(Constants::kManaColorless) != string::npos)
    {
        manaCost = s.substr(s.find(",") + 1, endIndex);
        color = Constants::MTG_COLOR_ARTIFACT;
    }
    else if (s.find(Constants::kManaGreen) != string::npos)
    {
        manaCost = s.substr(s.find(",") + 1, endIndex);
        color = Constants::MTG_COLOR_GREEN;
    }
    else if (s.find(Constants::kManaBlue) != string::npos)
    {
        manaCost = s.substr(s.find(",") + 1, endIndex);
        color = Constants::MTG_COLOR_BLUE;
    }
    else if (s.find(Constants::kManaRed) != string::npos)
    {
        manaCost = s.substr(s.find(",") + 1, endIndex);
        color = Constants::MTG_COLOR_RED;
    }
    else if (s.find(Constants::kManaBlack) != string::npos)
    {
        manaCost = s.substr(s.find(",") + 1, endIndex);
        color = Constants::MTG_COLOR_BLACK;
    }
    else if (s.find(Constants::kManaWhite) != string::npos)
    {
        manaCost = s.substr(s.find(",") + 1, endIndex);
        color = Constants::MTG_COLOR_WHITE;
    }
    else
    {
        DebugTrace("An error has happened in creating a Mana Redux Ability! " << s );
        return NULL;
    }
    // figure out the mana cost
    int amount = atoi(manaCost.c_str());
    return NEW AAlterCost(id, card, target, amount, color);
}

MTGAbility::MTGAbility(int id, MTGCardInstance * card) :
    ActionElement(id)
{
    game = GameObserver::GetInstance();
    source = card;
    target = card;
    aType = MTGAbility::UNKNOWN;
    cost = NULL;
    forceDestroy = 0;
    oneShot = 0;
    canBeInterrupted = true;
}

MTGAbility::MTGAbility(int id, MTGCardInstance * _source, Targetable * _target) :
    ActionElement(id)
{
    game = GameObserver::GetInstance();
    source = _source;
    target = _target;
    aType = MTGAbility::UNKNOWN;
    cost = NULL;
    forceDestroy = 0;
    oneShot = 0;
    canBeInterrupted = true;
}

int MTGAbility::stillInUse(MTGCardInstance * card)
{
    if (card == source || card == target)
        return 1;
    return 0;
}

MTGAbility::~MTGAbility()
{
    if (!isClone)
    {
        SAFE_DELETE(cost);
    }
}

int MTGAbility::addToGame()
{
    GameObserver::GetInstance()->addObserver(this);
    return 1;
}

int MTGAbility::removeFromGame()
{
    GameObserver::GetInstance()->removeObserver(this);
    return 1;
}

//returns 1 if this ability needs to be removed from the list of active abilities
int MTGAbility::testDestroy()
{
GameObserver * g=g->GetInstance();
    if (game->mLayers->stackLayer()->has(this))
        return 0;
    if (waitingForAnswer)
        return 0;
    if (forceDestroy == 1)
        return 1;
    if (forceDestroy == -1)
        return 0;
    if(source->graveEffects && game->isInGrave(source))
        return 0;
    if(source->exileEffects && game->isInExile(source))
        return 0;
    if (!game->isInPlay(source))
        return 1;
    if (target && !game->isInPlay((MTGCardInstance *) target))
        return 1;
    return 0;
}

int MTGAbility::fireAbility()
{
    if (canBeInterrupted)
        game->mLayers->stackLayer()->addAbility(this);
    else
        resolve();
    return 1;
}

ostream& MTGAbility::toString(ostream& out) const
{
    return out << "MTGAbility ::: menuText : " << menuText << " ; game : " << game << " ; forceDestroy : " << forceDestroy
                    << " ; cost : " << cost << " ; target : " << target << " ; aType : " << aType << " ; source : " << source;
}

NestedAbility::NestedAbility(MTGAbility * _ability)
{
    ability = _ability;
}

//

ActivatedAbility::ActivatedAbility(int id, MTGCardInstance * card, ManaCost * _cost, int restrictions,string limit,MTGAbility * sideEffect,string usesBeforeSideEffects) :
    MTGAbility(id, card), restrictions(restrictions), needsTapping(0),limit(limit),sideEffect(sideEffect),usesBeforeSideEffects(usesBeforeSideEffects)
{
    counters = 0;
    cost = _cost;
    abilityCost = 0;
    sa = NULL;
}

int ActivatedAbility::isReactingToClick(MTGCardInstance * card, ManaCost * mana)
{        
    if(card->isPhased)
        return 0;
        
    Player * player = game->currentlyActing();
    int cPhase = game->getCurrentGamePhase();
    switch (restrictions)
    {
    case PLAYER_TURN_ONLY:
        if (player != game->currentPlayer)
            return 0;
        break;
    case OPPONENT_TURN_ONLY:
        if (player == game->currentPlayer)
            return 0;
            break;
    case AS_SORCERY:
        if (player != game->currentPlayer)
            return 0;
        if (cPhase != Constants::MTG_PHASE_FIRSTMAIN && cPhase != Constants::MTG_PHASE_SECONDMAIN)
            return 0;
        break;
    }
    if (restrictions >= MY_BEFORE_BEGIN && restrictions <= MY_AFTER_EOT)
    {
        if (player != game->currentPlayer)
            return 0;
        if (cPhase != restrictions - MY_BEFORE_BEGIN + Constants::MTG_PHASE_BEFORE_BEGIN)
            return 0;
    }

    if (restrictions >= OPPONENT_BEFORE_BEGIN && restrictions <= OPPONENT_AFTER_EOT)
    {
        if (player == game->currentPlayer)
            return 0;
        if (cPhase != restrictions - OPPONENT_BEFORE_BEGIN + Constants::MTG_PHASE_BEFORE_BEGIN)
            return 0;
    }

    if (restrictions >= BEFORE_BEGIN && restrictions <= AFTER_EOT)
    {
        if (cPhase != restrictions - BEFORE_BEGIN + Constants::MTG_PHASE_BEFORE_BEGIN)
            return 0;
    }
    limitPerTurn = 0;
    if(limit.size())
    {
        WParsedInt * value = NEW WParsedInt(limit.c_str(),NULL,source);
        limitPerTurn = value->getValue();
        delete value;
        //only run this check if we have a valid limit string.
        //limits on uses are based on when the ability is used, not when it is resolved
        //incrementing of counters directly after the fireability()
        //as ability limits are supposed to count the use regaurdless of the ability actually
        //resolving. this check was previously located in genericactivated, and incrementing was handled in the resolve.
        if (limitPerTurn && counters >= limitPerTurn)
            return 0;
    }
    if (card == source && source->controller() == player && (!needsTapping || (!source->isTapped()
                    && !source->hasSummoningSickness())))
    {
        if (!cost)
            return 1;
        cost->setExtraCostsAction(this, card);
        if (!mana)
            mana = player->getManaPool();
        if (!mana->canAfford(cost))
            return 0;
        if (!cost->canPayExtra())
            return 0;
        return 1;
    }
    return 0;
}

int ActivatedAbility::reactToClick(MTGCardInstance * card)
{
    if (!isReactingToClick(card))
        return 0;
    Player * player = game->currentlyActing();
    if (cost)
    {
        if (!cost->isExtraPaymentSet())
        {
            game->mExtraPayment = cost->extraCosts;
            return 0;
        }
        ManaCost * previousManaPool = NEW ManaCost(player->getManaPool());
        game->currentlyActing()->getManaPool()->pay(cost);
        cost->doPayExtra();
        SAFE_DELETE(abilityCost);
        abilityCost = previousManaPool->Diff(player->getManaPool());
        delete previousManaPool;
    }
    return ActivatedAbility::activateAbility();
}

int ActivatedAbility::reactToTargetClick(Targetable * object)
{
    if (!isReactingToTargetClick(object))
        return 0;
    Player * player = game->currentlyActing();
    if (cost)
    {
        if (object->typeAsTarget() == TARGET_CARD)
            cost->setExtraCostsAction(this, (MTGCardInstance *) object);
        if (!cost->isExtraPaymentSet())
        {
            game->mExtraPayment = cost->extraCosts;
            return 0;
        }
        ManaCost * previousManaPool = NEW ManaCost(player->getManaPool());
        game->currentlyActing()->getManaPool()->pay(cost);
        cost->doPayExtra();
        SAFE_DELETE(abilityCost);
        abilityCost = previousManaPool->Diff(player->getManaPool());
        delete previousManaPool;
    }
    return ActivatedAbility::activateAbility();
}

int ActivatedAbility::activateAbility()
{
    MTGAbility * fmp = NULL;
    bool wasTappedForMana = false;
    //taking foreach manaproducers off the stack and sending tapped for mana events.
    AbilityFactory af;
    fmp = af.getCoreAbility(this);
    AManaProducer * amp = dynamic_cast<AManaProducer *> (this);
    AManaProducer * femp = dynamic_cast<AManaProducer *> (fmp);
    if((amp||femp) && cost && cost->extraCosts)
    {
        for(unsigned int i = 0; i < cost->extraCosts->costs.size();i++)
        {
            ExtraCost * tapper = dynamic_cast<TapCost*>(cost->extraCosts->costs[i]);
            if(tapper)
                needsTapping = 1;
                wasTappedForMana = true;
        }
    }
    else if(amp||femp)
    {
        if(amp)
            needsTapping = amp->tap;
        else
            needsTapping = femp->tap;
    }
    if (needsTapping && (source->isInPlay()|| wasTappedForMana))
    {
        if (amp||femp)
        {
            GameObserver *g = GameObserver::GetInstance();
            WEvent * e = NEW WEventCardTappedForMana(source, 0, 1);
            g->receiveEvent(e);
        }
    }
    if (amp||femp)
    {
        counters++;
        if(sideEffect && usesBeforeSideEffects.size())
        {
            activateSideEffect();
        }
        this->resolve();
        return 1;
    }
    counters++;
    if(sideEffect && usesBeforeSideEffects.size())
    {
        activateSideEffect();
    }
    fireAbility();
    return 1;
}

void ActivatedAbility::activateSideEffect()
{
    WParsedInt * use = NEW WParsedInt(usesBeforeSideEffects.c_str(),NULL,source);
    uses = use->getValue();
    delete use;
    if(counters == uses)
    {
        sa = sideEffect->clone();
        sa->target = this->target;
        sa->source = this->source;
        if(sa->oneShot)
        {
            sa->fireAbility();
        }
        else
        {
            GenericInstantAbility * wrapper = NEW GenericInstantAbility(1, source, (Damageable *) (this->target), sa);
            wrapper->addToGame();
        }
    }
    return;
}

ActivatedAbility::~ActivatedAbility()
{
    //Ok, this will probably lead to crashes, maybe with lord abilities involving "X" costs.
    // If that's the case, we need to improve the clone() method of GenericActivatedAbility and GenericTargetAbility, I think
    // Erwan 2004/04/25
    //if (!isClone){
    SAFE_DELETE(abilityCost);
    SAFE_DELETE(sideEffect);
    SAFE_DELETE(sa);
    //}
}

ostream& ActivatedAbility::toString(ostream& out) const
{
    out << "ActivatedAbility ::: restrictions : " << restrictions << " ; needsTapping : " << needsTapping << " (";
    return MTGAbility::toString(out) << ")";
}

TargetAbility::TargetAbility(int id, MTGCardInstance * card, TargetChooser * _tc, ManaCost * _cost, int _playerturnonly) :
    ActivatedAbility(id, card, _cost, _playerturnonly), NestedAbility(NULL)
{
    tc = _tc;
}

TargetAbility::TargetAbility(int id, MTGCardInstance * card, ManaCost * _cost, int _playerturnonly) :
    ActivatedAbility(id, card, _cost, _playerturnonly), NestedAbility(NULL)
{
    tc = NULL;
}

int TargetAbility::reactToTargetClick(Targetable * object)
{
    if (object->typeAsTarget() == TARGET_CARD)
        return reactToClick((MTGCardInstance *) object);
    if (waitingForAnswer)
    {
        if (tc->toggleTarget(object) == TARGET_OK_FULL)
        {
            waitingForAnswer = 0;
            game->mLayers->actionLayer()->setCurrentWaitingAction(NULL);
            return ActivatedAbility::reactToClick(source);
        }
        return 1;
    }
    return 0;
}

int TargetAbility::reactToClick(MTGCardInstance * card)
{
    if (!waitingForAnswer)
    {
        if (isReactingToClick(card))
        {
            if (cost && !cost->isExtraPaymentSet())
            {
                game->mExtraPayment = cost->extraCosts;
                return 0;
            }

            waitingForAnswer = 1;
            game->mLayers->actionLayer()->setCurrentWaitingAction(this);
            tc->initTargets();
            return 1;
        }
    }
    else
    {
        if (card == source && (tc->targetsReadyCheck() == TARGET_OK || tc->targetsReadyCheck() == TARGET_OK_FULL))
        {
            waitingForAnswer = 0;
            game->mLayers->actionLayer()->setCurrentWaitingAction(NULL);
            return ActivatedAbility::reactToClick(source);
        }
        else
        {
            if (tc->toggleTarget(card) == TARGET_OK_FULL)
            {
                int result = ActivatedAbility::reactToClick(source);
                if (result)
                {
                    waitingForAnswer = 0;
                    game->mLayers->actionLayer()->setCurrentWaitingAction(NULL);
                    if(tc->targetter)
                    {
                    WEvent * e = NEW WEventTarget(card,source);
                    game->receiveEvent(e);
                    }
                }
                return result;
            }
            return 1;
        }
    }
    return 0;
}

void TargetAbility::Render()
{
    //TODO ?
}

int TargetAbility::resolve()
{
    Targetable * t = tc->getNextTarget();
    if (t && ability)
    {
        ManaCost * diff = abilityCost->Diff(cost);
        source->X = diff->hasX();
        delete (diff);
        ability->target = t;
        //do nothing if the target controller responded by phasing out the target.
        if (t->typeAsTarget() == TARGET_CARD && ((MTGCardInstance*)t)->isPhased)
            return 0;
        if (ability->oneShot)
            return ability->resolve();
        MTGAbility * a = ability->clone();
        return a->addToGame();
    }
    return 0;
}

const char * TargetAbility::getMenuText()
{
    if (ability)
        return ability->getMenuText();
    return ActivatedAbility::getMenuText();
}

TargetAbility::~TargetAbility()
{
    if (!isClone)
        SAFE_DELETE(ability);
}

ostream& TargetAbility::toString(ostream& out) const
{
    out << "TargetAbility ::: (";
    return ActivatedAbility::toString(out) << ")";
}

//


TriggeredAbility::TriggeredAbility(int id, MTGCardInstance * card, Targetable * _target) :
    MTGAbility(id, card, _target)
{
}

TriggeredAbility::TriggeredAbility(int id, MTGCardInstance * card) :
    MTGAbility(id, card)
{
}

int TriggeredAbility::receiveEvent(WEvent * e)
{
    if (triggerOnEvent(e))
    {
    if(dynamic_cast<WEventTarget*>(e))
    {
    //@targetted trigger as per mtg rules is a state based trigger
    //that resolves instantly before the event that targetted it.
        resolve();
        return 1;
    }
        fireAbility();
        return 1;
    }
    return 0;
}

void TriggeredAbility::Update(float dt)
{
    if (trigger())
        fireAbility();
}

ostream& TriggeredAbility::toString(ostream& out) const
{
    out << "TriggeredAbility ::: (";
    return MTGAbility::toString(out) << ")";
}

//
InstantAbility::InstantAbility(int _id, MTGCardInstance * source) :
    MTGAbility(_id, source)
{
    init = 0;
}

void InstantAbility::Update(float dt)
{
    if (!init)
    {
        init = resolve();
    }
}

InstantAbility::InstantAbility(int _id, MTGCardInstance * source, Targetable * _target) :
    MTGAbility(_id, source, _target)
{
    init = 0;
}

//Instant abilities last generally until the end of the turn
int InstantAbility::testDestroy()
{
    int newPhase = game->getCurrentGamePhase();
    if (newPhase != currentPhase && newPhase == Constants::MTG_PHASE_AFTER_EOT)
        return 1;
    currentPhase = newPhase;
    return 0;

}

ostream& InstantAbility::toString(ostream& out) const
{
    out << "InstantAbility ::: init : " << init << " (";
    return MTGAbility::toString(out) << ")";
}

bool ListMaintainerAbility::canTarget(MTGGameZone * zone)
{
    if (tc)
        return tc->targetsZone(zone);
    for (int i = 0; i < 2; i++)
    {
        Player * p = game->players[i];
        if (zone == p->game->inPlay)
            return true;
    }
    return false;
}

void ListMaintainerAbility::updateTargets()
{
    //remove invalid ones
    map<MTGCardInstance *, bool> temp;
    for (map<MTGCardInstance *, bool>::iterator it = cards.begin(); it != cards.end(); ++it)
    {
        MTGCardInstance * card = (*it).first;
        if (!canBeInList(card))
        {
            temp[card] = true;
        }
    }

    for (map<MTGCardInstance *, bool>::iterator it = temp.begin(); it != temp.end(); ++it)
    {
        MTGCardInstance * card = (*it).first;
        cards.erase(card);
        removed(card);
    }

    temp.clear();

    //add New valid ones
    for (int i = 0; i < 2; i++)
    {
        Player * p = game->players[i];
        MTGGameZone * zones[] = { p->game->inPlay, p->game->graveyard, p->game->hand, p->game->library };
        for (int k = 0; k < 4; k++)
        {
            MTGGameZone * zone = zones[k];
            if (canTarget(zone))
            {
                for (int j = 0; j < zone->nb_cards; j++)
                {
                    if (canBeInList(zone->cards[j]))
                    {
                        if (cards.find(zone->cards[j]) == cards.end())
                        {
                            temp[zone->cards[j]] = true;
                        }
                    }
                }
            }
        }
    }

    for (map<MTGCardInstance *, bool>::iterator it = temp.begin(); it != temp.end(); ++it)
    {
        MTGCardInstance * card = (*it).first;
        cards[card] = true;
        added(card);
    }

    temp.clear();

    for (int i = 0; i < 2; ++i)
    {
        Player * p = game->players[i];
        if (!players[p] && canBeInList(p))
        {
            players[p] = true;
            added(p);
        }
        else if (players[p] && !canBeInList(p))
        {
            players[p] = false;
            removed(p);
        }
    }

}

void ListMaintainerAbility::Update(float dt)
{
    updateTargets();
}

//Destroy the spell -> remove all targets
int ListMaintainerAbility::destroy()
{
    map<MTGCardInstance *, bool>::iterator it = cards.begin();

    while (it != cards.end())
    {
        MTGCardInstance * card = (*it).first;
        cards.erase(card);
        removed(card);
        it = cards.begin();
    }
    return 1;
}

ostream& ListMaintainerAbility::toString(ostream& out) const
{
    out << "ListMaintainerAbility ::: (";
    return MTGAbility::toString(out) << ")";
}

TriggerAtPhase::TriggerAtPhase(int id, MTGCardInstance * source, Targetable * target, int _phaseId, int who, bool sourceUntapped, bool sourceTap,bool lifelost,int lifeamount) :
    TriggeredAbility(id, source, target), phaseId(_phaseId), who(who), sourceUntapped(sourceUntapped), sourceTap(sourceTap),lifelost(lifelost),lifeamount(lifeamount)
{
    GameObserver * g = GameObserver::GetInstance();
    if (g)
    {
        newPhase = g->getCurrentGamePhase();
        currentPhase = newPhase;
    }
    }

    int TriggerAtPhase::trigger()
    {
        if(source->isPhased) return 0;
        if(lifelost)
        {
            int lifeloss = source->controller()->opponent()->lifeLostThisTurn;
            if(lifeloss < lifeamount)
                return 0;
        }
        if (sourceUntapped  && source->isTapped() == 1)
            return 0;
        if (sourceTap  && !source->isTapped())
            return 0;
        if (testDestroy())
            return 0; // http://code.google.com/p/wagic/issues/detail?id=426
        GameObserver * g = GameObserver::GetInstance();
        int result = 0;
        if (currentPhase != newPhase && newPhase == phaseId)
        {
        result = 0;
        switch (who)
        {
        case 1:
            if (g->currentPlayer == source->controller())
                result = 1;
            break;
        case -1:
            if (g->currentPlayer != source->controller())
                result = 1;
            break;
        case -2:
            if (source->target)
            {
                if (g->currentPlayer == source->target->controller())
                    result = 1;
            }
            else
            {
                if (g->currentPlayer == source->controller())
                    result = 1;
            }
            break;
        default:
            result = 1;
            break;
        }
    }
    return result;
}

TriggerAtPhase* TriggerAtPhase::clone() const
{
    TriggerAtPhase * a = NEW TriggerAtPhase(*this);
    a->isClone = 1;
    return a;
}

TriggerNextPhase::TriggerNextPhase(int id, MTGCardInstance * source, Targetable * target, int _phaseId, int who,bool sourceUntapped, bool sourceTap) :
    TriggerAtPhase(id, source, target, _phaseId, who, sourceUntapped, sourceTap)
{
    destroyActivated = 0;
}

int TriggerNextPhase::testDestroy()
{
    //dirty hack because of http://code.google.com/p/wagic/issues/detail?id=426
    if (newPhase <= phaseId && !destroyActivated)
        destroyActivated = 1;
    if (destroyActivated > 1 || (newPhase > phaseId && destroyActivated))
    {
        destroyActivated++;
        return 1;
    }
    return 0;
}

TriggerNextPhase* TriggerNextPhase::clone() const
{
    TriggerNextPhase * a = NEW TriggerNextPhase(*this);
    a->isClone = 1;
    return a;
}

GenericTriggeredAbility::GenericTriggeredAbility(int id, MTGCardInstance * _source, TriggeredAbility * _t, MTGAbility * a,
                MTGAbility * dc, Targetable * _target) :
    TriggeredAbility(id, _source, _target), NestedAbility(a)
{
    if (!target)
        target = source;
    t = _t;
    destroyCondition = dc;

    t->source = source;
    t->target = target;
    ability->source = source;
    ability->target = target;
    if (destroyCondition)
    {
        destroyCondition->source = source;
        destroyCondition->target = target;
    }
}

int GenericTriggeredAbility::trigger()
{
    return t->trigger();
}

int GenericTriggeredAbility::triggerOnEvent(WEvent * e)
{
    if (t->triggerOnEvent(e))
    {
        targets.push(getTriggerTarget(e, ability));
        return 1;
    }
    return 0;
}

Targetable * GenericTriggeredAbility::getTriggerTarget(WEvent * e, MTGAbility * a)
{
    TriggerTargetChooser * ttc = dynamic_cast<TriggerTargetChooser *> (a->tc);
    if (ttc)
        return e->getTarget(ttc->triggerTarget);

    NestedAbility * na = dynamic_cast<NestedAbility *> (a);
    if (na)
        return getTriggerTarget(e, na->ability);

    MultiAbility * ma = dynamic_cast<MultiAbility *> (a);
    if (ma)
    {
        for (size_t i = 0; i < ma->abilities.size(); i++)
        {
            return getTriggerTarget(e, ma->abilities[i]);
        }
    }

    return NULL;
}

void GenericTriggeredAbility::setTriggerTargets(Targetable * ta, MTGAbility * a)
{
    TriggerTargetChooser * ttc = dynamic_cast<TriggerTargetChooser *> (a->tc);
    if (ttc)
    {
        a->target = ta;
        ttc->target = ta;
    }

    NestedAbility * na = dynamic_cast<NestedAbility *> (a);
    if (na)
        setTriggerTargets(ta, na->ability);

    MultiAbility * ma = dynamic_cast<MultiAbility *> (a);
    if (ma)
    {
        for (size_t i = 0; i < ma->abilities.size(); i++)
        {
            setTriggerTargets(ta, ma->abilities[i]);
        }
    }
}

void GenericTriggeredAbility::Update(float dt)
{
    GameObserver * g = GameObserver::GetInstance();
    int newPhase = g->getCurrentGamePhase();
    t->newPhase = newPhase;
    TriggeredAbility::Update(dt);
    t->currentPhase = newPhase;
}

int GenericTriggeredAbility::resolve()
{
    if(source->isPhased) return 0;
    if (targets.size())
    {
        setTriggerTargets(targets.front(), ability);
        targets.pop();
    }
    if (ability->oneShot)
        return ability->resolve();
    MTGAbility * clone = ability->clone();
    clone->addToGame();
    return 1;
}

int GenericTriggeredAbility::testDestroy()
{
    if (!TriggeredAbility::testDestroy())
        return 0;
    if (destroyCondition)
        return (destroyCondition->testDestroy());
    return t->testDestroy();
}

GenericTriggeredAbility::~GenericTriggeredAbility()
{
    if (!isClone)
    {
        delete t;
        delete ability;
        SAFE_DELETE(destroyCondition);
    }
}

const char * GenericTriggeredAbility::getMenuText()
{
    return ability->getMenuText();
}

GenericTriggeredAbility* GenericTriggeredAbility::clone() const
{
    GenericTriggeredAbility * a = NEW GenericTriggeredAbility(*this);
    a->isClone = 1;
    return a;
}

/*Mana Producers (lands)
 //These have a reactToClick function, and therefore two manaProducers on the same card conflict with each other
 //That means the player has to choose one. although that is perfect for cards such as birds of paradise or badlands,
 other solutions need to be provided for abilities that add mana (ex: mana flare)
 */

AManaProducer::AManaProducer(int id, MTGCardInstance * card, Targetable * t, ManaCost * _output, ManaCost * _cost,
                int who) :
    ActivatedAbilityTP(id, card, t, _cost, who)
{

    aType = MTGAbility::MANA_PRODUCER;
    cost = _cost;
    output = _output;

    menutext = "";
}

int AManaProducer::isReactingToClick(MTGCardInstance * _card, ManaCost * mana)
{
    int result = 0;
    if (!mana)
        mana = game->currentlyActing()->getManaPool();
    if (_card == source && (!tap || !source->isTapped()) && game->currentlyActing()->game->inPlay->hasCard(source)
                    && (source->hasType(Subtypes::TYPE_LAND) || !tap || !source->hasSummoningSickness()) && !source->isPhased)
    {
        if (!cost || mana->canAfford(cost))
        {
            result = 1;
            if (cost && cost->extraCosts != NULL)
            {
                return cost->canPayExtra();
            }
        }
    }
    return result;
}

int AManaProducer::resolve()
{
    Targetable * _target = getTarget();
    Player * player;
    if (_target)
    {
        if (_target->typeAsTarget() == TARGET_CARD)
        {
            player = ((MTGCardInstance *) _target)->controller();
        }
        else
        {
            player = (Player *) _target;
        }
        player->getManaPool()->add(output, source);
        return 1;
    }
    return 0;
}

int AManaProducer::reactToClick(MTGCardInstance * _card)
{
    if (!isReactingToClick(_card))
        return 0;
    if(!ActivatedAbility::isReactingToClick(_card))
        return 0;
    if (cost)
    {
        cost->setExtraCostsAction(this, _card);
        if (!cost->isExtraPaymentSet())
        {
            GameObserver::GetInstance()->mExtraPayment = cost->extraCosts;
            return 0;
        }
        GameObserver::GetInstance()->currentlyActing()->getManaPool()->pay(cost);
        cost->doPayExtra();
    }

    if (options[Options::SFXVOLUME].number > 0)
    {
        JSample * sample = WResourceManager::Instance()->RetrieveSample("mana.wav");
        if (sample)
            JSoundSystem::GetInstance()->PlaySample(sample);
    }
    return ActivatedAbility::activateAbility();
}

const char * AManaProducer::getMenuText()
{
    if (menutext.size())
        return menutext.c_str();
    menutext = _("Add ");
    char buffer[128];
    int alreadyHasOne = 0;
    for (int i = 0; i < 6; i++)
    {
        int value = output->getCost(i);
        if (value)
        {
            if (alreadyHasOne)
                menutext.append(",");
            sprintf(buffer, "%i ", value);
            menutext.append(buffer);
            switch (i)
            {
            case Constants::MTG_COLOR_RED:
                menutext.append(_("red"));
                break;
            case Constants::MTG_COLOR_BLUE:
                menutext.append(_("blue"));
                break;
            case Constants::MTG_COLOR_GREEN:
                menutext.append(_("green"));
                break;
            case Constants::MTG_COLOR_WHITE:
                menutext.append(_("white"));
                break;
            case Constants::MTG_COLOR_BLACK:
                menutext.append(_("black"));
                break;
            default:
                break;
            }
            alreadyHasOne = 1;
        }
    }
    menutext.append(_(" mana"));
    return menutext.c_str();
}

AManaProducer::~AManaProducer()
{
    SAFE_DELETE(cost);
    SAFE_DELETE(output);
}

AManaProducer * AManaProducer::clone() const
{
    AManaProducer * a = NEW AManaProducer(*this);
    a->cost = NEW ManaCost();
    a->output = NEW ManaCost();
    a->cost->copy(cost);
    a->output->copy(output);
    a->isClone = 1;
    return a;
}


AbilityTP::AbilityTP(int id, MTGCardInstance * card, Targetable * _target, int who) :
    MTGAbility(id, card), who(who)
{
    if (_target)
        target = _target;
}

Targetable * AbilityTP::getTarget()
{
    switch (who)
    {
    case TargetChooser::TARGET_CONTROLLER:
        if (target)
        {
            switch (target->typeAsTarget())
            {
            case TARGET_CARD:
                return ((MTGCardInstance *) target)->controller();
            case TARGET_STACKACTION:
                return ((Interruptible *) target)->source->controller();
            default:
                return (Player *) target;
            }
        }
        return NULL;
    case TargetChooser::CONTROLLER:
        return source->controller();
    case TargetChooser::OPPONENT:
        return source->controller()->opponent();
    case TargetChooser::OWNER:
        return source->owner;
    default:
        return target;
    }
    return NULL;
}

ActivatedAbilityTP::ActivatedAbilityTP(int id, MTGCardInstance * card, Targetable * _target, ManaCost * cost, int who) :
    ActivatedAbility(id, card, cost, 0), who(who)
{
    if (_target)
        target = _target;
}

Targetable * ActivatedAbilityTP::getTarget()
{
    switch (who)
    {
    case TargetChooser::TARGET_CONTROLLER:
        if (target)
        {
            switch (target->typeAsTarget())
            {
            case TARGET_CARD:
                return ((MTGCardInstance *) target)->controller();
            case TARGET_STACKACTION:
                return ((Interruptible *) target)->source->controller();
            default:
                return (Player *) target;
            }
        }
        return NULL;
    case TargetChooser::CONTROLLER:
        return source->controller();
    case TargetChooser::OPPONENT:
        return source->controller()->opponent();
    case TargetChooser::OWNER:
        return source->owner;
    default:
        return target;
    }
    return NULL;
}

InstantAbilityTP::InstantAbilityTP(int id, MTGCardInstance * card, Targetable * _target,int who) :
    InstantAbility(id, card), who(who)
{
    if (_target)
        target = _target;    
}

//This is the same as Targetable * ActivatedAbilityTP::getTarget(), anyway to move them together?
Targetable * InstantAbilityTP::getTarget()
{
    switch (who)
    {
    case TargetChooser::TARGET_CONTROLLER:
        if (target)
        {
            switch (target->typeAsTarget())
            {
            case TARGET_CARD:
                return ((MTGCardInstance *) target)->controller();
            case TARGET_STACKACTION:
                return ((Interruptible *) target)->source->controller();
            default:
                return (Player *) target;
            }
        }
        return NULL;
    case TargetChooser::CONTROLLER:
        return source->controller();
    case TargetChooser::OPPONENT:
        return source->controller()->opponent();
    case TargetChooser::OWNER:
        return source->owner;
    default:
        return target;
    }
    return NULL;
}
