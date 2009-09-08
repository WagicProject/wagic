/*---------------------------------------------
  Card Instance
  Instance of a given MTGCard in the game
  Although there is only one MTGCard of each type, there can be as much Instances of it as needed in the game
  --------------------------------------------
*/
#include "../include/config.h"
#include "../include/MTGCardInstance.h"
#include "../include/CardDescriptor.h"
#include "../include/Counters.h"
#include "../include/Subtypes.h"
#include <algorithm>
using namespace std;

MTGCardInstance::MTGCardInstance(): MTGCard(), Damageable(0), view(NULL){
  LOG("==Creating MTGCardInstance==");
  initMTGCI();
  LOG("==Creating MTGCardInstance Successful==");
}
MTGCardInstance::MTGCardInstance(MTGCard * card, MTGPlayerCards * arg_belongs_to): MTGCard(card), Damageable(card->getToughness()), view(NULL){
  LOG("==Creating MTGCardInstance==");
  initMTGCI();
  model = card;
  attacker = 0;
  lifeOrig = life;
  belongs_to = arg_belongs_to;
  owner = NULL;
  if (arg_belongs_to) owner = arg_belongs_to->library->owner;
  lastController = owner;
  defenser = NULL;
  banding = NULL;
  life = toughness;
  LOG("==Creating MTGCardInstance Successful==");

}

void MTGCardInstance::copy(MTGCardInstance * card){
 MTGCard * source = card->model;
 for(map<int,int>::const_iterator it = source->basicAbilities.begin(); it != source->basicAbilities.end(); ++it){
   int i = it->first;
   basicAbilities[i] = source->basicAbilities[i];
 }
  for (int i = 0; i< MAX_TYPES_PER_CARD; i++){
    types[i] = source->types[i];
  }
  nb_types = source->nb_types;
  for (int i = 0; i< Constants::MTG_NB_COLORS; i++){
    colors[i] = source->colors[i];
  }
  manaCost.copy(source->getManaCost());

  text = source->text;
  name = source->name;
  //strcpy(image_name, source->image_name);

  //rarity = source->rarity;
  power = source->power;
  toughness = source->toughness;
  life = toughness;
  lifeOrig = life;
  //mtgid = source->mtgid;
  //setId = source->setId;
  magicText = source->magicText;
  spellTargetType = source->spellTargetType;
  alias = source->alias;

  //Now this is dirty...
  int backupid = mtgid;
  mtgid = source->getId();
  Spell * spell = NEW Spell(this);
  AbilityFactory af;
  GameObserver * g = GameObserver::GetInstance();
  af.addAbilities(g->mLayers->actionLayer()->getMaxId(), spell);
  delete spell;
  mtgid = backupid;
}

MTGCardInstance::~MTGCardInstance(){
  LOG("==Deleting MTGCardInstance==");
  SAFE_DELETE(untapBlockers);
  SAFE_DELETE(counters);
  SAFE_DELETE(previous);
  LOG("==Deleting MTGCardInstance Succesfull==");
}
void MTGCardInstance::initMTGCI(){
  sample = "";
  model=NULL;
  isToken = false;
  lifeOrig = 0;
  doDamageTest = 1;
  belongs_to=NULL;
  tapped = 0;
  untapBlockers = NULL;
  untapping = 0;
  summoningSickness = 0;
  target = NULL;
  nbprotections = 0;
  type_as_damageable = DAMAGEABLE_MTGCARDINSTANCE;
  banding = NULL;
  owner = NULL;
  changedZoneRecently = 0;
  counters = NEW Counters(this);
  previousZone = NULL;
  previous = NULL;
  next = NULL;
  lastController = NULL;
  regenerateTokens = 0;
  blocked = false;
}


const string MTGCardInstance::getDisplayName() const {
  return getName();
}

void MTGCardInstance::addType(int type){
  types[nb_types] = type;
  nb_types++;
}

UntapBlockers * MTGCardInstance::getUntapBlockers(){
  if (!untapBlockers) untapBlockers = NEW UntapBlockers();
  return untapBlockers;
}

int MTGCardInstance::isInPlay(){
  GameObserver * game = GameObserver::GetInstance();
  for (int i = 0 ; i < 2 ; i++){
    MTGGameZone * zone = game->players[i]->game->inPlay;
    if (zone->hasCard(this)) return 1;
  }
  return 0;
}

int MTGCardInstance::afterDamage(){
  if (!doDamageTest) return 0;
  doDamageTest = 0;
  if (!isCreature()) return 0;
  if (life <=0 && isInPlay()){
    return destroy();
  }
  return 0;
}

int MTGCardInstance::bury(){
    Player * p = controller();
	if (!basicAbilities[Constants::INDESTRUCTIBLE]){
    p->game->putInZone(this,p->game->inPlay,owner->game->graveyard);
    return 1;
	}
	return 0;
}
int MTGCardInstance::destroy(){
    if (!triggerRegenerate()) return bury();
    return 0;
}

MTGGameZone * MTGCardInstance::getCurrentZone(){
  GameObserver * game = GameObserver::GetInstance();
  for (int i = 0; i < 2; i++){
    MTGPlayerCards * g = game->players[i]->game;
    MTGGameZone * zones[] = {g->inPlay,g->graveyard,g->hand, g->library};
    for (int k = 0; k < 4; k++){
      MTGGameZone * zone = zones[k];
      if (zone->hasCard(this)) return zone;
    }
  }
  return NULL;
}

int MTGCardInstance::has(int basicAbility){
  return basicAbilities[basicAbility];
}

//Taps the card
void MTGCardInstance::tap(){
  if (tapped) return;
  tapped = 1;
  WEvent * e = NEW WEventCardTap(this, 0, 1);
  GameObserver * game = GameObserver::GetInstance();
  game->receiveEvent(e);
}

void MTGCardInstance::untap(){
  if (!tapped) return;
  tapped = 0;
  WEvent * e = NEW WEventCardTap(this, 1, 0);
  GameObserver * game = GameObserver::GetInstance();
  game->receiveEvent(e);
}


void MTGCardInstance::setUntapping(){
  untapping = 1;
}

int MTGCardInstance::isUntapping(){
  return untapping;
}

//Tries to Untap the card
void MTGCardInstance::attemptUntap(){
  if (untapping){
    untap();
    untapping = 0;
  }
}

//Tells if the card is tapped or not
int MTGCardInstance::isTapped(){
  return tapped;
}

void MTGCardInstance::resetAllDamage(){
  nb_damages = 0;
}

int MTGCardInstance::regenerate(){
  return ++regenerateTokens;
}

int MTGCardInstance::triggerRegenerate(){
  if (! regenerateTokens) return 0;
  regenerateTokens--;
  tap();
  life = toughness;
  initAttackersDefensers();
  return 1;
}


int MTGCardInstance::initAttackersDefensers(){
  setAttacker(0);
  setDefenser(NULL);
  banding = NULL;
  blockers.clear();
  blocked = false;
  return 1;
}

//Function to call to remove all damages, etc to a card (generally at the end of the turn)
int MTGCardInstance::cleanup(){
  initAttackersDefensers();
  life=toughness;
  GameObserver * game = GameObserver::GetInstance();
  if (!game || game->currentPlayer == controller()) summoningSickness = 0;
  if (previous && !previous->stillInUse()){
    SAFE_DELETE(previous);
  }
  regenerateTokens = 0;
  return 1;
}

int MTGCardInstance::stillInUse(){
GameObserver * game = GameObserver::GetInstance();
if (game->mLayers->actionLayer()->stillInUse(this)) return 1;
if (!previous) return 0;
return previous->stillInUse();
}

/* Summoning Sickness
 * 212.3f A creature's activated ability with the tap symbol or the untap symbol in its activation cost
 * can't be played unless the creature has been under its controller's control since the start of his or
 * her most recent turn. A creature can't attack unless it has been under its controller's control
 * since the start of his or her most recent turn. This rule is informally called the "summoning
 * sickness" rule. Ignore this rule for creatures with haste (see rule 502.5).
 */
int MTGCardInstance::hasSummoningSickness(){
  if (!summoningSickness) return 0;
  if (basicAbilities[Constants::HASTE]) return 0;
  if (!isCreature()) return 0;
  return 1;
}

MTGCardInstance * MTGCardInstance::changeController(Player * newController){
  Player * originalOwner = controller();
  if (originalOwner  == newController) return 0;
  MTGCardInstance * copy = originalOwner->game->putInZone(this, originalOwner->game->inPlay, newController->game->inPlay);
  copy->summoningSickness = 1;
  return copy;
}

//Reset the card parameters
int MTGCardInstance::reset(){
  cleanup();
  untap();
  SAFE_DELETE(counters);
  counters = NEW Counters(this);
  return 1;
}


Player * MTGCardInstance::controller(){
  GameObserver * game = GameObserver::GetInstance();
  if (!game) return NULL;
  for (int i = 0; i < 2; ++i){
    if (game->players[i]->game->inPlay->hasCard(this)) return  game->players[i];
    if (game->players[i]->game->stack->hasCard(this)) return  game->players[i];
    if (game->players[i]->game->graveyard->hasCard(this)) return  game->players[i];
    if (game->players[i]->game->hand->hasCard(this)) return  game->players[i];
    if (game->players[i]->game->library->hasCard(this)) return  game->players[i];
  }
  return lastController;
}

int MTGCardInstance::canAttack(){
  if (tapped) return 0;
  if (hasSummoningSickness()) return 0;
  if (basicAbilities[Constants::DEFENSER] || basicAbilities[Constants::CANTATTACK]) return 0;
  if (!isCreature()) return 0;
  if (!isInPlay()) return 0;
  return 1;
}


int MTGCardInstance::addToToughness(int value){
  toughness+=value;
  life+=value;
  doDamageTest = 1;
  return 1;
}

int MTGCardInstance::setToughness(int value){
  toughness=value;
  life=value;
  doDamageTest = 1;
  return 1;
}

int MTGCardInstance::canBlock(){
  if (tapped) return 0;
  if (basicAbilities[Constants::CANTBLOCK]) return 0;
  if (!isCreature()) return 0;
  if (!isInPlay()) return 0;
  return 1;
}

int MTGCardInstance::canBlock(MTGCardInstance * opponent){
  if (!canBlock()) return 0;
  if (!opponent) return 1;
  if (!opponent->isAttacker()) return 0;
  // Comprehensive rule 502.7f : If a creature with protection attacks, it can't be blocked by creatures that have the stated quality.
  if (opponent->protectedAgainst(this)) return 0;
  if (opponent->basicAbilities[Constants::UNBLOCKABLE]) return 0;
  if (opponent->basicAbilities[Constants::FEAR] && !(hasColor(Constants::MTG_COLOR_ARTIFACT) || hasColor(Constants::MTG_COLOR_BLACK))) return 0;
  if (opponent->basicAbilities[Constants::FLYING] && !( basicAbilities[Constants::FLYING] || basicAbilities[Constants::REACH])) return 0;
  //Can block only creatures with flying if has cloud
  if (basicAbilities[Constants::CLOUD] && !( opponent->basicAbilities[Constants::FLYING])) return 0;
  // If opponent has shadow and a creature does not have either shadow or reachshadow it cannot be blocked
  if (opponent->basicAbilities[Constants::SHADOW] && !( basicAbilities[Constants::SHADOW] || basicAbilities[Constants::REACHSHADOW])) return 0;
  // If opponent does not have shadow and a creature has shadow it cannot be blocked
  if (!opponent->basicAbilities[Constants::SHADOW] && basicAbilities[Constants::SHADOW]) return 0;
  if (opponent->basicAbilities[Constants::SWAMPWALK] && controller()->game->inPlay->hasType("swamp")) return 0;
  if (opponent->basicAbilities[Constants::FORESTWALK] && controller()->game->inPlay->hasType("forest")) return 0;
  if (opponent->basicAbilities[Constants::ISLANDWALK] && controller()->game->inPlay->hasType("island")) return 0;
  if (opponent->basicAbilities[Constants::MOUNTAINWALK] && controller()->game->inPlay->hasType("mountain")) return 0;
  if (opponent->basicAbilities[Constants::PLAINSWALK] && controller()->game->inPlay->hasType("plains")) return 0;
  return 1;
}

MTGCardInstance * MTGCardInstance::getNextPartner(){
  MTGInPlay * inplay = controller()->game->inPlay;
  MTGCardInstance * bandingPartner = inplay->getNextAttacker(banding);
  while (bandingPartner){
    if (basicAbilities[Constants::BANDING] || bandingPartner->basicAbilities[Constants::BANDING]) return bandingPartner;
    bandingPartner = inplay->getNextAttacker(bandingPartner);
  }
  return NULL;
}

void MTGCardInstance::unband(){
  if (!banding) return;

  MTGCardInstance * _banding = banding;
  banding = NULL;
  MTGCardInstance * newbanding = NULL;
  MTGInPlay * inplay = controller()->game->inPlay;
  int nbpartners = inplay->nbPartners(this);
  MTGCardInstance * card = inplay->getNextAttacker(NULL);
  while(card){
    if (card != this){
      if (card->banding == _banding){
	if (nbpartners == 1){
	  card->banding = NULL;
	  return;
	}else{
	  if (!newbanding) newbanding = card;
	  card->banding = newbanding;
	}
      }
    }
    card = inplay->getNextAttacker(card);
  }
  return ;
}

int MTGCardInstance::setAttacker(int value){
  Targetable * previousTarget = NULL;
  Targetable * target = NULL;
  Player * p  = controller()->opponent();
  if (value) target = p;
  if (attacker) previousTarget = p;
  attacker = value;
  WEvent * e = NEW WEventCreatureAttacker(this,previousTarget, target);
  GameObserver::GetInstance()->receiveEvent(e);
  return 1;
}

int MTGCardInstance::toggleAttacker(){
  if (!attacker){
    if (!basicAbilities[Constants::VIGILANCE]) tap();
    setAttacker(1);
    return 1;
  }else{
    //Banding needs to be debugged...
    /*MTGCardInstance * bandingPartner = getNextPartner();
    if (bandingPartner){
      if (banding) unband();
      if (!bandingPartner->banding) bandingPartner->banding = bandingPartner;
      banding = bandingPartner->banding;
      return 1;
    }else{*/
      untap();
      setAttacker(0);
      return 1;
    //}
  }
  return 0;
}

int MTGCardInstance::isAttacker(){
  return attacker;
}

MTGCardInstance * MTGCardInstance::isDefenser(){
  return defenser;
}


int MTGCardInstance::nbOpponents(){
  int result= 0;
  MTGCardInstance*  opponent = getNextOpponent();
  while (opponent){
    result++;
    opponent = getNextOpponent(opponent);
  }
  return result;
}

MTGCardInstance * MTGCardInstance::getNextDefenser(MTGCardInstance * previous){
  int found_previous = 0;
  if (!previous) found_previous = 1;
  list<MTGCardInstance *>::iterator it;
  for (it= blockers.begin(); it != blockers.end(); ++it){
    MTGCardInstance * c = *it;
    if (found_previous && c->isInPlay()) return c;
    if (c == previous) found_previous = 1;
  }
  return NULL;
}

int MTGCardInstance::raiseBlockerRankOrder(MTGCardInstance * blocker){
  list<MTGCardInstance *>::iterator it1 = find(blockers.begin(), blockers.end(), blocker);
  list<MTGCardInstance *>::iterator it2 = it1;
  if (blockers.begin() == it2) ++it2; else --it2;

  std::iter_swap(it1,it2);
  WEvent* e = NEW WEventCreatureBlockerRank(*it1,*it2,this);
  GameObserver::GetInstance()->receiveEvent(e);
  //delete(e);
  return 1;
}

int MTGCardInstance::bringBlockerToFrontOfOrder(MTGCardInstance * blocker){
  list<MTGCardInstance *>::iterator it1 = find(blockers.begin(), blockers.end(), blocker);
  list<MTGCardInstance *>::iterator it2 = blockers.begin();
  if (it2 != it1)
    {
      std::iter_swap(it1,it2);
      WEvent* e = NEW WEventCreatureBlockerRank(blocker,*it2,this);
      GameObserver::GetInstance()->receiveEvent(e);
      //delete(e);
    }
  return 1;
}

int MTGCardInstance::removeBlocker(MTGCardInstance * blocker){
  blockers.remove(blocker);
  if (!blockers.size()) blocked = false;
  return 1;
}

int MTGCardInstance::addBlocker(MTGCardInstance * blocker){
  blockers.push_back(blocker);
  blocked = true;
  return 1;
}

//Returns opponents to this card for this turn. This * should * take into account banding
MTGCardInstance * MTGCardInstance::getNextOpponent(MTGCardInstance * previous){
  GameObserver * game = GameObserver::GetInstance();
  int foundprevious = 0;
  if (!previous) foundprevious = 1;
  if (attacker && game->currentPlayer->game->inPlay->hasCard(this)){
    MTGInPlay * inPlay = game->opponent()->game->inPlay;
    for (int i = 0; i < inPlay->nb_cards; i ++){
      MTGCardInstance * current = inPlay->cards[i];
      if (current == previous){
	foundprevious = 1;
      }else if (foundprevious){
	MTGCardInstance * defensersOpponent = current->isDefenser();
	if (defensersOpponent && (defensersOpponent == this || (banding && defensersOpponent->banding == banding))){
	  return current;
	}
      }
    }
  }else if (defenser && game->opponent()->game->inPlay->hasCard(this)){
    MTGInPlay * inPlay = game->currentPlayer->game->inPlay;
    for (int i = 0; i < inPlay->nb_cards; i ++){
      MTGCardInstance * current = inPlay->cards[i];
      if (current == previous){
	foundprevious = 1;
      }else if (foundprevious){
	if (defenser == current || (current->banding && defenser->banding == current->banding)){
	  return current;
	}
      }
    }
  }
  return NULL;
}

int MTGCardInstance::setDefenser(MTGCardInstance * opponent){
  GameObserver * g = GameObserver::GetInstance();
  if (defenser) {
    if (g->players[0]->game->battlefield->hasCard(defenser) ||
      g->players[1]->game->battlefield->hasCard(defenser) ) {
      defenser->removeBlocker(this);
    }
  }
  WEvent * e = NULL;
  if (defenser != opponent) e = NEW WEventCreatureBlocker(this, defenser, opponent);
  defenser = opponent;
  if (defenser) defenser->addBlocker(this);
  if (e) g->receiveEvent(e);
  return 1;
}

int MTGCardInstance::toggleDefenser(MTGCardInstance * opponent){
  if (canBlock()){
    if (canBlock(opponent)){
      setDefenser(opponent);
      return 1;
    }
  }
  return 0;
}


int MTGCardInstance::addProtection(CardDescriptor * cd){
  protections[nbprotections] = cd;
  nbprotections++;
  return nbprotections;
}

int MTGCardInstance::removeProtection(CardDescriptor * cd, int erase){
  for (int i = 0; i < nbprotections ; i++){
    if (protections[i] == cd){
      if (erase) delete (protections[i]);
      protections[i] = protections[nbprotections -1];
      protections[nbprotections -1] = NULL;
      nbprotections--;
      return 1;
    }
  }
  return 0;
}

int MTGCardInstance::protectedAgainst(MTGCardInstance * card){
  //Basic protections
  for (int i=Constants::PROTECTIONGREEN; i <= Constants::PROTECTIONWHITE; i++){
    if (basicAbilities[i] && card->hasColor( i - Constants::PROTECTIONGREEN + Constants::MTG_COLOR_GREEN )) return 1;
  }

  //General protections
  for (int i = 0; i < nbprotections ; i++){
    if (protections[i]->match(card)) return 1;
  }
  return 0;
}

/* Choose a sound sample to associate to that card */
JSample * MTGCardInstance::getSample(){
  if (!sample.size()){
    for (int i = nb_types-1; i>0; i--){
      string type = Subtypes::subtypesList->find(types[i]);
      type = type + ".wav";
      if (fileExists(resources.sfxFile(type).c_str())){
        sample = string(type);
        break;
      }
    }
  }
  if (!sample.size()){
    for(map<int,int>::const_iterator it = basicAbilities.begin(); it != basicAbilities.end(); ++it){
      int i = it->first;
      if (!basicAbilities[i]) continue;
      string type = Constants::MTGBasicAbilities[i];
      type = type + ".wav";
      if (fileExists(resources.sfxFile(type).c_str())){
        sample = type;
        break;
      }
    }
  }
  if (!sample.size()){
    string type = Subtypes::subtypesList->find(types[0]);
    type = type + ".wav";
    if (fileExists(resources.sfxFile(type).c_str())){
      sample = type;
    }
  }

  if (sample.size()) return resources.RetrieveSample(sample);

  return NULL;
}

int MTGCardInstance::stepPower(CombatStep step)
{
  switch (step)
    {
    case FIRST_STRIKE :
    case END_FIRST_STRIKE :
      if (has(Constants::FIRSTSTRIKE) || has(Constants::DOUBLESTRIKE)) return MAX(0, power); else return 0;
    case DAMAGE :
    case END_DAMAGE :
    default :
      if (has(Constants::FIRSTSTRIKE)) return 0; else return MAX(0, power);
    }
}

std::ostream& MTGCardInstance::toString(std::ostream& out) const
{
  return out << name;
}

std::ostream& operator<<(std::ostream& out, const MTGCardInstance& c)
{
  return c.toString(out);
}
