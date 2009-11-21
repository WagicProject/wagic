#include "../include/config.h"
#include "../include/TargetChooser.h"
#include "../include/CardDescriptor.h"
#include "../include/MTGGameZones.h"
#include "../include/GameObserver.h"
#include "../include/Subtypes.h"



TargetChooser * TargetChooserFactory::createTargetChooser(string s, MTGCardInstance * card, MTGAbility * ability){
  if (!s.size()) return NULL;

  int zones[10];
  int nbzones = 0;
  size_t found;
  bool other = false;

  found = s.find("mytgt");
  if (found == 0){
    MTGCardInstance * target = card->target;
    if (ability) target = (MTGCardInstance *) (ability->target);
    return NEW CardTargetChooser(target,card);
  };

  found = s.find("other ");
  if (found == 0){
    other = true;
    s=s.substr(6);
  }

  found = s.find("player");
  if (found != string::npos){
    int maxtargets = 1;
    unsigned int several = s.find_first_of('s',5);
    if (several != string::npos) maxtargets = -1;
    found = s.find("creature");
    if (found != string::npos) return NEW DamageableTargetChooser(card,maxtargets,other); //Any Damageable target (player, creature)
    return NEW PlayerTargetChooser(card,maxtargets); //Any player
  }

  string s1;
  found = s.find("|");
  if (found != string::npos){
    string s2;
    s1 = s.substr(0,found);
    s2 = s.substr(found+1);
    while(s2.size()){
      found = s2.find(",");
      string zoneName;
      if (found != string::npos){
        zoneName = s2.substr(0,found);
        s2 = s2.substr(found+1);
      }else{
        zoneName = s2;
        s2 = "";
      }
      zones[nbzones] = MTGGameZone::MY_BATTLEFIELD;

      //Graveyards
      if(zoneName.compare("graveyard") == 0){
        zones[nbzones] = MTGGameZone::MY_GRAVEYARD;
        nbzones++;
        zones[nbzones] = MTGGameZone::OPPONENT_GRAVEYARD;
      }else if(zoneName.compare("battlefield") == 0 || zoneName.compare("inplay") == 0){
          zones[nbzones] = MTGGameZone::MY_BATTLEFIELD;
          nbzones++;
          zones[nbzones] = MTGGameZone::OPPONENT_BATTLEFIELD;
      }else if(zoneName.compare("stack") == 0){
        zones[nbzones] = MTGGameZone::MY_STACK;
        nbzones++;
        zones[nbzones] = MTGGameZone::OPPONENT_STACK;
      }else{
          int zone = MTGGameZone::zoneStringToId(zoneName);
          if (zone) zones[nbzones] = zone;
      }
      nbzones++;
    }
  }else{
    s1 = s;
    nbzones = 2;
    zones[0]= MTGGameZone::MY_BATTLEFIELD;
    zones[1]= MTGGameZone::OPPONENT_BATTLEFIELD;
  }

  TargetChooser * tc = NULL;
  int maxtargets = 1;
  CardDescriptor * cd = NULL;

  while(s1.size()){
    found = s1.find(",");
    string typeName;
    if (found != string::npos){
      typeName = s1.substr(0,found);
      s1 = s1.substr(found+1);
    }else{
      typeName = s1;
      s1 = "";
    }

    //Advanced cards caracteristics ?
    found = typeName.find("[");
    if (found != string::npos){
      int nbminuses = 0;
      int end = typeName.find("]");
      string attributes = typeName.substr(found+1,end-found-1);
      cd = NEW CardDescriptor();
      while(attributes.size()){
        unsigned int found2 = attributes.find(";");
        string attribute;
        if (found2 != string::npos){
          cd->mode = CD_OR;
          attribute = attributes.substr(0,found2);
          attributes = attributes.substr(found2+1);
        }else{
          attribute = attributes;
          attributes = "";
        }
        int minus = 0;
        if (attribute[0] == '-'){
          minus = 1;
          nbminuses++;
          attribute=attribute.substr(1);
        }
        //Attacker
        if (attribute.find("attacking") != string::npos){
          if (minus){
	          cd->attacker = -1;
          }else{
	          cd->attacker = 1;
          }
          //Blocker
        }else if (attribute.find("blocking") != string::npos){
          if (minus){
            cd->defenser = & MTGCardInstance::NoCard;
          }else{
	          cd->defenser = & MTGCardInstance::AnyCard;
          }
        //Tapped, untapped
        }else if (attribute.find("tapped") != string::npos){
          if (minus){
	          cd->unsecureSetTapped(-1);
          }else{
	          cd->unsecureSetTapped(1);
          }
        }else{
          int attributefound = 0;
          //Colors
          for (int cid = 0; cid < Constants::MTG_NB_COLORS; cid++){
	          if (attribute.find(Constants::MTGColorStrings[cid]) != string::npos){
	            attributefound = 1;
	            if (minus){
	              cd->colors[cid] = -1;
	            }else{
	              cd->colors[cid] = 1;
	            }
	          }
          }
          if (!attributefound){
	          //Abilities
	          for (int j = 0; j < Constants::NB_BASIC_ABILITIES; j++){
	            if (attribute.find(Constants::MTGBasicAbilities[j]) != string::npos){
	              attributefound = 1;
	              if (minus){
	                cd->basicAbilities[j] = -1;
	              }else{
	                cd->basicAbilities[j] = 1;
	              }
	            }
	          }
          }

          if (!attributefound){
	          //Subtypes
	          if (minus){
	            cd->setNegativeSubtype(attribute);
	          }else{
	            cd->setSubtype(attribute);
	          }
          }
        }
      }
      if (nbminuses) cd->mode = CD_AND;
      typeName = typeName.substr(0,found);
    }
    //X targets allowed ?
    if (typeName.at(typeName.length()-1) == 's' && !Subtypes::subtypesList->find(typeName,false) && typeName.compare("this")!=0){
      typeName = typeName.substr(0,typeName.length()-1);
      maxtargets = -1;
    }
    if (cd){
      if (!tc){
        if (typeName.compare("*")!=0) cd->setSubtype(typeName);

        tc = NEW DescriptorTargetChooser(cd,zones,nbzones,card,maxtargets,other);
      }else{
        delete(cd);
        return NULL;
      }
    }else{
      if (!tc){
        if (typeName.compare("*")==0){
          return NEW TargetZoneChooser(zones, nbzones,card, maxtargets,other);
        }else if (typeName.compare("this")==0){
          return NEW CardTargetChooser(card,card,zones, nbzones);
        }else{
          tc =  NEW TypeTargetChooser(typeName.c_str(), zones, nbzones, card,maxtargets,other);
        }
      }else{
        ((TypeTargetChooser *)tc)->addType(typeName.c_str());
        tc->maxtargets = maxtargets;
      }
    }
  }
  return tc;
}

TargetChooser * TargetChooserFactory::createTargetChooser(MTGCardInstance * card){
  int id = card->getId();
  string s = card->spellTargetType;
  if (card->alias){
    id = card->alias;
    //TODO load target as well... ?
  }
  TargetChooser * tc = createTargetChooser(s, card);
  if (tc) return tc;
  //Any target than cannot be defined automatically is determined by its id
  switch (id){
    //Spell
  case 1224: //Spell blast
    {
#if defined (WIN32) || defined (LINUX)
      OutputDebugString ("Counter Spell !\n");
#endif
      return NEW SpellTargetChooser(card);
    }
    //Spell Or Permanent
  case 1282: //ChaosLace
  case 1152: //DeathLace
  case 1358: //PureLace
  case 1227: //ThoughLace
  case 1257: //Lifelace
    {
      return NEW SpellOrPermanentTargetChooser(card);
    }
    //Red Spell or Permanent
  case 1191: //Blue Elemental Blast
    {
      return NEW SpellOrPermanentTargetChooser(card,Constants::MTG_COLOR_RED);
    }
    //Blue Spell or Permanent
  case 1312: //Red Elemental Blast
    {
      return NEW SpellOrPermanentTargetChooser(card,Constants::MTG_COLOR_BLUE);
    }
    //Damage History
  case 1344: //Eye for an Eye
    {
      return NEW DamageTargetChooser(card,-1,1,RESOLVED_OK);
    }
  default:
    {
      return NULL;
    }
  }
}


TargetChooser::TargetChooser(MTGCardInstance * card, int _maxtargets, bool _other): TargetsList(){
  forceTargetListReady = 0;
  source = card;
  targetter = card;
  maxtargets = _maxtargets;
  other = _other;
}

//Default targetter : every card can be targetted, unless it is protected from the targetter card
// For spells that do not "target" a specific card, set targetter to NULL
bool TargetChooser::canTarget(Targetable * target){
  if (target->typeAsTarget() == TARGET_CARD){
    MTGCardInstance * card = (MTGCardInstance *) target;
    if (other){
      MTGCardInstance * tempcard = card;
      while (tempcard) {
        if (tempcard == source) return false;
        tempcard = tempcard->previous;
      }
    }
    if (source && targetter && card->isInPlay() && (card->has(Constants::SHROUD)|| card->protectedAgainst(targetter) )) return false;
    if (source && targetter && card->isInPlay() && (targetter->controller() != card->controller()) && (card->has(Constants::OPPONENTSHROUD) || card->protectedAgainst(targetter))) return false;
    return true;
  }
  else if (target->typeAsTarget() == TARGET_STACKACTION)
    return true;
  return false;
}


int TargetChooser::addTarget(Targetable * target){
  if (canTarget(target)){
    TargetsList::addTarget(target);
  }

  return targetsReadyCheck();
}


int TargetChooser::ForceTargetListReady(){
  int state =  targetsReadyCheck() ;
  if (state == TARGET_OK){
    forceTargetListReady = 1;
  }
  return forceTargetListReady;
}

int TargetChooser::targetsReadyCheck(){
  if (cursor == 0){
    return TARGET_NOK;
  }
  if (full()){
    return TARGET_OK_FULL;
  }
  if (!ready()){
    return TARGET_OK_NOT_READY;
  }
  return TARGET_OK;
}

int TargetChooser::targetListSet(){
  int state = targetsReadyCheck();
  if (state == TARGET_OK_FULL || forceTargetListReady){
    return 1;
  }
  return 0;
}

/**
  a specific Card
**/
CardTargetChooser::CardTargetChooser(MTGCardInstance * _card, MTGCardInstance * source,int * _zones, int _nbzones):TargetZoneChooser(_zones,_nbzones,source){
  validTarget = _card;
}

bool CardTargetChooser::canTarget(Targetable * target ){
  if (!target) return false;
  if (target->typeAsTarget() != TARGET_CARD) return false;
  if (!nbzones && !TargetChooser::canTarget(target)) return false;
  if (nbzones && !TargetZoneChooser::canTarget(target)) return false;
  MTGCardInstance * card = (MTGCardInstance *) target;
  while (card) {
    if (card == validTarget) return true;
    card = card->previous;
  }
  return false;
}

/**
   Choose anything that has a given list of types
**/
TypeTargetChooser::TypeTargetChooser(const char * _type, MTGCardInstance * card, int _maxtargets,bool other):TargetZoneChooser(card, _maxtargets,other){
  int id = Subtypes::subtypesList->find(_type);
  nbtypes = 0;
  addType(id);
  int default_zones[] = {MTGGameZone::MY_BATTLEFIELD, MTGGameZone::OPPONENT_BATTLEFIELD};
  init(default_zones,2);
}

TypeTargetChooser::TypeTargetChooser(const char * _type, int * _zones, int nbzones, MTGCardInstance * card, int _maxtargets,bool other):TargetZoneChooser(card, _maxtargets,other){
  int id = Subtypes::subtypesList->find(_type);
  nbtypes = 0;
  addType(id);
  if (nbzones == 0){
    int default_zones[] = {MTGGameZone::MY_BATTLEFIELD, MTGGameZone::OPPONENT_BATTLEFIELD};
    init(default_zones,2);
  }else{
    init(_zones, nbzones);
  }
}

void TypeTargetChooser::addType(const char * _type){
  int id = Subtypes::subtypesList->find(_type);
  addType(id);
}

void TypeTargetChooser::addType(int type){
  types[nbtypes] = type;
  nbtypes++;
}

bool TypeTargetChooser::canTarget(Targetable * target){
  if (!TargetZoneChooser::canTarget(target)) return false;
  if (target->typeAsTarget() == TARGET_CARD){
    MTGCardInstance * card = (MTGCardInstance *) target;
    for (int i= 0; i < nbtypes; i++){
      if (card->hasSubtype(types[i])) return true;
      if (Subtypes::subtypesList->find(card->getLCName()) == types[i]) return true;
    }
    return false;
  }else if (target->typeAsTarget() == TARGET_STACKACTION){
    Interruptible * action = (Interruptible *) target;
    if (action->type == ACTION_SPELL && action->state==NOT_RESOLVED){
      Spell * spell = (Spell *) action;
      MTGCardInstance * card = spell->source;
      for (int i= 0; i < nbtypes; i++){
        if (card->hasSubtype(types[i])) return true;
        if (Subtypes::subtypesList->find(card->name) == types[i]) return true;
      }
      return false;
    }
  }
  return false;
}


/**
    A Target Chooser associated to a Card Descriptor object, for fine tuning of targets description
**/
DescriptorTargetChooser::DescriptorTargetChooser(CardDescriptor * _cd, MTGCardInstance * card, int _maxtargets, bool other):TargetZoneChooser(card, _maxtargets, other){
  int default_zones[] = {MTGGameZone::MY_BATTLEFIELD, MTGGameZone::OPPONENT_BATTLEFIELD};
  init(default_zones,2);
  cd = _cd;
}

DescriptorTargetChooser::DescriptorTargetChooser(CardDescriptor * _cd, int * _zones, int nbzones, MTGCardInstance * card, int _maxtargets, bool other):TargetZoneChooser(card, _maxtargets, other){
  if (nbzones == 0){
    int default_zones[] = {MTGGameZone::MY_BATTLEFIELD, MTGGameZone::OPPONENT_BATTLEFIELD};
    init(default_zones,2);
  }else{
    init(_zones, nbzones);
  }
  cd = _cd;
}

bool DescriptorTargetChooser::canTarget(Targetable * target){
  if (!TargetZoneChooser::canTarget(target)) return false;
  if (target->typeAsTarget() == TARGET_CARD){
    MTGCardInstance * _target = (MTGCardInstance *) target;
    if (cd->match(_target)) return true;
  }else if (target->typeAsTarget() == TARGET_STACKACTION){
    Interruptible * action = (Interruptible *) target;
    if (action->type == ACTION_SPELL && action->state==NOT_RESOLVED){
      Spell * spell = (Spell *) action;
      MTGCardInstance * card = spell->source;
      if (cd->match(card)) return true;
    }
  }
  return false;
}

DescriptorTargetChooser::~DescriptorTargetChooser(){
  SAFE_DELETE(cd);
}

/**
   Choose a creature
**/

CreatureTargetChooser::CreatureTargetChooser( MTGCardInstance * card, int _maxtargets, bool other):TargetZoneChooser(card, _maxtargets, other){
  int default_zones[] = {MTGGameZone::MY_BATTLEFIELD, MTGGameZone::OPPONENT_BATTLEFIELD};
  init(default_zones,2);
  maxpower=  -1;
  maxtoughness=  -1;
}

CreatureTargetChooser::CreatureTargetChooser(int * _zones, int nbzones, MTGCardInstance * card, int _maxtargets, bool other):TargetZoneChooser(card, _maxtargets, other){
  if (nbzones == 0){
    int default_zones[] = {MTGGameZone::MY_BATTLEFIELD, MTGGameZone::OPPONENT_BATTLEFIELD};
    init(default_zones,2);
  }else{
    init(_zones, nbzones);
  }
  maxpower = -1;
  maxtoughness=  -1;
}


bool CreatureTargetChooser::canTarget(Targetable * target){
  if (!TargetZoneChooser::canTarget(target)) return false;
  if (target->typeAsTarget() == TARGET_CARD){
    MTGCardInstance * card = (MTGCardInstance *) target;
    if (maxpower != -1 && card->power > maxpower) return false;
    if (maxtoughness != -1 && card->toughness > maxtoughness) return false;
    return card->isCreature();
  }
  return false;
}


/* TargetzoneChooser targets everything in a given zone */
TargetZoneChooser::TargetZoneChooser(MTGCardInstance * card, int _maxtargets, bool other):TargetChooser(card,_maxtargets, other){
  init(NULL,0);
}

TargetZoneChooser::TargetZoneChooser(int * _zones, int _nbzones,MTGCardInstance * card, int _maxtargets, bool other):TargetChooser(card,_maxtargets, other){
  init(_zones, _nbzones);
}

int TargetZoneChooser::init(int * _zones, int _nbzones){
  for (int i = 0; i < _nbzones; i++){
    zones[i] = _zones[i];
  }
  nbzones = _nbzones;
  return nbzones;
}

int TargetZoneChooser::setAllZones(){
    int zones[] = {
      MTGGameZone::MY_BATTLEFIELD, 
      MTGGameZone::MY_EXILE,
      MTGGameZone::MY_GRAVEYARD,
      MTGGameZone::MY_HAND,
      MTGGameZone::MY_LIBRARY,
      MTGGameZone::MY_STACK,
      MTGGameZone::OPPONENT_BATTLEFIELD, 
      MTGGameZone::OPPONENT_EXILE,
      MTGGameZone::OPPONENT_GRAVEYARD,
      MTGGameZone::OPPONENT_HAND,
      MTGGameZone::OPPONENT_LIBRARY,
      MTGGameZone::OPPONENT_STACK
    };

    init(zones,12);
  return 1;
} 


bool TargetZoneChooser::canTarget(Targetable * target){
  if (!TargetChooser::canTarget(target)) return false;
  if (target->typeAsTarget() == TARGET_CARD){
    MTGCardInstance * card = (MTGCardInstance *) target;
    for (int i = 0; i<nbzones; i++)
      if (MTGGameZone::intToZone(zones[i],source,card)->hasCard(card)) return true;
  }else if (target->typeAsTarget() == TARGET_STACKACTION){
OutputDebugString ("CHECKING INTERRUPTIBLE\n");
    Interruptible * action = (Interruptible *) target;
    if (action->type == ACTION_SPELL && action->state==NOT_RESOLVED){
      Spell * spell = (Spell *) action;
      MTGCardInstance * card = spell->source;
      for (int i = 0; i<nbzones; i++)
	if (MTGGameZone::intToZone(zones[i],source,card)->hasCard(card)) return true;
    }
  }
  return false;
}


bool TargetZoneChooser::targetsZone(MTGGameZone * z){
  for (int i = 0; i < nbzones; i++)
    if (MTGGameZone::intToZone(zones[i],source) == z) return true;
  return false;
}

/* Player Target */
PlayerTargetChooser::PlayerTargetChooser(MTGCardInstance * card, int _maxtargets, Player *p):TargetChooser(card, _maxtargets), p(p){
}

bool PlayerTargetChooser::canTarget(Targetable * target){
  return (target->typeAsTarget() == TARGET_PLAYER) && (!p || p == (Player*)target);
}

/*Damageable Target */
bool DamageableTargetChooser::canTarget(Targetable * target){
  if (target->typeAsTarget() == TARGET_PLAYER){
    return true;
  }
  return CreatureTargetChooser::canTarget(target);
}


/*Spell */



SpellTargetChooser::SpellTargetChooser(MTGCardInstance * card,int _color, int _maxtargets, bool other ):TargetChooser(card, _maxtargets, other){
  color = _color;
}

bool SpellTargetChooser::canTarget(Targetable * target){
  MTGCardInstance * card = NULL;
  if (target->typeAsTarget() == TARGET_STACKACTION){
    Interruptible * action = (Interruptible *) target;
    if (action->type == ACTION_SPELL && action->state==NOT_RESOLVED){
      Spell * spell = (Spell *) action;
      card = spell->source;
      if (card && (color == -1 || card->hasColor(color))) return true;
    }
  }

  return false;
}


/*Spell or Permanent */
SpellOrPermanentTargetChooser::SpellOrPermanentTargetChooser(MTGCardInstance * card,int _color, int _maxtargets, bool other):TargetZoneChooser(card, _maxtargets, other){
  int default_zones[] = {MTGGameZone::MY_BATTLEFIELD, MTGGameZone::OPPONENT_BATTLEFIELD};
  init(default_zones,2);
  color = _color;
}

bool SpellOrPermanentTargetChooser::canTarget(Targetable * target){
  MTGCardInstance * card = NULL;
  if (target->typeAsTarget() == TARGET_CARD){
    card = (MTGCardInstance *) target;
    if (color == -1 || card->hasColor(color)) return TargetZoneChooser::canTarget(target);
  }else if (target->typeAsTarget() == TARGET_STACKACTION){
    Interruptible * action = (Interruptible *) target;
    if (action->type == ACTION_SPELL && action->state==NOT_RESOLVED){
      Spell * spell = (Spell *) action;
      card = spell->source;
      if (card && (color == -1 || card->hasColor(color))) return true;
    }
  }
  return false;
}



/*Damage */
DamageTargetChooser::DamageTargetChooser(MTGCardInstance * card,int _color, int _maxtargets, int _state):TargetChooser(card, _maxtargets){
  color = _color;
  state = _state;
}

bool DamageTargetChooser::canTarget(Targetable * target){
  MTGCardInstance * card = NULL;
  if (target->typeAsTarget() == TARGET_STACKACTION){
    Interruptible * action = (Interruptible *) target;
    if (action->type == ACTION_DAMAGE && (action->state == state || state == -1)){
      Damage * damage = (Damage *) action;
      card = damage->source;
      if (card && (color == -1 || card->hasColor(color))) return true;
    }
  }
  return false;
}
