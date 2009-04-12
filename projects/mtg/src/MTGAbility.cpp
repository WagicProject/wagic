#include "../include/config.h"
#include "../include/MTGAbility.h"
#include "../include/ManaCost.h"
#include "../include/MTGGameZones.h"
#include "../include/AllAbilities.h"
#include "../include/Damage.h"
#include "../include/TargetChooser.h"
#include "../include/CardGui.h"
#include "../include/MTGDeck.h"


int AbilityFactory::countCards(TargetChooser * tc, Player * player, int option){
  int result = 0;
  GameObserver * game = GameObserver::GetInstance();
  for (int i = 0; i < 2 ; i++){
    if (player && player!= game->players[i]) continue;
    for (int j = game->players[i]->game->inPlay->nb_cards-1; j >=0 ; j--){
      MTGCardInstance * current =  game->players[i]->game->inPlay->cards[j];
      if (tc->canTarget(current)){
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
  return result;
}

int AbilityFactory::destroyAllInPlay(TargetChooser * tc, int bury){
  tc->source = NULL; // This is to prevent protection from... as objects that destroy all do not actually target
  GameObserver * game = GameObserver::GetInstance();
  for (int i = 0; i < 2 ; i++){
    for (int j = game->players[i]->game->inPlay->nb_cards-1; j >=0 ; j--){
      MTGCardInstance * current =  game->players[i]->game->inPlay->cards[j];
      if (tc->canTarget(current)){
	if (bury){
	  game->players[i]->game->putInGraveyard(current);
	}else{
	  game->mLayers->stackLayer()->addPutInGraveyard(current);
	}
      }
    }
  }
  return 1;
}


int AbilityFactory::putInPlayFromZone(MTGCardInstance * card, MTGGameZone * zone, Player * p){
  MTGCardInstance * copy = p->game->putInZone(card,  zone, p->game->stack);
  Spell * spell = NEW Spell(copy);
  spell->resolve();
  delete spell;
  return 1;
}

Damageable * AbilityFactory::parseCollateralTarget(MTGCardInstance * card, string s){
  size_t found = s.find("controller");
  if (found != string::npos) return card->controller();
  return NULL;
}

int AbilityFactory::parsePowerToughness(string s, int *power, int *toughness){
    size_t found = s.find("/");
    if (found != string::npos){
      int search_from = found - 4;
      if (search_from < 0) search_from = 0;
      size_t start = s.find(':', search_from);
      if (start == string::npos) start = s.find(" ", search_from);
      if (start == string::npos) start = -1;
      *power = atoi(s.substr(start+1,s.size()-found).c_str());
      size_t end = s.find(" ",start);
      if (end != string::npos){
	      *toughness = atoi(s.substr(found+1,end-found-1).c_str());
      }else{
	      *toughness = atoi(s.substr(found+1).c_str());
      }
      return 1;
    }
    return 0;
}

Trigger * AbilityFactory::parseTrigger(string magicText){
  size_t found = magicText.find("@");
  if (found == string::npos) return NULL;

  //Next Time...
  found = magicText.find("next");
  if (found != string::npos){
    for (int i = 0; i < Constants::NB_MTG_PHASES; i++){
      found = magicText.find(Constants::MTGPhaseCodeNames[i]);
      if (found != string::npos){
	      return NEW TriggerNextPhase(i);
      }
    }
  }

  return NULL;
}




//Some basic functionalities that can be added automatically in the text file
/*
 * Several objects are computed from the text string, and have a direct influence on what action we should take
 * (direct impact on the game such as draw a card immediately, or create a new GameObserver and add it to the Abilities,etc..)
 * These objects are:
 *   - trigger (if there is an "@" in the string, this is a triggered ability)
 *   - target (if there ie a "target(" in the string, then this is a TargetAbility)
 *   - doTap (a dirty way to know if tapping is included in the cost...
 */
int AbilityFactory::magicText(int id, Spell * spell, MTGCardInstance * card){
  int dryMode = 0;
  if (!spell) dryMode = 1;
  int dryModeResultSet = 0;
  int dryModeResult = 0;

  GameObserver * game = GameObserver::GetInstance();
  if (!card) card = spell->source;
  MTGCardInstance * target = card->target;
  if (!target) target = card;
  string magicText = card->magicText;
  if (card->alias && magicText.size() == 0){
    //An awful way to get access to the aliasedcard
    magicText = GameObserver::GetInstance()->players[0]->game->collection->getCardById(card->alias)->magicText;
  }
  string line;
  int size = magicText.size();
  if (size == 0) return 0;
  unsigned int found;
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
#if defined (WIN32) || defined (LINUX)
    char buf[4096];
    sprintf(buf, "AUTO ACTION: %s\n", line.c_str());
    OutputDebugString(buf);
#endif


    MultiAbility * multi = NULL;
    unsigned int delimiter = line.find("}:");
    ManaCost * cost = NULL;
    if (delimiter!= string::npos){
      cost = ManaCost::parseManaCost(line.substr(0,delimiter+1));
    }
    OutputDebugString("Parsing cost\n");
    if (cost && cost->isNull()){
      OutputDebugString("Cost is null\n");
      SAFE_DELETE(cost);
    }
  
    int may = 0;
    if (line.find("may ") != string::npos) may = 1;

    int doTap = 0;
    //Tap in the cost ?
    if (line.find("{t}") != string::npos) doTap = 1;

    TargetChooser * tc;
    TargetChooser * lordTargets;
    Trigger * trigger;
    while (line.size()){
      string s;
      found = line.find("&&");
      if (found != string::npos){
        s = line.substr(0,found);
        line = line.substr(found+2);
        if (!multi){
          OutputDebugString("Multi initializing\n");
          if (!dryMode) {
            multi = NEW MultiAbility(id, card, cost,doTap);
            game->addObserver(multi);
          }
          OutputDebugString("Multi initialized\n");
        }
      }else{
        s = line;
        line = "";
      }

      tc = NULL;
      lordTargets = NULL;
      int lordIncludeSelf = 1;
      int lordType = 0;
      string lordTargetsString;

      trigger = parseTrigger(s);
      //Dirty way to remove the trigger text (could get in the way)
      if (trigger){
        found = s.find(":");
        s = s.substr(found+1);
      }


      //Target Abilities
      found = s.find("target(");
      if (found != string::npos){
        int end = s.find(")", found);
        string starget = s.substr(found + 7,end - found - 7);
        TargetChooserFactory tcf;
        tc = tcf.createTargetChooser(starget, card);

      }

      //Lord
      found = s.find("lord(");
      if (found != string::npos){
        if (dryMode) {
          dryModeResult = BAKA_EFFECT_GOOD;
          dryModeResultSet = 1;
          break;
        }
        unsigned int end = s.find(")", found+5);
        if (end != string::npos){
	        lordTargetsString = s.substr(found+5,end-found-5).c_str();
          lordType = PARSER_LORD;
        }
      }
      found = s.find("foreach(");
      if (found != string::npos){
        if (dryMode) {
          dryModeResult = BAKA_EFFECT_GOOD;
          dryModeResultSet = 1;
          break;
        }
        unsigned int end = s.find(")", found+8);
        if (end != string::npos){
	        lordTargetsString = s.substr(found+8,end-found-8).c_str();
          lordType = PARSER_FOREACH;
        }
      }
      found = s.find("aslongas(");
      if (found != string::npos){
        if (dryMode) {
          dryModeResult = BAKA_EFFECT_GOOD;
          dryModeResultSet = 1;
          break;
        }
        unsigned int end = s.find(")", found+9);
        if (end != string::npos){
	        lordTargetsString = s.substr(found+9,end-found-9).c_str();
          lordType = PARSER_ASLONGAS;
        }
      }
      if (lordTargetsString.size()){
          TargetChooserFactory tcf;
          lordTargets = tcf.createTargetChooser(lordTargetsString, card);
          if (s.find("other") != string::npos) lordIncludeSelf = 0;
      }


      //Untapper (Ley Druid...)
      found = s.find("untap");
      if (found != string::npos){
        if (dryMode) {
          dryModeResult = BAKA_EFFECT_GOOD;
          dryModeResultSet = 1;
          break;
        }
        if (tc){
	        game->addObserver(NEW AUntaper(id, card, cost, tc));
        }else{
	         target->tapped = 0;
        }
        result++;
        continue;
      }


      //Regeneration
      found = s.find("}:regenerate");
      if (found != string::npos){
        if (dryMode) {
          dryModeResult = BAKA_EFFECT_GOOD;
          dryModeResultSet = 1;
          break;
        }

        if (lordTargets){
	        game->addObserver(NEW ALord(id,card,lordTargets,lordIncludeSelf,0,0,-1,cost));
        }else{
	        if (tc){
	          //TODO
	        }else{
	          game->addObserver(NEW AStandardRegenerate(id, card, target, cost));
	          //TODO death ward !
	        }
        }
        result++;
        continue;
      }


      //Token creator. Name, type, p/t, abilities
      found = s.find("token(");
      if (found != string::npos){
        if (dryMode) {
          dryModeResult = BAKA_EFFECT_GOOD;
          dryModeResultSet = 1;
          break;
        }
        int end = s.find(",", found);
        string sname = s.substr(found + 6,end - found - 6);
        int previous = end+1;
        end = s.find(",",previous);
        string stypes = s.substr(previous,end - previous);
        previous = end+1;
        end = s.find(",",previous);
        string spt = s.substr(previous,end - previous);
        int power, toughness;
        parsePowerToughness(spt,&power, &toughness);
        string sabilities = s.substr(end+1);
        int multiplier = 1;
        found = s.find("*");
        if (found != string::npos)multiplier = atoi(s.substr(found+1).c_str());
        if(cost || doTap){
          game->addObserver(NEW ATokenCreator(id,card,cost,sname,stypes,power,toughness,sabilities,doTap));
        }else{
          ATokenCreator * tok = NEW ATokenCreator(id,card,cost,sname,stypes,power,toughness,sabilities,doTap);
          for (int i=0; i < multiplier; i++){
            tok->resolve();
          }
          delete tok;
        }
        result++;
        continue;
      }

      //MoveTo Move a card from a zone to another
      found = s.find("moveto(");
      if (found != string::npos){
        if (dryMode) {
          dryModeResult = BAKA_EFFECT_BAD;
          dryModeResultSet = 1;
          break;
        } //TODO : depends on where from, where to...
	      int end = s.find(")",found+1);
	      string szone = s.substr(found + 7,end - found - 7);
        if (tc){
          //if (cost){
            AZoneMover * a = NEW AZoneMover(id,card,tc,szone,cost,doTap);
            if (may){
              game->addObserver(NEW MayAbility(id,a,card));
            }else{
              game->addObserver(a);
            }
         // }
        }else{
          if (cost){
            MTGAbility * a = NEW AZoneSelfMover(id,card,szone,cost,doTap);
            if (may){
              game->addObserver(NEW MayAbility(id,a,card));
            }else{
              game->addObserver(a);
            }
          }else{
            MTGGameZone * fromZone = target->getCurrentZone();//this is technically incorrect. The initial zone should be as described in the targetchooser
            MTGGameZone * destZone = MTGGameZone::stringToZone(szone, card, target);
            target->controller()->game->putInZone(target,fromZone,destZone);
          }
        }
        result++;
        continue;
      }

      //Copy a target
      found = s.find("copy ");
      if (found != string::npos){
        if (dryMode) {
          dryModeResult = BAKA_EFFECT_GOOD;
          dryModeResultSet = 1;
          break;
        } //TODO : 
        if (tc){
            ACopier * a = NEW ACopier(id,card,tc,cost);
            if (may){
              game->addObserver(NEW MayAbility(id,a,card));
              OutputDebugString("may!\n");
            }else{
              game->addObserver(a);
            }
        }else{
         //TODO
        }
        result++;
        continue;
      }

      //Bury
      found = s.find("bury");
      if (found != string::npos){
        if (trigger){
          if (dryMode) {
            dryModeResult = BAKA_EFFECT_BAD;
            dryModeResultSet = 1;
            break;
          }
	        BuryEvent * action = NEW BuryEvent();
	        game->addObserver(NEW GenericTriggeredAbility(id, card,trigger,action));
        }else{
	        found = s.find("all(");
	        if (found != string::npos){
	          int end = s.find(")");
	          string starget = s.substr(found + 4,end - found - 4);
            TargetChooserFactory tcf;
            TargetChooser * targetAll = tcf.createTargetChooser(starget, card);
            if (dryMode){
              int myNbCards = countCards(targetAll,card->controller());
              int opponentNbCards = countCards(targetAll, card->controller()->opponent());
              int myCardsPower = countCards(targetAll,card->controller(),COUNT_POWER);
              int opponentCardsPower = countCards(targetAll, card->controller()->opponent(),COUNT_POWER);
              delete targetAll;
              if (myNbCards < opponentNbCards || myCardsPower < opponentCardsPower) dryModeResult =  BAKA_EFFECT_GOOD;
              else dryModeResult =  BAKA_EFFECT_BAD;
              break;
            }else{
              if (cost){
                game->addObserver(NEW AAllDestroyer(id, card,targetAll,1,cost,doTap));
              }else{
                this->destroyAllInPlay(targetAll,1);
                delete targetAll;
              }
            }
	        }else{
            if (dryMode){
              dryModeResult = BAKA_EFFECT_BAD;
              break;
            }
	          if (tc){
	            game->addObserver(NEW ABurier(id, card,tc));
	          }else{
	            target->controller()->game->putInGraveyard(target);
	          }
	        }
        }
        result++;
        continue;
      }

      //Destroy
      found = s.find("destroy");
      if (found != string::npos){

        found = s.find("all(");
        if (found != string::npos){
	        int end = s.find(")");
	        string starget = s.substr(found + 4,end - found - 4);
	        TargetChooserFactory tcf;
	        TargetChooser * targetAll = tcf.createTargetChooser(starget, card);
	        if (dryMode){
	          int myNbCards = countCards(targetAll,card->controller());
	          int opponentNbCards = countCards(targetAll, card->controller()->opponent());
	          int myCardsPower = countCards(targetAll,card->controller(),COUNT_POWER);
	          int opponentCardsPower = countCards(targetAll, card->controller()->opponent(),COUNT_POWER);
	          delete targetAll;
	          if (myNbCards < opponentNbCards || myCardsPower < opponentCardsPower) dryModeResult =  BAKA_EFFECT_GOOD;
            else dryModeResult =  BAKA_EFFECT_BAD;
            break;
	        }else{
             if (cost){
                game->addObserver(NEW AAllDestroyer(id, card,targetAll,0,cost,doTap));
              }else{
                this->destroyAllInPlay(targetAll);
                delete targetAll;
              }
	        }
        }else{
          if (dryMode){
            dryModeResult =  BAKA_EFFECT_BAD;
            break;
          }
	        if (tc){
	          game->addObserver(NEW ADestroyer(id, card,tc,0,cost));
	        }else{
	          game->mLayers->stackLayer()->addPutInGraveyard(target);
	        }
        }
        result++;
        continue;
      }

      //Damage
      found = s.find("damage");
      if (found != string::npos){
        unsigned int start = s.find(":",found);
        if (start == string::npos) start = s.find(" ",found);
        unsigned int end = s.find(" ",start);
        int damage;
        if (end != string::npos){
	        damage = atoi(s.substr(start+1,end-start-1).c_str());
        }else{
	        damage = atoi(s.substr(start+1).c_str());
        }
        if (dryMode){
            dryModeResult =  BAKA_EFFECT_BAD;
            break;
        }
        if (tc){
	        MTGAbility * a = NEW ADamager(id, card, cost, damage, tc,doTap);
          if (multi){
            multi->Add(a);
          }else{
            game->addObserver(a);
          }
        }else{
          if (multi){
            Damageable * target = parseCollateralTarget(card, s);
            if (!target) target = spell->getNextDamageableTarget();
            if (!target)OutputDebugString("NO TARGET FOR DAMAGE\n");
            multi->Add(NEW DamageEvent(card,target,damage));
          }else{
	          game->mLayers->stackLayer()->addDamage(card,spell->getNextDamageableTarget(), damage);
          }
        }
        result++;
        continue;
      }

      //gain/lose life
      found = s.find("life:");
      if (found != string::npos){
        unsigned int start = found+4;
        unsigned int end = s.find(" ",start);
        int life;
        if (end != string::npos){
	        life = atoi(s.substr(start+1,end-start-1).c_str());
        }else{
	        life = atoi(s.substr(start+1).c_str());
        }
        if (dryMode){
          dryModeResult =  BAKA_EFFECT_GOOD;
          break;
        }
        if (tc){
	        //TODO ?
        }else{
	        if (!cost && !doTap){
	          card->controller()->life+=life;
	        }else{
            game->addObserver(NEW ALifeGiver(id, card,cost, life, doTap));
	        }
        }
        result++;
        continue;
      }

      //Draw
      found = s.find("draw:");
      if (found != string::npos){
        unsigned int start = s.find(":",found);
        unsigned int end = s.find(" ",start);
        int nbcards;
        if (end != string::npos){
	        nbcards = atoi(s.substr(start+1,end-start-1).c_str());
        }else{
	        nbcards = atoi(s.substr(start+1).c_str());
        }
        if (dryMode){
          dryModeResult =  BAKA_EFFECT_GOOD;
          break;
        }
        if (trigger){
	        DrawEvent * action = NEW DrawEvent(card->controller(),nbcards);
	        game->addObserver(NEW GenericTriggeredAbility(id, card,trigger,action));
        }else{
	        if (tc){
	          //TODO ?
	        }else{
	          if (!cost){
	            game->mLayers->stackLayer()->addDraw(card->controller(),nbcards);
	          }else{
	            game->addObserver(NEW ADrawer(id,card,cost,nbcards,doTap));
	          }
	        }
        }
        result++;
        continue;
      }

      //Change Power/Toughness
      int power, toughness;
      if ( parsePowerToughness(s,&power, &toughness)){
        if (dryMode){
	        if (power >=0 && toughness >= 0 ) {
            dryModeResult =  BAKA_EFFECT_GOOD;
          }else{
            dryModeResult =  BAKA_EFFECT_BAD;
          }
          break;
        }
        int limit = 0;
        unsigned int limit_str = s.find("limit:");
        if (limit_str != string::npos){
	        limit = atoi(s.substr(limit_str+6).c_str());
        }


        if (lordType == PARSER_LORD){
	        game->addObserver(NEW ALord(id,card,lordTargets,lordIncludeSelf,power,toughness));
        }else{
	        if(tc){
	            game->addObserver(NEW ATargetterPowerToughnessModifierUntilEOT(id, card,power,toughness, cost, tc,doTap));
	        }else{
            if (lordType == PARSER_FOREACH){
	            game->addObserver(NEW AForeach(id,card,target,lordTargets,lordIncludeSelf,power,toughness));
            }else if (lordType == PARSER_ASLONGAS){
	            game->addObserver(NEW AKirdApe(id,card,lordTargets,lordIncludeSelf,power,toughness));
            }else{
	            if (!cost){
	              if(card->hasType("enchantment")){
	                game->addObserver(NEW APowerToughnessModifier(id, card, target,power,toughness));
	              }else{
	                game->addObserver(NEW AInstantPowerToughnessModifierUntilEOT(id, card, target,power,toughness));
	              }
	            }else{
	              game->addObserver(NEW APowerToughnessModifierUntilEndOfTurn(id, card, target,power,toughness, cost, limit));
	            }
            }
	        }
        }
        result++;
        continue;
      }

      //Mana Producer
      found = s.find("add");
      if (found != string::npos){
        if (dryMode){
          dryModeResult =  BAKA_EFFECT_GOOD;
          break;
        }
        ManaCost * input = ManaCost::parseManaCost(s.substr(0,found));
        ManaCost * output = ManaCost::parseManaCost(s.substr(found));
        if (!input->isNull() || doTap){
          SAFE_DELETE(cost); //erk
          if (input->isNull()){
            SAFE_DELETE(input);
          }
          MTGAbility * a = NEW AManaProducer(id, target, output, input,doTap);
          if (multi){
            multi->Add(a);
          }else{
	          game->addObserver(a);
          }
        }else{
          OutputDebugString ("uh oh\n");
          card->controller()->getManaPool()->add(output);
          delete output;
        }
        result++;
        continue;
      }

      //Gain/loose Ability
      for (int j = 0; j < Constants::NB_BASIC_ABILITIES; j++){
        found = s.find(Constants::MTGBasicAbilities[j]);
        if (found!= string::npos){
	        int modifier = 1;
	        if (found > 0 && s[found-1] == '-') modifier = 0;
	        if (dryMode){
	          if (j == Constants::DEFENDER){
	            if (modifier == 1) dryModeResult = BAKA_EFFECT_BAD;
	            else dryModeResult = BAKA_EFFECT_GOOD;
	          }else{
	            if (modifier == 1) dryModeResult = BAKA_EFFECT_GOOD;
              else dryModeResult = BAKA_EFFECT_BAD;
	          }
            dryModeResultSet = 1;
            break;
          }else{
	          if (lordType == PARSER_LORD){
	            game->addObserver(NEW ALord(id,card,lordTargets,lordIncludeSelf,0,0,j,0,modifier));
	          }else if (lordType == PARSER_ASLONGAS){
	              game->addObserver(NEW AKirdApe(id,card,lordTargets,lordIncludeSelf,0,0,j,modifier));
            }else{
	            if (tc){
	              game->addObserver(NEW ABasicAbilityModifierUntilEOT(id, card, j, cost,tc, modifier));
	            }else{
	              if (!cost){
	                if(card->hasType("enchantment")){
		                game->addObserver(NEW ABasicAbilityModifier(id, card,target, j,modifier));
	                }else{
		                game->addObserver(NEW AInstantBasicAbilityModifierUntilEOT(id, card,target, j,modifier));
	                }
	              }else{
	                game->addObserver(NEW ABasicAbilityAuraModifierUntilEOT(id, card,target, cost,j,modifier));
	              }
	            }
	          }
	          result++;
	          continue;
          }
        }
      }
      if (dryModeResultSet) break;

      //Tapper (icy manipulator)
      found = s.find("tap");
      if (found != string::npos){
        if (dryMode){
          dryModeResult = BAKA_EFFECT_GOOD;
          break;
        }
        if (tc){
          game->addObserver(NEW ATapper(id, card, cost, tc));
        }else{
          target->tapped = 1;
        }
        result++;
        continue;
      }
#if defined (WIN32) || defined (LINUX)
    char buf[4096];
    sprintf(buf, "AUTO ACTION PARSED: %s\n", line.c_str());
    OutputDebugString(buf);
#endif
    } 
    if (dryMode){
      SAFE_DELETE(tc);
      SAFE_DELETE(lordTargets);
      SAFE_DELETE(multi);
      SAFE_DELETE(cost);
      SAFE_DELETE(trigger);
      return dryModeResult;
    }
  }

  return result;
}

void AbilityFactory::addAbilities(int _id, Spell * spell){
  MTGCardInstance * card = spell->source;

  if (spell->cursor==1) card->target =  spell->getNextCardTarget();
  _id = magicText(_id, spell);
  int putSourceInGraveyard = 0; //For spells that are not already InstantAbilities;


  GameObserver * game = GameObserver::GetInstance();
  int id = card->getId();
  if (card->alias) id = card->alias;
  switch (id){
  case 1092: //Aladdin's lamp
    {
      AAladdinsLamp * ability = NEW AAladdinsLamp(_id, card);
      game->addObserver(ability);
      break;
    }
  case 130550: //Ancestor's chosen
    {
      int life = card->controller()->game->graveyard->nb_cards;
      card->controller()->life+= life;
      break;
    }
  case 1190: //Animate Artifact
    {
      int x =  card->target->getManaCost()->getConvertedCost();
      game->addObserver(NEW AConvertToCreatureAura(_id, card,card->target,x,x));
      break;
    }
  case 1094: //Ank Of Mishra
    {
      AAnkhOfMishra * ability = NEW AAnkhOfMishra(_id,card);
      game->addObserver(ability);
      break;
    }
  case 1095: //Armageddon clock
    {
      AArmageddonClock * ability = NEW AArmageddonClock(_id,card);
      game->addObserver(ability);
      break;
    }

  case 1096: //Basalt Monolith
    {
      int cost[] = {Constants::MTG_COLOR_ARTIFACT, 3};
      AManaProducer * ability = NEW AManaProducer(_id, card, NEW ManaCost(cost,1));
      AUntapManaBlocker * ability2 = NEW AUntapManaBlocker(_id+1, card, NEW ManaCost(cost,1));
      AUnBlocker * ability3 = NEW AUnBlocker(_id+1, card,card, NEW ManaCost(cost,1));

      game->addObserver(ability);
      game->addObserver(ability2);
      game->addObserver(ability3);
      break;
    }
  case 1097: //Black Vise
    {
      game->addObserver( NEW ALifeZoneLink(_id ,card, Constants::MTG_PHASE_UPKEEP, 4));
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
  case 1099: //Brass Man
    {
      int cost[] = {Constants::MTG_COLOR_ARTIFACT, 1};
      game->addObserver(NEW AUntapManaBlocker(_id, card, NEW ManaCost(cost,1)));
      break;
    }
  case 1237: //Channel
    {
      game->addObserver(NEW AChannel(_id, card));
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
  case 1196: //Counterspell
    {
      Spell * starget = spell->getNextSpellTarget();
      if (starget) game->mLayers->stackLayer()->Fizzle(starget);
      break;
    }
  case 1197: //Creature Bond
    {
      game->addObserver(NEW ACreatureBond(_id,card, card->target));
      break;
    }
  case 1103: //Crystal Rod
    {
      int cost[] = {Constants::MTG_COLOR_BLUE, 1};
      ASpellCastLife* ability = NEW ASpellCastLife(_id, card, Constants::MTG_COLOR_WHITE,NEW ManaCost(cost,1) , 1);
      game->addObserver(ability);
      break;
    }
  case 1151: //Deathgrip
    {
      int _cost[] = {Constants::MTG_COLOR_BLACK, 2};
      game->addObserver(NEW ASpellCounterEnchantment(_id, card, NEW ManaCost(_cost, 1),Constants::MTG_COLOR_GREEN));
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
  case 1105: //dingus Egg
    {
      ADingusEgg * ability = NEW ADingusEgg(_id,card);
      game->addObserver(ability);
      break;
    }
  case 1106: //Disrupting Scepter
    {
      ADisruptingScepter * ability = NEW ADisruptingScepter(_id,card);
      game->addObserver(ability);
      break;
    }
  case 1284: //Dragon Whelp
    {
      game->addObserver(NEW ADragonWhelp(_id,card));
      break;
    }
  case 1108: //Ebony Horse
    {
      AEbonyHorse * ability = NEW AEbonyHorse(_id,card);
      game->addObserver(ability);
      break;
    }
  case 1345: //Farmstead
    {
      game->addObserver(NEW AFarmstead(_id, card,card->target));
      break;
    }
  case 1291: //Fireball
    {
      int x = spell->cost->getConvertedCost() - 1; //TODO BETTER
      game->addObserver(NEW AFireball(_id, card,spell, x));
      break;
    }
  case 1245: //Force of Nature
    {
      game->addObserver(NEW AForceOfNature(_id,card));
      break;
    }
  case 1110: //Glasses Of Urza
    {
      AGlassesOfUrza * ability = NEW AGlassesOfUrza(_id,card);
      game->addObserver(ability);
      break;
    }
  case 1112: //Howling Mine
    {
      game->addObserver(NEW AHowlingMine(_id, card));
      break;
    }
  case 1252: //Instill Energy
    {
      game->addObserver(NEW AUntaperOnceDuringTurn(_id, card, card->target, NEW ManaCost()));
      break;
    }
  case 1113: //Iron Star
    {
      int cost[] = {Constants::MTG_COLOR_ARTIFACT, 1};
      ASpellCastLife* ability = NEW ASpellCastLife(_id, card, Constants::MTG_COLOR_RED,NEW ManaCost(cost,1) , 1);
      game->addObserver(ability);
      break;
    }
  case 1351: // Island Sancturay
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
  case 1115: //Ivory Tower
    {
      game->addObserver(NEW ALifeZoneLink(_id ,card, Constants::MTG_PHASE_UPKEEP, 4, 1, 1));
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
  case 1256: //LifeForce
    {
      int _cost[] = {Constants::MTG_COLOR_GREEN, 2};
      game->addObserver(NEW ASpellCounterEnchantment(_id, card, NEW ManaCost(_cost, 1),Constants::MTG_COLOR_BLACK));
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
  case 1205: //Lifetap
    {
      game->addObserver(NEW AGiveLifeForTappedType(_id, card, "forest"));
      break;
    }
  case 1259: //Living lands
    {
      game->addObserver(NEW AConvertLandToCreatures(id, card, "forest"));
      break;
    }
  case 1124: //Mana Vault
    {
      int output[] = {Constants::MTG_COLOR_ARTIFACT, 3};
      game->addObserver(NEW AManaProducer(_id,card,NEW ManaCost(output,1)));
      int cost[] = {Constants::MTG_COLOR_ARTIFACT, 4};
      game->addObserver(NEW AUntapManaBlocker(_id+1, card, NEW ManaCost(cost,1)));
      game->addObserver(NEW ARegularLifeModifierAura(_id+2, card, card, Constants::MTG_PHASE_DRAW, -1, 1));
      break;
    }
  case 1126:// Millstone
    {
      game->addObserver( NEW AMillstone(_id ,card));
      break;
    }
  case 1215: //Power Leak
    {
      game->addObserver( NEW APowerLeak(_id ,card, card->target));
      break;
    }
  case 1311: //Power Surge
    {
      game->addObserver( NEW APowerSurge(_id ,card));
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
  case 1136: //Soul Net
    {
      game->addObserver( NEW ASoulNet(_id ,card));
      break;
    }
  case 1139: //The Rack
    {
      game->addObserver( NEW ALifeZoneLink(_id ,card, Constants::MTG_PHASE_UPKEEP, -3));
      break;
    }
  case 1140: //Throne of bones
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
  case 1148 : //Cursed lands
    {
      game->addObserver(NEW AWanderlust(_id, card, card->target));
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
  case 1164: //Howl from beyond
    {
      int x = spell->cost->getConvertedCost() - 1; //TODO, this is not enough, Spells shouls have a function like "xCost" because the spell might cost more than expected to launch
      AInstantPowerToughnessModifierUntilEOT * ability = NEW AInstantPowerToughnessModifierUntilEOT( _id, card, card->target, x, 0);
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
  case 1166: //Lord Of The Pit
    {
      game->addObserver(NEW ALordOfThePit( _id, card));
      break;
    }
  case 1209: //Mana Short
    {
      Player * player = spell->getNextPlayerTarget();
      if (player){
	MTGInPlay * inplay = player->game->inPlay;
	for (int i = 0; i < inplay->nb_cards; i++){
	  MTGCardInstance * current = inplay->cards[i];
	  if (current->hasType("land")) current->tapped = 1;
	}
	player->getManaPool()->init();
      }
      break;
    }
  case 1167: //Mind Twist
    {
      int xCost = spell->cost->getConvertedCost() - 1;
      for (int i = 0; i < xCost; i++){
	game->opponent()->game->discardRandom(game->opponent()->game->hand);
      }
      break;
    }
  case 1171: //Paralysis
    {
      int cost[] = {Constants::MTG_COLOR_ARTIFACT, 4};
      game->addObserver(NEW AUntapManaBlocker(_id, card,card->target, NEW ManaCost(cost,1)));
      card->target->tapped = 1;
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
      int x = spell->cost->getConvertedCost() - 1;
      Spell * starget = spell->getNextSpellTarget();
      if (starget){
	if (starget->cost->getConvertedCost() <= x) game->mLayers->stackLayer()->Fizzle(starget);
      }
      break;
    }
  case 1185: //Warp Artifact
    {
      game->addObserver(NEW ARegularLifeModifierAura(_id, card, card->target, Constants::MTG_PHASE_UPKEEP, -1));
      break;
    }
  case 1192:	//BrainGeyser
    {
      Player * player = ((Player * )spell->targets[0]);
      int x = spell->cost->getConvertedCost() - 2;
      for (int i = 0; i < x ; i++){
	player->game->drawFromLibrary();
      }
      break;
    }
  case 1194: //Control Magic
    {
      game->addObserver(NEW AControlStealAura(_id, card, card->target));
      break;
    }

  case 1200 : //Feedback
    {
      game->addObserver(NEW AWanderlust(_id, card, card->target));
      break;
    }

  case 1203: //Island Fish
    {
      int cost[] = {Constants::MTG_COLOR_BLUE, 3};
      game->addObserver(NEW AUntapManaBlocker(_id, card, NEW ManaCost(cost,1)));
      game->addObserver(NEW AStrongLandLinkCreature(_id, card, "island"));
      break;
    }
  case 1214: //Pirate Ship
    {
      game->addObserver(NEW AStrongLandLinkCreature(_id, card, "island"));
      game->addObserver(NEW ADamager(_id+1, card, NEW ManaCost(), 1));
      break;
    }
  case 1218: //Psychic Venom
    {
      game->addObserver(NEW APsychicVenom(_id, card, card->target));
      break;
    }
  case 1220: //Sea Serpent
    {
      game->addObserver(NEW AStrongLandLinkCreature(_id, card, "island"));
      break;
    }
  case 1221: //Serendib Efreet
    {
      game->addObserver( NEW ASerendibEfreet(_id, card));
      break;
    }
  case 1226: //Steal Artifact
    {
      game->addObserver( NEW AControlStealAura(_id, card, card->target));
      break;
    }
  case 1228: //Unstable mutation
    {
      game->addObserver(NEW APowerToughnessModifier(_id, card, card->target, 3, 3));
      game->addObserver(NEW APowerToughnessModifierRegularCounter(_id, card, card->target, Constants::MTG_PHASE_UPKEEP, -1, -1));
      break;
    }

  case 1235: //Aspect of Wolf
    {
      game->addObserver(NEW AAspectOfWolf(_id, card, card->target));
      break;
    }
  case 1236: //Birds of Paradise
    {
      for (int i = Constants::MTG_COLOR_GREEN; i <= Constants::MTG_COLOR_WHITE; i++){
	int output[]={i,1};
	game->addObserver(NEW AManaProducer(_id + i, card, NEW ManaCost(output,1)));
      }
      break;
    }
  case 1240: //Crumble
    {
      card->target->controller()->game->putInGraveyard(card->target);
      card->target->controller()->life+= card->target->getManaCost()->getConvertedCost();
      break;
    }
  case 1251: //Hurricane
    {
      int x = spell->cost->getConvertedCost() - 1;
      for (int i = 0; i < 2 ; i++){
	game->mLayers->stackLayer()->addDamage(card, game->players[i], x);
	for (int j = 0; j < game->players[i]->game->inPlay->nb_cards; j++){
	  MTGCardInstance * current =  game->players[i]->game->inPlay->cards[j];
	  if (current->basicAbilities[Constants::FLYING] && current->isACreature()){
	    game->mLayers->stackLayer()->addDamage(card, current, x);
	  }
	}
      }
      break;
    }
  case 1262: //Regeneration
    {
      int cost[] = {Constants::MTG_COLOR_GREEN, 1};
      game->addObserver(NEW AStandardRegenerate(_id,card,card->target,NEW ManaCost(cost,1)));
      break;
    }

  case 1266: //stream of life
    {
      int x = spell->cost->getConvertedCost() - 1; //TODO Improve that !
      spell->getNextPlayerTarget()->life += x;
      break;
    }


  case 1231: //Volcanic Eruption
    {
      int x = spell->cost->getConvertedCost() - 3;
      int _x = x;
      MTGCardInstance * target = spell->getNextCardTarget();
      while(target && _x){
	game->mLayers->stackLayer()->addPutInGraveyard(target);
	_x--;
	target = spell->getNextCardTarget(target);
      }
      x-=_x;
      for (int i = 0; i < 2 ; i++){
	game->mLayers->stackLayer()->addDamage(card, game->players[i], x);
	for (int j = 0; j < game->players[i]->game->inPlay->nb_cards; j++){
	  MTGCardInstance * current =  game->players[i]->game->inPlay->cards[j];
	  if (current->isACreature()){
	    game->mLayers->stackLayer()->addDamage(card, current, x);
	  }
	}
      }
      break;
    }

  case 1285: //Dwarven Warriors{
    {
      CreatureTargetChooser * tc = NEW CreatureTargetChooser(card);
      tc->maxpower = 2;
      game->addObserver(NEW ABasicAbilityModifierUntilEOT(_id, card, Constants::UNBLOCKABLE, NULL,tc));
      break;
    }
  case 1288: //EarthBind
    {
      game->addObserver(NEW AEarthbind(_id, card, card->target));
      break;
    }
  case 1289: //earthquake
    {
      int x = spell->cost->getConvertedCost() - 1;
      for (int i = 0; i < 2 ; i++){
	game->mLayers->stackLayer()->addDamage(card, game->players[i], x);
	for (int j = 0; j < game->players[i]->game->inPlay->nb_cards; j++){
	  MTGCardInstance * current =  game->players[i]->game->inPlay->cards[j];
	  if (!current->basicAbilities[Constants::FLYING] && current->isACreature()){
	    game->mLayers->stackLayer()->addDamage(card, current, x);
	  }
	}
      }
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
  case 1309: //Orcish Artillery
    {
      game->addObserver(NEW AOrcishArtillery(_id, card));
      break;
    }
  case 1326: //Wheel of fortune
    {
      for (int i = 0; i < 2; i++){
	//	MTGLibrary * library = game->players[i]->game->library;
	MTGHand * hand = game->players[i]->game->hand;
	for (int j = hand->nb_cards-1; j>=0; j--){
	  game->players[i]->game->putInGraveyard(hand->cards[j]);
	}
	for(int j = 0; j < 7; j++){
	  game->players[i]->game->drawFromLibrary();
	}
      }
      break;
    }
  case 1331: //Black Ward
    {
      game->addObserver(NEW AProtectionFrom( _id,card, card->target, Constants::MTG_COLOR_BLACK));
      break;
    }
  case 1333: //Blue  Ward
    {
      game->addObserver(NEW AProtectionFrom( _id,card, card->target, Constants::MTG_COLOR_BLUE));
      break;
    }
  case 1238: //Cockatrice
    {
      game->addObserver(NEW AOldSchoolDeathtouch(_id,card));
      break;
    }
  case 1346: //Green Ward
    {
      game->addObserver(NEW AProtectionFrom( _id,card, card->target, Constants::MTG_COLOR_GREEN));
      break;
    }
  case 1352: //Karma
    {
      game->addObserver(NEW AKarma(_id, card));
      break;
    }
  case 1355: //Northern Paladin
    {
      game->addObserver(NEW ANorthernPaladin(_id, card));
      break;
    }
  case 1359: //Red Ward
    {
      game->addObserver(NEW AProtectionFrom( _id,card, card->target, Constants::MTG_COLOR_RED));
      break;
    }
  case 1362: //Reverse polarity
    {
      ActionStack * as = game->mLayers->stackLayer();
      Player * controller = card->controller();
      Damage * current = ((Damage *)as->getNext(NULL,ACTION_SPELL, RESOLVED_OK));
      while(current){
	if (current->target == controller && current->source->hasType("artifact")){
	  controller->life+= current->damage * 2;
	}
	current = ((Damage *)as->getNext(current,ACTION_SPELL, RESOLVED_OK));
      }
      break;
    }
  case 1225: //Stasis
    {
      game->addObserver(NEW AStasis(_id, card));
      break;
    }

  case 1367: //Swords to Plowshares
    {
      Player * p = card->target->controller();
      p->life+= card->target->power;
      p->game->putInZone(card->target,p->game->inPlay,card->owner->game->removedFromGame);
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
  case 1371: //White Ward
    {
      game->addObserver(NEW AProtectionFrom( _id,card, card->target, Constants::MTG_COLOR_WHITE));
      break;
    }

    //Addons The Dark

  case 1797: //Inferno does 6 damage to all players and all creatures.
    {
      for (int i = 0; i < 2 ; i++){
	game->mLayers->stackLayer()->addDamage(card, game->players[i], 6);
	for (int j = 0; j < game->players[i]->game->inPlay->nb_cards; j++){
	  MTGCardInstance * current =  game->players[i]->game->inPlay->cards[j];
	  if (current->isACreature()){
	    game->mLayers->stackLayer()->addDamage(card, current, 6);
	  }
	}
      }
      break;
    }

  case 1773 : //People of the Woods
    {
      game->addObserver(NEW APeopleOfTheWoods(_id, card));
      break;
    }


    //Addons Legends
  case 1427: //Abomination
    {
      game->addObserver(NEW AAbomination(_id,card));
      break;
    }
  case 1533: //Livingplane
    {
      game->addObserver(NEW AConvertLandToCreatures(id, card, "land"));
      break;
    }
  case 1607: //Divine Offering
    {
      card->target->controller()->game->putInGraveyard(card->target);
      game->currentlyActing()->life+= card->target->getManaCost()->getConvertedCost();
      break;
    }
  case 1625: //Lifeblood
    {
      game->addObserver(NEW AGiveLifeForTappedType (_id, card, "island"));
      break;
    }
    //Addons ICE-AGE Cards

  case 2650: //Pyroclasm Need to be improved copied from hurricane with does 0 dammage to player and does 2 dammage to each creature
    {
      int x = 2;
      for (int i = 0; i < 2 ; i++){
	game->mLayers->stackLayer()->addDamage(card, game->players[i], 0);// To be removed ?
	for (int j = 0; j < game->players[i]->game->inPlay->nb_cards; j++){
	  MTGCardInstance * current =  game->players[i]->game->inPlay->cards[j];
	  if (current->isACreature()){
	    game->mLayers->stackLayer()->addDamage(card, current, x);
	  }
	}
      }
      break;
    }
  case 2660: //Word of Blasting
    {
      card->target->controller()->game->putInGraveyard(card->target);
      card->target->controller()->life-= card->target->getManaCost()->getConvertedCost();
      break;
    }
  case 2443: //Dark Banishing
    {
      if (card->target->hasColor(Constants::MTG_COLOR_BLACK)){
      }else{
	card->target->controller()->game->putInGraveyard(card->target);
      }
      break;
    }
  case 2593: //Thoughtleech
    {
      game->addObserver(NEW AGiveLifeForTappedType (_id, card, "island"));
      break;
    }
  case 2484: //Songs of the Damned
    {
      int mana = card->controller()->game->graveyard->countByType("creature");
      game->currentlyActing()->getManaPool()->add(Constants::MTG_COLOR_BLACK, mana);
      break;
    }

  case 2474: //Minion of Leshrac
    {
      game->addObserver(NEW AMinionofLeshrac( _id, card));
      break;
    }
  case 2421: //Shield of the Age
    {
      game->addObserver(NEW AShieldOfTheAge( _id, card));
      break;
    }
  case 2487: //Spoil of Evil
    {
      int mana_cr = game->opponent()->game->graveyard->countByType("creature");
      int mana_ar = game->opponent()->game->graveyard->countByType("artifact");
      int spoil = mana_ar + mana_cr;
      game->currentlyActing()->getManaPool()->add(Constants::MTG_COLOR_ARTIFACT, spoil);
      game->currentlyActing()->life+= spoil;
      break;
    }
  case 2435: //Whalebone Glider
    {
      int cost[] = {Constants::MTG_COLOR_ARTIFACT,2};
      CreatureTargetChooser * tc = NEW CreatureTargetChooser(card);
      tc->maxpower = 3;
      game->addObserver(NEW ABasicAbilityModifierUntilEOT(_id, card, Constants::FLYING, NEW ManaCost(cost,1),tc));
      break;
    }
  case 2393: //Aegis of the Meek work but work also for 0/1 creatures... :D
    {
      int cost[] = {Constants::MTG_COLOR_ARTIFACT,1};
      CreatureTargetChooser * tc = NEW CreatureTargetChooser(card);
      tc->maxpower = 1;
      tc->maxtoughness =1;
      game->addObserver(NEW ATargetterPowerToughnessModifierUntilEOT(id, card, 1,2, NEW ManaCost(cost,1),tc));
      break;
    }
  default:
    break;
  }

  /* Erwan - 2008/11/13: We want to get rid of these basicAbility things.
   * basicAbilities themselves are alright, but creating new object depending on them is dangerous
   * The main reason is that classes that add an ability to a card do NOT create these objects, and therefore do NOT
   * Work.
   * For example, setting LIFELINK for a creature is not enough right now...
   * It shouldn't be necessary to add an object. State based abilities could do the trick
   */


  for (int i=Constants::PROTECTIONGREEN; i <= Constants::PROTECTIONWHITE; i++){
    if (card->basicAbilities[i]){
      game->addObserver(NEW AProtectionFrom(_id, card, card, i - Constants::PROTECTIONGREEN + Constants::MTG_COLOR_GREEN));
    }
  }

  if (card->basicAbilities[Constants::EXALTED]){
    game->addObserver(NEW AExalted(_id, card));
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

  // New Abilities Flanking and Rampage

  if (card->basicAbilities [Constants::RAMPAGE1]){
    game->addObserver (NEW ARampageAbility(_id, card, 1, 1));
  }

  //Instants are put in the graveyard automatically if that's not already done
  if (!putSourceInGraveyard){
    if (card->hasType("instant") || card->hasType("sorcery")){
      putSourceInGraveyard = 1;
    }
  }
  if (putSourceInGraveyard == 1){
    MTGPlayerCards * zones = card->controller()->game;
    card = zones->putInGraveyard(card);
  }
}

MTGAbility::MTGAbility(int id, MTGCardInstance * card):ActionElement(id){
  game = GameObserver::GetInstance();
  source = card;
  target = card;
  aType = MTGAbility::UNKNOWN;
  cost = NULL;
  forceDestroy = 0;
}

MTGAbility::MTGAbility(int id, MTGCardInstance * _source,Damageable * _target ):ActionElement(id){
  game = GameObserver::GetInstance();
  source = _source;
  target = _target;
  aType = MTGAbility::UNKNOWN;
  cost = NULL;
  forceDestroy = 0;
}

MTGAbility::~MTGAbility(){
  SAFE_DELETE(cost);
}

//returns 1 if this ability needs to be removed from the list of active abilities
int MTGAbility::testDestroy(){
  if (game->mLayers->stackLayer()->has(this)) return 0;
  if (waitingForAnswer) return 0;
  if (forceDestroy) return 1;
  if (!game->isInPlay(source) ){
    OutputDebugString("Destroying Ability !!!\n");
    return 1;
  }
  if (target && !game->isInPlay((MTGCardInstance *)target)){
    source->controller()->game->putInGraveyard(source);//TODO put this in a better place ???
    return 1;
  }
  return 0;
}



int MTGAbility::fireAbility(){
  game->mLayers->stackLayer()->addAbility(this);
  return 1;
}

//

ActivatedAbility::ActivatedAbility(int id, MTGCardInstance * card, ManaCost * _cost, int _playerturnonly,int tap):MTGAbility(id,card), playerturnonly(_playerturnonly), needsTapping(tap){
  cost = _cost;
}


int ActivatedAbility::isReactingToClick(MTGCardInstance * card, ManaCost * mana){
  Player * player = game->currentPlayer;
  if (!playerturnonly) player = game->currentlyActing();
  if (card == source && source->controller()==player && player==game->currentlyActing() && (!needsTapping || (!source->isTapped() && !source->hasSummoningSickness()))){
    if (!cost) return 1;
    if (!mana) mana = player->getManaPool();
    if (!mana->canAfford(cost)) return 0;
    char buf[4096];
    sprintf(buf, "Will react to Click : %i\n", aType);
    OutputDebugString(buf);
    return 1;
  }
  return 0;
}

int ActivatedAbility::reactToClick(MTGCardInstance * card){
  if (!isReactingToClick(card)) return 0;
  OutputDebugString("React To click 1\n");
  if (cost){
    cost->setExtraCostsAction(this, card);
    OutputDebugString("React To click 2\n");
    if (!cost->isExtraPaymentSet()){
      OutputDebugString("React To click 3\n");
      
      game->waitForExtraPayment = cost->extraCosts;
      return 0;
    }
    game->currentlyActing()->getManaPool()->pay(cost);
    cost->doPayExtra();
  }
  if (needsTapping) source->tapped = 1;
  fireAbility();

  return 1;

}

int ActivatedAbility::reactToTargetClick(Targetable * object){
  if (!isReactingToTargetClick(object)) return 0;
  if (needsTapping) source->tapped = 1;
  if (cost){
    if (object->typeAsTarget() == TARGET_CARD) cost->setExtraCostsAction(this, (MTGCardInstance *) object);
    OutputDebugString("React To click 2\n");
    if (!cost->isExtraPaymentSet()){
      OutputDebugString("React To click 3\n");   
      game->waitForExtraPayment = cost->extraCosts;
      return 0;
    }
    game->currentlyActing()->getManaPool()->pay(cost);
    cost->doPayExtra();    
  }
  fireAbility();
  return 1;

}



//The whole targetAbility mechanism is messed up, mainly because of its interactions with 
// the ActionLayer, GameObserver, and parent class ActivatedAbility.
// Currently choosing a target is a complete different mechanism for put into play and for other abilities.
// It probably shouldn't be the case.

TargetAbility::TargetAbility(int id, MTGCardInstance * card, TargetChooser * _tc,ManaCost * _cost, int _playerturnonly,int tap):ActivatedAbility(id, card,_cost,_playerturnonly, tap){
  tc = _tc;
}

TargetAbility::TargetAbility(int id, MTGCardInstance * card,ManaCost * _cost, int _playerturnonly,int tap):ActivatedAbility(id, card,_cost,_playerturnonly, tap){
  tc = NULL;
}

void TargetAbility::Update(float dt){
  JGE * mEngine = JGE::GetInstance();
  if (waitingForAnswer){
    if(mEngine->GetButtonClick(PSP_CTRL_CROSS)){
      waitingForAnswer = 0;
    }else if(tc->targetsReadyCheck() == TARGET_OK_FULL){
      //waitingForAnswer = 0;
      //ActivatedAbility::reactToClick(source);
    }
  }
}

int TargetAbility::reactToTargetClick(Targetable * object){
  if (object->typeAsTarget() == TARGET_CARD) return reactToClick((MTGCardInstance *)object);
  if (waitingForAnswer){
      if (tc->toggleTarget(object) == TARGET_OK_FULL){
	      waitingForAnswer = 0;
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
      tc->initTargets();
      return 1;
    }
  }else{
    if (card == source && (tc->targetsReadyCheck() == TARGET_OK || tc->targetsReadyCheck() == TARGET_OK_FULL)){
      waitingForAnswer = 0;
      return ActivatedAbility::reactToClick(source);
    }else{
      if (tc->toggleTarget(card) == TARGET_OK_FULL){
	      
	      int result = ActivatedAbility::reactToClick(source);
        if (result) waitingForAnswer = 0;
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


//


TriggeredAbility::TriggeredAbility(int id, MTGCardInstance * card, Damageable * _target):MTGAbility(id,card, _target){
}


TriggeredAbility::TriggeredAbility(int id, MTGCardInstance * card):MTGAbility(id,card){
}

void TriggeredAbility::Update(float dt){
  if (trigger()) fireAbility();
}



//
InstantAbility::InstantAbility(int _id, MTGCardInstance * source):MTGAbility(_id, source){
  init = 0;
  for (int i = 0; i < 2; i++){
    if(game->players[i]->game->inPlay->hasCard(source)){
      game->players[i]->game->putInGraveyard(source);
    }
  }
}

void InstantAbility::Update(float dt){
  if (!init){
    init = resolve();
  }
}

InstantAbility::InstantAbility(int _id, MTGCardInstance * source, Damageable * _target):MTGAbility(_id, source, _target){
  init = 0;
  for (int i = 0; i < 2; i++){
    if(game->players[i]->game->inPlay->hasCard(source)){
      game->players[i]->game->putInGraveyard(source);
    }
  }
}



//Instant abilities last generally until the end of the turn
int InstantAbility::testDestroy(){
  int newPhase = game->getCurrentGamePhase();
  if (newPhase != currentPhase && newPhase == Constants::MTG_PHASE_UNTAP) return 1;
  currentPhase = newPhase;
  return 0;

}


void ListMaintainerAbility::Update(float dt){
  map<MTGCardInstance *,bool>::iterator it=cards.begin();
  while(it != cards.end()){
    MTGCardInstance * card = (*it).first;
    it++;
    int doDelete = 1;
    for (int i = 0; i < 2; i++){
      Player * p = game->players[i];
      MTGGameZone * zones[] = {p->game->inPlay,p->game->graveyard,p->game->hand};
      for (int k = 0; k < 3; k++){
      //MTGGameZone * zones[] = {p->game->inPlay};
      //for (int k = 0; k < 1; k++){
	MTGGameZone * zone = zones[k];
	if (zone->hasCard(card)){
	  doDelete = 0;
	  break;
	}
      }
    }
    if (doDelete || !canBeInList(card)){
      cards.erase(card);
      removed(card);
    }
  }
  for (int i = 0; i < 2; i++){
    Player * p = game->players[i];
    MTGGameZone * zones[] = {p->game->inPlay,p->game->graveyard,p->game->hand};
    for (int k = 0; k < 3; k++){
    //  MTGGameZone * zones[] = {p->game->inPlay};
    //  for (int k = 0; k < 1; k++){
      MTGGameZone * zone = zones[k];
      for (int j = 0; j < zone->nb_cards; j++){
	if (canBeInList(zone->cards[j])){
	  if(cards.find(zone->cards[j]) == cards.end()){
	    cards[zone->cards[j]] = true;
	    added(zone->cards[j]);
	  }
	}
      }
    }
  }
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


/* An attempt to globalize triggered abilities as much as possible */

MTGAbilityBasicFeatures::MTGAbilityBasicFeatures(){
  game = GameObserver::GetInstance();
}
MTGAbilityBasicFeatures::MTGAbilityBasicFeatures(MTGCardInstance * _source, Damageable * _target):target(_target),source(_source){
  if (!target) target = source;
  game = GameObserver::GetInstance();
}
void MTGAbilityBasicFeatures::init(MTGCardInstance * _source, Damageable * _target){
  source = source;
  target=_target;
  if (!target) target = source;
}



TriggerAtPhase::TriggerAtPhase(int _phaseId):Trigger(),phaseId(_phaseId){
  currentPhase = game->getCurrentGamePhase();
  newPhase = game->getCurrentGamePhase();
}

int TriggerAtPhase::trigger(){
  int result = 0;
  newPhase = game->getCurrentGamePhase();
  if (currentPhase != newPhase && newPhase == phaseId){
    result = 1;
  }
  currentPhase = newPhase;
  return result;
}

TriggerNextPhase::TriggerNextPhase(int _phaseId):TriggerAtPhase(_phaseId){
  destroyActivated = 0;
}

int TriggerNextPhase::testDestroy(){
  if (newPhase <= phaseId) destroyActivated = 1;
  if ( newPhase > phaseId && destroyActivated){
    return 1;
  }
  return 0;
}

TriggeredEvent::TriggeredEvent():MTGAbilityBasicFeatures(){}

TriggeredEvent::TriggeredEvent(MTGCardInstance * _source, Damageable * _target):MTGAbilityBasicFeatures(_source, _target){}

DamageEvent::DamageEvent(MTGCardInstance * _source, Damageable * _target, int _damage):TriggeredEvent(_source,_target),damage(_damage){
}

int DamageEvent::resolve(){
  game->mLayers->stackLayer()->addDamage(source,target, damage);
  return damage;
}

DrawEvent::DrawEvent(Player * _player, int _nbcards):TriggeredEvent(),player(_player),nbcards(_nbcards){
}

int DrawEvent::resolve(){
  game->mLayers->stackLayer()->addDraw(player,nbcards);
  return nbcards;
}

int BuryEvent::resolve(){
  MTGCardInstance * _target = (MTGCardInstance *) target;
  _target->controller()->game->putInGraveyard(_target);
  return 1;
}

int DestroyCondition::testDestroy(){
  if (!game->isInPlay(source)){
    return 1;
  }
  if (target && !game->isInPlay((MTGCardInstance *)target)){
    source->controller()->game->putInGraveyard(source);//TODO put this in a better place ???
    return 1;
  }
  return 0;
}



GenericTriggeredAbility::GenericTriggeredAbility(int id, MTGCardInstance * _source, Trigger * _t, TriggeredEvent * _te, DestroyCondition * _dc , Damageable * _target ): TriggeredAbility(id, _source,_target){
  if (!target) target = source;
  t = _t;
  te = _te;
  dc = _dc;

  t->init(source,target);
  te->init(source,target);
  if (dc) dc->init(source,target);
}

int GenericTriggeredAbility::trigger(){
  return t->trigger();
}

int GenericTriggeredAbility::resolve(){
  return te->resolve();
}

int GenericTriggeredAbility::testDestroy(){
  if (dc) return dc->testDestroy();
  return t->testDestroy();
}

GenericTriggeredAbility::~GenericTriggeredAbility(){
  delete t;
  delete te;
  SAFE_DELETE(dc);
}


/*Mana Producers (lands)
//These have a reactToClick function, and therefore two manaProducers on the same card conflict with each other
//That means the player has to choose one. although that is perfect for cards such as birds of paradise or badlands,
other solutions need to be provided for abilities that add mana (ex: mana flare)
*/
/*
  Currently the mana is added to the pool AFTER the animation
  This is VERY BAD, since we don't have any control on the duration of the animation. This can lead to bugs with
  the AI, who is expecting to have the mana in its manapool right after clicking the land card !!!
  The sum of "dt" has to be 0.25 for the mana to be in the manapool currently
*/


   AManaProducer::AManaProducer(int id, MTGCardInstance * card, ManaCost * _output, ManaCost * _cost , int doTap):MTGAbility(id, card), tap(doTap){
   
     LOG("==Creating ManaProducer Object");
     aType=MTGAbility::MANA_PRODUCER;
    cost = _cost;
    output=_output;
    x1 = 10;
    y1 = 220;
    Player * player = card->controller();
    if (player == game->players[1]) y1 = 100;
    x = x1;
    y = y1;
    animation = 0.f;
    mParticleSys = NULL;
    menutext = "";

    int landColor = output->getMainColor();

    if (landColor == Constants::MTG_COLOR_RED){
      mParticleSys = NEW hgeParticleSystem("graphics/manared.psi",GameApp::CommonRes->GetQuad("particles"));
    }else if (landColor == Constants::MTG_COLOR_BLUE){
      mParticleSys = NEW hgeParticleSystem("graphics/manablue.psi", GameApp::CommonRes->GetQuad("particles"));
    }else if (landColor == Constants::MTG_COLOR_GREEN){
      mParticleSys = NEW hgeParticleSystem("graphics/managreen.psi", GameApp::CommonRes->GetQuad("particles"));
    }else if (landColor == Constants::MTG_COLOR_BLACK){
      mParticleSys = NEW hgeParticleSystem("graphics/manablack.psi", GameApp::CommonRes->GetQuad("particles"));
    }else if (landColor == Constants::MTG_COLOR_WHITE){
      mParticleSys = NEW hgeParticleSystem("graphics/manawhite.psi", GameApp::CommonRes->GetQuad("particles"));
    }else{
      mParticleSys = NEW hgeParticleSystem("graphics/mana.psi", GameApp::CommonRes->GetQuad("particles"));
    }



    LOG("==ManaProducer Object Creation successful !");
  }

  void AManaProducer::Update(float dt){
    if (mParticleSys) mParticleSys->Update(dt);
    if (animation){
      x = (1.f - animation)*x1 + animation * x0;
      y = (1.f - animation)*y1 + animation * y0;
      if (mParticleSys) mParticleSys->MoveTo(x, y);
      if (mParticleSys && animation == 1.f) mParticleSys->Fire();
      animation -= 4 *dt;
      if (!animation) animation = -1;
      if (animation < 0){
	      animation = 0;
        currentlyTapping--;
	      resolve();
	      if (mParticleSys) mParticleSys->Stop();
      }
    }

  }

  void AManaProducer::Render(){
    JRenderer * renderer = JRenderer::GetInstance();
    if (animation){
      renderer->SetTexBlend(BLEND_SRC_ALPHA, BLEND_ONE);
      if (mParticleSys) mParticleSys->Render();
      // set normal blending
      renderer->SetTexBlend(BLEND_SRC_ALPHA, BLEND_ONE_MINUS_SRC_ALPHA);
    }

  }

  int AManaProducer::isReactingToClick(MTGCardInstance *  _card, ManaCost * mana){
    int result = 0;
    if (!mana) mana = game->currentlyActing()->getManaPool();
    if (_card == source && (!tap || !source->isTapped())  && game->currentlyActing()->game->inPlay->hasCard(source) && (source->hasType("land") || !tap || !source->hasSummoningSickness()) ){
      if (!cost || mana->canAfford(cost)) result =  1;
    }
    return result;
  }

  int AManaProducer::resolve(){
    controller = source->controller();
    controller->getManaPool()->add(output);
    return 1;
  }

  int AManaProducer::reactToClick(MTGCardInstance *  _card){
    if (!isReactingToClick( _card)) return 0;
    OutputDebugString("React To click 1\n");
    if (cost){
      cost->setExtraCostsAction(this, _card);
      OutputDebugString("React To click 2\n");
      if (!cost->isExtraPaymentSet()){
        OutputDebugString("React To click 3\n");
      
        GameObserver::GetInstance()->waitForExtraPayment = cost->extraCosts;
        return 0;
      }
      GameObserver::GetInstance()->currentlyActing()->getManaPool()->pay(cost);
      cost->doPayExtra();
    } 
    if (tap) source->tapped = 1;
    currentlyTapping++;

    animation = 1.f;
    CardGui * cardg = game->mLayers->playLayer()->getByCard(source);
    if (cardg){
      x0 = cardg->x + 15;
      y0 = cardg->y + 20;
    }


    if (GameOptions::GetInstance()->values[OPTIONS_SFXVOLUME].getIntValue() > 0 && currentlyTapping < 3){
      JSample * sample = SampleCache::GetInstance()->getSample("sound/sfx/mana.wav");
      if (sample) JSoundSystem::GetInstance()->PlaySample(sample);
    }
    return 1;
  }

  const char * AManaProducer::getMenuText(){
    if (menutext.size())return menutext.c_str();
    menutext = "Add ";
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
	  menutext.append("red");
	  break;
	case Constants::MTG_COLOR_BLUE:
	  menutext.append("blue");
	  break;
	case Constants::MTG_COLOR_GREEN:
	  menutext.append("green");
	  break;
	case Constants::MTG_COLOR_WHITE:
	  menutext.append("white");
	  break;
	case Constants::MTG_COLOR_BLACK:
	  menutext.append("black");
	  break;
	default:
	  break;
	}
	alreadyHasOne = 1;
      }
    }
    menutext.append(" mana");
    return menutext.c_str();
  }

  int AManaProducer::testDestroy(){
    if (animation >0) return 0;
    return MTGAbility::testDestroy();
  }

  AManaProducer::~AManaProducer(){
    LOG("==Destroying ManaProducer Object");
    SAFE_DELETE(cost);
    SAFE_DELETE(output);
    SAFE_DELETE(mParticleSys);
    LOG("==Destroying ManaProducer Object Successful!");
  }

int AManaProducer::currentlyTapping = 0;