#include "../include/config.h"
#include "../include/TargetChooser.h"
#include "../include/CardDescriptor.h"
#include "../include/MTGGameZones.h"
#include "../include/GameObserver.h"
#include "../include/Subtypes.h"



TargetChooser * TargetChooserFactory::createTargetChooser(string s, MTGCardInstance * card){
  if (!s.size()) return NULL;

  GameObserver * game = GameObserver::GetInstance();
  MTGGameZone * zones[10];
  int nbzones = 0;
  unsigned int found;

  found = s.find("player");
  if (found != string::npos){
    int maxtargets = 1;
    unsigned int several = s.find_first_of('s',5);
    if (several != string::npos) maxtargets = -1;
    found = s.find("creature");
    if (found != string::npos) return NEW DamageableTargetChooser(card,maxtargets); //Any Damageable target (player, creature)
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
      zones[nbzones] = game->currentlyActing()->game->inPlay;
    
      //Graveyards
      if(zoneName.compare("graveyard") == 0){
        zones[nbzones] = game->players[0]->game->graveyard;
        nbzones++;
        zones[nbzones] = game->players[1]->game->graveyard;
      }else if(zoneName.compare("inplay") == 0){
          zones[nbzones] = game->players[0]->game->inPlay;
          nbzones++;
          zones[nbzones] = game->players[1]->game->inPlay;
      }else if(zoneName.compare("stack") == 0){
        zones[nbzones] = game->players[0]->game->stack;
        nbzones++;
        zones[nbzones] = game->players[1]->game->stack;
      }else{
          MTGGameZone * zone = MTGGameZone::stringToZone(zoneName, card,card);
          if (zone) zones[nbzones] = zone;
      }
      nbzones++;
    }
  }else{
    s1 = s;
    nbzones = 2;
    zones[0]= game->players[0]->game->inPlay;
    zones[1]= game->players[1]->game->inPlay;
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
#ifdef WIN32
      OutputDebugString("Advanced Attributes 1 \n");
#endif
      string attributes = typeName.substr(found+1,end-found-1);
#ifdef WIN32
      OutputDebugString(attributes.c_str());
      OutputDebugString("\n");
#endif
      cd = NEW CardDescriptor();
      while(attributes.size()){
        unsigned int found2 = attributes.find(";");
        string attribute;
        if (found2 != string::npos){
          attribute = attributes.substr(0,found2);
          attributes = attributes.substr(found2+1);
        }else{
          attribute = attributes;
          attributes = "";
        }
        int minus = 0;
        if (attribute[0] == '-'){
#ifdef WIN32
          OutputDebugString("MINUS\n");
#endif
          minus = 1;
          nbminuses++;
          attribute=attribute.substr(1);
        }
#ifdef WIN32
        OutputDebugString(attribute.c_str());
        OutputDebugString("\n");
#endif
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
	          cd->defenser = (MTGCardInstance *)-1; //Oh yeah, that's ugly....
          }else{
	          cd->defenser = (MTGCardInstance *)1;
          }
        //Tapped, untapped
        }else if (attribute.find("tapped") != string::npos){
          if (minus){
	          cd->tapped = -1;
          }else{
	          cd->tapped = 1;
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
      if (nbminuses == 0) cd->mode = CD_OR;
      typeName = typeName.substr(0,found);
    }
    //X targets allowed ?
    if (typeName.at(typeName.length()-1) == 's' && !Subtypes::subtypesList->find(typeName)){
      typeName = typeName.substr(0,typeName.length()-1);
      maxtargets = -1;
    }
    if (cd){
      if (!tc){
        if (typeName.compare("*")!=0) cd->setSubtype(typeName);

        tc = NEW DescriptorTargetChooser(cd,zones,nbzones,card,maxtargets);
      }else{
        delete(cd);
        return NULL;
      }
    }else{
      if (!tc){
        if (typeName.compare("*")==0){
          return NEW TargetZoneChooser(zones, nbzones,card, maxtargets);
        }else{
          tc =  NEW TypeTargetChooser(typeName.c_str(), zones, nbzones, card,maxtargets);
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


TargetChooser::TargetChooser(MTGCardInstance * card, int _maxtargets): TargetsList(){
  forceTargetListReady = 0;
  source = card;
  maxtargets = _maxtargets;
}

//Default targetter : every card can be targetted, unless it is protected from the source card
// For spells that do not "target" a specific card, set source to NULL
int TargetChooser::canTarget(Targetable * target){
  if (target->typeAsTarget() == TARGET_CARD){
    MTGCardInstance * card = (MTGCardInstance *) target;
    if (source && (card->protectedAgainst(source) || card->has(Constants::SHROUD))) return 0;
    return 1;
  }else if (target->typeAsTarget() == TARGET_STACKACTION){
    return 1;
  }
  return 0;
}


int TargetChooser::addTarget(Targetable * target){
  if (canTarget(target) && TargetsList::addTarget(target)){
  }

#if defined (WIN32) || defined (LINUX)
  char buf[4096];
  sprintf(buf, "Nb targets : %i\n", cursor);
  OutputDebugString(buf);
#endif
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
   Choose anything that has a given list of types
**/
TypeTargetChooser::TypeTargetChooser(const char * _type, MTGCardInstance * card, int _maxtargets):TargetZoneChooser(card, _maxtargets){
  int id = Subtypes::subtypesList->Add(_type);
  nbtypes = 0;
  addType(id);
  GameObserver * game = GameObserver::GetInstance();
  MTGGameZone * default_zones[] = {game->players[0]->game->inPlay, game->players[1]->game->inPlay};
  init(default_zones,2);
}

TypeTargetChooser::TypeTargetChooser(const char * _type, MTGGameZone ** _zones, int nbzones, MTGCardInstance * card, int _maxtargets):TargetZoneChooser(card, _maxtargets){
  int id = Subtypes::subtypesList->Add(_type);
  nbtypes = 0;
  addType(id);
  GameObserver * game = GameObserver::GetInstance();
  if (nbzones == 0){
    MTGGameZone * default_zones[] = {game->players[0]->game->inPlay, game->players[1]->game->inPlay};
    init(default_zones,2);
  }else{
    init(_zones, nbzones);
  }
}

void TypeTargetChooser::addType(const char * _type){
  int id = Subtypes::subtypesList->Add(_type);
  addType(id);
}

void TypeTargetChooser::addType(int type){
  types[nbtypes] = type;
  nbtypes++;
}

int TypeTargetChooser::canTarget(Targetable * target ){
  if (!TargetZoneChooser::canTarget(target)) return 0;
  if (target->typeAsTarget() == TARGET_CARD){
    MTGCardInstance * card = (MTGCardInstance *) target;
    for (int i= 0; i < nbtypes; i++){
      if (card->hasSubtype(types[i])) return 1;
      if (Subtypes::subtypesList->find(card->name) == types[i]) return 1;
    }
    return 0;
  }else if (target->typeAsTarget() == TARGET_STACKACTION){
    Interruptible * action = (Interruptible *) target;
    if (action->type == ACTION_SPELL && action->state==NOT_RESOLVED){
      Spell * spell = (Spell *) action;
      MTGCardInstance * card = spell->source;
      for (int i= 0; i < nbtypes; i++){
        if (card->hasSubtype(types[i])) return 1;
        if (Subtypes::subtypesList->find(card->name) == types[i]) return 1;
      }
      return 0;
    }
  }
  return 0;
}


/**
    A Target Chooser associated to a Card Descriptor object, for fine tuning of targets description
**/
DescriptorTargetChooser::DescriptorTargetChooser(CardDescriptor * _cd, MTGCardInstance * card, int _maxtargets):TargetZoneChooser(card, _maxtargets){
  GameObserver * game = GameObserver::GetInstance();
  MTGGameZone * default_zones[] = {game->players[0]->game->inPlay, game->players[1]->game->inPlay};
  init(default_zones,2);
  cd = _cd;
}

DescriptorTargetChooser::DescriptorTargetChooser(CardDescriptor * _cd, MTGGameZone ** _zones, int nbzones, MTGCardInstance * card, int _maxtargets):TargetZoneChooser(card, _maxtargets){
  GameObserver * game = GameObserver::GetInstance();
  if (nbzones == 0){
    MTGGameZone * default_zones[] = {game->players[0]->game->inPlay, game->players[1]->game->inPlay};
    init(default_zones,2);
  }else{
    init(_zones, nbzones);
  }
  cd = _cd;
}

int DescriptorTargetChooser::canTarget(Targetable * target){
  if (!TargetZoneChooser::canTarget(target)) return 0;
  if (target->typeAsTarget() == TARGET_CARD){
    MTGCardInstance * _target = (MTGCardInstance *) target;
    if (cd->match(_target)) return 1;
  }else if (target->typeAsTarget() == TARGET_STACKACTION){
    Interruptible * action = (Interruptible *) target;
    if (action->type == ACTION_SPELL && action->state==NOT_RESOLVED){
      Spell * spell = (Spell *) action;
      MTGCardInstance * card = spell->source;
      if (cd->match(card)) return 1;
    }
  }
  return 0;
}

DescriptorTargetChooser::~DescriptorTargetChooser(){
  SAFE_DELETE(cd);
}

/**
   Choose a creature
**/

CreatureTargetChooser::CreatureTargetChooser( MTGCardInstance * card, int _maxtargets):TargetZoneChooser(card, _maxtargets){
  GameObserver * game = GameObserver::GetInstance();
  MTGGameZone * default_zones[] = {game->players[0]->game->inPlay, game->players[1]->game->inPlay};
  init(default_zones,2);
  maxpower=  -1;
  maxtoughness=  -1;
}

CreatureTargetChooser::CreatureTargetChooser(MTGGameZone ** _zones, int nbzones, MTGCardInstance * card, int _maxtargets):TargetZoneChooser(card, _maxtargets){
  GameObserver * game = GameObserver::GetInstance();
  if (nbzones == 0){
    MTGGameZone * default_zones[] = {game->players[0]->game->inPlay, game->players[1]->game->inPlay};
    init(default_zones,2);
  }else{
    init(_zones, nbzones);
  }
  maxpower = -1;
  maxtoughness=  -1;
}


int CreatureTargetChooser::canTarget(Targetable * target){
  if (!TargetZoneChooser::canTarget(target)) return 0;
  if (target->typeAsTarget() == TARGET_CARD){
    MTGCardInstance * card = (MTGCardInstance *) target;
    if (maxpower != -1 && card->power > maxpower) return 0;
    if (maxtoughness != -1 && card->toughness > maxtoughness) return 0;
    return card->isACreature();
  }
  return 0;
}


TargetZoneChooser::TargetZoneChooser(MTGCardInstance * card, int _maxtargets){
  init(NULL,0);
  source = card;
  maxtargets = _maxtargets;
}

TargetZoneChooser::TargetZoneChooser(MTGGameZone ** _zones, int _nbzones,MTGCardInstance * card, int _maxtargets){
  init(_zones, _nbzones);
  source = card;
  maxtargets = _maxtargets;
}

int TargetZoneChooser::init(MTGGameZone ** _zones, int _nbzones){
  for (int i = 0; i < _nbzones; i++){
    zones[i] = _zones[i];
  }
  nbzones = _nbzones;
  return nbzones;
}

int TargetZoneChooser::canTarget(Targetable * target){
  if (!TargetChooser::canTarget(target)) return 0;
  if (target->typeAsTarget() == TARGET_CARD){
    MTGCardInstance * card = (MTGCardInstance *) target;
    for (int i = 0; i<nbzones; i++){
      if (zones[i]->hasCard(card)) return 1;
    }
  }else if (target->typeAsTarget() == TARGET_STACKACTION){
OutputDebugString ("CHECKING INTERRUPTIBLE\n");
    Interruptible * action = (Interruptible *) target;
    if (action->type == ACTION_SPELL && action->state==NOT_RESOLVED){
      Spell * spell = (Spell *) action;
      MTGCardInstance * card = spell->source;
      for (int i = 0; i<nbzones; i++){
          if (zones[i]->hasCard(card)) return 1;
      }
    }
  }
  return 0;
}


/* Player Target */
int PlayerTargetChooser::canTarget(Targetable * target){
  if (target->typeAsTarget() == TARGET_PLAYER){
    return 1;
  }
  return 0;
}

/*Damageable Target */
int DamageableTargetChooser::canTarget(Targetable * target){
  if (target->typeAsTarget() == TARGET_PLAYER){
#if defined (WIN32) || defined (LINUX)
    OutputDebugString("Targetting Player !!!\n");
#endif
    return 1;
  }
  return CreatureTargetChooser::canTarget(target);
}


/*Spell */



SpellTargetChooser::SpellTargetChooser(MTGCardInstance * card,int _color, int _maxtargets ):TargetChooser(card, _maxtargets){
  color = _color;
}

int SpellTargetChooser::canTarget(Targetable * target){
  MTGCardInstance * card = NULL;
  if (target->typeAsTarget() == TARGET_STACKACTION){
    Interruptible * action = (Interruptible *) target;
    if (action->type == ACTION_SPELL && action->state==NOT_RESOLVED){
      Spell * spell = (Spell *) action;
      card = spell->source;
      if (card && (color == -1 || card->hasColor(color))) return 1;
    }
  }

  return 0;

}


/*Spell or Permanent */
SpellOrPermanentTargetChooser::SpellOrPermanentTargetChooser(MTGCardInstance * card,int _color, int _maxtargets):TargetZoneChooser(card, _maxtargets){
  GameObserver * game = GameObserver::GetInstance();
  MTGGameZone * default_zones[] = {game->players[0]->game->inPlay, game->players[1]->game->inPlay};
  init(default_zones,2);
  color = _color;
}

int SpellOrPermanentTargetChooser::canTarget(Targetable * target){
  MTGCardInstance * card = NULL;
  if (target->typeAsTarget() == TARGET_CARD){
    card = (MTGCardInstance *) target;
    if (color == -1 || card->hasColor(color)) return TargetZoneChooser::canTarget(target);
  }else if (target->typeAsTarget() == TARGET_STACKACTION){
    Interruptible * action = (Interruptible *) target;
    if (action->type == ACTION_SPELL && action->state==NOT_RESOLVED){
      Spell * spell = (Spell *) action;
      card = spell->source;
      if (card && (color == -1 || card->hasColor(color))) return 1;
    }
  }

  return 0;

}



/*Damage */
DamageTargetChooser::DamageTargetChooser(MTGCardInstance * card,int _color, int _maxtargets, int _state):TargetChooser(card, _maxtargets){
  color = _color;
  state = _state;
}

int DamageTargetChooser::canTarget(Targetable * target){
  MTGCardInstance * card = NULL;
  if (target->typeAsTarget() == TARGET_STACKACTION){
    Interruptible * action = (Interruptible *) target;
    if (action->type == ACTION_DAMAGE && (action->state == state || state == -1)){
      Damage * damage = (Damage *) action;
      card = damage->source;
      if (card && (color == -1 || card->hasColor(color))) return 1;
    }
  }

  return 0;

}


/*Damage or Permanent */
DamageOrPermanentTargetChooser::DamageOrPermanentTargetChooser(MTGCardInstance * card,int _color, int _maxtargets):TargetZoneChooser(card, _maxtargets){
  GameObserver * game = GameObserver::GetInstance();
  MTGGameZone * default_zones[] = {game->players[0]->game->inPlay, game->players[1]->game->inPlay};
  init(default_zones,2);
  color = _color;
}

int DamageOrPermanentTargetChooser::canTarget(Targetable * target){
  MTGCardInstance * card = NULL;
  if (target->typeAsTarget() == TARGET_CARD){
    card = (MTGCardInstance *) target;
    if (color == -1 || card->hasColor(color)) return TargetZoneChooser::canTarget(target);
  }else if (target->typeAsTarget() == TARGET_STACKACTION){
    Interruptible * action = (Interruptible *) target;
    if (action->type == ACTION_DAMAGE){
      Damage * damage = (Damage *) action;
      card = damage->source;
      if (card && (color == -1 || card->hasColor(color))) return 1;
    }
  }

  return 0;

}
