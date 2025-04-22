#include "PrecompiledHeader.h"
#include "AllAbilities.h"

int WParsedInt::computeX(Spell * spell, MTGCardInstance * card)
{
    if (spell) 
        return spell->computeX(card);
    if (card) 
        return card->X;
    return 1; //this should only hapen when the ai calls the ability. This is to give it an idea of the "direction" of X (positive/negative)
}

void WParsedInt::init(string s, Spell * spell, MTGCardInstance * card)
{
    if(!s.size())
        return;
    if (!card)
    {
        intValue = atoi(s.c_str()); //if there is no card, try parsing a number.
        return;
    }
    MTGCardInstance * target = card->target;
    if(!card->storedCard)
        card->storedCard = card->storedSourceCard;
    intValue = 0;
    bool halfup = false;
    bool halfdown = false;
    bool thirdup = false;
    bool thirddown = false;
    bool twice = false;
    bool thrice = false;
    bool fourtimes = false;
    bool fivetimes = false;
    bool other = false;//othertype:[subtype]

    if (!target) target = card;
    int multiplier = 1;
    if (s[0] == '-')
    {
        s = s.substr(1);
        multiplier = -1;
        if(s.find("stored") != string::npos)
        {
            string altered ="-";
            altered.append(s.substr(+6));
            return init(altered,spell,card->storedCard);
        }
        if(s.find("mytarg") != string::npos)
        {
            string altered ="-";
            altered.append(s.substr(+6));
            return init(altered,spell,card->target); //we refer the target (e.g. Redirect)
        }
    }
    if(s[0] == '+')
    {
        //ignore "+" signs....
        s = s.substr(1);
    }
    if(s.find("stored") != string::npos)
    {
        return init(s.substr(+6),spell,card->storedCard);
    }
    if(s.find("mytarg") != string::npos)
    {
        return init(s.substr(+6),spell,card->target); //we refer the target (e.g. Redirect)
    }
    //rounding values, the words can be written anywhere in the line,
    //they are erased after parsing.
    if(s.find("halfup") != string::npos)
    {
        halfup = true;
        size_t hU = s.find("halfup");
        s.erase(hU,hU + 6);
    }
    if(s.find("halfdown") != string::npos)
    {
        halfdown = true;
        size_t hD = s.find("halfdown");
        s.erase(hD,hD + 8);
    }
    if(s.find("thirdup") != string::npos)
    {
        thirdup = true;
        size_t tU = s.find("thirdup");
        s.erase(tU,tU + 7);
    }
    if(s.find("thirddown") != string::npos)
    {
        thirddown = true;
        size_t tD = s.find("thirddown");
        s.erase(tD,tD + 9);
    }
    if(s.find("twice") != string::npos)
    {
        twice = true;
        size_t tXX = s.find("twice");
        s.erase(tXX,tXX + 5);
    }
    if(s.find("thrice") != string::npos)
    {
        thrice = true;
        size_t tXXX = s.find("thrice");
        s.erase(tXXX,tXXX + 6);
    }
    if(s.find("fourtimes") != string::npos)
    {
        fourtimes = true;
        size_t tXXX = s.find("fourtimes");
        s.erase(tXXX,tXXX + 9);
    }
    if(s.find("fivetimes") != string::npos)
    {
        fivetimes = true;
        size_t tXXX = s.find("fivetimes");
        s.erase(tXXX,tXXX + 9);
    }
    if(s.find("othertype") != string::npos)
    {
        other = true;
        size_t oth = s.find("othertype");
        s.erase(oth,oth + 5);
    }
    if(s.find("otherpower") != string::npos)
    {
        other = true;
        size_t otp = s.find("otherpower");
        s.erase(otp,otp + 5);
    }
    if(s.find("othertoughness") != string::npos)
    {
        other = true;
        size_t ott = s.find("othertoughness");
        s.erase(ott,ott + 5);
    }
    if(s.find("otherconvertedcost") != string::npos)
    {
        other = true;
        size_t otc = s.find("otherconvertedcost");
        s.erase(otc,otc + 5);
    }

    if (s.find("plusend") != string::npos || s.find("minusend") != string::npos || s.find("math") != string::npos)
    {
        //plus#plusend and minus#minusend splits the first part and second parts and parses the
        //ints for each part, then either adds or subtracts those 2 variables as specified.
        vector<string>mathFound = parseBetween(s, "math", "mathend", true);
        if (mathFound.size())//maths allows us to get the value before applying multipliers
        {
            WParsedInt numPar(mathFound[1], NULL, card);
            intValue = numPar.getValue();

        }
        else
        {
            vector<string>plusSplit = parseBetween(s, "", "plus", true);
            if (plusSplit.size())
            {
                WParsedInt numPar(plusSplit[1], NULL, card);
                intValue = numPar.getValue();
            }
            vector<string>plusFound = parseBetween(s, "plus", "plusend", true);
            if (plusFound.size())
            {
                WParsedInt numPar(plusFound[1], NULL, card);
                intValue += numPar.getValue();
            }
            vector<string>minusSplit = parseBetween(s, "", "minus", true);
            if (minusSplit.size())
            {
                WParsedInt numPar(minusSplit[1], NULL, card);
                intValue = numPar.getValue();
            }
            vector<string>minusFound = parseBetween(s, "minus", "minusend", true);
            if (minusFound.size())
            {
                WParsedInt numPar(minusFound[1], NULL, card);
                intValue -= numPar.getValue();
            }
        }
    }
    else if(s == "prex")
    {
        if (card->setX > -1)
        {
            intValue = card->setX;
        }
        else
        {
            ManaCost * cX = card->controller()->getManaPool()->Diff(card->getManaCost());
            intValue = cX->getCost(Constants::NB_Colors);
            delete cX;
        }
    }
    else if (s == "x" || s == "X" || s == "fullpaid")
    {
        intValue = computeX(spell, card);
        if(intValue < 0) intValue = 0;
    }
    else if (s == "xx" || s == "XX" || s == "halfpaid")
    {
        intValue = computeX(spell, card) / 2;
        if(intValue < 0) intValue = 0;
    }
    else if (s == "xxx" || s == "XXX" || s == "thirdpaid")
    {
        intValue = computeX(spell, card) / 3;
        if(intValue < 0) intValue = 0;
    }
    else if (s == "castx" || s == "Iroas")//calculate castx - devotion to red white
    {
        intValue = (s == "castx")?card->castX:countDevotionTo(card, card->controller()->inPlay(), Constants::MTG_COLOR_RED, Constants::MTG_COLOR_WHITE);
    }
    else if (s == "azorius" || s == "boros")//devotion blue white - devotion red white
    {
        intValue = (s == "azorius")?countDevotionTo(card,card->controller()->inPlay(),Constants::MTG_COLOR_BLUE,Constants::MTG_COLOR_WHITE):countDevotionTo(card,card->controller()->inPlay(),Constants::MTG_COLOR_WHITE,Constants::MTG_COLOR_RED);
    }
    else if (s == "dimir" || s == "golgari")//devotion blue black - devotion to green black
    {
        intValue = (s == "dimir")?countDevotionTo(card,card->controller()->inPlay(),Constants::MTG_COLOR_BLACK,Constants::MTG_COLOR_BLUE):countDevotionTo(card,card->controller()->inPlay(),Constants::MTG_COLOR_BLACK,Constants::MTG_COLOR_GREEN);
    }
    else if (s == "gruul" || s == "izzet")//devotion to green red - devotion to red blue
    {
        intValue = (s == "gruul")?countDevotionTo(card,card->controller()->inPlay(),Constants::MTG_COLOR_GREEN,Constants::MTG_COLOR_RED):countDevotionTo(card,card->controller()->inPlay(),Constants::MTG_COLOR_BLUE,Constants::MTG_COLOR_RED);
    }
    else if (s == "orzhov" || s == "rakdos")//devotion to white black - devotion to red black
    {
        intValue = (s == "orzhov")?countDevotionTo(card,card->controller()->inPlay(),Constants::MTG_COLOR_BLACK,Constants::MTG_COLOR_WHITE):countDevotionTo(card,card->controller()->inPlay(),Constants::MTG_COLOR_BLACK,Constants::MTG_COLOR_RED);
    }
    else if (s == "selesnya" || s == "simic")//devotion to green white - devotion to green blue
    {
        intValue = (s == "selesnya")?countDevotionTo(card,card->controller()->inPlay(),Constants::MTG_COLOR_GREEN,Constants::MTG_COLOR_WHITE):countDevotionTo(card,card->controller()->inPlay(),Constants::MTG_COLOR_BLUE,Constants::MTG_COLOR_GREEN);
    }
    else if (s == "gear" || s == "auras")
    {
        intValue = (s == "gear")?target->equipment:target->auras;
    }
    else if (s == "mutations" || s == "colors")
    {
        intValue = (s == "mutations")?target->mutation:target->countColors();
    }
    else if (s.find("type:") != string::npos)
    {
        size_t begins = s.find("type:");
        string theType = s.substr(begins + 5);
        size_t typepos = theType.find("chosentype");
        if(typepos != string::npos && card->chooseasubtype != "")
            theType.replace(typepos, 10, card->chooseasubtype);
        size_t colpos = theType.find("chosencolor");
        if(colpos != string::npos && card->chooseacolor >= 0){
            if(card->chooseacolor == Constants::MTG_COLOR_ARTIFACT)
                theType.replace(colpos, 11, "artifact");
            else if(card->chooseacolor == Constants::MTG_COLOR_GREEN)
                theType.replace(colpos, 11, "green");
            else if(card->chooseacolor == Constants::MTG_COLOR_BLUE)
                theType.replace(colpos, 11, "blue");
            else if(card->chooseacolor == Constants::MTG_COLOR_RED)
                theType.replace(colpos, 11, "red");
            else if(card->chooseacolor == Constants::MTG_COLOR_BLACK)
                theType.replace(colpos, 11, "black");
            else if(card->chooseacolor == Constants::MTG_COLOR_WHITE)
                theType.replace(colpos, 11, "white");
            else if(card->chooseacolor == Constants::MTG_COLOR_WASTE)
                theType.replace(colpos, 11, "waste");
            else if(card->chooseacolor == Constants::MTG_COLOR_LAND)
                theType.replace(colpos, 11, "land");
        }
        size_t zoned = theType.find(":");
        if(zoned == string::npos)
        {
            theType.append("|mybattlefield");
        }
        else
        {
            replace(theType.begin(), theType.end(), ':', '|');
        }
        int color = 0;
        if (theType.find("mana") != string::npos) {
            color = ManaCost::parseManaSymbol(theType[4]);
            theType.replace(0, 5, "*");
        }
        TargetChooserFactory tf(card->getObserver());
        TargetChooser * tc = tf.createTargetChooser(theType.c_str(),NULL);
        tc->other = other;
        for (int i = 0; i < 2; i++)
        {
            Player * p = card->getObserver()->players[i];
            MTGGameZone * zones[] = { p->game->inPlay, p->game->graveyard, p->game->hand, p->game->library, p->game->exile, p->game->commandzone, p->game->sideboard };
            for (int k = 0; k < 7; k++)
            {
                MTGGameZone * zone = zones[k];
                if (tc->targetsZone(zone, card))
                {
                    if (color)
                    {
                        intValue += card->controller()->devotionOffset + zone->countTotalManaSymbols(tc, color); // Increase total devotion with an offset (e.g. Altar of the Pantheon)
                    }
                    else
                    {
                        intValue += zone->countByCanTarget(tc);
                    }
                }
            }
        }
        SAFE_DELETE(tc);
    }
    else if (s.find("restriction{") != string::npos)
    {
        vector<string> splitRest = parseBetween(s,"restriction{","}");
        if (splitRest.size())
        {
            AbilityFactory abf(target->getObserver());
            int checkCond = abf.parseCastRestrictions(target,target->controller(),splitRest[1].c_str());
            if(checkCond)
                intValue = 1;
        }
    }
    else if (s.find("counter{") != string::npos)
    {
        intValue = 0;
        vector<string>counterName = parseBetween(s,"counter{","}");
        if(counterName.size())
        {
            AbilityFactory abf(target->getObserver());
            Counter * counter = abf.parseCounter(counterName[1], NULL);
            if(counter && target->counters && target->counters->hasCounter(counter->name.c_str(),counter->power,counter->toughness))
            {
                Counter * targetCounter = target->counters->hasCounter(counter->name.c_str(), counter->power, counter->toughness);
                intValue = targetCounter->nb;
            }
            SAFE_DELETE(counter);
        }
    }
    else if (s.find("convertedcost:") != string::npos || s.find("power:") != string::npos || s.find("pwr:") != string::npos || s.find("toughness:") != string::npos || s.find("ths:") != string::npos)
    {
        bool powerCheck = false;
        bool toughnessCheck = false;
        bool costCheck = false;
        intValue = 0;
        vector<string>convertedType = parseBetween(s,"convertedcost:",":");
        if(convertedType.size())
            costCheck = true;
        else
        {
            size_t pos = s.find("pwr:");
            if(pos != string::npos)
                s.replace(pos, 4, "power:");
            convertedType = parseBetween(s,"power:",":");
            if(convertedType.size()){
                powerCheck = true;
            }
            else
            {
                size_t pos = s.find("ths:");
                if(pos != string::npos)
                    s.replace(pos, 4, "toughness:");
                convertedType = parseBetween(s,"toughness:",":");
                if(convertedType.size())
                    toughnessCheck = true;
            }
        }
        if(!convertedType.size())
            return;
        bool high = false;
        int highest = 0;
        int lowest = 5000;
        if(convertedType[1].find("highest") != string::npos)
            high = true;

        string theType = convertedType[2];
        size_t zoned = theType.find(":");
        if(zoned == string::npos)
        {
            theType.append("|mybattlefield");
        }
        else
        {
            replace(theType.begin(), theType.end(), ':', '|');
        }
        TargetChooserFactory tf(card->getObserver());
        TargetChooser * tc = tf.createTargetChooser(theType.c_str(),NULL);
        tc->other = other;
        int check = 0;
        for (int i = 0; i < 2; i++)
        {
            Player * p = card->getObserver()->players[i];
            MTGGameZone * zones[] = { p->game->battlefield, p->game->graveyard, p->game->hand, p->game->library, p->game->exile, p->game->commandzone, p->game->sideboard, p->game->reveal, p->game->stack };
            for (int k = 0; k < 9; k++)
            {
                MTGGameZone * zone = zones[k];
                if(tc->targetsZone(zone,target))
                {
                    for(unsigned int w = 0;w < zone->cards.size();++w)
                    {
                        MTGCardInstance * cCard = zone->cards[w];
                        if(tc->canTarget(cCard))
                        {
                            if(costCheck)
                                check = cCard->getManaCost()->getConvertedCost();
                            if(powerCheck)
                                check = cCard->power;
                            if(toughnessCheck)
                                check = cCard->toughness;

                            if(check > highest)
                                highest = check;
                            if(check <= lowest)
                                lowest = check;
                        }
                    }
                }
            }
        }
        if(lowest == 5000)
            lowest = 0;
        SAFE_DELETE(tc);
        intValue = high?highest:lowest;
    }
    else if (s == "sunburst")
    {
        intValue = 0;
        if (card && card->previous && card->previous->previous)
        {
            intValue = card->previous->previous->sunburst;
        }
    }
    else if (s == "converge" || s == "totmanaspent")
    {
        intValue = 0;
        if(s == "converge"){
            for (int i = Constants::MTG_COLOR_GREEN; i <= Constants::MTG_COLOR_WHITE; ++i){
                if(card->getManaCost()->getManaUsedToCast() && card->getManaCost()->getManaUsedToCast()->hasColor(i))
                    intValue +=1;
            }
        } else if(s == "totmanaspent") { // Return the real amount of mana spent to cast the card (e.g. Memory Deluge)
            if(card->getManaCost()->getManaUsedToCast())
                intValue = card->getManaCost()->getManaUsedToCast()->getConvertedCost();
            else {
                if(card->alternateCostPaid[ManaCost::MANA_PAID_WITH_RETRACE] == 1 && card->getManaCost()->getRetrace())
                    intValue = card->getManaCost()->getRetrace()->getConvertedCost();
                else if(card->alternateCostPaid[ManaCost::MANA_PAID_WITH_FLASHBACK] == 1 && card->getManaCost()->getFlashback())
                    intValue = card->getManaCost()->getFlashback()->getConvertedCost();
                else if(card->alternateCostPaid[ManaCost::MANA_PAID_WITH_ALTERNATIVE] == 1 && card->getManaCost()->getAlternative())
                    intValue = card->getManaCost()->getAlternative()->getConvertedCost();
                else if(card->alternateCostPaid[ManaCost::MANA_PAID_WITH_BESTOW] == 1 && card->getManaCost()->getBestow())
                    intValue = card->getManaCost()->getBestow()->getConvertedCost();
                else if(card->alternateCostPaid[ManaCost::MANA_PAID_WITH_BUYBACK] == 1 && card->getManaCost()->getBuyback())
                    intValue = card->getManaCost()->getBuyback()->getConvertedCost();
                else if(card->alternateCostPaid[ManaCost::MANA_PAID_WITH_MORPH] == 1 && card->getManaCost()->getMorph())
                    intValue = card->getManaCost()->getMorph()->getConvertedCost();
                else if(card->alternateCostPaid[ManaCost::MANA_PAID_WITH_SUSPEND] == 1 && card->getManaCost()->getSuspend())
                    intValue = card->getManaCost()->getSuspend()->getConvertedCost();
                else
                    intValue = card->getManaCost()->getConvertedCost();
            }
        }
    }
    else if (s == "penergy" || s == "oenergy")
    {
        intValue = (s == "penergy")?card->controller()->energyCount:card->controller()->opponent()->energyCount;
    }
    else if (s == "pyidarocount" || s == "oyidarocount")
    {
        intValue = (s == "pyidarocount")?card->controller()->yidaroCount:card->controller()->opponent()->yidaroCount;
    }
    else if (s == "pmonarch" || s == "omonarch")
    {
        intValue = (s == "pmonarch")?card->controller()->monarch:card->controller()->opponent()->monarch;
    }
    else if (s == "psurveiloffset" || s == "osurveiloffset")
    {
        intValue = (s == "psurveiloffset")?card->controller()->surveilOffset:card->controller()->opponent()->surveilOffset;
    }
    else if (s == "pdevotionoffset" || s == "odevotionoffset")
    {
        intValue = (s == "pdevotionoffset")?card->controller()->devotionOffset:card->controller()->opponent()->devotionOffset;
    }
    else if (s == "pattackedcount" || s == "oattackedcount")
    {
        intValue = (s == "pattackedcount")?card->controller()->raidcount:card->controller()->opponent()->raidcount;
    }
    else if (s == "pstormcount" || s == "ostormcount")
    {
        intValue = (s == "pstormcount")?card->controller()->game->stack->seenThisTurn("*", Constants::CAST_ALL):card->controller()->opponent()->game->stack->seenThisTurn("*", Constants::CAST_ALL);
    }
    else if (s == "countallspell")
    {
        intValue = card->controller()->game->stack->seenThisTurn("*", Constants::CAST_ALL) + card->controller()->opponent()->game->stack->seenThisTurn("*", Constants::CAST_ALL);
    }
    else if (s == "countmycrespell" || s == "countmynoncrespell")
    {
        intValue = (s == "countmycrespell")?card->controller()->game->stack->seenThisTurn("creature", Constants::CAST_ALL):card->controller()->game->stack->seenThisTurn("*[-creature]", Constants::CAST_ALL);
    }
    else if(s == "numofcommandcast" || s == "pnumofcommandcast" || s == "onumofcommandcast" ||  s == "pnumofidentitycols" || s == "onumofidentitycols")
    {
        intValue = 0;
        if(s == "pnumofcommandcast") //Return how many times controller casted a commander (e.g. Skull Storm).
            intValue = card->controller()->numOfCommandCast;
        else if(s == "onumofcommandcast") //Return how many times controller casted a commander (e.g. Commander's Insight).
            intValue = card->controller()->opponent()->numOfCommandCast;
        else if(s == "numofcommandcast") //Return how many times this commander has been casted from command zone (e.g. Opal Palace).
            intValue = card->numofcastfromcommandzone;
        else if (s == "pnumofidentitycols" || s == "onumofidentitycols") //Return the total amount of commander identity colors for controller or opponent (e.g. War Room)
        {
            intValue = 0;
            bool blueFound = false;
            bool redFound = false;
            bool whiteFound = false;
            bool greenFound = false;
            bool blackFound = false;
            Player* p = card->controller();
            if (s == "onumofidentitycols")
                p = card->controller()->opponent();
            MTGGameZone * zones[] = { p->game->inPlay, p->game->graveyard, p->game->hand, p->game->library, p->game->exile, p->game->commandzone, p->game->sideboard };
            for(int i = 0; i < 7; i++){
                for(int j = 0; j < zones[i]->nb_cards; j++){
                    if(zones[i]->cards[j]->isCommander && zones[i]->cards[j]->hasColor(Constants::MTG_COLOR_RED) && !redFound){
                        intValue++;
                        redFound = true;
                    }
                    if(zones[i]->cards[j]->isCommander && zones[i]->cards[j]->hasColor(Constants::MTG_COLOR_BLACK) && !blackFound){
                        intValue++;
                        blackFound = true;
                    }
                    if(zones[i]->cards[j]->isCommander && zones[i]->cards[j]->hasColor(Constants::MTG_COLOR_BLUE) && !blueFound){
                        intValue++;
                        blueFound = true;
                    }
                    if(zones[i]->cards[j]->isCommander && zones[i]->cards[j]->hasColor(Constants::MTG_COLOR_GREEN) && !greenFound){
                        intValue++;
                        greenFound = true;
                    }
                    if(zones[i]->cards[j]->isCommander && zones[i]->cards[j]->hasColor(Constants::MTG_COLOR_WHITE) && !whiteFound){
                        intValue++;
                        whiteFound = true;
                    }
                }
            }
        }
    }
    else if (s == "isflipped" || s == "iscopied") // Return 1 if card has been flipped -- Return 1 if card has copied another card
    {
        intValue = (s == "isflipped")?card->isFlipped:card->isACopier;
    }
    else if (s == "evictmc" || s == "hasevict")
    {
        if(card->imprintedCards.size() > 0) intValue = (s == "evictmc")?card->imprintedCards.back()->getManaCost()->getConvertedCost():card->imprintedCards.size();
    }
    else if (s == "evictpw" || s == "evictth")
    {
        if(card->imprintedCards.size() > 0) intValue = (s == "evictpw")?card->imprintedCards.back()->getPower():card->imprintedCards.back()->getToughness();
    }
    else if (s == "evictg" || s == "evictu")
    {
        intValue = (s == "evictg")?card->imprintG:card->imprintU;
    }
    else if (s == "evictr" || s == "evictb")
    {
        intValue = (s == "evictr")?card->imprintR:card->imprintB;
    }
    else if (s == "evictw" || s == "commongreen")
    {
        intValue = (s == "evictw")?card->imprintW:mostCommonColor(Constants::MTG_COLOR_GREEN, card);
    }
    else if (s == "commonblue" || s == "commonred")
    {
        intValue = (s == "commonblue")?mostCommonColor(Constants::MTG_COLOR_BLUE, card):mostCommonColor(Constants::MTG_COLOR_RED, card);
    }
    else if (s == "commonblack" || s == "commonwhite")
    {
        intValue = (s == "commonblack")?mostCommonColor(Constants::MTG_COLOR_BLACK, card):mostCommonColor(Constants::MTG_COLOR_WHITE, card);
    }
    else if (s == "targetedcurses")
    {
        intValue = 0;
        for (int j = card->controller()->game->battlefield->nb_cards - 1; j >= 0; --j)
        {
            MTGCardInstance * curse = card->controller()->game->battlefield->cards[j];
            if (curse->hasType("Curse") && curse->playerTarget == card->playerTarget)
                intValue++;
        }
        for (int j = card->controller()->opponent()->game->battlefield->nb_cards - 1; j >= 0; --j)
        {
            MTGCardInstance * curse = card->controller()->opponent()->game->battlefield->cards[j];
            if (curse->hasType("Curse") && curse->playerTarget == card->playerTarget)
                intValue++;
        }
    }
    else if (s == "lifetotal" || s == "opponentlifetotal")
    {
        intValue = (s == "lifetotal")?target->controller()->life:target->controller()->opponent()->life;
    }
    else if (s == "startinglife" || s == "ostartinglife")
    {
        intValue = (s == "startinglife")?target->controller()->initLife:target->controller()->opponent()->initLife;
    }
    else if (s == "pdiffinitlife" || s == "odiffinitlife")
    {
        intValue = (s == "pdiffinitlife")?(target->controller()->life - target->controller()->initLife):(target->controller()->opponent()->life - target->controller()->opponent()->initLife);
    }
    else if (s == "phalfinitlife" || s == "ohalfinitlife")
    {
        if(s == "phalfinitlife"){
            intValue = (target->controller()->initLife > 2*target->controller()->life)?1:0;
        } else {
            intValue = (target->controller()->opponent()->initLife > 2*target->controller()->opponent()->life)?1:0;
        }
    }
    else if (s == "abundantlife")//current life is morethan or equal to starting life
    {
        intValue = (target->controller()->life >= target->controller()->initLife)?1:0;
    }
    else if (s == "plibrarycount")
    {
        intValue = (target->controller()->game->library->nb_cards)?target->controller()->game->library->nb_cards:0;
    }
    else if (s == "olibrarycount")
    {
        intValue = (target->controller()->opponent()->game->library->nb_cards)?target->controller()->opponent()->game->library->nb_cards:0;
    }
    else if (s == "highestlifetotal")
    {
        intValue = target->controller()->life <= target->controller()->opponent()->life?target->controller()->opponent()->life:target->controller()->life;
    }
    else if (s == "lowestlifetotal")
    {
        intValue = target->controller()->life <= target->controller()->opponent()->life?target->controller()->life:target->controller()->opponent()->life;
    }
    else if (s == "thatmuch")
    {
        //the value that much is a variable to be used with triggered abilities.
        //ie:when ever you gain life, draw that many cards. when used in a trigger draw:thatmuch, will return the value
        //that the triggered event stored in the card for "that much".
        intValue = target->thatmuch;
        int checkagain = 0;
        if(target->hasSubtype(Subtypes::TYPE_AURA) || target->hasSubtype(Subtypes::TYPE_EQUIPMENT)){
            if(target->target){
                checkagain = target->target->thatmuch;
            }
        }
        if(checkagain > intValue)
            intValue = checkagain;
        if(card && card->thatmuch > intValue)
            intValue = card->thatmuch;
    }
    else if (s == "excessdamage") // Return the amount of exceeded damage of a target
    {
        if(target->exceededDamage < 0)
            intValue = target->exceededDamage;
        int checkagain = 0;
        if(target->hasSubtype(Subtypes::TYPE_AURA) || target->hasSubtype(Subtypes::TYPE_EQUIPMENT)){
            if(target->target){
                if(target->target->exceededDamage < 0)
                    checkagain = target->target->exceededDamage;
            }
        }
        if(checkagain < intValue)
            intValue = checkagain;
        if(card && card->exceededDamage < intValue)
            intValue = card->exceededDamage;
        if(intValue < 0)
            intValue = abs(intValue);
    }
    else if (s == "lifelost" || s == "oplifelost")
    {
        intValue = (s == "lifelost")?target->controller()->lifeLostThisTurn:target->controller()->opponent()->lifeLostThisTurn;
    }
    else if (s == "lifegain" || s == "oplifegain")
    {
        intValue = (s == "lifegain")?target->controller()->lifeGainedThisTurn:target->controller()->opponent()->lifeGainedThisTurn;
    }
    else if (s == "pdcount" || s == "odcount")
    {
        intValue = (s == "pdcount")?target->controller()->damageCount:target->controller()->opponent()->damageCount;
    }
    else if (s == "pdnoncount" || s == "odnoncount")
    {
        intValue = (s == "pdnoncount")?target->controller()->nonCombatDamage:target->controller()->opponent()->nonCombatDamage;
    }
    else if (s == "mypoisoncount" || s == "opponentpoisoncount")
    {
        intValue = (s == "mypoisoncount")?target->controller()->poisonCount:target->controller()->opponent()->poisonCount;
    }
    else if(s == "lastrollresult" || s == "lastrollchoice" || s == "lastdiefaces" || s == "srclastrollresult" || s == "srclastrollchoice" || s == "srclastdiefaces")
    {
        intValue = 0;
        if(s == "lastrollresult")
            intValue = target->lastRollResult;
        else if(s == "srclastrollresult")
            intValue = card->lastRollResult;
        else if(s == "lastrollchoice")
            intValue = target->dieSide;
        else if(s == "srclastrollchoice")
            intValue = card->dieSide;
        else if(s == "lastdiefaces")
            intValue = target->dieNumFaces;
        else if(s == "srclastdiefaces")
            intValue = card->dieNumFaces;
    }
    else if(s == "lastflipresult" || s == "lastflipchoice")
    {
        intValue = (s == "lastflipresult")?target->lastFlipResult:target->coinSide;
    }
    else if (s == "pdrewcount" || s == "odrewcount")
    {
        intValue = (s == "pdrewcount")?target->controller()->drawCounter:target->controller()->opponent()->drawCounter;
    }
    else if (s == "epicactivated" || s == "hasstorecard")
    {
        intValue = (s == "epicactivated")?target->controller()->epic:(target->storedCard != 0);
    }
    else if (s == "currentphase" || s == "currentturn" )
    {
        intValue =  (s == "currentphase")?target->getObserver()->getCurrentGamePhase():target->getObserver()->turn;
    }
    else if (s == "canforetellcast")
    {
        intValue = (target->foretellTurn < 0)?0:(target->getObserver()->turn-target->foretellTurn); // Check if you can use the foretell cost from exile (CurrentTurn > ForetellTurn).
    }
    else if (s.find("hasability") != string::npos) //Return 1 if card has the specified ability
    {
        intValue = 0;
        for(size_t i = 0; i < Constants::NB_BASIC_ABILITIES; i++)
            if(Constants::MTGBasicAbilities[i] == s.substr(10))
                intValue = card->basicAbilities[i];
    }
    else if (s.find("hascnt") != string::npos) //Return the amount of specific counters on card (use "anycnt" to count all of them e.g. Nils, Discipline Enforcer)
    {
        intValue = 0;
        if (card->counters){
            Counters * counters = card->counters;
            for(size_t i = 0; i < counters->counters.size(); ++i){
                Counter * counter = counters->counters[i];
                if(s.substr(6) == "anycnt"){
                    intValue += counter->nb;
                } else if(counter->name == "" && (s.substr(6) == "11" || s.substr(6) == "-1-1")){
                    if((counter->power == 1 && counter->toughness == 1 && s.substr(6) == "11") || (counter->power == -1 && counter->toughness == -1 && s.substr(6) == "-1-1")){
                        intValue = counter->nb;
                        break;
                    }
                } else if(counter->name ==  s.substr(6)){
                    intValue = counter->nb;
                    break;
                }
            }
        }
    }
    else if (s.find("genrand") != string::npos) //Return a random value between 0 and a specific number (minus 1);
    {
        intValue = 0;
        WParsedInt * value = NEW WParsedInt(s.substr(7).c_str(), NULL, card);
        if(value){
            intValue = std::rand() % value->getValue();
            SAFE_DELETE(value);
        }
    }
    else if (s == "manacost") //Return the converted manacost
    {
        intValue = (target->currentZone == target->controller()->game->stack)?(target->myconvertedcost + target->castX):target->myconvertedcost;//X is 0 except if it's on the stack
    }
    else if(s == "snowdiffmana") //Return 1 if the difference between snowpool and converted manacost is more than 0
    {
        int snowpool = target->controller()->snowManaG + target->controller()->snowManaU + target->controller()->snowManaR + target->controller()->snowManaB + target->controller()->snowManaW + target->controller()->snowManaC;
        int manacost = (target->currentZone == target->controller()->game->stack)?(target->myconvertedcost + target->castX):target->myconvertedcost;//X is 0 except if it's on the stack
        intValue = (snowpool >= manacost)?1:0;
    }
    else if (s == "mysnowpoolcount" || s == "opponentsnowpoolcount") // snowpoolcount is just to count the number of snow mana produced ...
    {
        intValue = (s == "mysnowpoolcount")?(target->controller()->snowManaG + target->controller()->snowManaU + target->controller()->snowManaR + target->controller()->snowManaB + target->controller()->snowManaW + target->controller()->snowManaC):(target->controller()->opponent()->snowManaG + target->controller()->opponent()->snowManaU + target->controller()->opponent()->snowManaR + target->controller()->opponent()->snowManaB + target->controller()->opponent()->snowManaW + target->controller()->opponent()->snowManaC);
    }
    else if (s == "mysnowgreenpoolcount" || s == "opponentsnowgreenpoolcount")
    {
        intValue = (s == "mysnowgreenpoolcount")?target->controller()->snowManaG:target->controller()->opponent()->snowManaG;
    }
    else if (s == "mysnowredpoolcount" || s == "opponentsnowredpoolcount")
    {
        intValue = (s == "mysnowredpoolcount")?target->controller()->snowManaR:target->controller()->opponent()->snowManaR;
    }
    else if (s == "mysnowbluepoolcount" || s == "opponentsnowbluepoolcount")
    {
        intValue = (s == "mysnowbluepoolcount")?target->controller()->snowManaU:target->controller()->opponent()->snowManaU;
    }
    else if (s == "mysnowwhitepoolcount" || s == "opponentsnowwhitepoolcount")
    {
        intValue = (s == "mysnowwhitepoolcount")?target->controller()->snowManaW:target->controller()->opponent()->snowManaW;
    }
    else if (s == "mysnowblackpoolcount" || s == "opponentsnowblackpoolcount")
    {
        intValue = (s == "mysnowblackpoolcount")?target->controller()->snowManaB:target->controller()->opponent()->snowManaB;
    }
    else if (s == "mysnowcolorlesspoolcount" || s == "opponentsnowcolorlesspoolcount")
    {
        intValue = (s == "mysnowcolorlesspoolcount")?target->controller()->snowManaC:target->controller()->opponent()->snowManaC;
    }
    else if (s == "mypoolcount" || s == "opponentpoolcount") // total manapool
    {
        intValue = (s == "mypoolcount")?target->controller()->getManaPool()->getConvertedCost():target->controller()->opponent()->getManaPool()->getConvertedCost();
    }
    else if (s == "mygreenpoolcount" || s == "opponentgreenpoolcount")
    {
        intValue = (s == "mygreenpoolcount")?target->controller()->getManaPool()->getManaSymbols(Constants::MTG_COLOR_GREEN):target->controller()->opponent()->getManaPool()->getManaSymbols(Constants::MTG_COLOR_GREEN);
    }
    else if (s == "myredpoolcount" || s == "opponentredpoolcount")
    {
        intValue = (s == "myredpoolcount")?target->controller()->getManaPool()->getManaSymbols(Constants::MTG_COLOR_RED):target->controller()->opponent()->getManaPool()->getManaSymbols(Constants::MTG_COLOR_RED);
    }
    else if (s == "mybluepoolcount" || s == "opponentbluepoolcount")
    {
        intValue = (s == "mybluepoolcount")?target->controller()->getManaPool()->getManaSymbols(Constants::MTG_COLOR_BLUE):target->controller()->opponent()->getManaPool()->getManaSymbols(Constants::MTG_COLOR_BLUE);
    }
    else if (s == "mywhitepoolcount" || s == "opponentwhitepoolcount")
    {
        intValue = (s == "mywhitepoolcount")?target->controller()->getManaPool()->getManaSymbols(Constants::MTG_COLOR_WHITE):target->controller()->opponent()->getManaPool()->getManaSymbols(Constants::MTG_COLOR_WHITE);
    }
    else if (s == "myblackpoolcount" || s == "opponentblackpoolcount")
    {
        intValue = (s == "myblackpoolcount")?target->controller()->getManaPool()->getManaSymbols(Constants::MTG_COLOR_BLACK):target->controller()->opponent()->getManaPool()->getManaSymbols(Constants::MTG_COLOR_BLACK);
    }
    else if (s == "mycolorlesspoolcount" || s == "opponentcolorlesspoolcount")
    {
        intValue = (s == "mycolorlesspoolcount")?target->controller()->getManaPool()->getManaSymbols(Constants::MTG_COLOR_ARTIFACT):target->controller()->opponent()->getManaPool()->getManaSymbols(Constants::MTG_COLOR_ARTIFACT);
    }
    else if (s == "p" || s == "power")
    {
        intValue = target->getCurrentPower();
    }
    else if (s == "t" || s == "toughness")
    {
        intValue = target->getCurrentToughness();
    }
    else if (s == "countedamount" || s == "countedbamount")
    {
        intValue = (s == "countedamount")?target->CountedObjects:target->CountedObjectsB;
    }
    else if (s == "handsize" || s == "ohandsize")
    {
        intValue = (s == "ohandsize")?card->controller()->opponent()->handsize:target->controller()->handsize;
    }
    else if (s == "olandg" || s == "olandu")
    {
        intValue = (s == "olandg")?countManaProducedby(Constants::MTG_COLOR_GREEN, target, target->controller()->opponent()):countManaProducedby(Constants::MTG_COLOR_BLUE, target, target->controller()->opponent());
    }
    else if (s == "olandr" || s == "olandb")
    {
        intValue = (s == "olandr")?countManaProducedby(Constants::MTG_COLOR_RED, target, target->controller()->opponent()):countManaProducedby(Constants::MTG_COLOR_BLACK, target, target->controller()->opponent());
    }
    else if (s == "olandw" || s == "olandc")
    {
        intValue = (s == "olandw")?countManaProducedby(Constants::MTG_COLOR_WHITE, target, target->controller()->opponent()):(countManaProducedby(Constants::MTG_COLOR_ARTIFACT, target, target->controller()->opponent()) + countManaProducedby(Constants::MTG_COLOR_WASTE, target, target->controller()->opponent()));
    }
    else if (s == "plandg" || s == "plandu")
    {
        intValue = (s == "plandg")?countManaProducedby(Constants::MTG_COLOR_GREEN, target, target->controller()):countManaProducedby(Constants::MTG_COLOR_BLUE, target, target->controller());
    }
    else if (s == "plandr" || s == "plandb")
    {
        intValue = (s == "plandr")?countManaProducedby(Constants::MTG_COLOR_RED, target, target->controller()):countManaProducedby(Constants::MTG_COLOR_BLACK, target, target->controller());
    }
    else if (s == "plandw" || s == "plandc")
    {
        intValue = (s == "plandw")?countManaProducedby(Constants::MTG_COLOR_WHITE, target, target->controller()):(countManaProducedby(Constants::MTG_COLOR_ARTIFACT, target, target->controller()) + countManaProducedby(Constants::MTG_COLOR_WASTE, target, target->controller()));
    }
    else if (s == "cantargetmycre" || s == "cantargetoppocre")// can target my creature - can target opponent creature
    {
        intValue = (s == "cantargetmycre")?countCanTargetby("creature", card, card->controller()):countCanTargetby("creature", card, card->controller()->opponent());
    }
    else if (s == "cantargetcre" || s == "myname")// can target any creature - name of the card you control
    {
        intValue = (s == "cantargetcre")?countCanTargetby("creature", card, card->controller()) + countCanTargetby("creature", card, card->controller()->opponent()):countCardNameinZone(card->name,card->controller()->inPlay());
    }
    else if (s == "controllerturn")//intvalue = 1 if its your turn this(variable{controllerturn})
    {
        intValue = (target->controller() == target->getObserver()->currentPlayer)?1:0;
    }
    else if (s == "opponentturn")//intvalue = 1 if its your turn this(variable{opponentturn})
    {
        intValue = (target->controller()->opponent() == target->getObserver()->currentPlayer)?1:0;
    }
    else if (s == "phandcount" || s == "ohandcount")
    {
        intValue = (s == "phandcount")?target->controller()->game->hand->nb_cards:target->controller()->opponent()->game->hand->nb_cards;
    }
    else if (s == "urzatron")//Urza lands
    {
        intValue = (card->controller()->game->battlefield->hasAlias(4192) && card->controller()->game->battlefield->hasAlias(4193) && card->controller()->game->battlefield->hasAlias(4194))?1:0;
    }
    else if (s == "worshipped")//Worship
    {
        intValue = (card->controller()->game->battlefield->hasType("creature"))?card->controller()->life:0;
    }
    else if (s == "crewtotalpower")//crew count total power
    {
        intValue = 0;
        for (int j = card->controller()->game->battlefield->nb_cards - 1; j >= 0; --j)
        {
            MTGCardInstance * crew = card->controller()->game->battlefield->cards[j];
            if (crew != card && crew->isCreature() && !crew->isTapped() && !crew->isPhased && !crew->has(Constants::CANTCREW))
                intValue += crew->power;
        }
    }
    else if (s == "pancientooze")//Ancient Ooze
    {
        intValue = 0;
        for (int j = card->controller()->game->inPlay->nb_cards - 1; j >= 0; --j)
            if (card->controller()->game->inPlay->cards[j]->hasType(Subtypes::TYPE_CREATURE) && card->controller()->game->inPlay->cards[j] != card)
                intValue += card->controller()->game->inPlay->cards[j]->getManaCost()->getConvertedCost();
    }
    else if (s == "pdauntless")//Dauntless Dourbark
    {
        MTGGameZone * checkZone = card->controller()->inPlay();
        intValue =
            countCardTypeinZone("forest",checkZone) +
            countCardTypeinZone("treefolk",checkZone);
    }
    else if (s == "pbasiclandtypes")//Basic Land types
    {
        MTGGameZone * checkZone = card->controller()->inPlay();
        intValue = //mtg rules 205.4c
            cardHasTypeinZone("waste", checkZone) +
            cardHasTypeinZone("forest", checkZone) +
            cardHasTypeinZone("plains", checkZone) +
            cardHasTypeinZone("swamp", checkZone) +
            cardHasTypeinZone("island", checkZone) +
            cardHasTypeinZone("mountain", checkZone) +
            cardHasTypeinZone("snow-covered forest", checkZone) +
            cardHasTypeinZone("snow-covered plains", checkZone) +
            cardHasTypeinZone("snow-covered swamp", checkZone) +
            cardHasTypeinZone("snow-covered island", checkZone) +
            cardHasTypeinZone("snow-covered mountain", checkZone);
    }
    else if (s == "pdomain")//player domain
    {
        MTGGameZone * checkZone = card->controller()->inPlay();
        intValue = cardHasTypeinZone("forest", checkZone) +
            cardHasTypeinZone("plains", checkZone) +
            cardHasTypeinZone("swamp", checkZone) +
            cardHasTypeinZone("island", checkZone) +
            cardHasTypeinZone("mountain", checkZone);
    }
    else if (s == "odomain")//opponent domain
    {
        MTGGameZone * checkZone = card->controller()->opponent()->inPlay();
        intValue = cardHasTypeinZone("forest", checkZone) +
            cardHasTypeinZone("plains", checkZone) +
            cardHasTypeinZone("swamp", checkZone) +
            cardHasTypeinZone("island", checkZone) +
            cardHasTypeinZone("mountain", checkZone);
    }
    else if (s == "allmyname")//Plague Rats and others
    {
        intValue = 0;
        for (int i = 0; i < 2; i++)
            intValue += countCardNameinZone(card->name,card->getObserver()->players[i]->game->battlefield);
    }
    else if (s == "pgbzombie")//Soulless One
    {
        intValue = 0;
        for (int i = 0; i < 2; i++)
        {
            intValue += countCardTypeinZone("zombie",card->getObserver()->players[i]->game->graveyard);
            intValue += countCardTypeinZone("zombie",card->getObserver()->players[i]->game->battlefield);
        }
    }
    else if (s == "pginstantsorcery")//Spellheart Chimera
    {
        intValue = 0;
        for (int j = card->controller()->game->graveyard->nb_cards - 1; j >= 0; --j)
        {
            if (card->controller()->game->graveyard->cards[j]->hasType(Subtypes::TYPE_INSTANT)
                ||card->controller()->game->graveyard->cards[j]->hasType(Subtypes::TYPE_SORCERY))
                intValue += 1;
        }
    }
    else if (s == "pgmanainstantsorcery")//Inferno Project
    {
        intValue = 0;
        for (int j = card->controller()->game->graveyard->nb_cards - 1; j >= 0; --j)
        {
            if (card->controller()->game->graveyard->cards[j]->hasType(Subtypes::TYPE_INSTANT)
                ||card->controller()->game->graveyard->cards[j]->hasType(Subtypes::TYPE_SORCERY))
                intValue += card->controller()->game->graveyard->cards[j]->myconvertedcost;
        }
    }
    else if (s == "powertotalinplay" || s == "pwrtotalinplay")//Count Total Power of Creatures you control... Formidable
    {
        intValue = 0;
        for (int j = card->controller()->game->inPlay->nb_cards - 1; j >= 0; --j)
        {
            if (card->controller()->game->inPlay->cards[j]->hasType(Subtypes::TYPE_CREATURE))
                intValue += card->controller()->game->inPlay->cards[j]->getCurrentPower();
        }
    }
    else if (s == "toughnesstotalinplay" || s == "thstotalinplay")//Count Total toughness of Creatures you control...
    {
        intValue = 0;
        for (int j = card->controller()->game->inPlay->nb_cards - 1; j >= 0; --j)
        {
            if (card->controller()->game->inPlay->cards[j]->hasType(Subtypes::TYPE_CREATURE))
                intValue += card->controller()->game->inPlay->cards[j]->getCurrentToughness();
        }
    }
    else if (s == "calculateparty")//Count Total Creatures of party (a party consists of up to one each of Cleric, Rogue, Warrior, and Wizard)... Zendikar Rising
    {
        intValue = 0;
        bool found_cleric = false;
        bool found_warrior = false;
        bool found_wizard = false;
        bool found_rogue = false;
        for (int j = card->controller()->game->inPlay->nb_cards - 1; j >= 0 && intValue < 4; --j)
        {
            if (card->controller()->game->inPlay->cards[j]->hasType("Cleric") && !found_cleric){
                intValue += 1;
                found_cleric = true;
                continue;
            }
            if (card->controller()->game->inPlay->cards[j]->hasType("Rogue") && !found_rogue){
                intValue += 1;
                found_rogue = true;
                continue;
            }
            if (card->controller()->game->inPlay->cards[j]->hasType("Wizard") && !found_wizard){
                intValue += 1;
                found_wizard = true;
                continue;
            }
            if (card->controller()->game->inPlay->cards[j]->hasType("Warrior") && !found_warrior){
                intValue += 1;
                found_warrior = true;
                continue;
            }
        }
    }
    else if (s.find("cardcountabil") != string::npos)//Count Total cards with specific ability
    {
        intValue = 0;
        bool different_names = (s.find("diffcardcountabil")!=string::npos)?true:false;
        string ability = (s.find("diffcardcountabil")!=string::npos)?s.substr(17):s.substr(13);
        vector<string> list;
        for (int j = card->controller()->game->inPlay->nb_cards - 1; j >= 0; --j)
        {
            if (card->controller()->game->inPlay->cards[j]->basicAbilities[Constants::GetBasicAbilityIndex(ability)] == 1){
                if(!different_names)
                    intValue += 1;
                else{
                    bool name_found = false;
                    for(unsigned int i = 0; i < list.size() && !name_found; i++){
                        if(list[i] == card->controller()->game->inPlay->cards[j]->name)
                            name_found = true;
                    }
                    if(!name_found){
                        list.push_back(card->controller()->game->inPlay->cards[j]->name);
                        intValue += 1;
                    }
                }
            }
        }
    }
    else if (s.find("cardcounttype") != string::npos)//Count Total cards of specific type
    {
        intValue = 0;
        bool different_names = (s.find("diffcardcounttype")!=string::npos)?true:false;
        string type = (s.find("diffcardcounttype")!=string::npos)?s.substr(17):s.substr(13);
        vector<string> list;
        for (int j = card->controller()->game->inPlay->nb_cards - 1; j >= 0; --j)
        {
            if (card->controller()->game->inPlay->cards[j]->hasType(type)){
                if(!different_names)
                    intValue += 1;
                else{
                    bool name_found = false;
                    for(unsigned int i = 0; i < list.size() && !name_found; i++){
                        if(list[i] == card->controller()->game->inPlay->cards[j]->name)
                            name_found = true;
                    }
                    if(!name_found){
                        list.push_back(card->controller()->game->inPlay->cards[j]->name);
                        intValue += 1;
                    }
                }
            }
        }
    }
    else if (s.find("sametypecreatures") != string::npos)//Count the greatest number creatures that share same subtype
    {
        intValue = 0;
        bool opponent = (s.find("oppsametypecreatures")!=string::npos)?true:false;
        vector<int> list;
        vector<string> values = MTGAllCards::getCreatureValuesById();
        for (size_t i = 0; i < values.size(); ++i){
            list.push_back(0);
            if(opponent){
                for (int j = card->controller()->opponent()->game->inPlay->nb_cards - 1; j >= 0; --j){
                    if (card->controller()->opponent()->game->inPlay->cards[j]->hasType(values[i])){
                        list[i]++;
                    }
                }
            } else {
                for (int j = card->controller()->game->inPlay->nb_cards - 1; j >= 0; --j){
                    if (card->controller()->game->inPlay->cards[j]->hasType(values[i])){
                        list[i]++;
                    }
                }
            }
            if (list[i] > intValue)
                intValue = list[i];
        }
    }
    else if ((s == "cursedscrollresult" || s == "magusofscrollresult") && (card->controller()->game->hand->nb_cards > 0))//return 1 if the choosen card has to give damage (e.g. Cursed Scroll, Magus od the Scroll).
    {
        intValue = (card->controller()->game->hand->cards[std::rand() % card->controller()->game->hand->nb_cards]->name == card->name)?1:0;
    }
    else if (s == "mypos" || s == "bushidopoints")//hand,exile,grave & library only (library zpos is inverted so the recent one is always the top) -- bushido point
    {
        intValue = (s == "mypos")?card->zpos:card->bushidoPoints;
    }
    else if ((s == "revealedp" || s == "revealedt" || s == "revealedmana") && card->revealedLast)
    {
        if (s == "revealedp" || s == "revealedt")
            intValue = (s == "revealedp")?card->revealedLast->power:card->revealedLast->toughness;
        else
            intValue = card->revealedLast->getManaCost()->getConvertedCost();
    }
    else if (s.find("findfirsttype") != string::npos)//find the index of first card with specified type in target player library
    {
        intValue = 0;
        bool opponent = (s.find("oppofindfirsttype")!=string::npos)?true:false;
        string type = (s.find("oppofindfirsttype")!=string::npos)?s.substr(17):s.substr(13);
        bool negate = (type.find("non")!=string::npos)?true:false;
        type = negate?type.substr(3):type;
        Player* p = card->controller();
        if (opponent)
            p = card->controller()->opponent();
        for (int j = p->game->library->nb_cards - 1; j >= 0; --j){
            if (type == "permanent" && !negate && !p->game->library->cards[j]->hasType(Subtypes::TYPE_INSTANT) && !p->game->library->cards[j]->hasType(Subtypes::TYPE_SORCERY)){
                intValue = p->game->library->nb_cards - j;
                break;
            }
            else if (type == "permanent" && negate && (p->game->library->cards[j]->hasType(Subtypes::TYPE_INSTANT) || p->game->library->cards[j]->hasType(Subtypes::TYPE_SORCERY))){
                intValue = p->game->library->nb_cards - j;
                break;
            }
            else if (type != "permanent" && !negate && p->game->library->cards[j]->hasType(type)){
                intValue = p->game->library->nb_cards - j;
                break;
            }
            else if (type != "permanent" && negate && !p->game->library->cards[j]->hasType(type)){
                intValue = p->game->library->nb_cards - j;
                break;
            }
        }
    }
    else if (s.find("findlasttype") != string::npos)//find the index of latest card with specified type in target player graveyard
    {
        intValue = 0;
        bool opponent = (s.find("oppofindlasttype")!=string::npos)?true:false;
        string type = (s.find("oppofindlasttype")!=string::npos)?s.substr(16):s.substr(12);
        bool negate = (type.find("non")!=string::npos)?true:false;
        type = negate?type.substr(3):type;
        Player* p = card->controller();
        if (opponent)
            p = card->controller()->opponent();
        for (int j = p->game->graveyard->nb_cards - 1; j >= 0; --j){
            if (type == "permanent" && !negate && !p->game->graveyard->cards[j]->hasType(Subtypes::TYPE_INSTANT) && !p->game->graveyard->cards[j]->hasType(Subtypes::TYPE_SORCERY)){
                intValue = j + 1;
                break;
            }
            else if (type == "permanent" && negate && (p->game->graveyard->cards[j]->hasType(Subtypes::TYPE_INSTANT) || p->game->graveyard->cards[j]->hasType(Subtypes::TYPE_SORCERY))){
                intValue = j + 1;
                break;
            }
            else if (type != "permanent" && !negate && p->game->graveyard->cards[j]->hasType(type)){
                intValue = j + 1;
                break;
            }
            else if (type != "permanent" && negate && !p->game->graveyard->cards[j]->hasType(type)){
                intValue = j + 1;
                break;
            }
        }
    }
    else if (s == "scryedcards" || s == "numoftypes")//returns how many card have been scryed from current card -- returns the number of types of the card
    {
        if(s == "scryedcards")
            intValue = card->scryedCards;
        else {
            intValue = 0;
            if(card->hasType(Subtypes::TYPE_PLANESWALKER))
                intValue++;
            if(card->hasType(Subtypes::TYPE_TRIBAL))
                intValue++;
            if(card->hasType(Subtypes::TYPE_SORCERY))
                intValue++;
            if(card->hasType(Subtypes::TYPE_LAND))
                intValue++;
            if(card->hasType(Subtypes::TYPE_INSTANT))
                intValue++;
            if(card->hasType(Subtypes::TYPE_ENCHANTMENT))
                intValue++;
            if(card->hasType(Subtypes::TYPE_CREATURE))
                intValue++;
            if(card->hasType(Subtypes::TYPE_ARTIFACT))
                intValue++;
            if(card->hasType(Subtypes::TYPE_BATTLE))
                intValue++;
        }
    }
    else if (s == "pcycledcount" || s == "ocycledcount") //return how may cards have been cycled this turn from a specific player.
    {
        intValue = (s == "pcycledcount")?card->controller()->cycledCount:card->controller()->opponent()->cycledCount;
    }
    else //Continue parsing in another method to avoid compiler C1061 error.
    {
        extendedParse(s, spell, card);
    }
    if (intValue > 0)//dont divide by 0 the rest are valid.
    {
        if (halfup)
        {
            if (intValue % 2 == 1)
                intValue++;
            intValue = intValue / 2;
        }
        if (halfdown)
            intValue = intValue / 2;

        if (thirdup)
        {
            if (intValue % 3 > 0)
                intValue = (intValue / 3) + 1;
            else 
                intValue = intValue / 3;
        }
        if (thirddown)
            intValue = intValue / 3;
    }
    if (twice)
        intValue = intValue * 2;
    if (thrice)
        intValue = intValue * 3;
    if (fourtimes)
        intValue = intValue * 4;
    if (fivetimes)
        intValue = intValue * 5;
    if (intValue < 0)
    {
        //we remove "-" at the start and are parsing for real values.
        //if we ended up with a value less than 0, then we return just 0
        intValue = 0;
    }

    intValue *= multiplier;
}

void WParsedInt::extendedParse(string s, Spell * spell, MTGCardInstance * card)
{
    if (s == "mybattlefieldcardtypes" || s == "oppbattlefieldcardtypes" || s == "allbattlefieldcardtypes")//Count number of card types on battlefield
    {
        intValue = 0;
        int pc = 0, tc = 0, sc = 0, lc = 0, ic = 0, ec = 0, cc = 0, ac = 0;
        if(s == "allbattlefieldcardtypes") {
            for (int j = 0; j < 2; j++) {
                MTGGameZone * checkZone = card->getObserver()->players[j]->game->inPlay;
                if(cardHasTypeinZone("planeswalker",checkZone))
                    pc = 1;
                if(cardHasTypeinZone("tribal",checkZone))
                    tc = 1;
                if(cardHasTypeinZone("sorcery",checkZone))
                    sc = 1;
                if(cardHasTypeinZone("land",checkZone))
                    lc = 1;
                if(cardHasTypeinZone("instant",checkZone))
                    ic = 1;
                if(cardHasTypeinZone("enchantment",checkZone))
                    ec = 1;
                if(cardHasTypeinZone("creature",checkZone))
                    cc = 1;
                if(cardHasTypeinZone("artifact",checkZone))
                    ac = 1;
                if(cardHasTypeinZone("battle",checkZone))
                    ac = 1;
            }
        } else {
            MTGGameZone * checkZone = (s.find("oppbattlefieldcardtypes")!=string::npos)?card->getObserver()->opponent()->game->inPlay:card->controller()->game->inPlay;
            if(cardHasTypeinZone("planeswalker",checkZone))
                pc = 1;
            if(cardHasTypeinZone("tribal",checkZone))
                tc = 1;
            if(cardHasTypeinZone("sorcery",checkZone))
                sc = 1;
            if(cardHasTypeinZone("land",checkZone))
                lc = 1;
            if(cardHasTypeinZone("instant",checkZone))
                ic = 1;
            if(cardHasTypeinZone("enchantment",checkZone))
                ec = 1;
            if(cardHasTypeinZone("creature",checkZone))
                cc = 1;
            if(cardHasTypeinZone("artifact",checkZone))
                ac = 1;
            if(cardHasTypeinZone("battle",checkZone))
                ac = 1;
        }
        intValue = pc+tc+sc+lc+ic+ec+cc+ac;
    }
    else if (s == "mygravecardtypes" || s == "oppgravecardtypes" || s == "allgravecardtypes")//Count number of card types in graveyards
    {
        intValue = 0;
        int pc = 0, tc = 0, sc = 0, lc = 0, ic = 0, ec = 0, cc = 0, ac = 0;
        if(s == "allgravecardtypes") {
            for (int j = 0; j < 2; j++) {
                MTGGameZone * checkZone = card->getObserver()->players[j]->game->graveyard;
                if(cardHasTypeinZone("planeswalker",checkZone))
                    pc = 1;
                if(cardHasTypeinZone("tribal",checkZone))
                    tc = 1;
                if(cardHasTypeinZone("sorcery",checkZone))
                    sc = 1;
                if(cardHasTypeinZone("land",checkZone))
                    lc = 1;
                if(cardHasTypeinZone("instant",checkZone))
                    ic = 1;
                if(cardHasTypeinZone("enchantment",checkZone))
                    ec = 1;
                if(cardHasTypeinZone("creature",checkZone))
                    cc = 1;
                if(cardHasTypeinZone("artifact",checkZone))
                    ac = 1;
                if(cardHasTypeinZone("battle",checkZone))
                    ac = 1;
            }
        } else {
            MTGGameZone * checkZone = (s.find("oppgravecardtypes")!=string::npos)?card->getObserver()->opponent()->game->graveyard:card->controller()->game->graveyard;
            if(cardHasTypeinZone("planeswalker",checkZone))
                pc = 1;
            if(cardHasTypeinZone("tribal",checkZone))
                tc = 1;
            if(cardHasTypeinZone("sorcery",checkZone))
                sc = 1;
            if(cardHasTypeinZone("land",checkZone))
                lc = 1;
            if(cardHasTypeinZone("instant",checkZone))
                ic = 1;
            if(cardHasTypeinZone("enchantment",checkZone))
                ec = 1;
            if(cardHasTypeinZone("creature",checkZone))
                cc = 1;
            if(cardHasTypeinZone("artifact",checkZone))
                ac = 1;
            if(cardHasTypeinZone("battle",checkZone))
                ac = 1;
        }
        intValue = pc+tc+sc+lc+ic+ec+cc+ac;
    }
    else if (s.find("totcnt") != string::npos) //Return the total amount of all specific counters on each card (use "anycnt" to count all of them e.g. Deepwood Denizen)
    {
        intValue = 0;
        for (int j = card->controller()->game->inPlay->nb_cards - 1; j >= 0; --j){
            if ((s.find("totcntcre") != string::npos && card->controller()->game->inPlay->cards[j]->hasType(Subtypes::TYPE_CREATURE)) || 
                (s.find("totcntpla") != string::npos && card->controller()->game->inPlay->cards[j]->hasType(Subtypes::TYPE_PLANESWALKER)) || 
                (s.find("totcntart") != string::npos && card->controller()->game->inPlay->cards[j]->hasType(Subtypes::TYPE_ARTIFACT)) ||
                (s.find("totcntenc") != string::npos && card->controller()->game->inPlay->cards[j]->hasType(Subtypes::TYPE_ENCHANTMENT)) ||
                (s.find("totcntlan") != string::npos && card->controller()->game->inPlay->cards[j]->hasType(Subtypes::TYPE_LAND)) || 
                (s.find("totcntbat") != string::npos && card->controller()->game->inPlay->cards[j]->hasType(Subtypes::TYPE_BATTLE)) || 
                s.find("totcntall") != string::npos){
                if (card->controller()->game->inPlay->cards[j]->counters){
                    Counters * counters = card->controller()->game->inPlay->cards[j]->counters;
                    for(size_t i = 0; i < counters->counters.size(); ++i){
                        Counter * counter = counters->counters[i];
                        if(s.substr(9) == "anycnt"){
                            intValue += counter->nb;
                        } else if(counter->name == "" && (s.substr(9) == "11" || s.substr(9) == "-1-1")){
                            if((counter->power == 1 && counter->toughness == 1 && s.substr(9) == "11") || (counter->power == -1 && counter->toughness == -1 && s.substr(9) == "-1-1")){
                                intValue += counter->nb;
                                break;
                            }
                        } else if(counter->name ==  s.substr(9)){
                            intValue += counter->nb;
                            break;
                        }
                    }
                }
            }
        }
    }
    else if (s.find("totalmana") != string::npos)//find the cards with specified total mana in target player library
    {
        intValue = 0;
        bool opponent = (s.find("oppototalmana")!=string::npos)?true:false;
        int manavalue = atoi((s.find("oppototalmana")!=string::npos)?s.substr(13).c_str():s.substr(9).c_str());
        int totalmana = 0;
        Player* p = card->controller();
        if (opponent)
            p = card->controller()->opponent();
        for (int j = p->game->library->nb_cards - 1; j >= 0  && totalmana < manavalue; --j){
            totalmana += p->game->library->cards[j]->getManaCost()->getConvertedCost();
            intValue = p->game->library->nb_cards - j;
        }
    }
    else if (s == "pdungeoncompleted" || s == "odungeoncompleted")
    {
        intValue = (s == "pdungeoncompleted")?card->controller()->dungeonCompleted:card->controller()->opponent()->dungeonCompleted;
    }
    else if (s == "pinitiative" || s == "oinitiative") // Which player has the initiative
    {
        intValue = (s == "pinitiative")?card->controller()->initiative:card->controller()->opponent()->initiative;
    }
    else if (s == "pwrtotatt" || s == "thstotatt")//count Total Power or toughness of attacking creatures (e.g. Battle Cry Goblin)
    {
        intValue = 0;
        for (int j = card->controller()->game->inPlay->nb_cards - 1; j >= 0; --j)
        {
            if (card->controller()->game->inPlay->cards[j]->hasType(Subtypes::TYPE_CREATURE) && card->controller()->game->inPlay->cards[j]->attacker){
                if(s == "pwrtotatt")
                    intValue += card->controller()->game->inPlay->cards[j]->getCurrentPower();
                else
                    intValue += card->controller()->game->inPlay->cards[j]->getCurrentToughness();
            }
        }
    }
    else if (s == "pwrtotblo" || s == "thstotblo")//count Total Power or toughness of blocking creatures
    {
        intValue = 0;
        for (int j = card->controller()->game->inPlay->nb_cards - 1; j >= 0; --j)
        {
            if (card->controller()->game->inPlay->cards[j]->hasType(Subtypes::TYPE_CREATURE) && card->controller()->game->inPlay->cards[j]->defenser){
                if(s == "pwrtotblo")
                    intValue += card->controller()->game->inPlay->cards[j]->getCurrentPower();
                else
                    intValue += card->controller()->game->inPlay->cards[j]->getCurrentToughness();
            }
        }
    }
    else if (s == "ishuman" || s == "mycolnum")//return if controller is Human or AI - return the number of colors of a card.
    {
        intValue = 0;
        if (s == "ishuman")
            intValue = (card->controller()->isAI())?0:1;
        else if (s == "mycolnum") {
            for (int i = Constants::MTG_COLOR_GREEN; i <= Constants::MTG_COLOR_WHITE; ++i){
                if(card->hasColor(i))
                    intValue +=1;
            }
        }
    }
    else if (s == "pexperience" || s == "oexperience")
    {
        intValue = (s == "pexperience")?card->controller()->experienceCount:card->controller()->opponent()->experienceCount;
    }
    else if (s == "plastshlturn" || s == "olastshlturn")
    {
        intValue = (s == "plastshlturn")?card->controller()->lastShuffleTurn:card->controller()->opponent()->lastShuffleTurn;
    }
    else if (s == "hasprey" || s == "dualfaced" || s == "withpartner" || s == "totaldmg")
    {
        if (s == "hasprey")
            intValue = (card->hauntedCard)?1:0;
        else if (s == "dualfaced")
            intValue = (card->backSide != "")?1:0;
        else if (s == "withpartner")
            intValue = (card->partner != "")?1:0;
        else if (s == "totaldmg")
            intValue = (card->damageToController + card->damageToCreature + card->damageToOpponent);
    }
    else if (s.find("totalcololorsinplay") != string::npos || s.find("oppototalcololorsinplay") != string::npos) //Return the total amount of colors on controller or opponent battlefield (e.g. Moonveil Regent)
    {
        intValue = 0;
        bool blueFound = false;
        bool redFound = false;
        bool whiteFound = false;
        bool greenFound = false;
        bool blackFound = false;
        Player* p = card->controller();
        if (s == "oppototalcololorsinplay")
            p = card->controller()->opponent();
        for( int j = 0; j < p->inPlay()->nb_cards; j++){
            if(p->inPlay()->cards[j]->hasColor(Constants::MTG_COLOR_RED) && !redFound){
                intValue++;
                redFound = true;
            }
            if(p->inPlay()->cards[j]->hasColor(Constants::MTG_COLOR_BLACK) && !blackFound){
                intValue++;
                blackFound = true;
            }
            if(p->inPlay()->cards[j]->hasColor(Constants::MTG_COLOR_BLUE) && !blueFound){
                intValue++;
                blueFound = true;
            }
            if(p->inPlay()->cards[j]->hasColor(Constants::MTG_COLOR_GREEN) && !greenFound){
                intValue++;
                greenFound = true;
            }
            if(p->inPlay()->cards[j]->hasColor(Constants::MTG_COLOR_WHITE) && !whiteFound){
                intValue++;
                whiteFound = true;
            }
        }
    }
    else if(s.find("pcoven") != string::npos || s.find("ocoven") != string::npos){ //Player or opponent controls three or more creatures with different powers (e.g. Augur of Autumn)
        intValue = 0;
        bool opponent = (s.find("ocoven")!=string::npos)?true:false;
        Player* p = card->controller();
        if (opponent)
            p = card->controller()->opponent();
        for(unsigned int i = 0; i < p->game->inPlay->cards.size() && intValue == 0; i++){
            if(p->game->inPlay->cards[i]->hasType(Subtypes::TYPE_CREATURE)){
                for(unsigned int j = i+1; j < p->game->inPlay->cards.size() && intValue == 0; j++){
                    if(p->game->inPlay->cards[j]->hasType(Subtypes::TYPE_CREATURE) && p->game->inPlay->cards[j]->power != p->game->inPlay->cards[i]->power){
                        for(unsigned int k = j+1; k < p->game->inPlay->cards.size() && intValue == 0; k++){
                            if(p->game->inPlay->cards[k]->hasType(Subtypes::TYPE_CREATURE) && (p->game->inPlay->cards[k]->power != p->game->inPlay->cards[i]->power && p->game->inPlay->cards[k]->power != p->game->inPlay->cards[j]->power)){
                                intValue = 1;
                            }
                        }
                    }
                }
            }
        }
    }
    else if(s.find("pnumcreswp") != string::npos || s.find("onumcreswp") != string::npos){ //Number of creatures that have toughness greater than their power.
        intValue = 0;
        bool opponent = (s.find("onumcreswp")!=string::npos)?true:false;
        Player* p = card->controller();
        if (opponent)
            p = card->controller()->opponent();
        for(unsigned int i = 0; i < p->game->inPlay->cards.size(); i++){
            if(p->game->inPlay->cards[i]->hasType(Subtypes::TYPE_CREATURE) && p->game->inPlay->cards[i]->toughness > p->game->inPlay->cards[i]->power){
                intValue++;
            }
        }
    }
    else if(s.find("startingplayer") != string::npos){ // Return who was the starting player (0 is controller, 1 is opponent).
        intValue = card->controller()->getObserver()->turn%2;
        if(card->controller()->getObserver()->currentlyActing() != card->controller())
            intValue = 1 - intValue;
    }
    else if (s == "pinstsorcount" || s == "oinstsorcount") //Return the number of instant or sorceries that were casted this turn by controller or opponent.
    {
        intValue = (s == "pinstsorcount")?card->controller()->game->stack->seenThisTurn("*[instant;sorcery]", Constants::CAST_ALL):card->controller()->opponent()->game->stack->seenThisTurn("*[instant;sorcery]", Constants::CAST_ALL);
    }
    else if ((s.find("palldead") != string::npos) || (s.find("oalldead") != string::npos)) //Return the number of cards of a specific type that died this turn for controller or opponent.
    {
        int hasdeadtype = 0;
        MTGGameZone * grave = (s.find("oalldead") != string::npos)?card->controller()->opponent()->game->graveyard:card->controller()->game->graveyard;
        Player * checkCurrent = (s.find("oalldead") != string::npos)?card->controller()->opponent():card->controller();
        string checktype = s.substr(8);
        for(unsigned int gy = 0; gy < grave->cardsSeenThisTurn.size(); gy++)
        {
            MTGCardInstance * checkCard = grave->cardsSeenThisTurn[gy];
            if(checkCard->hasType(checktype) &&
                ((checkCard->previousZone == checkCurrent->game->battlefield)||
                (checkCard->previousZone == checkCurrent->opponent()->game->battlefield)) //died from battlefield
                )
            {
                hasdeadtype++;
            }
        }
        intValue = hasdeadtype;
    }
    else if (s.find("bothalldead") != string::npos) //Return the number of cards of a specific type that died this turn.
    {
        int hasdeadtype = 0;
        string checktype = s.substr(11);
        for(int cp = 0; cp < 2; cp++)
        {
            Player * checkCurrent = card->getObserver()->players[cp];
            MTGGameZone * grave = checkCurrent->game->graveyard;
            for(unsigned int gy = 0; gy < grave->cardsSeenThisTurn.size(); gy++)
            {
                MTGCardInstance * checkCard = grave->cardsSeenThisTurn[gy];
                if(checkCard->hasType(checktype) &&
                    ((checkCard->previousZone == checkCurrent->game->battlefield)||
                    (checkCard->previousZone == checkCurrent->opponent()->game->battlefield))//died from battlefield
                    )
                {
                    hasdeadtype++;
                }
            }
        }
        intValue = hasdeadtype;
    }
    else if(s.find("hasmansym") != string::npos){
        string manatocheck = s.substr(9);
        if(manatocheck == "c")
            intValue = card->getManaCost()->getManaSymbolsHybridMerged(Constants::MTG_COLOR_ARTIFACT);
        else if(manatocheck == "g")
            intValue = card->getManaCost()->getManaSymbolsHybridMerged(Constants::MTG_COLOR_GREEN);
        else if(manatocheck == "u")
            intValue = card->getManaCost()->getManaSymbolsHybridMerged(Constants::MTG_COLOR_BLUE);
        else if(manatocheck == "r")
            intValue = card->getManaCost()->getManaSymbolsHybridMerged(Constants::MTG_COLOR_RED);
        else if(manatocheck == "b")
            intValue = card->getManaCost()->getManaSymbolsHybridMerged(Constants::MTG_COLOR_BLACK);
        else if(manatocheck == "w")
            intValue = card->getManaCost()->getManaSymbolsHybridMerged(Constants::MTG_COLOR_WHITE);
        else intValue = 0;
    }
    else if(s.find("prodmana") != string::npos){
        intValue = 0;
        string manatocheck = s.substr(8);
        if(card->getProducedMana()){
            if(manatocheck == "c")
                intValue = card->getProducedMana()->getManaSymbolsHybridMerged(Constants::MTG_COLOR_ARTIFACT);
            else if(manatocheck == "g")
                intValue = card->getProducedMana()->getManaSymbolsHybridMerged(Constants::MTG_COLOR_GREEN);
            else if(manatocheck == "u")
                intValue = card->getProducedMana()->getManaSymbolsHybridMerged(Constants::MTG_COLOR_BLUE);
            else if(manatocheck == "r")
                intValue = card->getProducedMana()->getManaSymbolsHybridMerged(Constants::MTG_COLOR_RED);
            else if(manatocheck == "b")
                intValue = card->getProducedMana()->getManaSymbolsHybridMerged(Constants::MTG_COLOR_BLACK);
            else if(manatocheck == "w")
                intValue = card->getProducedMana()->getManaSymbolsHybridMerged(Constants::MTG_COLOR_WHITE);
            else if(manatocheck == "tot")
                intValue = card->getProducedMana()->getConvertedCost();
        }
    }
    else if(s.find("usedmana") != string::npos){
        intValue = 0;
        string manatocheck = s.substr(8);
        if(card->getManaCost() && card->getManaCost()->getManaUsedToCast()){
            if(manatocheck == "g")
                intValue = card->getManaCost()->getManaUsedToCast()->getManaSymbols(Constants::MTG_COLOR_GREEN);
            else if(manatocheck == "u")
                intValue = card->getManaCost()->getManaUsedToCast()->getManaSymbols(Constants::MTG_COLOR_BLUE);
            else if(manatocheck == "r")
                intValue = card->getManaCost()->getManaUsedToCast()->getManaSymbols(Constants::MTG_COLOR_RED);
            else if(manatocheck == "b")
                intValue = card->getManaCost()->getManaUsedToCast()->getManaSymbols(Constants::MTG_COLOR_BLACK);
            else if(manatocheck == "w")
                intValue = card->getManaCost()->getManaUsedToCast()->getManaSymbols(Constants::MTG_COLOR_WHITE);
            else if(manatocheck == "tot")
                intValue = card->getManaCost()->getManaUsedToCast()->getConvertedCost();
        }
    }
    else if(s.find("toxicity") != string::npos){ //Return the toxicity of card.
        intValue = card->getToxicity();
    }
    else if (s.find("ninelands") != string::npos) //Count the number of lands with different names among the Basic, Sphere, and Locus lands you control.
    {
        intValue = 0;
        vector<string> list;
        for (int j = card->controller()->game->inPlay->nb_cards - 1; j >= 0; --j)
        {
            if (card->controller()->game->inPlay->cards[j]->isLand() && 
                (card->controller()->game->inPlay->cards[j]->hasType("basic") || card->controller()->game->inPlay->cards[j]->hasType("sphere") || card->controller()->game->inPlay->cards[j]->hasType("locus"))){
                bool name_found = false;
                for(unsigned int i = 0; i < list.size() && !name_found; i++){
                    if(list[i] == card->controller()->game->inPlay->cards[j]->name)
                        name_found = true;
                }
                if(!name_found){
                    list.push_back(card->controller()->game->inPlay->cards[j]->name);
                    intValue += 1;
                }
            }
        }
    }
    else if (s == "pringtemptations" || s == "oringtemptations") //How many times the player has been tempted by the Ring.
    {
        intValue = (s == "pringtemptations")?card->controller()->ringTemptations:card->controller()->opponent()->ringTemptations;
    }
    else if (s == "iscommander" || s == "ringbearer") //Return 1 if card is the commander -- Return 1 if card is the Ring bearer
    {
        intValue = (s == "iscommander")?card->isCommander:card->isRingBearer;
    }
    else if (s == "oppotgt" || s == "ctrltgt") //Return 1 if card targeted the opponent -- Return 1 if card targeted its controller
    {
        intValue = 0;
        Player* p = (s == "oppotgt")?card->controller()->opponent():card->controller();
        if(card->playerTarget == p)
            intValue = 1;
    }
    else if (s == "isattacker" || s == "couldattack") //Return 1 if creature is attacking. -- Return 1 if creature can attack.
    {
        intValue = (s == "isattacker")?card->isAttacker():card->canAttack();
    }
    else if (s == "kicked") //Return the number of times kicker has been paid
    {
        intValue = card->kicked;
    }
    else if(!intValue) //Found nothing, try parsing a atoi
    {
        intValue = atoi(s.c_str());
    }
}

int WParsedInt::countDevotionTo(MTGCardInstance * card, MTGGameZone * zone, int color1, int color2)
{
    int counthybrid = 0;
    TargetChooserFactory dtf(card->getObserver());
    TargetChooser * dtc = dtf.createTargetChooser("*",NULL);
    if (dtc->targetsZone(zone, card))
    {
        counthybrid += zone->countDevotion(dtc, color1, color2);
    }
    SAFE_DELETE(dtc);
    return card->controller()->devotionOffset + counthybrid; // Increase total devotion with an offset (e.g. Altar of the Pantheon)
}

int WParsedInt::countCardNameinZone(string name, MTGGameZone * zone)
{
    int count = 0;
    for( int i= 0; i < zone->nb_cards; i ++)
        if(zone->cards[i]->name == name)
            count += 1;
    return count;
}

int WParsedInt::countCardsInPlaybyColor(int color, GameObserver * observer)
{
    int count = 0;
    for (int i = 0; i < 2; i++)
    {
        for( int j= 0; j < observer->players[i]->inPlay()->nb_cards; j++)
            if(observer->players[i]->inPlay()->cards[j]->hasColor(color))
                count += 1;
    }
    return count;
}

int WParsedInt::mostCommonColor(int color, MTGCardInstance * card)
{
    int maxColor = 0;
    vector<int> colors;

    for(int i = 1; i < 6; i++)
        colors.push_back( countCardsInPlaybyColor(i, card->getObserver()) );
        
    for(int j = 0; j < 5; j++)
        if ( colors[j] > maxColor )
            maxColor = colors[j];

    if (countCardsInPlaybyColor(color, card->getObserver()) >= maxColor && maxColor > 0)
        return 1;

    return 0;
}

int WParsedInt::countCardTypeinZone(string type, MTGGameZone * zone)
{
    int count = 0;
    for( int i= 0; i < zone->nb_cards; i ++)
        if(zone->cards[i]->hasType(type))
            count += 1;
    return count;
}

int WParsedInt::cardHasTypeinZone(const char * type, MTGGameZone * zone)
{
    int count = 0;
    if(zone->hasType(type))
        count = 1;
    return count;
}

int WParsedInt::countCanTargetby(string type, MTGCardInstance * card, Player * player)
{
    int count = 0;
        for (int j = player->game->battlefield->nb_cards - 1; j >= 0; --j)
        {
            if (player->game->battlefield->cards[j]->hasType(type) && !player->game->battlefield->cards[j]->protectedAgainst(card))
                count += 1;
        }
    return count;
}

int WParsedInt::countManaProducedby(int color, MTGCardInstance * target, Player * player)
{
    int count = 0;
    MTGGameZone * zone = player->game->battlefield;
    for(int k = 0; k < zone->nb_cards; k++)
    {
        MTGCardInstance * card = zone->cards[k];
        if(card->isLand() && (card != target) && card->hasSubtype("forest") && color == 1)
            count++;
        if(card->isLand() && (card != target) && card->hasSubtype("island") && color == 2)
            count++;
        if(card->isLand() && (card != target) && card->hasSubtype("mountain") && color == 3)
            count++;
        if(card->isLand() && (card != target) && card->hasSubtype("swamp") && color == 4)
            count++;
        if(card->isLand() && (card != target) && card->hasSubtype("plains") && color == 5)
            count++;
        if(card->isLand() && (card != target) && card->cardsAbilities.size())
        {
            for(unsigned int j = 0; j < card->cardsAbilities.size(); j++)
            {
                if(dynamic_cast<AManaProducer*> (card->cardsAbilities[j]) && dynamic_cast<AManaProducer*> (card->cardsAbilities[j])->output->hasColor(color) )
                    count++;
            }
        }
    }
    return count;
}

WParsedInt::WParsedInt(int value)
{
    intValue = value;
}

WParsedInt::WParsedInt(string s, Spell * spell, MTGCardInstance * card)
{
    init(s, spell, card);
}

WParsedInt::WParsedInt(string s, MTGCardInstance * card)
{
    init(s, NULL, card);
}

int WParsedInt::getValue()
{
    return intValue;
}

string WParsedInt::getStringValue()
{
    stringstream sval;
    sval << intValue;
    return sval.str();
}

WParsedPT::WParsedPT(int p, int t)
{
    power.intValue = p;
    toughness.intValue = t;
    ok = true;
}

WParsedPT::WParsedPT(string s, Spell * spell, MTGCardInstance * card)
{
    size_t found = s.find("/");
    ok = false;
    if (found != string::npos)
    {
        size_t end = s.find(" ", found);
        if (end == string::npos) end = s.size();
        size_t start = s.find_last_of(" ", found);
        if (start == string::npos)
            start = 0;
        else
            start++;
        power = WParsedInt(s.substr(start, found - start), spell, card);
        toughness = WParsedInt(s.substr(found + 1, end - found - 1), spell, card);

        ok = true;
    }
}