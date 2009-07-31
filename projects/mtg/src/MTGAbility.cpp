#include "../include/config.h"
#include "../include/MTGAbility.h"
#include "../include/ManaCost.h"
#include "../include/MTGGameZones.h"
#include "../include/AllAbilities.h"
#include "../include/Damage.h"
#include "../include/TargetChooser.h"
#include "../include/CardGui.h"
#include "../include/MTGDeck.h"
#include "../include/Blocker.h"


int AbilityFactory::countCards(TargetChooser * tc, Player * player, int option){
  int result = 0;
  GameObserver * game = GameObserver::GetInstance();
  for (int i = 0; i < 2 ; i++){
    if (player && player!= game->players[i]) continue;
        MTGGameZone * zones[] = {game->players[i]->game->inPlay,game->players[i]->game->graveyard,game->players[i]->game->hand};
                        for (int k = 0; k < 3; k++){
                                for (int j = zones[k]->nb_cards-1; j >=0 ; j--){
                                MTGCardInstance * current =  zones[k]->cards[j];
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
  }
  return result;
}



int AbilityFactory::CantBlock(TargetChooser * tc){
  GameObserver * g = GameObserver::GetInstance();
  MTGCardInstance * source = tc->source;
    for (int j = g->opponent()->game->inPlay->nb_cards-1; j >=0 ; j--){
      MTGCardInstance * current =  g->opponent()->game->inPlay->cards[j];	  
		  if (tc->canTarget(current)){
			  current->canBlock(source);
			  return 0;
		  }
	}
  return 1;
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

TriggeredAbility * AbilityFactory::parseTrigger(string magicText, int id, Spell * spell, MTGCardInstance *card, Targetable * target){
  size_t found = magicText.find("@");
  if (found == string::npos) return NULL;

  found = magicText.find(":");
  if (found == string::npos) return NULL;
  string s = magicText.substr(0,found);

  //Card Changed Zone
  found = s.find("movedto(");
  if (found != string::npos){
    size_t end = s.find (")");
    string starget = s.substr(found+8,end - found - 8);
    TargetChooserFactory tcf;
    TargetChooser *toTc = tcf.createTargetChooser(starget,card);
    toTc->targetter = NULL;

    TargetChooser *fromTc = NULL;
    found = s.find("from(");
    if (found != string::npos){
      end = s.find (")", found);
      starget = s.substr(found+5,end - found - 5);
      if (starget.find("|") == string::npos) starget.insert(0,"*|");
      fromTc = tcf.createTargetChooser(starget,card);
      fromTc->targetter = NULL;
    }
    return NEW TrCardAddedToZone(id,card,toTc,(TargetZoneChooser *)fromTc);
  }

  int who = 0;
  if (s.find("my") != string::npos) who = 1;
  if (s.find("opponent") != string::npos) who = -1;

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





//Parses a string and returns the corresponding MTGAbility object
// Returns NULL if parsing failed
MTGAbility * AbilityFactory::parseMagicLine(string s, int id, Spell * spell, MTGCardInstance *card, int activated){
  size_t found;
  
  //TODO This block redundant with calling function
  if (!card && spell) card = spell->source;
  if (!card) return NULL;
  MTGCardInstance * target = card->target;
  if (!target) target = card; 


  TriggeredAbility * trigger = NULL;
  trigger = parseTrigger(s,id,spell,card,target);
  //Dirty way to remove the trigger text (could get in the way)
  if (trigger){
    found = s.find(":");
    string s1 = s.substr(found+1);
    MTGAbility * a = parseMagicLine(s1, id, spell, card,activated);
    if (!a){
      delete trigger;
      return NULL;
    }
    return NEW GenericTriggeredAbility(id,card,trigger,a,NULL,target);
  }

  int doTap = 0; //Tap in the cost ?
  if (s.find("{t}") != string::npos) doTap = 1;

  size_t delimiter = s.find("}:");
  size_t firstNonSpace = s.find_first_not_of(" ");
  if (delimiter!= string::npos && firstNonSpace !=string::npos && s[firstNonSpace] == '{'){
    ManaCost * cost  = ManaCost::parseManaCost(s.substr(0,delimiter+1),NULL,card);
    if (doTap || (cost && !cost->isNull())){
      string s1 = s.substr(delimiter+2);
      
      MTGAbility * a = parseMagicLine(s1, id, spell, card, 1);
      if (!a){
        OutputDebugString("Error parsing:");
        OutputDebugString(s.c_str());
        OutputDebugString("\n");
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
      unsigned int limit_str = s.find("limit:");
      if (limit_str != string::npos){
        limit = atoi(s.substr(limit_str+6).c_str());
      }

      TargetChooser * tc = NULL;
      //Target Abilities
      found = s.find("target(");
      if (found != string::npos){
        int end = s.find(")", found);
        string starget = s.substr(found + 7,end - found - 7);
        TargetChooserFactory tcf;
        tc = tcf.createTargetChooser(starget, card);
      }

      if (tc) return NEW GenericTargetAbility(id, card, tc, a,cost, doTap,limit);
      return NEW GenericActivatedAbility(id, card, a,cost,doTap,limit);
    }
    SAFE_DELETE(cost);
  }



  //Multiple abilities for ONE cost
  found = s.find("&&");
  if (found != string::npos){
    string s1 = s.substr(0,found);
    string s2 = s.substr(found+2);
    MultiAbility * multi = NEW MultiAbility(id, card,target,NULL,NULL);
    MTGAbility * a1 = parseMagicLine(s1,id,spell, card);
    MTGAbility * a2 = parseMagicLine(s2,id,spell, card);
    multi->Add(a1);
    multi->Add(a2);
    multi->oneShot=1;
    return multi;
  }

  //Lord, foreach, aslongas
  string lords[] = {"lord(","foreach(", "aslongas(", "all("};
  found = string::npos;
  int i = -1;
  for (int j = 0; j < 4; ++j){
    size_t found2 = s.find(lords[j]);
    if (found2!=string::npos && ((found == string::npos) || found2 < found)){
      found = found2;
      i = j;
    }
  }
  if (found != string::npos){
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
      size_t other = s.find("other", end);
      if ( other != string::npos){
        lordIncludeSelf = 0;
        s.replace(other, 5,"");
      }
      string lordTargetsString = s.substr(found+header,end-found-header);
      TargetChooserFactory tcf;
      TargetChooser * lordTargets = tcf.createTargetChooser(lordTargetsString, card);
      
      
      MTGAbility * a = parseMagicLine(s1,id,spell, card);
      if (!a){
        SAFE_DELETE(lordTargets);
        return NULL;
      }
      MTGAbility * result = NULL;
      int oneShot = 0;
      if (card->hasType("sorcery") || card->hasType("instant")) oneShot = 1;
      if (i == 3) oneShot = 1;
      if (a->oneShot) oneShot = 1;
      Damageable * _target = NULL;
	  if (spell) _target = spell->getNextDamageableTarget();
      if (!_target) _target = target;
      switch(i){
        case 0: result =  NEW ALord(id, card, lordTargets, lordIncludeSelf, a); break;
        case 1: result =  NEW AForeach(id, card, _target,lordTargets, lordIncludeSelf, a); break;
        case 2: result =  NEW AAsLongAs(id, card, _target,lordTargets, lordIncludeSelf, a); break;
        case 3: result =  NEW ALord(id, card, lordTargets,  lordIncludeSelf, a); break;
        default: result =  NULL;
      }
      if (result) result->oneShot = oneShot;
      return result;
    }
    return NULL;
  }

  //When...comes into play, you may...
  found = s.find("may ");
  if (found != string::npos){
    string s1 = s.substr(found+4);
    MTGAbility * a1 = parseMagicLine(s1,id,spell, card);
    if (!a1) return NULL;
    TargetChooser * tc = NULL;
    //Target Abilities
    found = s.find("target(");
    if (found != string::npos){
      int end = s.find(")", found);
      string starget = s.substr(found + 7,end - found - 7);
      TargetChooserFactory tcf;
      tc = tcf.createTargetChooser(starget, card);
    }
    if (tc) a1 = NEW GenericTargetAbility(id, card, tc, a1);
    return NEW MayAbility(id,a1,card);
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
           

  //Untapper (Ley Druid...)
  found = s.find("untap");
  if (found != string::npos){
    MTGAbility * a = NEW AAUntapper(id,card,target);
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
    ATokenCreator * tok = NEW ATokenCreator(id,card,NULL,sname,stypes,power,toughness,sabilities,0, multiplier);
    tok->oneShot = 1;
    return tok;
  }


  //MoveTo Move a card from a zone to another
  found = s.find("moveto(");
  if (found != string::npos){
    int end = s.find(")",found+1);
    string szone = s.substr(found + 7,end - found - 7);
    MTGAbility * a = NEW AAMover(id,card,target,szone);
    a->oneShot = 1;
    return a;

  }

  //Copy a target
  found = s.find("copy ");
  if (found != string::npos){
    MTGAbility * a = NEW AACopier(id,card,target);
    a->oneShot = 1;
    return a;
  }


  //Bury, destroy
  string destroys[] = {"bury","destroy"};
  int destroyTypes[]= {1, 0};
  for (int i = 0; i < 2; ++i){
    found = s.find(destroys[i]);
    if (found != string::npos){
      int bury = destroyTypes[i];
      MTGAbility * a = NEW AADestroyer(id,card,target,bury);
      a->oneShot = 1;
      return a;
    }
  }


  int who = TargetChooser::UNSET;
  if (s.find(" controller") != string::npos) who=TargetChooser::CONTROLLER;
  if (s.find(" opponent") != string::npos) who=TargetChooser::OPPONENT;
  if (s.find(" targetcontroller") != string::npos) who=TargetChooser::TARGET_CONTROLLER;

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

    Damageable * d = NULL;
    if (spell) d = spell->getNextDamageableTarget();
    MTGAbility * a =  NEW AADamager(id,card,d, damage, NULL, 0, who);
    a->oneShot = 1;
    return a;
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

    Damageable * d = NULL;
    if (spell) d = spell->getNextPlayerTarget();
    MTGAbility * a =  NEW AALifer(id,card,d,life,NULL,0,who);
    a->oneShot = 1;
    return a;
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

    Targetable * t = NULL;
    if (spell) t = spell->getNextPlayerTarget();
    MTGAbility * a = NEW AADrawer(id,card,t,NULL,nbcards,0,who);
    a->oneShot = 1;
    return a;
  }

  //Deplete
  found = s.find("deplete:");
  if (found != string::npos){
    unsigned int start = s.find(":",found);
    unsigned int end = s.find(" ",start);
    int nbcards;
    if (end != string::npos){
      nbcards = atoi(s.substr(start+1,end-start-1).c_str());
    }else{
      nbcards = atoi(s.substr(start+1).c_str());
    }

    Targetable * t = NULL;
    if (spell) t = spell->getNextPlayerTarget();
    MTGAbility * a = NEW AADepleter(id,card,t,nbcards,NULL,0,who);
    a->oneShot = 1;
    return a;
  }


  /*
  //CannotBeBlockedBy
  found = s.find("cantbeblockedby(");
  if (found != string::npos){
    int end = s.find(")",found);
    string starget = s.substr(16, end - 16);
    TargetChooserFactory tcf;
    tc = tcf.createTargetChooser(starget,card);
    return NULL; //NEW ACantBlock(tc); //hu ? CantBlock(tc);
  }
	   
*/
  //Discard
  found = s.find("discard:");
  if (found != string::npos){
    unsigned int start = s.find(":",found);
    unsigned int end = s.find(" ",start);
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
	    return NEW  ARampageAbility(id,card,power,toughness,MaxOpponent);
    }
    return NULL;
  }


  //counter
  found = s.find("counter(");
  if (found != string::npos){
    found+=8;
    int nb = 1;
    size_t end = s.find(")", found);
    size_t separator = s.find(",", found);
    if (separator != string::npos){
      string nbstr = s.substr(separator+1,end-separator-1);
      nb = atoi(nbstr.c_str()); 
      end = separator;
    }
    string spt = s.substr(found,end-found);
    int power, toughness;
    if ( parsePowerToughness(spt,&power, &toughness)){
      MTGAbility * a = NEW AACounter(id,card,target,power,toughness,nb);
	    a->oneShot = 1;
      return a;
    }
    return NULL;
  }


 
  //Change Power/Toughness
  int power, toughness;
  if ( parsePowerToughness(s,&power, &toughness)){
    if (!activated){
      if(card->hasType("instant") || card->hasType("sorcery")){
        return NEW AInstantPowerToughnessModifierUntilEOT(id, card, target,power,toughness);
      }
      return NEW APowerToughnessModifier(id, card, target,power,toughness);
    }
    return NEW APowerToughnessModifierUntilEndOfTurn(id,card,target,power,toughness);
  }



  //Mana Producer
  found = s.find("add");
  if (found != string::npos){
    ManaCost * output = ManaCost::parseManaCost(s.substr(found));
    MTGAbility * a =  NEW AManaProducer(id, target, output);
    a->oneShot = 1;
    return a;
  }


  //Gain/loose Ability
  for (int j = 0; j < Constants::NB_BASIC_ABILITIES; j++){
    found = s.find(Constants::MTGBasicAbilities[j]);
    if (found!= string::npos){
      int modifier = 1;
      if (found > 0 && s[found-1] == '-') modifier = 0;
      if (!activated){
        if(card->hasType("instant") || card->hasType("sorcery") ) return NEW AInstantBasicAbilityModifierUntilEOT(id, card,target, j,modifier);   
        return NEW ABasicAbilityModifier(id, card,target, j,modifier);
      }
      return NEW ABasicAbilityAuraModifierUntilEOT(id, card,target, NULL,j,modifier);
    }
  }

  //Tapper (icy manipulator)
  found = s.find("tap");
  if (found != string::npos){
    MTGAbility * a = NEW AATapper(id,card,target);
    a->oneShot = 1;
    return a;
  }

  return NULL;

}

//Tells the AI if the ability should target itself or an ennemy
int AbilityFactory::abilityEfficiency(MTGAbility * a, Player * p, int mode){
  if (!a) return BAKA_EFFECT_DONTKNOW;

  GenericTargetAbility * gta = dynamic_cast<GenericTargetAbility*>(a);
  if (gta) {
    if (mode == MODE_PUTINTOPLAY) return BAKA_EFFECT_GOOD;
    return abilityEfficiency(gta->ability,p, mode);
  }
  
  GenericActivatedAbility * gaa = dynamic_cast<GenericActivatedAbility*>(a);
  if (gaa) {
    if (mode == MODE_PUTINTOPLAY) return BAKA_EFFECT_GOOD;
    return abilityEfficiency(gaa->ability,p, mode);
  }
  
  MultiAbility * mua = dynamic_cast<MultiAbility*>(a);
  if (mua) return abilityEfficiency(mua->abilities[0],p, mode);

  MayAbility * maya = dynamic_cast<MayAbility*>(a);
  if (maya) return abilityEfficiency(maya->ability,p, mode);

  ALord * alord = dynamic_cast<ALord *>(a);
  if (alord) {
    int myCards = countCards(alord->tc, p);
    int theirCards = countCards(alord->tc, p->opponent());
    int efficiency = abilityEfficiency(alord->ability,p, mode);
    if (myCards > theirCards) return efficiency;
    return -efficiency;
  }

  AAsLongAs * ala = dynamic_cast<AAsLongAs *>(a);
  if (ala) {
    return abilityEfficiency(ala->ability,p, mode);
  }

  AForeach * af = dynamic_cast<AForeach *>(a);
  if (af) {
    return abilityEfficiency(af->ability,p, mode);
  }

  AAFizzler  * aaf = dynamic_cast<AAFizzler *>(a);
  if (aaf){
    return BAKA_EFFECT_BAD;
  }

  AAUntapper * aau = dynamic_cast<AAUntapper *>(a);
  if (aau){
    return BAKA_EFFECT_GOOD;
  }


  AATapper * aat = dynamic_cast<AATapper *>(a);
  if (aat){
    return BAKA_EFFECT_BAD;
  }


  ATokenCreator * aatc = dynamic_cast<ATokenCreator *>(a);
  if (aatc){
    return BAKA_EFFECT_GOOD;
  }


  AAMover * aam = dynamic_cast<AAMover *>(a);
  if (aam){
    //TODO
    return BAKA_EFFECT_BAD;
  }


  AACopier * aac = dynamic_cast<AACopier  *>(a);
  if (aac){
    return BAKA_EFFECT_GOOD;
  }


  AADestroyer * aad = dynamic_cast<AADestroyer *>(a);
  if (aad){
    return BAKA_EFFECT_BAD;
  }

  AStandardRegenerate * asr = dynamic_cast<AStandardRegenerate *>(a);
  if (asr){
    return BAKA_EFFECT_GOOD;
  }

  AADamager * aada = dynamic_cast<AADamager *>(a);
  if (aada){
    return BAKA_EFFECT_BAD;
  }


  AALifer * aal = dynamic_cast<AALifer *>(a);
  if (aal){
    if (aal->life > 0) return BAKA_EFFECT_GOOD;
    return BAKA_EFFECT_BAD;
  }


  AADepleter * aade = dynamic_cast<AADepleter *>(a);
  if (aade){
    return BAKA_EFFECT_BAD;
  }


  AADrawer * aadr = dynamic_cast<AADrawer *>(a);
  if (aadr){
    return BAKA_EFFECT_GOOD;
  }

  AARandomDiscarder * aard = dynamic_cast<AARandomDiscarder *>(a);
  if (aard){
    return BAKA_EFFECT_BAD;
  }

  ARampageAbility * ara = dynamic_cast<ARampageAbility *>(a);
  if (ara){
    return BAKA_EFFECT_GOOD;
  }

  AInstantPowerToughnessModifierUntilEOT * aiptm = dynamic_cast<AInstantPowerToughnessModifierUntilEOT *>(a);
  if (aiptm){
    if (aiptm->power>=0 && aiptm->toughness>=0) return BAKA_EFFECT_GOOD;
    return BAKA_EFFECT_BAD;
  }

  APowerToughnessModifier * aptm = dynamic_cast<APowerToughnessModifier *>(a);
  if (aptm){
    if (aptm->power>=0 && aptm->toughness>=0) return BAKA_EFFECT_GOOD;
    return BAKA_EFFECT_BAD;
  }

  APowerToughnessModifierUntilEndOfTurn * aptmu = dynamic_cast<APowerToughnessModifierUntilEndOfTurn *>(a);
  if (aptmu){
    return abilityEfficiency(aptmu->ability, p, mode);
  }

  map<int,bool> badAbilities;
  badAbilities[Constants::CANTATTACK] = true;
  badAbilities[Constants::CANTBLOCK] = true;
  badAbilities[Constants::CLOUD] = true;
  badAbilities[Constants::DEFENDER] = true;
  badAbilities[Constants::DOESNOTUNTAP] = true;
  badAbilities[Constants::MUSTATTACK] = true;

  AInstantBasicAbilityModifierUntilEOT * aibam = dynamic_cast<AInstantBasicAbilityModifierUntilEOT *>(a);
  if (aibam){
    int result = BAKA_EFFECT_GOOD;
    if (badAbilities[aibam->ability]) result = BAKA_EFFECT_BAD;
    if (aibam->value <= 0) result = -result;
    return result;
  }

  ABasicAbilityModifier * abam = dynamic_cast<ABasicAbilityModifier *>(a);
  if (abam){
    int result = BAKA_EFFECT_GOOD;
    if (badAbilities[abam->ability]) result = BAKA_EFFECT_BAD;
    if (abam->modifier <= 0) result = -result;
    return result;
  }

  ABasicAbilityAuraModifierUntilEOT * abamu = dynamic_cast<ABasicAbilityAuraModifierUntilEOT *>(a);
  if (abamu){
    return abilityEfficiency(abamu->ability, p, mode);
  }

  AManaProducer * amp = dynamic_cast<AManaProducer*>(a);
  if (amp) return BAKA_EFFECT_GOOD;

  return BAKA_EFFECT_DONTKNOW;
}

//Returns the "X" cost that was paid for a spell
int AbilityFactory::computeX(Spell * spell, MTGCardInstance * card){
  ManaCost * c = spell->cost->Diff(card->getManaCost());
  int x = c->getCost(Constants::MTG_NB_COLORS);
  delete c;
  return x;
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

  if (!card && spell) card = spell->source;
  if (!card) return 0;
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

    MTGAbility * a = parseMagicLine(line, result, spell, card); 
    if (dryMode){
      result = abilityEfficiency(a, card->controller(),MODE_PUTINTOPLAY);
      SAFE_DELETE(a);
      return result;
    }

    if (a){
      if (a->oneShot){
        a->resolve();
        delete(a); 
      }else{
        a->addToGame();
      }
      result++;
    }else{
      OutputDebugString("ERROR: Parser returned NULL\n");
      //return result;
    }
  }
  return result;
 
}

void AbilityFactory::addAbilities(int _id, Spell * spell){
  MTGCardInstance * card = spell->source;


  if (spell->cursor==1) card->target =  spell->getNextCardTarget();
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
  case 1190: //Animate Artifact
    {
      int x =  card->target->getManaCost()->getConvertedCost();
      game->addObserver(NEW AConvertToCreatureAura(_id, card,card->target,x,x));
      break;
    }
  case 1095: //Armageddon clock
    {
      AArmageddonClock * ability = NEW AArmageddonClock(_id,card);
      game->addObserver(ability);
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
      int x = computeX(spell,card); 
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
  case 1164: //Howl from beyond
    {
      int x = computeX(spell,card);
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
	  if (current->hasType("land")) current->tap();
	}
	player->getManaPool()->init();
      }
      break;
    }
  case 1167: //Mind Twist
    {
      int xCost = computeX(spell,card);
      for (int i = 0; i < xCost; i++){
	game->opponent()->game->discardRandom(game->opponent()->game->hand);
      }
      break;
    }
  case 1171: //Paralyze
    {
      int cost[] = {Constants::MTG_COLOR_ARTIFACT, 4};
      game->addObserver(NEW AUntapManaBlocker(_id, card,card->target, NEW ManaCost(cost,1)));
      card->target->tap();
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
  case 1218: //Psychic Venom
    {
      game->addObserver(NEW APsychicVenom(_id, card, card->target));
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
      int x = computeX(spell,card);
      spell->getNextPlayerTarget()->life += x;
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
	        if (current->isACreature()){
	          game->mLayers->stackLayer()->addDamage(card, current, x);
	        }
	      }
      }
      break;
    }

  case 1285: //Dwarven Warriors
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
      int x = computeX(spell,card);
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

    //Addons Legends
  case 1427: //Abomination (does not work make the game crash)
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
  case 1480: //Energy Tap
	{
	card->target->tap();
	int mana = card->target->getManaCost()->getConvertedCost();
	game->currentlyActing()->getManaPool()->add(Constants::MTG_COLOR_ARTIFACT, mana);
	}

  case 1614: // Great Defender
	{
	int toughness = card->target->getManaCost()->getConvertedCost();
	int power = 0;
	game->addObserver(NEW AInstantPowerToughnessModifierUntilEOT(id, card, card->target,power,toughness));
	}
  
  case 1703: //Pendelhaven
    {
      CreatureTargetChooser * tc = NEW CreatureTargetChooser(card);
      tc->maxpower = 1;
      tc->maxtoughness =1;
      game->addObserver(NEW ATargetterPowerToughnessModifierUntilEOT(id, card, 1,2, NEW ManaCost(),tc));
      break;
    }

    //Addons ICE-AGE Cards

  case 2660: //Word of Blasting
    {
      card->target->controller()->game->putInGraveyard(card->target);
      card->target->controller()->life-= card->target->getManaCost()->getConvertedCost();
      break;
    }
  case 2593: //Thoughtleech
    {
      game->addObserver(NEW AGiveLifeForTappedType (_id, card, "island"));
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

//---addon Alliance---
	
    case 3194: // Exile
    {
	Player * p = card->target->controller();
    p->game->putInZone(card->target,p->game->inPlay,card->owner->game->removedFromGame);
	game->currentlyActing()->life+=  card->target->toughness;
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


//-- addon Urza Saga---
  case 8818: //Goblin Offensive
    {
      int x = computeX(spell,card);
      ATokenCreator * tok = NEW ATokenCreator(id,card,NEW ManaCost(),"Goblin","creature Goblin",1,1,"Red",0);
      for (int i=0; i < x; i++){
        tok->resolve();
      } 
      delete(tok);
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
	case 130542: //Flowstone Slide
		{
			int x = computeX(spell,card);
      MTGAbility * a = NEW AInstantPowerToughnessModifierUntilEOT(id, card, card,x,-x);
			TargetChooserFactory tcf;
      TargetChooser * lordTargets = tcf.createTargetChooser("creature", card);
      game->addObserver(NEW ALord(id, card, lordTargets, 0, a));
			break;
		}

  case 130373: //Lavaborn Muse
    {
      game->addObserver( NEW ALavaborn(_id ,card, Constants::MTG_PHASE_UPKEEP, -3,-3));
      break;
    }
  case 135246: //Dreamborn Muse
    {
      game->addObserver( NEW ADreambornMuse(_id ,card));
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
		  player->life+=player->life;
		  zones->putInZone(card,zones->stack,zones->library);
		  zones->library->shuffle();
		  break;
	  }
   case 135262:// Beacon of Destruction & unrest
	  {
		  zones->putInZone(card,zones->stack,zones->library);
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

  case 129521: //Dehydratation
  // Don't understand why but target automatically untap when cast...
    {
			game->addObserver(NEW UntapBlocker(_id,card,card->target));
      break;
    }
	
	case 130369: // Soulblast
    {
	    int damage = 0;
	    Damageable * target = spell->getNextDamageableTarget();
	    for (int j = card->controller()->game->inPlay->nb_cards-1; j >=0 ; --j){
			  MTGCardInstance * current =  card->controller()->game->inPlay->cards[j];
			  if (current->hasType("Creature")){
				  card->controller()->game->putInGraveyard(current);
				  damage+= current->power;
			  }
	    }
	    game->mLayers->stackLayer()->addDamage(card, target, damage);
      break;
	}

// --- addon Invasion---
    case 23195: //Artifact Mutation
    {
      card->target->controller()->game->putInGraveyard(card->target);
      int x = card->target->getManaCost()->getConvertedCost();
      ATokenCreator * tok = NEW ATokenCreator(id,card,NEW ManaCost(),"Saproling","creature Saproling",1,1,"green",0);
      for (int i=0; i < x; i++){
        tok->resolve();
      }
      delete(tok);
      break;
    }
//--- addon Eventide ----

	case 151114: //Rise of the Hobgoblins
    {
      int x = computeX(spell,card);
      ATokenCreator * tok = NEW ATokenCreator(id,card,NEW ManaCost(),"Goblin Soldier","creature Goblin Soldier",1,1,"red white",0);
      for (int i=0; i < x; i++){
        tok->resolve();
      } 
      delete(tok);
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

	// --- addon ARB---
    case 179614: // Morbid Bloom
    {
      card->target->controller()->game->putInZone(card->target, card->target->controller()->game->inPlay,card->owner->game->removedFromGame);
      int x = card->target->toughness;
      ATokenCreator * tok = NEW ATokenCreator(id,card,NEW ManaCost(),"Saproling","creature Saproling",1,1,"green",0);
      for (int i=0; i < x; i++){
        tok->resolve();
      }
      delete(tok);
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

  if (card->basicAbilities[Constants::DOESNOTUNTAP]){
    game->addObserver(NEW UntapBlocker(_id, card));
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
    zones->putInZone(card,zones->stack,zones->graveyard);
  }


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
  if (needsTapping) source->tap();
  fireAbility();

  return 1;

}

int ActivatedAbility::reactToTargetClick(Targetable * object){
  if (!isReactingToTargetClick(object)) return 0;
  if (needsTapping) source->tap();
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

ostream& ActivatedAbility::toString(ostream& out) const
{
  out << "ActivatedAbility ::: playerturnonly : " << playerturnonly
      << " ; needsTapping : " << needsTapping
      << " (";
  return MTGAbility::toString(out) << ")";
}


//The whole targetAbility mechanism is messed up, mainly because of its interactions with
// the ActionLayer, GameObserver, and parent class ActivatedAbility.
// Currently choosing a target is a complete different mechanism for put into play and for other abilities.
// It probably shouldn't be the case.

TargetAbility::TargetAbility(int id, MTGCardInstance * card, TargetChooser * _tc,ManaCost * _cost, int _playerturnonly,int tap):ActivatedAbility(id, card,_cost,_playerturnonly, tap){
  tc = _tc;
  ability = NULL;
}

TargetAbility::TargetAbility(int id, MTGCardInstance * card,ManaCost * _cost, int _playerturnonly,int tap):ActivatedAbility(id, card,_cost,_playerturnonly, tap){
  tc = NULL;
  ability = NULL;
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


int TargetAbility::resolve(){
  Targetable * t = tc->getNextTarget();
  if (t && ability){
    ability->target = t;
    return ability->resolve();
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
  if (newPhase != currentPhase && newPhase == Constants::MTG_PHASE_UNTAP) return 1;
  currentPhase = newPhase;
  return 0;

}

ostream& InstantAbility::toString(ostream& out) const
{
  out << "InstantAbility ::: init : " << init
      << " (";
  return MTGAbility::toString(out) << ")";
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

  //add new valid ones
  for (int i = 0; i < 2; i++){
    Player * p = game->players[i];
    MTGGameZone * zones[] = {p->game->inPlay,p->game->graveyard,p->game->hand};
    for (int k = 0; k < 3; k++){
      MTGGameZone * zone = zones[k];
      for (int j = 0; j < zone->nb_cards; j++){
	      if (canBeInList(zone->cards[j])){
	        if(cards.find(zone->cards[j]) == cards.end()){
	          temp[zone->cards[j]] = true;
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
  newPhase = g->getCurrentGamePhase();
  currentPhase = newPhase;
}

int TriggerAtPhase::trigger(){
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
  if (newPhase <= phaseId) destroyActivated = 1;
  if ( newPhase > phaseId && destroyActivated){
    return 1;
  }
  return 0;
}

TriggerNextPhase* TriggerNextPhase::clone() const{
    TriggerNextPhase * a =  NEW TriggerNextPhase(*this);
    a->isClone = 1;
    return a;
}

GenericTriggeredAbility::GenericTriggeredAbility(int id, MTGCardInstance * _source,  TriggeredAbility * _t, MTGAbility * a , MTGAbility * dc, Targetable * _target ): TriggeredAbility(id, _source,_target){
  if (!target) target = source;
  t = _t;
  ability = a;
  destroyCondition = dc;

  t->source = source;
  t->target = target;
  ability->source = source;
  ability->target = target;
  if (destroyCondition){
    destroyCondition->source = source;
    destroyCondition->target = target;;
  }
}

int GenericTriggeredAbility::trigger(){
  return t->trigger();
}


int GenericTriggeredAbility::triggerOnEvent(WEvent * e){
  return t->triggerOnEvent(e);
}

void GenericTriggeredAbility::Update(float dt){
  GameObserver * g = GameObserver::GetInstance();
  int newPhase = g->getCurrentGamePhase();
  t->newPhase = newPhase;
  TriggeredAbility::Update(dt);
  t->currentPhase = newPhase;
}

int GenericTriggeredAbility::resolve(){
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
	      resolve();
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
    animation = 0;
    if (currentlyTapping > 0) currentlyTapping--;
    controller = source->controller();
    controller->getManaPool()->add(output);
	  if (mParticleSys) mParticleSys->Stop();
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
    if (tap) source->tap();
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

  int AManaProducer::destroy(){
    if (animation >0) resolve(); //if we get destroyed while the animation was taking place (dirty...)
    return MTGAbility::destroy();
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
    if (isClone) return;
    LOG("==Destroying ManaProducer Object");
    SAFE_DELETE(cost);
    SAFE_DELETE(output);
    SAFE_DELETE(mParticleSys);
    LOG("==Destroying ManaProducer Object Successful!");
  }

  AManaProducer * AManaProducer::clone() const{
    AManaProducer * a =  NEW AManaProducer(*this);
    a->isClone = 1;
    return a;
  }

int AManaProducer::currentlyTapping = 0;

ostream& AManaProducer::toString(ostream& out) const
{
  out << "AManaProducer ::: cost : " << cost
      << " ; output : " << output
      << " ; menutext : " << menutext
      << " ; x0,y0 : " << x0 << "," << y0
      << " ; x1,y1 : " << x1 << "," << y1
      << " ; x,y : " << x << "," << y
      << " ; animation : " << animation
      << " ; controller : " << controller
      << " ; tap : " << tap
      << " (";
  return MTGAbility::toString(out) << ")";
}
