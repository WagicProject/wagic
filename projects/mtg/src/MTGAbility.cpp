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


int AbilityFactory::countCards(TargetChooser * tc, Player * player, int option)
{
  int result = 0;
  GameObserver * game = GameObserver::GetInstance();
  for (int i = 0; i < 2 ; i++)
  {
    if (player && player!= game->players[i]) continue;
    MTGGameZone * zones[] = {game->players[i]->game->inPlay,game->players[i]->game->graveyard,game->players[i]->game->hand};
    for (int k = 0; k < 3; k++)
    {
      for (int j = zones[k]->nb_cards-1; j >=0 ; j--)
      {
        MTGCardInstance * current =  zones[k]->cards[j];
        if (tc->canTarget(current))
        {
          switch (option){
          case COUNT_POWER:
            result+= current->power;
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

Counter * AbilityFactory::parseCounter(string s, MTGCardInstance * target, Spell * spell) {
  int nb = 1;
  string name = "";
  size_t start = 0;
  size_t end = s.length();
  size_t separator = s.find(",", start);
  if (separator == string::npos) separator = s.find(".", start);
  if (separator != string::npos){
    size_t separator2 = s.find(",", separator+1);
    if (separator2 == string::npos) separator2 = s.find(".", separator+1);
    if (separator2 != string::npos) {
      name = s.substr(separator2+1,end-separator2-1);
    }    
    string nbstr = s.substr(separator+1,separator2-separator-1);
    WParsedInt * wpi;
    if (target){
      wpi = NEW WParsedInt(nbstr,spell,target);
    }else{
      wpi = NEW WParsedInt(atoi(nbstr.c_str()));
    }
    nb = wpi->getValue();
    delete(wpi);
    end = separator;
  }
  
  string spt = s.substr(start,end-start);
  int power, toughness;
  if ( parsePowerToughness(spt,&power, &toughness)){
    Counter * counter = NEW Counter(target,name.c_str(),power,toughness);
    counter->nb = nb;
    return counter;
  }
  return NULL;
}

int AbilityFactory::parsePowerToughness(string s, int *power, int *toughness){
    size_t found = s.find("/");
    if (found != string::npos){
      size_t end = s.find(" ", found);
      if (end == string::npos) end = s.size();
      size_t start = s.find_last_of(" ",found);
      if (start == string::npos) start = -1;

      *power = atoi(s.substr(start+1,s.size()-found).c_str());
      *toughness = atoi(s.substr(found+1,end-found-1).c_str());

      return 1;
    }
    return 0;
}

// evaluate trigger ability
// ie auto=@attacking(mytgt):destroy target(*)
// eval only the text between the @ and the first :
TriggeredAbility * AbilityFactory::parseTrigger(string s, string magicText, int id, Spell * spell, MTGCardInstance *card, Targetable * target){
  size_t found = string::npos;

  //Card Changed Zone
  found = s.find("movedto(");
  if (found != string::npos){
    size_t end = s.find (")");
    string starget = s.substr(found+8,end - found - 8);
    TargetChooserFactory tcf;

    TargetChooser *toTc  = NULL;
    TargetChooser *toTcCard = NULL; 
    end = starget.find ("|");
    if (end == string::npos) {
      toTcCard = tcf.createTargetChooser("*",card);
      found = 0;
    }else{
      toTcCard = tcf.createTargetChooser(starget.substr(0, end).append("|*"),card);
      found = end + 1;
    }
    toTcCard->setAllZones();
    starget = starget.substr(found,end - found).insert(0,"*|");
    toTc = tcf.createTargetChooser(starget,card);
    toTc->targetter = NULL;

    TargetChooser *fromTc = NULL;
    TargetChooser * fromTcCard = NULL;
    found = s.find("from(");
    if (found != string::npos){
      end = s.find ("|", found);
      if (end == string::npos) {
        fromTcCard = tcf.createTargetChooser("*",card);
        found = found + 5;
      }else{
        fromTcCard = tcf.createTargetChooser(s.substr(found + 5, end - found - 5).append("|*"),card);
        found = end + 1;
      }
      fromTcCard->setAllZones();
      end = s.find (")", found);
      starget = s.substr(found,end - found).insert(0,"*|");
      fromTc = tcf.createTargetChooser(starget,card);
      fromTc->targetter = NULL;
    }
    return NEW TrCardAddedToZone(id,card,(TargetZoneChooser *)toTc, toTcCard,(TargetZoneChooser *)fromTc,fromTcCard);
  }

		  //Card unTapped
  found = s.find("untapped(");
  if (found != string::npos){
    size_t end = s.find (")");
    string starget = s.substr(found+9,end - found - 9);
    TargetChooserFactory tcf;
    TargetChooser *tc = tcf.createTargetChooser(starget,card);
    tc->targetter = NULL;

    return NEW TrCardTapped(id,card,tc,false);
  }

  //Card Tapped
  found = s.find("tapped(");
  if (found != string::npos){
    size_t end = s.find (")");
    string starget = s.substr(found+7,end - found - 7);
    TargetChooserFactory tcf;
    TargetChooser *tc = tcf.createTargetChooser(starget,card);
    tc->targetter = NULL;

    return NEW TrCardTapped(id,card,tc,true);
  }

	  //Card Tapped for mana
  found = s.find("tappedformana(");
  if (found != string::npos){
    size_t end = s.find (")");
    string starget = s.substr(found+14,end - found - 14);
    TargetChooserFactory tcf;
    TargetChooser *tc = tcf.createTargetChooser(starget,card);
    tc->targetter = NULL;

    return NEW TrCardTappedformana(id,card,tc,true);
  }

	 //Card is attacking
  found = s.find("attacking(");
  if (found != string::npos){
    size_t end = s.find (")");
    string starget = s.substr(found+10,end - found - 10);
    TargetChooserFactory tcf;
    TargetChooser *tc = tcf.createTargetChooser(starget,card);
    tc->targetter = NULL;

    return NEW TrCardAttacked(id,card,tc);
  }
		 //Card is attacking alone
  found = s.find("attackedalone(");
  if (found != string::npos){
    size_t end = s.find (")");
    string starget = s.substr(found+14,end - found - 14);
    TargetChooserFactory tcf;
    TargetChooser *tc = tcf.createTargetChooser(starget,card);
    tc->targetter = NULL;

    return NEW TrCardAttackedAlone(id,card,tc);
  }

	//Card card attacked and is not blocked
  found = s.find("notblocked(");
  if (found != string::npos){
    size_t end = s.find (")");
    string starget = s.substr(found+11,end - found - 11);
    TargetChooserFactory tcf;
    TargetChooser *tc = tcf.createTargetChooser(starget,card);
    tc->targetter = NULL;

    return NEW TrCardAttackedNotBlocked(id,card,tc);
  }

		//Card card attacked and is blocked
  found = s.find("blocked(");
  if (found != string::npos){
    size_t end = s.find (")");
    string starget = s.substr(found+8,end - found - 8);
    TargetChooserFactory tcf;
    TargetChooser *tc = tcf.createTargetChooser(starget,card);
    tc->targetter = NULL;
		 found = s.find("from(");
	
	  TargetChooser *fromTc = NULL;
	  if (found != string::npos){
        end = s.find (")", found);
        starget = s.substr(found+5,end - found - 5);
        fromTc = tcf.createTargetChooser(starget,card);
        fromTc->targetter = NULL;
	  }

    return NEW TrCardAttackedBlocked(id,card,tc,fromTc);
  }

		 //Card card is a blocker
  found = s.find("blocking(");
  if (found != string::npos){
    size_t end = s.find (")");
    string starget = s.substr(found+9,end - found - 9);
    TargetChooserFactory tcf;
    TargetChooser *tc = tcf.createTargetChooser(starget,card);
    tc->targetter = NULL;
		 found = s.find("from(");
	
	  TargetChooser *fromTc = NULL;
	  if (found != string::npos){
        end = s.find (")", found);
        starget = s.substr(found+5,end - found - 5);
        fromTc = tcf.createTargetChooser(starget,card);
        fromTc->targetter = NULL;
	  }

    return NEW TrCardBlocked(id,card,tc,fromTc);
  }

			 //Card card is drawn
  found = s.find("drawn(");
  if (found != string::npos){
    size_t end = s.find (")");
    string starget = s.substr(found+6,end - found - 6);
    TargetChooserFactory tcf;
    TargetChooser *tc = tcf.createTargetChooser(starget,card);
    tc->targetter = NULL;

    return NEW TrcardDrawn(id,card,tc);
  }

		 //Card is sacrificed
  found = s.find("sacrificed(");
  if (found != string::npos){
    size_t end = s.find (")");
    string starget = s.substr(found+11,end - found - 11);
    TargetChooserFactory tcf;
    TargetChooser *tc = tcf.createTargetChooser(starget,card);
    tc->targetter = NULL;

    return NEW TrCardSacrificed(id,card,tc);
  }

			 //Card is sacrificed
  found = s.find("discarded(");
  if (found != string::npos){
    size_t end = s.find (")");
    string starget = s.substr(found+10,end - found - 10);
    TargetChooserFactory tcf;
    TargetChooser *tc = tcf.createTargetChooser(starget,card);
    tc->targetter = NULL;

    return NEW TrCardDiscarded(id,card,tc);
  }


		    //Card Damaging non combat
  found = s.find("noncombatdamaged(");
  if (found != string::npos){
    size_t end = s.find (")");
    string starget = s.substr(found+17,end - found - 17);
    TargetChooserFactory tcf;
    TargetChooser *tc = tcf.createTargetChooser(starget,card);
    tc->targetter = NULL;
    found = s.find("from(");
	
	  TargetChooser *fromTc = NULL;
	  if (found != string::npos){
        end = s.find (")", found);
        starget = s.substr(found+5,end - found - 5);
        fromTc = tcf.createTargetChooser(starget,card);
        fromTc->targetter = NULL;
	  }
		return NEW TrDamaged(id,card,tc,fromTc, 2);
  }

			    //Card Damaging combat
  found = s.find("combatdamaged(");
  if (found != string::npos){
    size_t end = s.find (")");
    string starget = s.substr(found+14,end - found - 14);
    TargetChooserFactory tcf;
    TargetChooser *tc = tcf.createTargetChooser(starget,card);
    tc->targetter = NULL;
    found = s.find("from(");
	
	  TargetChooser *fromTc = NULL;
	  if (found != string::npos){
        end = s.find (")", found);
        starget = s.substr(found+5,end - found - 5);
        fromTc = tcf.createTargetChooser(starget,card);
        fromTc->targetter = NULL;
	  }
		return NEW TrDamaged(id,card,tc,fromTc, 1);
  }

    //Card Damaging
  found = s.find("damaged(");
  if (found != string::npos){
    size_t end = s.find (")");
    string starget = s.substr(found+8,end - found - 8);
    TargetChooserFactory tcf;
    TargetChooser *tc = tcf.createTargetChooser(starget,card);
    tc->targetter = NULL;
    found = s.find("from(");
	
	  TargetChooser *fromTc = NULL;
	  if (found != string::npos){
        end = s.find (")", found);
        starget = s.substr(found+5,end - found - 5);
        fromTc = tcf.createTargetChooser(starget,card);
        fromTc->targetter = NULL;
	  }
		return NEW TrDamaged(id,card,tc,fromTc, 0);
  }

  int who = 0;
  if (s.find("my") != string::npos) who = 1;
  if (s.find("opponent") != string::npos) who = -1;
  if (s.find("targetcontroller") != string::npos) who = -2;

  //Next Time...
  found = s.find("next");
  if (found != string::npos){
    for (int i = 0; i < Constants::NB_MTG_PHASES; i++){
      found = s.find(Constants::MTGPhaseCodeNames[i]);
      if (found != string::npos){
	      return NEW TriggerNextPhase(id, card,target,i,who);
      }
    }
  }

    //Each Time...
  found = magicText.find("each");
  if (found != string::npos){
    for (int i = 0; i < Constants::NB_MTG_PHASES; i++){
      found = magicText.find(Constants::MTGPhaseCodeNames[i]);
      if (found != string::npos){
	      return NEW TriggerAtPhase(id, card,target,i,who);
      }
    }
  }

  return NULL;
}

int AbilityFactory::parseRestriction(string s){
  if (s.find("myturnonly") != string::npos) return ActivatedAbility::PLAYER_TURN_ONLY;
  if (s.find("assorcery") != string::npos) return ActivatedAbility::AS_SORCERY;

  string types[] = {"my","opponent", ""};
  int starts[] = {ActivatedAbility::MY_BEFORE_BEGIN,ActivatedAbility::OPPONENT_BEFORE_BEGIN,ActivatedAbility::BEFORE_BEGIN};
  for (int j = 0; j < 3; ++j){
    size_t found = s.find(types[j]);
    if (found !=string::npos){
      for (int i = 0; i < Constants::NB_MTG_PHASES; i++){
        string toFind = types[j];
        toFind.append(Constants::MTGPhaseCodeNames[i]).append("only");
        found = s.find(toFind);
        if (found != string::npos){
	        return starts[j] + i;
        }
      }
    }
  }

  return ActivatedAbility::NO_RESTRICTION;
}

MTGAbility * AbilityFactory::getCoreAbility(MTGAbility * a){
  GenericTargetAbility * gta = dynamic_cast<GenericTargetAbility*>(a);
  if (gta) return getCoreAbility(gta->ability);

  GenericActivatedAbility * gaa = dynamic_cast<GenericActivatedAbility*>(a);
  if (gaa) return getCoreAbility(gaa->ability);

  if (MultiAbility * abi = dynamic_cast<MultiAbility*>(a)) return getCoreAbility(abi->abilities[0]);

  return a;
}

//Parses a string and returns the corresponding MTGAbility object
//Returns NULL if parsing failed
//Beware, Spell CAN be null when the function is called by the AI trying to analyze the effects of a given card
MTGAbility * AbilityFactory::parseMagicLine(string s, int id, Spell * spell, MTGCardInstance *card, int activated, int forceUEOT, int oneShot,int forceFOREVER, MTGGameZone * dest){
  size_t found;
  trim(s);

  //TODO This block redundant with calling function
  if (!card && spell) card = spell->source;
  if (!card) return NULL;
  MTGCardInstance * target = card->target;
  if (!target) target = card; 

  found = s.find("@");
  if ( found != string::npos )
  {
    found = s.find(":", found);
    if (found != string::npos)
    {

      TriggeredAbility * trigger = NULL;
      string triggerText = s.substr(0, found );
      trigger = parseTrigger(triggerText, s,id,spell,card,target);
      //Dirty way to remove the trigger text (could get in the way)
      if (trigger)
      {
        //found = s.find(":", found);
        string s1 = s.substr(found+1);
        MTGAbility * a = parseMagicLine(s1, id, spell, card,activated);
        if (!a){
          delete trigger;
          return NULL;
        }
        return NEW GenericTriggeredAbility(id,card,trigger,a,NULL,target);
      }
    }
  }
  int doTap = 0; //Tap in the cost ?
  if (s.find("{t}") != string::npos) doTap = 1;

  int restrictions = parseRestriction(s);

  TargetChooser * tc = NULL;
  string sWithoutTc = s;
  //Target Abilities
  found = s.find("target(");
  if (found != string::npos){
    int end = s.find(")", found);
    string starget = s.substr(found + 7,end - found - 7);
    TargetChooserFactory tcf;
    tc = tcf.createTargetChooser(starget, card);
    if (tc && s.find("notatarget(") != string::npos){
      tc->targetter = NULL;
      found = found - 4;
    }
    string temp = s.substr(0,found);
    temp.append(s.substr(end+1));
    sWithoutTc = temp;
  }

  size_t delimiter = sWithoutTc.find("}:");
  size_t firstNonSpace = sWithoutTc.find_first_not_of(" ");
  if (delimiter!= string::npos && firstNonSpace !=string::npos && sWithoutTc[firstNonSpace] == '{'){
    ManaCost * cost  = ManaCost::parseManaCost(sWithoutTc.substr(0,delimiter+1),NULL,card);
    if (doTap || cost){
      string s1 = sWithoutTc.substr(delimiter+2);
      
      MTGAbility * a = parseMagicLine(s1, id, spell, card, 1);
      if (!a){
        DebugTrace("ABILITYFACTORY Error parsing: " << sWithoutTc);
        return NULL;
      }

      //A stupid Special case for ManaProducers because they don't use the stack :(
      AManaProducer * amp = dynamic_cast<AManaProducer*>(a);
      if (amp){
        amp->cost = cost;
        amp->oneShot = 0;
        amp->tap = doTap;
        return amp;
      }

      int limit = 0;
      size_t limit_str = sWithoutTc.find("limit:");
      if (limit_str != string::npos){
        limit = atoi(sWithoutTc.substr(limit_str+6).c_str());
      }

      AEquip *ae = dynamic_cast<AEquip*>(a);
      if (ae){
        ae->cost = cost;
        if (!tc) {
          TargetChooserFactory tcf;
          tc = tcf.createTargetChooser("creature|myBattlefield", card);
        }
        ae->tc = tc;
        return ae;
			}

      if (tc) return NEW GenericTargetAbility(id, card, tc, a,cost, doTap,limit,restrictions,dest);
      return NEW GenericActivatedAbility(id, card, a,cost,doTap,limit,restrictions,dest);
    }
    SAFE_DELETE(cost);
  }
  
  // figure out alternative cost effects 
  string keyword;
  int costType = -1;
  if (s.find( Constants::kKickerKeyword ) == 0 )
  {
    costType = ManaCost::MANA_PAID_WITH_KICKER;
    keyword = Constants::kKickerKeyword;
  }
  if (s.find( Constants::kRetraceKeyword ) == 0 )
  {
    costType = ManaCost::MANA_PAID_WITH_RETRACE;
    keyword = Constants::kRetraceKeyword;
  }
  if (s.find( Constants::kAlternativeKeyword ) == 0 )
  {
    costType = ManaCost::MANA_PAID_WITH_ALTERNATIVE;
    keyword = Constants::kAlternativeKeyword ;
  }
  if (s.find( Constants::kBuyBackKeyword ) == 0 )
  {
    costType = ManaCost::MANA_PAID_WITH_BUYBACK;
    keyword = Constants::kBuyBackKeyword;
  }
  if (s.find( Constants::kFlashBackKeyword ) == 0 )
  {
    costType = ManaCost::MANA_PAID_WITH_FLASHBACK;
    keyword = Constants::kFlashBackKeyword;
  }

  if ( (costType > -1) && (!keyword.empty()) )
  {
    if ( spell && spell->FullfilledAlternateCost( costType )) 
    {
      string s1 = s.substr(keyword.length());
      return parseMagicLine(s1, id, spell, card);
    }
    DebugTrace("INFO parseMagicLine: Alternative Cost was not fulfilled for " << s);
    return NULL;
  }


  //When...comes into play, you may...
  found = s.find("may ");
  if (found == 0){
    string s1 = sWithoutTc.substr(found+4);
    MTGAbility * a1 = parseMagicLine(s1,id,spell, card);
    if (!a1) return NULL;

    if (tc) a1 = NEW GenericTargetAbility(id, card, tc, a1);
    else a1 =  NEW GenericActivatedAbility(id, card, a1,NULL);
    return NEW MayAbility(id,a1,card);
  }
    //When...comes into play, choose one...
  found = s.find("choice ");
  if (found == 0){
    string s1 = sWithoutTc.substr(found+7);
    MTGAbility * a1 = parseMagicLine(s1,id,spell, card);
    if (!a1) return NULL;

    if (tc) a1 = NEW GenericTargetAbility(id, card, tc, a1);
    else a1 =  NEW GenericActivatedAbility(id, card, a1,NULL);
    return NEW MayAbility(id,a1,card,true);
  }
  
  //Multiple abilities for ONE cost
  found = s.find("&&");
  if (found != string::npos){
    SAFE_DELETE(tc);
    string s1 = s.substr(0,found);
    string s2 = s.substr(found+2);
    MultiAbility * multi = NEW MultiAbility(id, card,target,NULL,NULL);
    MTGAbility * a1 = parseMagicLine(s1,id,spell, card,activated);
    MTGAbility * a2 = parseMagicLine(s2,id,spell, card,activated);
    multi->Add(a1);
    multi->Add(a2);
    multi->oneShot=1;
    return multi;
  }

  
  //rather dirty way to stop thises and lords from conflicting with each other.
  string prelords[] = {"foreach(","lord(","aslongas(","teach(", "all("};
  size_t lord = string::npos;
  for (int j = 0; j < 5; ++j){
    size_t found2 = s.find(prelords[j]);
    if (found2!=string::npos && ((found == string::npos) || found2 < found)){
      lord = found2;
    }
  }
  
 //This, ThisForEach;
  string thises[] = {"this(","thisforeach("};
  found = string::npos;
  int i = -1;
  for (int j = 0; j < 2; ++j){
    size_t found2 = s.find(thises[j]);
    if (found2!=string::npos && ((found == string::npos) || found2 < found)){
      found = found2;
      i = j;
    }
  }
  if (found != string::npos && found < lord) {
    //why does tc even exist here? This shouldn't happen...
    SAFE_DELETE(tc); //http://code.google.com/p/wagic/issues/detail?id=424

    size_t header = thises[i].size();
    size_t end = s.find(")", found+header);
    string s1;
    if (found == 0 || end != s.size()-1){
      s1 = s.substr(end+1);
    }else{
      s1 = s.substr(0, found);
    }
    if (end != string::npos){
      string thisDescriptorString = s.substr(found+header,end-found-header);
      ThisDescriptorFactory tdf;
      ThisDescriptor * td = tdf.createThisDescriptor(thisDescriptorString);

      if (!td){
        DebugTrace("MTGABILITY: Parsing Error:" << s);
        return NULL;
      }

      MTGAbility * a = parseMagicLine(s1,id,spell, card,0,activated); 
      if (!a){
        SAFE_DELETE(td);
        return NULL;
      }
      MTGAbility * result = NULL;
      int oneShot = 0;
      found = s.find(" oneshot");
      if (found !=string::npos) oneShot = 1;
      if (activated) oneShot = 1;
      if (card->hasType("sorcery") || card->hasType("instant")) oneShot = 1;
      if (a->oneShot) oneShot = 1;
      Damageable * _target = NULL;
      if (spell) _target = spell->getNextDamageableTarget();
      if (!_target) _target = target;

      switch(i){
            case 0: result =  NEW AThis(id, card, _target, td, a); break;
            case 1: result =  NEW AThisForEach(id, card, _target, td, a); break;
            default: result =  NULL;
      }
      if (result){ result->oneShot = oneShot;}
      return result;
    }
    return NULL;
  }


  //Lord, foreach, aslongas
  string lords[] = {"lord(","foreach(","aslongas(","teach(", "all("};
  found = string::npos;
  i = -1;
  for (int j = 0; j < 5; ++j){
    size_t found2 = s.find(lords[j]);
    if (found2!=string::npos && ((found == string::npos) || found2 < found)){
      found = found2;
      i = j;
    }
  }
  if (found != string::npos){
    SAFE_DELETE(tc);
    size_t header = lords[i].size();
    size_t end = s.find(")", found+header);
    string s1;
    if (found == 0 || end != s.size()-1){
      s1 = s.substr(end+1);
    }else{
      s1 = s.substr(0, found);
    }
    if (end != string::npos){
      int lordIncludeSelf = 1;
      size_t other = s1.find(" other");
      if ( other != string::npos){
        lordIncludeSelf = 0;
        s1.replace(other, 6,"");
      }
      string lordTargetsString = s.substr(found+header,end-found-header);
      TargetChooserFactory tcf;
      TargetChooser * lordTargets = tcf.createTargetChooser(lordTargetsString, card);
      
      if (!lordTargets){
        DebugTrace("MTGABILITY: Parsing Error: " << s);
        return NULL;
      }

      MTGAbility * a = parseMagicLine(s1,id,spell, card,0,activated); //activated lords usually force an end of turn ability
      if (!a){
        SAFE_DELETE(lordTargets);
        return NULL;
      }
      MTGAbility * result = NULL;
      int oneShot = 0;
	  found = s.find(" oneshot");
      if (found !=string::npos) oneShot = 1;
      if (activated) oneShot = 1;
      if (card->hasType("sorcery") || card->hasType("instant")) oneShot = 1;
      if (i == 4) oneShot = 1;
      if (a->oneShot) oneShot = 1;
      Damageable * _target = NULL;
	    if (spell) _target = spell->getNextDamageableTarget();
      if (!_target) _target = target;

      int mini = 0;
      int maxi = 0;

      found = s.find(" >");
      if (found !=string::npos) mini = atoi(s.substr(found+2,1).c_str());

      found = s.find(" <");
      if (found !=string::npos) maxi = atoi(s.substr(found+2,1).c_str());

      switch(i){
        case 0: result =  NEW ALord(id, card, lordTargets, lordIncludeSelf, a); break;
        case 1: result =  NEW AForeach(id, card, _target,lordTargets, lordIncludeSelf, a,mini,maxi); break;
        case 2: result =  NEW AAsLongAs(id, card, _target,lordTargets, lordIncludeSelf, a,mini,maxi); break;
				case 3: result =  NEW ATeach(id, card, lordTargets,lordIncludeSelf, a); break;
			  case 4: result =  NEW ALord(id, card, lordTargets,  lordIncludeSelf, a); break;
        default: result =  NULL;
      }
      if (result) result->oneShot = oneShot;
      return result;
    }
    return NULL;
  }

  if (!activated &&  tc){
      
      MTGAbility * a = parseMagicLine(sWithoutTc, id, spell, card);
      if (!a){
        DebugTrace("ABILITYFACTORY Error parsing: " << s);
        return NULL;
      }
      a = NEW GenericTargetAbility(id,card,tc,a);
      return NEW MayAbility(id,a,card,true);
  }

  SAFE_DELETE(tc);

  //Upkeep Cost
  found = s.find("upcost");
  if (found != string::npos){
    size_t start = s.find("[");
    size_t end = s.find("]",start);
    string s1 = s.substr(start + 1,end - start - 1);
    size_t seperator = s1.find(";");
    int phase = Constants::MTG_PHASE_UPKEEP;
    int once = 0;
    if (seperator != string::npos){
      for (int i = 0; i < Constants::NB_MTG_PHASES; i++){
        if (s1.find("next") != string::npos) once = 1;
        if(s1.find(Constants::MTGPhaseCodeNames[i]) != string::npos){
          phase = i;
        }
      }
      s1 = s1.substr(0,seperator - 1);
    }
    ManaCost * cost = ManaCost::parseManaCost(s1);

    if (!cost){
      DebugTrace("MTGABILITY: Parsing Error: " << s);
      return NULL;
    }
    
    string sAbility = s.substr(end + 1);
    MTGAbility * a = parseMagicLine(sAbility,id,spell,card);

    if (!a){
      DebugTrace("MTGABILITY: Parsing Error: " << s);
      delete(cost);
      return NULL;
    }

    return NEW AUpkeep(id,card,a,cost,doTap,restrictions,phase,once);
  }

   //Cycling
  found = s.find("cycling");
  if (found != string::npos){
    MTGAbility * a =  NEW ACycle(id,card,target);
    a->oneShot = 1;
    return a;
  }

	   //ninjutsu
  found = s.find("ninjutsu");
  if (found != string::npos){
    MTGAbility * a =  NEW ANinja(id,card,target);
    a->oneShot = 1;
    return a;
  }

	   //combat removel
  found = s.find("removefromcombat");
  if (found != string::npos){
    MTGAbility * a =  NEW ACombatRemovel(id,card,target);
    a->oneShot = 1;
    return a;
  }

  //Fizzle (counterspell...)
  found = s.find("fizzle");
  if (found != string::npos){
    Spell * starget = NULL;
    if (spell) starget = spell->getNextSpellTarget();
    MTGAbility * a = NEW AAFizzler(id,card,starget);
    a->oneShot = 1;
    return a;
  }
    
  //Regeneration
  found = s.find("regenerate");
  if (found != string::npos){
    MTGAbility * a =  NEW AStandardRegenerate(id,card,target);
    a->oneShot = 1;
    return a;
  }


  //Token creator. Name, type, p/t, abilities
  found = s.find("token(");
  if (found != string::npos){
    WParsedInt * multiplier = NULL; 
    size_t star = s.find("*");
    if (star != string::npos) multiplier = NEW WParsedInt(s.substr(star+1),spell,card);
    size_t end = s.find(")", found);
    int tokenId = atoi(s.substr(found + 6,end - found - 6).c_str());
		int who;
	  size_t opponent = s.find("opponent");
		if (opponent != string::npos){ 
			who = 1;
		}else{
			who = 0;
		}
    if (tokenId){
	    MTGCard * safetycard = GameApp::collection->getCardById(tokenId);
	    if (safetycard){//contenue
        ATokenCreator * tok = NEW ATokenCreator(id,card,NULL,tokenId,0, multiplier,who);
        tok->oneShot = 1;
        return tok;
	    }else{
	      tokenId = 0;
        ATokenCreator * tok = NEW ATokenCreator(id,card,NULL,"ID NOT FOUND","ERROR ID",NULL,NULL,"",0,NULL);
	      return tok;
	    }
	  }

    end = s.find(",", found);
    string sname = s.substr(found + 6,end - found - 6);
    size_t previous = end+1;
    end = s.find(",",previous);
    string stypes = s.substr(previous,end - previous);
    previous = end+1;
    end = s.find(",",previous);
    string spt = s.substr(previous,end - previous);
	  int value = 0;
    int power, toughness;
	  if(!spt.find("X/X") || !spt.find("x/x")){value = spell->computeX(card);}
	  if(!spt.find("XX/XX") || !spt.find("xx/xx")){value = spell->computeXX(card);}
	  parsePowerToughness(spt,&power, &toughness);
    string sabilities = s.substr(end+1);
		if(s.find("opponent")){
			if (opponent != string::npos){ 
			who = 1;
		}else{
			who = 0;
			}
		}
    ATokenCreator * tok = NEW ATokenCreator(id,card,NULL,sname,stypes,power + value,toughness + value,sabilities,0, multiplier,who);
    tok->oneShot = 1;
    return tok;
  }

  //name an ability line
  found = s.find("name(");
  if (found != string::npos){
    size_t end = s.find(")", found);
    string sname = s.substr(found + 5,end - found - 5);
	size_t previous = end+1;
    ANamer * tok = NEW ANamer(id,card,NULL,sname,0);
    return tok;
  }


  //Equipment
  found = s.find("equip");
  if (found != string::npos){
    MTGAbility * a = NEW AEquip(id,card);
    return a;
  }

  //Equipment (attach)
  found = s.find("attach");
  if (found != string::npos){
    MTGAbility * a = NEW AEquip(id,card,0,0,ActivatedAbility::NO_RESTRICTION);
    return a;
  }


  //MoveTo Move a card from a zone to another
  found = s.find("moveto(");
  if (found != string::npos){
    int end = s.find(")",found+1);
    string szone = s.substr(found + 7,end - found - 7);

    //hack for http://code.google.com/p/wagic/issues/detail?id=120
    //We assume that auras don't move their own target...
    if (card->hasType("aura")) target = card; 

    MTGAbility * a = NEW AAMover(id,card,target,szone);
    a->oneShot = 1;
    return a;

  }

  //Copy a target
  found = s.find("copy");
  if (found != string::npos){
    MTGAbility * a = NEW AACopier(id,card,target);
    a->oneShot = 1;
    return a;
  }

	  //clone
  found = s.find("clone");
  if (found != string::npos){
		int who;
		who = 0;
	 found = s.find("opponent");
		if (found != string::npos){ 
			who = 1;
		}
    string with;
    found = s.find("with(");
    if (found != string::npos){
      size_t end = s.find (")", found);
      with = s.substr(found+5,end - found - 5);
    }
     MTGAbility * a = NEW AACloner(id,card,target,0,who,with);
    a->oneShot = 1;
    return a;
	}

  //Bury, destroy, sacrifice, reject(discard)
  if ( s.find("bury") != string::npos )
  {
    MTGAbility *a = NEW AABuryCard(id, card, target, NULL, AABanishCard::BURY);
    a->oneShot = 1;
    return a;
  }
  else if ( s.find("destroy") != string::npos ) 
  {
    MTGAbility * a = NEW AADestroyCard(id, card, target, NULL, AABanishCard::DESTROY);
    a->oneShot = 1;
    return a;
  }
  else if ( s.find("sacrifice") != string::npos )
  {
    MTGAbility *a = NEW AASacrificeCard(id, card, target, NULL, AABanishCard::SACRIFICE);
    a->oneShot = 1;
    return a;
  }
  else if ( s.find("reject") != string::npos )
  {
    MTGAbility *a = NEW AADiscardCard(id, card, target, NULL, AABanishCard::DISCARD);
    a->oneShot = 1;
    return a;
  }

  int who = TargetChooser::UNSET;
  if (s.find(" controller") != string::npos) who=TargetChooser::CONTROLLER;
  if (s.find(" opponent") != string::npos) who=TargetChooser::OPPONENT;
  if (s.find(" targetcontroller") != string::npos) who=TargetChooser::TARGET_CONTROLLER;
	if (s.find(" owner") != string::npos) who=TargetChooser::OWNER;

  found = s.find("ueot");
  if (found!= string::npos) forceUEOT = 1;
  found = s.find("oneshot");
  if (found!= string::npos) oneShot = 1;
  found = s.find("forever");
  if (found!= string::npos) forceFOREVER = 1;
 
  //PreventCombat Damage
  found = s.find("preventallcombatdamage");
  if (found != string::npos){


    string to = "";
    string from = "";

    found = s.find("to(");
    if (found != string::npos){
      size_t end = s.find (")", found);
      to = s.substr(found+3,end - found - 3);
    }

    found = s.find("from(");
    if (found != string::npos){
      size_t end = s.find (")", found);
      from = s.substr(found+5,end - found - 5);
    }
    
    MTGAbility * ab;
    if (forceUEOT){
      ab = NEW APreventDamageTypesUEOT(id,card,to,from);
    }else{
      ab = NEW APreventDamageTypes(id,card,to,from);
    }
    return ab;
  }
		//Prevent all non combat damage Damage
  found = s.find("preventallnoncombatdamage");
  if (found != string::npos){
    string to = "";
    string from = "";
    found = s.find("to(");
    if (found != string::npos){
      size_t end = s.find (")", found);
      to = s.substr(found+3,end - found - 3);
		}
    found = s.find("from(");
    if (found != string::npos){
      size_t end = s.find (")", found);
      from = s.substr(found+5,end - found - 5);
    }
    MTGAbility * ab;
    if (forceUEOT){
      ab = NEW APreventDamageTypesUEOT(id,card,to,from,2);
    }else{
      ab = NEW APreventDamageTypes(id,card,to,from,2);
    }
    return ab;
  }
	  //Prevent all damage
  found = s.find("preventalldamage");
  if (found != string::npos){
    string to = "";
    string from = "";
    found = s.find("to(");
    if (found != string::npos){
      size_t end = s.find (")", found);
      to = s.substr(found+3,end - found - 3);
		}
    found = s.find("from(");
    if (found != string::npos){
      size_t end = s.find (")", found);
      from = s.substr(found+5,end - found - 5);
    }
    MTGAbility * ab;
    if (forceUEOT){
      ab = NEW APreventDamageTypesUEOT(id,card,to,from,1);
    }else{
      ab = NEW APreventDamageTypes(id,card,to,from,1);
    }
    return ab;
  }

  //PreventCombat Damage
  found = s.find("fog");
  if (found != string::npos){
    string to = "";
    string from = "";
    found = s.find("to(");
    if (found != string::npos){
      size_t end = s.find (")", found);
      to = s.substr(found+3,end - found - 3);
    }
      found = s.find("from(");
    if (found != string::npos){
      size_t end = s.find (")", found);
      from = s.substr(found+5,end - found - 5);
    }
    MTGAbility * a =  NEW APreventDamageTypesUEOT(id,card,to,from);
    a->oneShot = 1;
	return a;
  }

  //Damage
  found = s.find("damage");
  if (found != string::npos){
    size_t start = s.find(":",found);
    if (start == string::npos) start = s.find(" ",found);
    size_t end = s.find(" ",start);
    string d;
    if (end != string::npos){
      d = s.substr(start+1,end-start-1);
    }else{
      d = s.substr(start+1);
    }
    WParsedInt * damage = NEW WParsedInt(d,spell,card);
    Targetable * t = NULL;
    if (spell) t = spell->getNextTarget();
    MTGAbility * a =  NEW AADamager(id,card,t, damage, NULL, 0, who);
    a->oneShot = 1;
    return a;
  }

 //remove poison
  found = s.find("alterpoison:");
  if (found != string::npos){
    size_t start = s.find(":",found);
    size_t end = s.find(" ",start);
    int poison;
    if (end != string::npos){
      poison = atoi(s.substr(start+1,end-start-1).c_str());
    }else{
      poison = atoi(s.substr(start+1).c_str());
    }

    Targetable * t = NULL;
    if (spell) t = spell->getNextPlayerTarget();
    MTGAbility * a = NEW AAAlterPoison (id, card, t,poison,NULL,0,who);
	  a->oneShot = 1;
    return a;
  }
   //prevent next damage
  found = s.find("prevent:");
  if (found != string::npos){
    size_t start = s.find(":",found);
    size_t end = s.find(" ",start);
    int preventing;
    if (end != string::npos){
      preventing = atoi(s.substr(start+1,end-start-1).c_str());
    }else{
      preventing = atoi(s.substr(start+1).c_str());
    }

    Targetable * t = NULL;
		if (spell) t = spell->getNextDamageableTarget();
    MTGAbility * a = NEW AADamagePrevent (id, card, t,preventing,NULL,0,who);
	  a->oneShot = 1;
    return a;
  }
//set life total
    found = s.find("lifeset");
  if (found != string::npos){
    size_t start = s.find(":",found);
    if (start == string::npos) start = s.find(" ",found);
    size_t end = s.find(" ",start);
    string d;
    if (end != string::npos){
      d = s.substr(start+1,end-start-1);
    }else{
      d = s.substr(start+1);
    }
    WParsedInt * life = NEW WParsedInt(d,spell,card);
    Damageable * t = NULL;
    if (spell) t = spell->getNextDamageableTarget();
    MTGAbility * a =  NEW AALifeSet(id,card,t, life, NULL, 0, who);
    a->oneShot = 1;
    return a;
  }

  //gain/lose life
  found = s.find("life:");
  if (found != string::npos){
    size_t start = found+4;
    size_t end = s.find(" ",start);
    string life_s;
    if (end != string::npos){
      life_s = s.substr(start+1,end-start-1);
    }else{
      life_s = s.substr(start+1);
    }
    WParsedInt * life = NEW WParsedInt(life_s,spell,card);
    Targetable * t = NULL;
    if (spell) t = spell->getNextTarget();
    MTGAbility * a =  NEW AALifer(id,card,t,life,NULL,0,who);
    a->oneShot = 1;
    return a;
  }

  // Win the game
  found = s.find("wingame");
  if (found != string::npos){
    Damageable * d = NULL;
    if (spell) d = spell->getNextDamageableTarget();
    MTGAbility * a =  NEW AAWinGame(id,card,d,NULL,0,who);
    a->oneShot = 1;
    return a;
  }

  //Draw
  found = s.find("draw:");
  if (found != string::npos){
    size_t start = s.find(":",found);
    size_t end = s.find(" ",start);
    string nbcardsStr;
    if (end != string::npos){
      nbcardsStr = s.substr(start+1,end-start-1);
    }else{
      nbcardsStr = s.substr(start+1);
    }
    WParsedInt * nbcards = NEW WParsedInt(nbcardsStr,spell,card);
    Targetable * t = NULL;
    if (spell) t = spell->getNextTarget();
    MTGAbility * a = NEW AADrawer(id,card,t,NULL,nbcards,0,who);
    a->oneShot = 1;
    return a;
  }
    //additional lands per turn
  found = s.find("land:");
  if (found != string::npos){
    size_t start = s.find(":",found);
    size_t end = s.find(" ",start);
    string additionalStr;
    if (end != string::npos){
      additionalStr = s.substr(start+1,end-start-1);
    }else{
      additionalStr = s.substr(start+1);
    }
    WParsedInt * additional = NEW WParsedInt(additionalStr,spell,card);
    Targetable * t = NULL;
    if (spell) t = spell->getNextTarget();
    MTGAbility * a = NEW AAMoreLandPlz(id,card,t,NULL,additional,0,who);
    a->oneShot = 1;
    return a;
  }

  //Deplete
  found = s.find("deplete:");
  if (found != string::npos){
    size_t start = s.find(":",found);
    size_t end = s.find(" ",start);
    int nbcards;
    if (end != string::npos){
      nbcards = atoi(s.substr(start+1,end-start-1).c_str());
    }else{
      nbcards = atoi(s.substr(start+1).c_str());
    }

    Targetable * t = NULL;
    if (spell) t = spell->getNextTarget();
    MTGAbility * a = NEW AADepleter(id,card,t,nbcards,NULL,0,who);
    a->oneShot = 1;
    return a;
  }

    //Shuffle
  found = s.find("shuffle");
  if (found != string::npos){
    Targetable * t = NULL;
    if (spell) t = spell->getNextTarget();
	MTGAbility * a = NEW AAShuffle(id,card,t,NULL,0,who);
    a->oneShot = 1;
    return a;
    }
        
  
     //cantcastspells
  found = s.find("onlyonespell");
  if (found != string::npos){
    Targetable * t = NULL;
    if (spell) t = spell->getNextTarget();
	MTGAbility * a = NEW AAOnlyOne(id,card,t,NULL,0,who);
    a->oneShot = 1;
    return a;
    }
      //cantcastspells
  found = s.find("nospells");
  if (found != string::npos){
    Targetable * t = NULL;
    if (spell) t = spell->getNextTarget();
	MTGAbility * a = NEW AANoSpells(id,card,t,NULL,0,who);
    a->oneShot = 1;
    return a;
    }
      //cantcastcreature
  found = s.find("nocreatures");
  if (found != string::npos){
    Targetable * t = NULL;
    if (spell) t = spell->getNextTarget();
	MTGAbility * a = NEW AANoCreatures(id,card,t,NULL,0,who);
    a->oneShot = 1;
    return a;
    }

  //Discard
  found = s.find("discard:");
  if (found != string::npos){
    size_t start = s.find(":",found);
    size_t end = s.find(" ",start);
    int nbcards;
    if (end != string::npos){
      nbcards = atoi(s.substr(start+1,end-start-1).c_str());
    }else{
      nbcards = atoi(s.substr(start+1).c_str());
    }

    Targetable * t = NULL;
    if (spell) t = spell->getNextPlayerTarget();
    MTGAbility * a = NEW AARandomDiscarder (id, card, t,nbcards,NULL,0,who);
	  a->oneShot = 1;
    return a;
  }



  //rampage
  found = s.find("rampage(");
  if (found != string::npos){
    int end = s.find(",", found);
    string spt = s.substr(8,end - 1);
    int power, toughness;
    if (parsePowerToughness(spt,&power, &toughness)){
      int MaxOpponent = atoi(s.substr(end+1,end+2).c_str());
	    return NEW ARampageAbility(id,card,power,toughness,MaxOpponent);
		}
    return NULL;
  }

		  //bushido
  found = s.find("bushido(");
  if (found != string::npos){
	  int end = s.find(")", found);
    string spt = s.substr(8,end - 1);
    int power, toughness;
    if (parsePowerToughness(spt,&power, &toughness)){
	    return NEW  ABushidoAbility(id,card,power,toughness);
    }
    return NULL;
  }

  //counter
  found = s.find("counter(");
  if (found != string::npos){
    size_t start = s.find("(");
    size_t end = s.find(")");
    string counterString = s.substr(start+1,end-start-1);
    Counter * counter = parseCounter(counterString,target,spell);
    if (counter){
      MTGAbility * a = NEW AACounter(id,card,target,counter->name.c_str(),counter->power,counter->toughness,counter->nb);
      delete(counter);
	    a->oneShot = 1;
      return a;
    }
    return NULL;
  }
  //Becomes... (animate artifact...: becomes(Creature, manacost/manacost)
  found = s.find("becomes(");
  if (found != string::npos){
    size_t real_end = s.find(")", found);
    size_t end = s.find(",", found);
    if (end == string::npos) end = real_end;
    string stypes  = s.substr(found + 8,end - found - 8);
    WParsedPT * pt = NULL;
    string sabilities;
    if (end != real_end){
      int previous = end+1;
      end = s.find(",",previous);
      if (end == string::npos) end = real_end;
      string temp = s.substr(previous, end - previous);
      pt = NEW WParsedPT(temp,spell,card);
      if (!pt->ok){
        SAFE_DELETE(pt);
        sabilities = temp;
      }
    }
    if (pt && end != real_end){
      sabilities = s.substr(end+1, real_end - end);
    }
    MTGAbility * ab;
    if (forceUEOT){
      ab = NEW ABecomesUEOT(id,card,target,stypes,pt,sabilities);
    }else{
      ab = NEW ABecomes(id,card,target,stypes,pt,sabilities);
	}return ab;
  }

  //bloodthirst
  found = s.find("bloodthirst:");
  if (found != string::npos){
    size_t start = s.find(":",found);
    size_t end = s.find(" ",start);
    int amount;
    if (end != string::npos){
       amount = atoi(s.substr(start+1,end-start-1).c_str());
    } else {
      amount = atoi(s.substr(start+1).c_str());
    }
    MTGAbility * a = NEW ABloodThirst(id,card,target,amount);
    return a;
  }

   if ( s.find("altercost(") != string::npos )
    return getManaReduxAbility( s.substr( s.find("altercost(") + 10), id, spell, card, target );

  //resetcost dirty code
  found = s.find("resetcost");
  if (found != string::npos){
    MTGAbility * a = NEW AResetCost(id,card,target);
    return a;
  }
  //transform....(hivestone,living enchantment)
  found = s.find("transforms(");
  if (found != string::npos){
    size_t real_end = s.find(")", found);
    size_t end = s.find(",", found);
    if (end == string::npos) end = real_end;
    string stypes  = s.substr(found + 11,end - found - 11);
    string sabilities;
    if (end != real_end){
      int previous = end+1;
      if (end == string::npos) end = real_end;
      string temp = s.substr(previous, end - previous);
    }
    if (end != real_end){
      sabilities = s.substr(end+1, real_end - end);
    }
    MTGAbility * a;
    if(forceFOREVER){
      a = NEW ATransformerFOREVER(id,card,target,stypes,sabilities);
    }
    else if (forceUEOT){
      a = NEW ATransformerUEOT(id,card,target,stypes,sabilities);
    }
    else{
      a = NEW ATransformer(id,card,target,stypes,sabilities);
    }
    return a;
  }
  //Change Power/Toughness
  WParsedPT * wppt = NEW WParsedPT(s,spell,card);
  if (wppt->ok){
    if (!activated){
      if(card->hasType("instant") || card->hasType("sorcery") || forceUEOT){
        return NEW AInstantPowerToughnessModifierUntilEOT(id, card, target,wppt);
      }
      return NEW APowerToughnessModifier(id, card, target,wppt);
    }
    return NEW APowerToughnessModifierUntilEndOfTurn(id,card,target,wppt);
  }
  else{
    delete wppt;
  }

  //Mana Producer
  found = s.find("add");
  if (found != string::npos){
    ManaCost * output = ManaCost::parseManaCost(s.substr(found));
    Targetable * t = NULL;
    if (spell) t = spell->getNextTarget();
    MTGAbility * a =  NEW AManaProducer(id, card, t, output, NULL, 1, who);
    a->oneShot = 1;
    return a;
  }

  //Protection from...
  found = s.find("protection from(");
  if (found == 0){
    size_t end = s.find (")", found);
    string targets = s.substr(found+16,end - found - 16);
    TargetChooserFactory tcf;
    TargetChooser * fromTc = tcf.createTargetChooser(targets, card);
    if (!fromTc) return NULL;
    fromTc->setAllZones();
    if (!activated){
      if(card->hasType("instant") || card->hasType("sorcery")  || forceUEOT){ 
         return NULL; //TODO
      }
      return NEW AProtectionFrom(id, card,target,fromTc);
    }
    return NULL; //TODO
  }

  //Can't be blocked by...
  found = s.find("cantbeblockedby(");
  if (found == 0){
    size_t end = s.find (")", found);
    string targets = s.substr(found+16,end - found - 16);
    TargetChooserFactory tcf;
    TargetChooser * fromTc = tcf.createTargetChooser(targets, card);
    if (!fromTc) return NULL;
    //default target zone to opponentbattlefield here?
    if (!activated){
      if(card->hasType("instant") || card->hasType("sorcery")  || forceUEOT){ 
         return NULL; //TODO
      }
      return NEW ACantBeBlockedBy(id, card,target,fromTc);
    }
    return NULL; //TODO
  }

 //frozen, next untap this does not untap.
  found = s.find("frozen");
  if (found != string::npos){
    MTGAbility * a = NEW AAFrozen(id,card,target);
    a->oneShot = 1;
    return a;
  }

	 //frozen, next untap this does not untap.
	found = s.find("maxlevel:");
  if (found != string::npos){
    size_t start = s.find(":",found);
    size_t end = s.find(" ",start);
    int value;
    if (end != string::npos){
			value = atoi(s.substr(start+1,end-start-1).c_str());
		}
	else{
		value = atoi(s.substr(start+1).c_str());
	}
    MTGAbility * a = NEW AAWhatsMax(id,card,card,NULL,0,value);
    a->oneShot = 1;
    return a;
  }

	 //switch targest power with toughness
  found = s.find("swap");
  if (found != string::npos){
    MTGAbility * a = NEW ASwapPTUEOT(id,card,target);
    a->oneShot = 1;
    return a;
  }

  //Gain/loose simple Ability
  for (int j = 0; j < Constants::NB_BASIC_ABILITIES; j++){
    found = s.find(Constants::MTGBasicAbilities[j]);
    if (found == 0 || found == 1){
      int modifier = 1;
      if (found > 0 && s[found-1] == '-') modifier = 0;
      if (!activated){
        if(card->hasType("instant") || card->hasType("sorcery")  || forceUEOT){ 
           return NEW AInstantBasicAbilityModifierUntilEOT(id, card,target, j,modifier);
        }
        return NEW ABasicAbilityModifier(id, card,target, j,modifier);
      }
      return NEW ABasicAbilityAuraModifierUntilEOT(id, card,target, NULL,j,modifier);
    }
  }
  
  //Untapper (Ley Druid...)
  found = s.find("untap");
  if (found != string::npos){
    MTGAbility * a = NEW AAUntapper(id,card,target);
    a->oneShot = 1;
    return a;
  }

  //Tapper (icy manipulator)
  found = s.find("tap");
  if (found != string::npos){
    MTGAbility * a = NEW AATapper(id,card,target);
    a->oneShot = 1;
    return a;
  }

  DebugTrace(" no matching ability found. " << s);
  return NULL;
}

//Tells the AI if the ability should target itself or an ennemy
int AbilityFactory::abilityEfficiency(MTGAbility * a, Player * p, int mode, TargetChooser * tc){
  if (!a) return BAKA_EFFECT_DONTKNOW;

  if (GenericTargetAbility * abi = dynamic_cast<GenericTargetAbility*>(a)) {
    if (mode == MODE_PUTINTOPLAY) return BAKA_EFFECT_GOOD;
    return abilityEfficiency(abi->ability,p, mode, abi->tc);
  }
  if (GenericActivatedAbility * abi = dynamic_cast<GenericActivatedAbility*>(a)) {
    if (mode == MODE_PUTINTOPLAY) return BAKA_EFFECT_GOOD;
    return abilityEfficiency(abi->ability,p, mode,tc);
  }
  if (MultiAbility * abi = dynamic_cast<MultiAbility*>(a)) return abilityEfficiency(abi->abilities[0],p, mode,tc );
  if (MayAbility * abi = dynamic_cast<MayAbility*>(a)) return abilityEfficiency(abi->ability,p, mode,tc);
  if (ALord * abi = dynamic_cast<ALord *>(a)) {
    int myCards = countCards(abi->tc, p);
    int theirCards = countCards(abi->tc, p->opponent());
    int efficiency = abilityEfficiency(abi->ability,p, mode,tc);
    if (myCards > theirCards) return efficiency;
    return -efficiency;
  }
  if (AAsLongAs * abi = dynamic_cast<AAsLongAs *>(a)) return abilityEfficiency(abi->ability,p, mode,tc);
  if (AForeach * abi = dynamic_cast<AForeach *>(a)) return abilityEfficiency(abi->ability,p, mode,tc);
  if (dynamic_cast<AAFizzler *>(a)) return BAKA_EFFECT_BAD;
  if (dynamic_cast<AADamagePrevent *>(a)) return BAKA_EFFECT_GOOD;
	if (dynamic_cast<AACloner *>(a)) return BAKA_EFFECT_GOOD;
  if (dynamic_cast<ASwapPTUEOT *>(a)) return BAKA_EFFECT_BAD;
	if (dynamic_cast<AAUntapper *>(a)) return BAKA_EFFECT_GOOD;
  if (dynamic_cast<AATapper *>(a)) return BAKA_EFFECT_BAD;
  if (AACounter * ac = dynamic_cast<AACounter *>(a)) {
    bool negative_effect = ac->power < 0 || ac->toughness < 0;
    if ((ac->nb > 0 && negative_effect) || (ac->nb < 0 && !negative_effect)) return BAKA_EFFECT_BAD;
    return BAKA_EFFECT_GOOD ;
  }
  if (dynamic_cast<ATokenCreator *>(a)) return BAKA_EFFECT_GOOD;

  if (AAMover * aam = dynamic_cast<AAMover *>(a)) {
    MTGGameZone * z = aam->destinationZone();
    if (tc && tc->targetsZone(p->game->library)){
      if (z == p->game->hand || z == p->game->inPlay) return BAKA_EFFECT_GOOD;
    }
    return BAKA_EFFECT_BAD; //TODO
  }

  if (dynamic_cast<AACopier *>(a)) return BAKA_EFFECT_GOOD;
  if (dynamic_cast<AABuryCard *>(a)) return BAKA_EFFECT_BAD;
  if (dynamic_cast<AADestroyCard *>(a)) return BAKA_EFFECT_BAD;
  if (dynamic_cast<AStandardRegenerate *>(a)) return BAKA_EFFECT_GOOD;
  if (AALifer * abi = dynamic_cast<AALifer *>(a)) return abi->life > 0 ? BAKA_EFFECT_GOOD : BAKA_EFFECT_BAD;
  if (dynamic_cast<AADepleter *>(a)) return BAKA_EFFECT_BAD;
  if (dynamic_cast<AADrawer *>(a)) return BAKA_EFFECT_GOOD;
  if (dynamic_cast<AARandomDiscarder *>(a)) return BAKA_EFFECT_BAD;
  if (dynamic_cast<ARampageAbility *>(a)) return BAKA_EFFECT_GOOD;
	if (dynamic_cast<ABushidoAbility *>(a)) return BAKA_EFFECT_GOOD;
  if (AInstantPowerToughnessModifierUntilEOT * abi = dynamic_cast<AInstantPowerToughnessModifierUntilEOT *>(a)) return (abi->wppt->power.getValue()>=0 && abi->wppt->toughness.getValue()>=0) ? BAKA_EFFECT_GOOD : BAKA_EFFECT_BAD;
  if (APowerToughnessModifier * abi = dynamic_cast<APowerToughnessModifier *>(a)) return (abi->wppt->power.getValue()>=0 && abi->wppt->toughness.getValue()>=0) ? BAKA_EFFECT_GOOD : BAKA_EFFECT_BAD;
  if (APowerToughnessModifierUntilEndOfTurn * abi = dynamic_cast<APowerToughnessModifierUntilEndOfTurn *>(a)) return abilityEfficiency(abi->ability, p, mode,tc);

  if (dynamic_cast<ACantBeBlockedBy *>(a)) return BAKA_EFFECT_GOOD;
  if (dynamic_cast<AProtectionFrom *>(a)) return BAKA_EFFECT_GOOD;

  map<int,bool> badAbilities;
  badAbilities[Constants::CANTATTACK] = true;
  badAbilities[Constants::CANTBLOCK] = true;
  badAbilities[Constants::CLOUD] = true;
  badAbilities[Constants::DEFENDER] = true;
  badAbilities[Constants::DOESNOTUNTAP] = true;
  badAbilities[Constants::MUSTATTACK] = true;
  badAbilities[Constants::CANTREGEN] = true;

  if (AInstantBasicAbilityModifierUntilEOT * abi = dynamic_cast<AInstantBasicAbilityModifierUntilEOT *>(a)) {
      int result = badAbilities[abi->ability] ? BAKA_EFFECT_BAD : BAKA_EFFECT_GOOD;
      return (abi->value > 0) ? result : -result;
    }
  if (ABasicAbilityModifier * abi = dynamic_cast<ABasicAbilityModifier *>(a)){
    int result = (badAbilities[abi->ability]) ? BAKA_EFFECT_BAD : BAKA_EFFECT_GOOD;
    return (abi->modifier > 0) ? result : -result;
  }
  if (ABasicAbilityAuraModifierUntilEOT * abi = dynamic_cast<ABasicAbilityAuraModifierUntilEOT *>(a)) 
    return abilityEfficiency(abi->ability, p, mode);
  if (dynamic_cast<AManaProducer*>(a)) return BAKA_EFFECT_GOOD;
  return BAKA_EFFECT_DONTKNOW;
}

//Returns the "X" cost that was paid for a spell
int AbilityFactory::computeX(Spell * spell, MTGCardInstance * card){
  if (spell) return spell->computeX(card);
  return 0;
}

//Returns the "XX" cost that was paid for a spell
int AbilityFactory::computeXX(Spell * spell, MTGCardInstance * card){
  if (spell) return spell->computeXX(card);
  return 0;
}

int AbilityFactory::getAbilities(vector<MTGAbility *> * v, Spell * spell, MTGCardInstance * card, int id, MTGGameZone * dest){
 
  if (!card && spell) card = spell->source;
  if (!card) return 0;
  MTGCardInstance * target = card->target;
  if (!target) target = card;
  string magicText;
  if (dest) {
    GameObserver * g = GameObserver::GetInstance();
    for (int i = 0; i < 2 ; ++i){
      MTGPlayerCards * zones = g->players[i]->game;
      if (dest == zones->hand){
        magicText = card->magicTexts["hand"];
        break;
      }
      if (dest == zones->graveyard){
        magicText = card->magicTexts["graveyard"];
        break;
      }
	  if (dest == zones->stack){
        magicText = card->magicTexts["stack"];
        break;
      }
	  if (dest == zones->exile){
        magicText = card->magicTexts["exile"];
        break;
      }
	  if (dest == zones->library){
        magicText = card->magicTexts["library"];
        break;
      }
      //Other zones needed ?
      return 0;
    }
  }else{
    magicText = card->magicText;
  }
  if (card->alias && magicText.size() == 0 && !dest){
    MTGCard * c = GameApp::collection->getCardById(card->alias);
    if (!c) return 0;
    magicText = c->data->magicText;
  }
  string line;
  int size = magicText.size();
  if (size == 0) return 0;
  size_t found;
  int result = id;

  while (magicText.size()){
    found = magicText.find("\n");
    if (found != string::npos){
      line = magicText.substr(0,found);
      magicText = magicText.substr(found+1);
    }else{
      line = magicText;
      magicText = "";
    }

    MTGAbility * a = parseMagicLine(line, result, spell, card,0,0,0,0,dest); 
    if (a){
      v->push_back(a);
      result++;
    }else{
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
int AbilityFactory::magicText(int id, Spell * spell, MTGCardInstance * card, int mode, TargetChooser * tc,MTGGameZone * dest){
  int dryMode = 0;
  if (!spell && !dest) dryMode = 1;

  vector<MTGAbility *> v;
  int result = getAbilities(&v,spell,card,id,dest);

  for (size_t i = 0; i < v.size(); ++i){
    MTGAbility * a = v[i];
    if (dryMode){
      result = abilityEfficiency(a, card->controller(),mode,tc);
      for (size_t i = 0; i < v.size(); ++i)
        SAFE_DELETE(v[i]);
      return result;
    }

    if (a){
      if (a->oneShot){
        a->resolve();
        delete(a); 
      }else{
        a->addToGame();
      }
    }else{
      DebugTrace("ABILITYFACTORY ERROR: Parser returned NULL");
    }
  }
  return result;
 
}

void AbilityFactory::addAbilities(int _id, Spell * spell){
  MTGCardInstance * card = spell->source;


  if (spell->getNbTargets()==1){
    card->target =  spell->getNextCardTarget();
    if (card->target && !spell->tc->canTarget(card->target)){
      MTGPlayerCards * zones = card->controller()->game;
      zones->putInZone(card,spell->from,card->owner->game->graveyard);
      return; //fizzle
    }
  }
  _id = magicText(_id, spell);

  GameObserver * game = GameObserver::GetInstance();
  MTGPlayerCards * zones = card->controller()->game;


  int id = card->getId();
  if (card->alias) id = card->alias;
  switch (id){
  case 1092: //Aladdin's lamp
    {
      AAladdinsLamp * ability = NEW AAladdinsLamp(_id, card);
      game->addObserver(ability);
      break;
    }
  case 1095: //Armageddon clock
    {
      AArmageddonClock * ability = NEW AArmageddonClock(_id,card);
      game->addObserver(ability);
      break;
    }

  case 1191: //Blue Elemental Blast
    {
      if (card->target){
	card->target->controller()->game->putInGraveyard(card->target);
      }else{
	Spell * starget = spell->getNextSpellTarget();
	game->mLayers->stackLayer()->Fizzle(starget);
      }
      break;
    }
  case 1282: //Chaoslace
    {
      if (card->target){
	card->target->setColor(Constants::MTG_COLOR_RED, 1);
      }else{
	Spell * starget = spell->getNextSpellTarget();
	starget->source->setColor(Constants::MTG_COLOR_RED, 1);
      }
      break;
    }
  case 1335: //Circle of protection : black
    {
      game->addObserver(NEW ACircleOfProtection( _id,card, Constants::MTG_COLOR_BLACK));
      break;
    }
  case 1336: //Circle of protection : blue
    {
      game->addObserver(NEW ACircleOfProtection( _id,card, Constants::MTG_COLOR_BLUE));
      break;
    }
  case 1337: //Circle of protection : green
    {
      game->addObserver(NEW ACircleOfProtection( _id,card, Constants::MTG_COLOR_GREEN));
      break;
    }
  case 1338: //Circle of protection : red
    {
      game->addObserver(NEW ACircleOfProtection( _id,card, Constants::MTG_COLOR_RED));
      break;
    }
  case 1339: //Circle of protection : white
    {
      game->addObserver(NEW ACircleOfProtection( _id,card, Constants::MTG_COLOR_WHITE));
      break;
    }
  case 1101: //clockwork Beast
    {
      game->addObserver(NEW AClockworkBeast(_id,card));
      break;
    }
  case 1102: //Conservator
    {
      game->addObserver(NEW AConservator(_id,card));
      break;
    }

  case 1103: //Crystal Rod
    {
      int cost[] = {Constants::MTG_COLOR_ARTIFACT, 1};
      ASpellCastLife* ability = NEW ASpellCastLife(_id, card, Constants::MTG_COLOR_BLUE,NEW ManaCost(cost,1) , 1);
      game->addObserver(ability);
      break;
    }
  case 1152: //Deathlace
    {
      if (card->target){
	card->target->setColor(Constants::MTG_COLOR_BLACK, 1);
      }else{
	Spell * starget = spell->getNextSpellTarget();
	starget->source->setColor(Constants::MTG_COLOR_BLACK, 1);
      }
      break;
    }
  case 1284: //Dragon Whelp
    {
      game->addObserver(NEW ADragonWhelp(_id,card));
      break;
    }

  case 1345: //Farmstead
    {
      game->addObserver(NEW AFarmstead(_id, card,card->target));
      break;
    }
  case 1291: //Fireball
    {
      int x = computeX(spell,card); 
      game->addObserver(NEW AFireball(_id, card,spell, x));
      break;
    }
  case 1112: //Howling Mine
    {
      game->addObserver(NEW AHowlingMine(_id, card));
      break;
    }
  case 1113: //Iron Star
    {
      int cost[] = {Constants::MTG_COLOR_ARTIFACT, 1};
      ASpellCastLife* ability = NEW ASpellCastLife(_id, card, Constants::MTG_COLOR_RED,NEW ManaCost(cost,1) , 1);
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
      int cost[] = {Constants::MTG_COLOR_ARTIFACT, 1};
      ASpellCastLife* ability = NEW ASpellCastLife(_id, card, Constants::MTG_COLOR_WHITE,NEW ManaCost(cost,1) , 1);
      game->addObserver(ability);
      break;
    }
  case 1117: //Jandors Ring
    {
      game->addObserver(NEW AJandorsRing( _id, card));
      break;
    }
  case 1121: //Kormus Bell
    {
      game->addObserver(NEW AConvertLandToCreatures(id, card, "swamp"));
      break;
    }
  case 1254: //Kudzu
    {
      game->addObserver(NEW AKudzu(id, card, card->target));
      break;
    }
  case 1257: //Lifelace
    {
      if (card->target){
	card->target->setColor(Constants::MTG_COLOR_GREEN, 1);
      }else{
	Spell * starget = spell->getNextSpellTarget();
	starget->source->setColor(Constants::MTG_COLOR_GREEN, 1);
      }
      break;
    }

  case 1259: //Living lands
    {
      game->addObserver(NEW AConvertLandToCreatures(id, card, "forest"));
      break;
    }
  case 1124: //Mana Vault (the rest is softcoded!)
    {
      game->addObserver(NEW ARegularLifeModifierAura(_id+2, card, card, Constants::MTG_PHASE_DRAW, -1, 1));
      break;
    }
  case 1215: //Power Leak
    {
      game->addObserver( NEW APowerLeak(_id ,card, card->target));
      break;
    }
  case 1358: //Purelace
    {
      if (card->target){
	card->target->setColor(Constants::MTG_COLOR_WHITE, 1);
      }else{
	Spell * starget = spell->getNextSpellTarget();
	starget->source->setColor(Constants::MTG_COLOR_WHITE, 1);
      }
      break;
    }
  case 1312: //Red Elemental Blast
    {
      if (card->target){
		  card->target->controller()->game->putInGraveyard(card->target);
	  }else{
		  Spell * starget = spell->getNextSpellTarget();
		  game->mLayers->stackLayer()->Fizzle(starget);
	  }
	  break;
    }

  case 1139: //The Rack
    {
      game->addObserver( NEW ALifeZoneLink(_id ,card, Constants::MTG_PHASE_UPKEEP, -3));
      break;
    }
   
  case 1140: //Throne of Bone
    {
      int cost[] = {Constants::MTG_COLOR_ARTIFACT, 1};
      ASpellCastLife* ability = NEW ASpellCastLife(_id, card, Constants::MTG_COLOR_BLACK,NEW ManaCost(cost,1) , 1);
      game->addObserver(ability);
      break;
    }

  case 1142: //Wooden Sphere
    {
      int cost[] = {Constants::MTG_COLOR_ARTIFACT, 1};
      ASpellCastLife* ability = NEW ASpellCastLife(_id, card, Constants::MTG_COLOR_GREEN,NEW ManaCost(cost,1) , 1);
      game->addObserver(ability);
      break;
    }
  case 1143: //Animate Dead
    {
      AAnimateDead * a = NEW AAnimateDead(_id, card, card->target);
      game->addObserver(a);
      card->target = ((MTGCardInstance * )a->target);
      break;
    }
  case 1156: //Drain Life
    {
      Damageable * target = spell->getNextDamageableTarget();
      int x = spell->cost->getConvertedCost() - 2; //TODO Fix that !!! + X should be only black mana, that needs to be checked !
      game->mLayers->stackLayer()->addDamage(card, target, x);
      if (target->life < x) x = target->life;
      game->currentlyActing()->life+=x;
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
      if (player){
	for (int i = 0; i < 2; i++){
	  MTGInPlay * inplay = game->players[i]->game->inPlay;
	  for (int j= inplay->nb_cards -1 ; j >=0 ; j--){
	    MTGCardInstance * card = inplay->cards[j];
	    if (card->owner == player && card->hasType("artifact")){
	      player->game->putInZone(card, inplay, player->game->hand);
	    }
	  }
	}
      }
      break;
    }
  case 1165: //Hypnotic Specter
    {
      game->addObserver(NEW AHypnoticSpecter( _id, card));
      break;
    }
  case 1258: //Living Artifact
    {
      game->addObserver(NEW ALivingArtifact( _id, card, card->target));
      break;
    }
  case 1209: //Mana Short
    {
      Player * player = spell->getNextPlayerTarget();
      if (player){
	MTGInPlay * inplay = player->game->inPlay;
	for (int i = 0; i < inplay->nb_cards; i++){
	  MTGCardInstance * current = inplay->cards[i];
    if (current->hasType(Subtypes::TYPE_LAND)) current->tap();
	}
	player->getManaPool()->init();
      }
      break;
    }
  case 1167: //Mind Twist
    {
      int xCost = computeX(spell,card);
      for (int i = 0; i < xCost; i++){
	game->opponent()->game->discardRandom(game->opponent()->game->hand,card);
      }
      break;
    }
  case 1172: //Pestilence
    {
      game->addObserver(NEW APestilence(_id, card));
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
      int x = computeX(spell,card);
      Spell * starget = spell->getNextSpellTarget();
      if (starget){
	if (starget->cost->getConvertedCost() <= x) game->mLayers->stackLayer()->Fizzle(starget);
      }
      break;
    }
  case 1194: //Control Magic
    {
      game->addObserver(NEW AControlStealAura(_id, card, card->target));
      break;
    }
  case 1235: //Aspect of Wolf
    {
      game->addObserver(NEW AAspectOfWolf(_id, card, card->target));
      break;
    }
  case 1231: //Volcanic Eruption
    {
      int x = computeX(spell,card);
      int _x = x;
      MTGCardInstance * target = spell->getNextCardTarget();
      while(target && _x){
        target->destroy();
	      _x--;
	      target = spell->getNextCardTarget(target);
      }
      x-=_x;
      for (int i = 0; i < 2 ; i++){
	      game->mLayers->stackLayer()->addDamage(card, game->players[i], x);
	      for (int j = 0; j < game->players[i]->game->inPlay->nb_cards; j++){
	        MTGCardInstance * current =  game->players[i]->game->inPlay->cards[j];
	        if (current->isCreature()){
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
      if (damage){
	game->mLayers->stackLayer()->addDamage(card,damage->source->controller(),damage->damage);
      }
      break;
    }
  case 1243: //Fastbond
    {
      game->addObserver(NEW AFastbond(_id, card));
      break;
    }
  case 1238: //Cockatrice
    {
      game->addObserver(NEW AOldSchoolDeathtouch(_id,card));
      break;
    }
  case 1225: //Stasis
    {
      game->addObserver(NEW AStasis(_id, card));
      break;
    }
  case 1267: //Thicket Basilic
    {
      game->addObserver(NEW AOldSchoolDeathtouch(_id,card));
      break;
    }
  case 1227: //Toughtlace
    {
      if (card->target){
	card->target->setColor(Constants::MTG_COLOR_BLUE, 1);
      }else{
	Spell * starget = spell->getNextSpellTarget();
	starget->source->setColor(Constants::MTG_COLOR_BLUE, 1);
      }
      break;
    }

    //Addons Legends
  case 1533: //Livingplane
    {
      game->addObserver(NEW AConvertLandToCreatures(id, card, "land"));
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
      game->addObserver(NEW AMinionofLeshrac( _id, card));
      break;
    }

  case 2732: //Kjeldoran Frostbeast
	{
	  game->addObserver(NEW AKjeldoranFrostbeast(_id,card));
      break;
	}

    // --- addon Mirage ---

	  case 3410: //Seed of Innocence
    {
		GameObserver * game = GameObserver::GetInstance();
      for (int i = 0; i < 2 ; i++){
		for (int j = 0; j < game->players[i]->game->inPlay->nb_cards; j++){
			MTGCardInstance * current =  game->players[i]->game->inPlay->cards[j];
			if (current->hasType("Artifact")){
				game->players[i]->game->putInGraveyard(current);
				current->controller()->life+= current->getManaCost()->getConvertedCost();
			}
		}
	  }
      break;
	}
 
//-- addon 10E---

	case 129710: //Angelic Chorus
		{
			game->addObserver( NEW AAngelicChorus(_id,card));
			break;
		}

	case 129767: //Threaten
		{
			game->addObserver( NEW AInstantControlSteal(_id,card,card->target));
			break;
		}
  case 130373: //Lavaborn Muse
    {
      game->addObserver( NEW ALavaborn(_id ,card, Constants::MTG_PHASE_UPKEEP, -3,-3));
      break;
    }
  case 129774: // Traumatize
	  {
		  int nbcards;
		  Player * player = spell->getNextPlayerTarget();
		  MTGLibrary * library = player->game->library;
		  nbcards = (library->nb_cards)/2;
		  for (int i = 0; i < nbcards; i++){
			  if (library->nb_cards)
				  player->game->putInZone(library->cards[library->nb_cards-1],library, player->game->graveyard);
			}
		  break;
	  }


  case 135215: //Sylvan Basilisk
    {
      game->addObserver( NEW ABasilik (_id ,card));
      break;
    }
  case 130553:// Beacon of Immortality
	  {
		  Player * player = spell->getNextPlayerTarget();
		  if (player->life < (INT_MAX / 4) ) player->life += player->life;
		  zones->putInZone(card,spell->from,zones->library);
		  zones->library->shuffle();
		  break;
	  }
   case 135262:// Beacon of Destruction & unrest
	  {
		  zones->putInZone(card,spell->from,zones->library);
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
	    for (int j = card->controller()->game->inPlay->nb_cards-1; j >=0 ; --j){
			  MTGCardInstance * current =  card->controller()->game->inPlay->cards[j];
        if (current->hasType(Subtypes::TYPE_CREATURE)){
				  card->controller()->game->putInGraveyard(current);
				  damage+= current->power;
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
			for (int i = 0; i < nbcards; i++){
				if (graveyard->nb_cards)
					player->game->putInZone(graveyard->cards[graveyard->nb_cards-1],graveyard, library);
			}
			library->shuffle();
			break;
		}


// --- addon Ravnica---
  
	case 89114:	//Psychic Drain
    {
	Player * player = spell->getNextPlayerTarget();
	MTGLibrary * library = player->game->library;
	int x = computeX(spell,card);
	for (int i = 0; i < x; i++){
				if (library->nb_cards)
				player->game->putInZone(library->cards[library->nb_cards-1],library, player->game->graveyard);
	}
	game->currentlyActing()->life+= x;
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

  if (card->basicAbilities[Constants::EXALTED]){
    game->addObserver(NEW AExalted(_id, card));
  }

	if (card->basicAbilities[Constants::FLANKING]){
    game->addObserver(NEW AFlankerAbility(_id, card));
  }

  // Tested works the first r10 did not function because of the mistake in the array of the definition
  if (card->basicAbilities[Constants::FORESTHOME]){
    game->addObserver(NEW AStrongLandLinkCreature(_id, card, "forest"));
  }
  if (card->basicAbilities[Constants::ISLANDHOME]){
    game->addObserver(NEW AStrongLandLinkCreature(_id, card, "island"));
  }
  if (card->basicAbilities[Constants::MOUNTAINHOME]){
    game->addObserver(NEW AStrongLandLinkCreature(_id, card,"moutain"));
  }
  if (card->basicAbilities[Constants::SWAMPHOME]){
    game->addObserver(NEW AStrongLandLinkCreature(_id, card,"swamp"));
  }
  if (card->basicAbilities[Constants::PLAINSHOME]){
    game->addObserver(NEW AStrongLandLinkCreature(_id, card,"plains"));
  }

  if (card->hasType("instant") || card->hasType("sorcery")){    
    MTGPlayerCards * zones = card->controller()->game;
	if(card->boughtback > 0){
		zones->putInZone(card,zones->stack,zones->hand);
	}
	else if(card->flashedback > 0){
		zones->putInZone(card,zones->stack,zones->exile);
	}
	else{
		zones->putInZone(card,zones->stack,zones->graveyard);}
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
  if ( s.find( Constants::kManaColorless ) != string::npos) {
    manaCost = s.substr( s.find(",")+ 1, endIndex );
    color = Constants::MTG_COLOR_ARTIFACT;
  }
  else if (s.find( Constants::kManaGreen ) != string::npos){
    manaCost = s.substr( s.find(",")+ 1, endIndex );
    color = Constants::MTG_COLOR_GREEN;
  }
  else if ( s.find( Constants::kManaBlue ) != string::npos){
    manaCost = s.substr( s.find(",")+ 1, endIndex );
    color = Constants::MTG_COLOR_BLUE;
  }
  else if ( s.find( Constants::kManaRed ) != string::npos){
    manaCost = s.substr( s.find(",") + 1, endIndex );
    color = Constants::MTG_COLOR_RED;
  }
  else if ( s.find( Constants::kManaBlack ) != string::npos){
    manaCost = s.substr( s.find(",")+ 1, endIndex );
    color = Constants::MTG_COLOR_BLACK;
  }
  else if ( s.find( Constants::kManaWhite ) != string::npos){
    manaCost = s.substr( s.find(",")+ 1, endIndex );
    color = Constants::MTG_COLOR_WHITE;
  }
  else
  {
    DebugTrace("An error has happened in creating a Mana Redux Ability! " << s );
    return NULL;
  }
  // figure out the mana cost
  int amount = atoi(manaCost.c_str());
  return NEW AManaRedux(id, card, target, amount, color);
}




MTGAbility::MTGAbility(int id, MTGCardInstance * card):ActionElement(id){
  game = GameObserver::GetInstance();
  source = card;
  target = card;
  aType = MTGAbility::UNKNOWN;
  cost = NULL;
  forceDestroy = 0;
  oneShot = 0;
}

MTGAbility::MTGAbility(int id, MTGCardInstance * _source,Targetable * _target ):ActionElement(id){
  game = GameObserver::GetInstance();
  source = _source;
  target = _target;
  aType = MTGAbility::UNKNOWN;
  cost = NULL;
  forceDestroy = 0;
  oneShot = 0;
}

int MTGAbility::stillInUse(MTGCardInstance * card){
  if (card==source || card==target) return 1; 
  return 0;
}

MTGAbility::~MTGAbility(){
  if (!isClone){
    SAFE_DELETE(cost);
  }
}

int MTGAbility::addToGame(){
  GameObserver::GetInstance()->addObserver(this);
  return 1;
}

int MTGAbility::removeFromGame(){
  GameObserver::GetInstance()->removeObserver(this);
  return 1;
}

//returns 1 if this ability needs to be removed from the list of active abilities
int MTGAbility::testDestroy(){
  if (game->mLayers->stackLayer()->has(this)) return 0;
  if (waitingForAnswer) return 0;
  if (forceDestroy == 1) return 1;
  if (forceDestroy == -1) return 0;
  if (!game->isInPlay(source) ) return 1;
  if (target && !game->isInPlay((MTGCardInstance *)target)) return 1;
  return 0;
}



int MTGAbility::fireAbility(){
  game->mLayers->stackLayer()->addAbility(this);
  return 1;
}

ostream& MTGAbility::toString(ostream& out) const
{
  return out << "MTGAbility ::: menuText : " << menuText
	     << " ; game : " << game
	     << " ; forceDestroy : " << forceDestroy
	     << " ; cost : " << cost
	     << " ; target : " << target
	     << " ; aType : " << aType
	     << " ; source : " << source;
}

NestedAbility::NestedAbility(MTGAbility * _ability){
  ability = _ability;
}

//

ActivatedAbility::ActivatedAbility(int id, MTGCardInstance * card, ManaCost * _cost, int restrictions,int tap):MTGAbility(id,card), restrictions(restrictions), needsTapping(tap){
  cost = _cost;
  abilityCost = 0;
}


int ActivatedAbility::isReactingToClick(MTGCardInstance * card, ManaCost * mana){
  Player * player = game->currentlyActing();
  int cPhase = game->getCurrentGamePhase();
  switch(restrictions) {
    case PLAYER_TURN_ONLY:
      if (player != game->currentPlayer) return 0;
      break;
    case AS_SORCERY:
      if (player != game->currentPlayer) return 0;
      if (cPhase != Constants::MTG_PHASE_FIRSTMAIN && cPhase != Constants::MTG_PHASE_SECONDMAIN) return 0;
      break;
  }
  if (restrictions>= MY_BEFORE_BEGIN && restrictions <= MY_AFTER_EOT){
      if (player != game->currentPlayer) return 0;
      if (cPhase != restrictions - MY_BEFORE_BEGIN + Constants::MTG_PHASE_BEFORE_BEGIN) return 0;
  }

  if (restrictions>= OPPONENT_BEFORE_BEGIN && restrictions <= OPPONENT_AFTER_EOT){
      if (player == game->currentPlayer) return 0;
      if (cPhase != restrictions - OPPONENT_BEFORE_BEGIN + Constants::MTG_PHASE_BEFORE_BEGIN) return 0;
  }

  if (restrictions>= BEFORE_BEGIN && restrictions <= AFTER_EOT){
      if (cPhase != restrictions - BEFORE_BEGIN + Constants::MTG_PHASE_BEFORE_BEGIN) return 0;
  }

  if (card == source && source->controller()==player && (!needsTapping || (!source->isTapped() && !source->hasSummoningSickness()))){
    if (!cost) return 1;
    cost->setExtraCostsAction(this, card);
    if (!mana) mana = player->getManaPool();
    if (!mana->canAfford(cost)) return 0;
    if (!cost->canPayExtra()) return 0;
    return 1;
  }
  return 0;
}

int ActivatedAbility::reactToClick(MTGCardInstance * card){
//  if (cost) cost->setExtraCostsAction(this, card);
  if (!isReactingToClick(card)) return 0;
  Player * player = game->currentlyActing();
  if (cost){
    if (!cost->isExtraPaymentSet()){
      game->waitForExtraPayment = cost->extraCosts;
      return 0;
    }
    ManaCost * previousManaPool = NEW ManaCost(player->getManaPool());
    game->currentlyActing()->getManaPool()->pay(cost);
    cost->doPayExtra();
    SAFE_DELETE(abilityCost);
    abilityCost = previousManaPool->Diff(player->getManaPool());
    delete previousManaPool;
  }
  if (needsTapping && source->isInPlay()) source->tap();
  fireAbility();

  return 1;

}

int ActivatedAbility::reactToTargetClick(Targetable * object){
  if (!isReactingToTargetClick(object)) return 0;
  Player * player = game->currentlyActing();
  if (cost){
    if (object->typeAsTarget() == TARGET_CARD) cost->setExtraCostsAction(this, (MTGCardInstance *) object);
    if (!cost->isExtraPaymentSet()){
      game->waitForExtraPayment = cost->extraCosts;
      return 0;
    }
    ManaCost * previousManaPool = NEW ManaCost(player->getManaPool());
    game->currentlyActing()->getManaPool()->pay(cost);
    cost->doPayExtra();
    SAFE_DELETE(abilityCost);
    abilityCost = previousManaPool->Diff(player->getManaPool());
    delete previousManaPool;
  }
  if (needsTapping  && source->isInPlay()) source->tap();
  fireAbility();
  return 1;

}

ActivatedAbility::~ActivatedAbility(){
  //Ok, this will probably lead to crashes, maybe with lord abilities involving "X" costs.
  // If that's the case, we need to improve the clone() method of GenericActivatedAbility and GenericTargetAbility, I think
  // Erwan 2004/04/25
  //if (!isClone){
    SAFE_DELETE(abilityCost);
  //}
}

ostream& ActivatedAbility::toString(ostream& out) const
{
  out << "ActivatedAbility ::: restrictions : " << restrictions
      << " ; needsTapping : " << needsTapping
      << " (";
  return MTGAbility::toString(out) << ")";
}


TargetAbility::TargetAbility(int id, MTGCardInstance * card, TargetChooser * _tc,ManaCost * _cost, int _playerturnonly,int tap):ActivatedAbility(id, card,_cost,_playerturnonly, tap), NestedAbility(NULL){
  tc = _tc;
}

TargetAbility::TargetAbility(int id, MTGCardInstance * card,ManaCost * _cost, int _playerturnonly,int tap):ActivatedAbility(id, card,_cost,_playerturnonly, tap), NestedAbility(NULL){
  tc = NULL;
}


int TargetAbility::reactToTargetClick(Targetable * object){
  if (object->typeAsTarget() == TARGET_CARD) return reactToClick((MTGCardInstance *)object);
  if (waitingForAnswer){
      if (tc->toggleTarget(object) == TARGET_OK_FULL){
	      waitingForAnswer = 0;
        game->mLayers->actionLayer()->setCurrentWaitingAction(NULL);
	      return ActivatedAbility::reactToClick(source);
      }
      return 1;
  }
  return 0;
}


int TargetAbility::reactToClick(MTGCardInstance * card){
  if (!waitingForAnswer) {
    if (isReactingToClick(card)){
      waitingForAnswer = 1;
      game->mLayers->actionLayer()->setCurrentWaitingAction(this);
      tc->initTargets();
      return 1;
    }
  }else{
    if (card == source && (tc->targetsReadyCheck() == TARGET_OK || tc->targetsReadyCheck() == TARGET_OK_FULL)){
      waitingForAnswer = 0;
      game->mLayers->actionLayer()->setCurrentWaitingAction(NULL);
      return ActivatedAbility::reactToClick(source);
    }else{
      if (tc->toggleTarget(card) == TARGET_OK_FULL){
	      int result = ActivatedAbility::reactToClick(source);
        if (result) {
          waitingForAnswer = 0;
          game->mLayers->actionLayer()->setCurrentWaitingAction(NULL);
        }
        return result;
      }
      return 1;
    }
  }
  return 0;
}

void TargetAbility::Render(){
  //TODO ?
}


int TargetAbility::resolve(){
  Targetable * t = tc->getNextTarget();
  if (t && ability){
    ManaCost * diff = abilityCost->Diff(cost);
    source->X = diff->hasX();
    delete (diff);
    ability->target = t;
    if (ability->oneShot) return ability->resolve();
    MTGAbility * a =  ability->clone();
    return a->addToGame();
  }
  return 0;
}

const char * TargetAbility::getMenuText(){
  if (ability) return ability->getMenuText();
  return ActivatedAbility::getMenuText();
}

TargetAbility::~TargetAbility(){
  if (!isClone) SAFE_DELETE(ability);
}

ostream& TargetAbility::toString(ostream& out) const
{
  out << "TargetAbility ::: (";
  return ActivatedAbility::toString(out) << ")";
}

//


TriggeredAbility::TriggeredAbility(int id, MTGCardInstance * card, Targetable * _target):MTGAbility(id,card, _target){
}


TriggeredAbility::TriggeredAbility(int id, MTGCardInstance * card):MTGAbility(id,card){
}

int TriggeredAbility::receiveEvent(WEvent * e){
  if (triggerOnEvent(e)){
    fireAbility();
    return 1;
  }
  return 0; 
}

void TriggeredAbility::Update(float dt){
  if (trigger()) fireAbility();
}

ostream& TriggeredAbility::toString(ostream& out) const
{
  out << "TriggeredAbility ::: (";
  return MTGAbility::toString(out) << ")";
}


//
InstantAbility::InstantAbility(int _id, MTGCardInstance * source):MTGAbility(_id, source){
  init = 0;
}

void InstantAbility::Update(float dt){
  if (!init){
    init = resolve();
  }
}

InstantAbility::InstantAbility(int _id, MTGCardInstance * source, Damageable * _target):MTGAbility(_id, source, _target){
  init = 0;
}



//Instant abilities last generally until the end of the turn
int InstantAbility::testDestroy(){
  int newPhase = game->getCurrentGamePhase();
  if (newPhase != currentPhase && newPhase == Constants::MTG_PHASE_AFTER_EOT) return 1;
  currentPhase = newPhase;
  return 0;

}

ostream& InstantAbility::toString(ostream& out) const
{
  out << "InstantAbility ::: init : " << init
      << " (";
  return MTGAbility::toString(out) << ")";
}

bool ListMaintainerAbility::canTarget(MTGGameZone * zone){
  if (tc) return tc->targetsZone(zone);
  for (int i = 0; i < 2; i++){
    Player * p = game->players[i];
    if (zone == p->game->inPlay) return true;
  }
  return false;
}

void ListMaintainerAbility::updateTargets(){
  //remove invalid ones
  map<MTGCardInstance *,bool> temp;
  for (map<MTGCardInstance *,bool>::iterator it=cards.begin(); it != cards.end(); ++it){
    MTGCardInstance * card = (*it).first;
    if (!canBeInList(card)) temp[card] = true;
  }

  for (map<MTGCardInstance *,bool>::iterator it=temp.begin(); it != temp.end(); ++it){
    MTGCardInstance * card = (*it).first;
    cards.erase(card);
    removed(card);
  }

  temp.clear();

  //add New valid ones
  for (int i = 0; i < 2; i++){
    Player * p = game->players[i];
    MTGGameZone * zones[] = {p->game->inPlay,p->game->graveyard,p->game->hand,p->game->library};
    for (int k = 0; k < 4; k++){
      MTGGameZone * zone = zones[k];
      if (canTarget(zone)){
        for (int j = 0; j < zone->nb_cards; j++){
	        if (canBeInList(zone->cards[j])){
	          if(cards.find(zone->cards[j]) == cards.end()){
	            temp[zone->cards[j]] = true;
            }
          }
        }
      }
    }
  }

  for (map<MTGCardInstance *,bool>::iterator it=temp.begin(); it != temp.end(); ++it){
    MTGCardInstance * card = (*it).first;
    cards[card] = true;
    added(card);
  }

  temp.clear();

  for (int i = 0; i < 2; ++i){
    Player * p = game->players[i];
    if (!players[p] && canBeInList(p)){
      players[p] = true;
      added(p);
    }else if (players[p] && !canBeInList(p)){
      players[p] = false;
      removed(p);
    }
  }

}

void ListMaintainerAbility::Update(float dt){
  updateTargets();
}

//Destroy the spell -> remove all targets
int ListMaintainerAbility::destroy(){
  map<MTGCardInstance *,bool>::iterator it = cards.begin();

  while ( it!=cards.end()){
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



TriggerAtPhase::TriggerAtPhase(int id, MTGCardInstance * source, Targetable * target,int _phaseId, int who):TriggeredAbility(id, source,target),phaseId(_phaseId),who(who){
  GameObserver * g = GameObserver::GetInstance();
  if (g) {
    newPhase = g->getCurrentGamePhase();
    currentPhase = newPhase;
  }
}

int TriggerAtPhase::trigger(){
  if (testDestroy()) return 0; // http://code.google.com/p/wagic/issues/detail?id=426
  GameObserver * g = GameObserver::GetInstance();
  int result = 0;
  if (currentPhase != newPhase && newPhase == phaseId){
    result = 0;
    switch(who){
      case 1:
        if(g->currentPlayer == source->controller()) result = 1;
        break;
      case -1:
        if(g->currentPlayer != source->controller()) result = 1;
        break;
      case -2:
        if(source->target) {
          if (g->currentPlayer == source->target->controller()) result = 1;
        }else {
          if(g->currentPlayer == source->controller()) result = 1;
        }
        break;
      default:
        result = 1;
        break;
    }
  }
  return result;
}

TriggerAtPhase* TriggerAtPhase::clone() const{
    TriggerAtPhase * a =  NEW TriggerAtPhase(*this);
    a->isClone = 1;
    return a;
}

TriggerNextPhase::TriggerNextPhase(int id, MTGCardInstance * source, Targetable * target,int _phaseId,int who):TriggerAtPhase(id, source,target,_phaseId, who){
  destroyActivated = 0;
}

int TriggerNextPhase::testDestroy(){
  //dirty hack because of http://code.google.com/p/wagic/issues/detail?id=426
  if (newPhase <= phaseId && !destroyActivated) destroyActivated = 1;
  if (destroyActivated > 1 || (newPhase > phaseId && destroyActivated)){
    destroyActivated++;
    return 1;
  }
  return 0;
}

TriggerNextPhase* TriggerNextPhase::clone() const{
    TriggerNextPhase * a =  NEW TriggerNextPhase(*this);
    a->isClone = 1;
    return a;
}

GenericTriggeredAbility::GenericTriggeredAbility(int id, MTGCardInstance * _source,  TriggeredAbility * _t, MTGAbility * a , MTGAbility * dc, Targetable * _target ): TriggeredAbility(id, _source,_target), NestedAbility(a){
  if (!target) target = source;
  t = _t;
  destroyCondition = dc;

  t->source = source;
  t->target = target;
  ability->source = source;
  ability->target = target;
  if (destroyCondition){
    destroyCondition->source = source;
    destroyCondition->target = target;
  }
}

int GenericTriggeredAbility::trigger(){
  return t->trigger();
}


int GenericTriggeredAbility::triggerOnEvent(WEvent * e){
  if (t->triggerOnEvent(e)) {
    targets.push(getTriggerTarget(e,ability));
    return 1;
  }
  return 0;
}

Targetable * GenericTriggeredAbility::getTriggerTarget(WEvent * e,MTGAbility * a){
  TriggerTargetChooser * ttc = dynamic_cast<TriggerTargetChooser *>(a->tc);
  if (ttc)
    return e->getTarget(ttc->triggerTarget);
  
  NestedAbility * na = dynamic_cast<NestedAbility *>(a);
  if (na) return getTriggerTarget(e,na->ability);

  MultiAbility * ma = dynamic_cast<MultiAbility *>(a);
  if (ma) {
    for (size_t i = 0; i < ma->abilities.size(); i++) {
      return getTriggerTarget(e,ma->abilities[i]);
    }
  }

  return NULL;
}

void GenericTriggeredAbility::setTriggerTargets(Targetable * ta ,MTGAbility * a){
  TriggerTargetChooser * ttc = dynamic_cast<TriggerTargetChooser *>(a->tc);
  if (ttc) {
    a->target = ta;
    ttc->target = ta;
  }
  
  NestedAbility * na = dynamic_cast<NestedAbility *>(a);
  if (na) setTriggerTargets(ta,na->ability);

  MultiAbility * ma = dynamic_cast<MultiAbility *>(a);
  if (ma) {
    for (size_t i = 0; i < ma->abilities.size(); i++) {
      setTriggerTargets(ta,ma->abilities[i]);
    }
  }
}

void GenericTriggeredAbility::Update(float dt){
  GameObserver * g = GameObserver::GetInstance();
  int newPhase = g->getCurrentGamePhase();
  t->newPhase = newPhase;
  TriggeredAbility::Update(dt);
  t->currentPhase = newPhase;
}

int GenericTriggeredAbility::resolve(){
  if (targets.size()) {
    setTriggerTargets(targets.front() ,ability);
    targets.pop();
  }
  if (ability->oneShot) return ability->resolve();
  MTGAbility * clone = ability->clone();
  clone->addToGame();
  return 1;
}

int GenericTriggeredAbility::testDestroy(){
  if (!TriggeredAbility::testDestroy()) return 0;
  if (destroyCondition) return (destroyCondition->testDestroy());
  return t->testDestroy();
}

GenericTriggeredAbility::~GenericTriggeredAbility(){
  if (!isClone){
    delete t;
    delete ability;
    SAFE_DELETE(destroyCondition);
  }
}

 const char * GenericTriggeredAbility::getMenuText(){
   return ability->getMenuText();
 }

GenericTriggeredAbility* GenericTriggeredAbility::clone() const{
    GenericTriggeredAbility * a = NEW GenericTriggeredAbility(*this);
    a->isClone = 1;
    return a;
}

/*Mana Producers (lands)
//These have a reactToClick function, and therefore two manaProducers on the same card conflict with each other
//That means the player has to choose one. although that is perfect for cards such as birds of paradise or badlands,
other solutions need to be provided for abilities that add mana (ex: mana flare)
*/


AManaProducer::AManaProducer(int id, MTGCardInstance * card, Targetable * t, ManaCost * _output, ManaCost * _cost , int doTap, int who):ActivatedAbilityTP(id, card,t,_cost,doTap,who){

  aType = MTGAbility::MANA_PRODUCER;
  cost = _cost;
  output = _output;
 
  menutext = "";
}

  int AManaProducer::isReactingToClick(MTGCardInstance *  _card, ManaCost * mana){
    int result = 0;
    if (!mana) mana = game->currentlyActing()->getManaPool();
    if (_card == source && (!tap || !source->isTapped())  && game->currentlyActing()->game->inPlay->hasCard(source) && (source->hasType(Subtypes::TYPE_LAND) || !tap || !source->hasSummoningSickness()) ){
		if (!cost || mana->canAfford(cost))
			{
	result =  1;
			}
    }
    return result;
  }

  int AManaProducer::resolve(){
    Targetable * _target = getTarget();
    Player * player;
    if (_target){
      if (_target->typeAsTarget() == TARGET_CARD){
        player = ((MTGCardInstance *)_target)->controller();
      }else{
        player = (Player *) _target;
      }
      player->getManaPool()->add(output,source);
      return 1;
    }
    return 0;
  }

  int AManaProducer::reactToClick(MTGCardInstance *  _card){
    if (!isReactingToClick( _card)) return 0;
    if (cost){
      cost->setExtraCostsAction(this, _card);
      if (!cost->isExtraPaymentSet()){
        GameObserver::GetInstance()->waitForExtraPayment = cost->extraCosts;
        return 0;
      }
      GameObserver::GetInstance()->currentlyActing()->getManaPool()->pay(cost);
      cost->doPayExtra();
    }
		if (tap){
	  GameObserver *g = GameObserver::GetInstance();
    WEvent * e = NEW WEventCardTappedForMana(source, 0, 1);
    g->receiveEvent(e);
		source->tap();
		}

    if (options[Options::SFXVOLUME].number > 0){
      JSample * sample = resources.RetrieveSample("mana.wav");
      if (sample) JSoundSystem::GetInstance()->PlaySample(sample);
    }
    return resolve();
  }


  const char * AManaProducer::getMenuText(){
    if (menutext.size())return menutext.c_str();
    menutext = _("Add ");
    char buffer[128];
    int alreadyHasOne = 0;
    for (int i= 0; i < 6; i++){
      int value = output->getCost(i);
      if (value){
        if (alreadyHasOne) menutext.append(",");
        sprintf(buffer, "%i ", value);
        menutext.append(buffer);
        switch (i){
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


  AManaProducer::~AManaProducer(){
    SAFE_DELETE(cost);
    SAFE_DELETE(output);
  }

  AManaProducer * AManaProducer::clone() const{
    AManaProducer * a =  NEW AManaProducer(*this);
    a->cost = NEW ManaCost();
    a->output = NEW ManaCost();
    a->cost->copy(cost);
    a->output->copy(output);
    a->isClone = 1;
    return a;
  }



  ActivatedAbilityTP::ActivatedAbilityTP(int id, MTGCardInstance * card, Targetable * _target, ManaCost * cost, int doTap, int who):ActivatedAbility(id,card,cost,0,doTap),who(who){
    if (_target) target = _target;
  }

  Targetable * ActivatedAbilityTP::getTarget(){
    switch(who){
      case TargetChooser::TARGET_CONTROLLER:
        if (target){
          switch(target->typeAsTarget()) {
            case TARGET_CARD:
              return ((MTGCardInstance *)target)->controller();
            case TARGET_STACKACTION:
              return((Interruptible *)target)->source->controller();
            default:
              return (Player *)target;
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
