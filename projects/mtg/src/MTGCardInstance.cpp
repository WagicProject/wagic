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

MTGCardInstance MTGCardInstance::AnyCard = MTGCardInstance();
MTGCardInstance MTGCardInstance::NoCard = MTGCardInstance();

MTGCardInstance MTGCardInstance::ExtraRules[] = {MTGCardInstance(), MTGCardInstance()};



MTGCardInstance::MTGCardInstance(): CardPrimitive(), MTGCard(), Damageable(0), view(NULL){
  initMTGCI();
}
MTGCardInstance::MTGCardInstance(MTGCard * card, MTGPlayerCards * arg_belongs_to): CardPrimitive(card->data),MTGCard(card), Damageable(card->data->getToughness()), view(NULL){
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
  preventable = 0;
}

void MTGCardInstance::copy(MTGCardInstance * card){
 MTGCard * source = card->model;
 CardPrimitive * data = source->data;
 for(map<int,int>::const_iterator it = data->basicAbilities.begin(); it != data->basicAbilities.end(); ++it){
   int i = it->first;
   basicAbilities[i] = data->basicAbilities[i];
 }
  for (size_t i = 0; i< data->types.size(); i++){
    types.push_back(data->types[i]);
  }
  for (int i = 0; i< Constants::MTG_NB_COLORS; i++){
    colors[i] = data->colors[i];
  }
  manaCost.copy(data->getManaCost());

  text = data->text;
  setName(data->name);

  power = data->power;
  toughness = data->toughness;
  life = toughness;
  lifeOrig = life;

  magicText = data->magicText;
  spellTargetType = data->spellTargetType;
  alias = data->alias;

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
  SAFE_DELETE(counters);
  SAFE_DELETE(previous);
}

int MTGCardInstance::init(){
  MTGCard::init();
  CardPrimitive::init();
  data = this;
  X = 0;
  XX = 0;
  return 1;
}

void MTGCardInstance::initMTGCI(){
  sample = "";
  model=NULL;
  isToken = false;
  lifeOrig = 0;
  doDamageTest = 1;
  belongs_to=NULL;
  tapped = 0;
  untapping = 0;
  frozen = 0;
  boughtback = 0;
  flashedback = 0;
  paymenttype = 0;
  summoningSickness = 1;
  preventable = 0;
  target = NULL;
  type_as_damageable = DAMAGEABLE_MTGCARDINSTANCE;
  banding = NULL;
  owner = NULL;
  counters = NEW Counters(this);
  previousZone = NULL;
  previous = NULL;
  next = NULL;
  lastController = NULL;
  regenerateTokens = 0;
  blocked = false;
  currentZone = NULL;
  data = this; //an MTGCardInstance point to itself for data, allows to update it without killing the underlying database item
}


const string MTGCardInstance::getDisplayName() const {
  return getName();
}

void MTGCardInstance::addType(int type){
  bool before = hasType(type);
  CardPrimitive::addType(type);
  WEvent * e = NEW WEventCardChangeType(this,type,before,true);
  GameObserver * go = GameObserver::GetInstance();
  if(go) go->receiveEvent(e);
  else SAFE_DELETE(e);
}

void MTGCardInstance::addType(char * type_text){
  setSubtype(type_text);
}

void MTGCardInstance::setType(const char * type_text){
  setSubtype(type_text);
}

void MTGCardInstance::setSubtype(string value){
  int id = Subtypes::subtypesList->find(value);
  addType(id);
}
int MTGCardInstance::removeType(string value,int removeAll){
  int id = Subtypes::subtypesList->find(value);
  return removeType(id,removeAll);
}

int MTGCardInstance::removeType(int id, int removeAll){
  bool before = hasType(id);
  int result = CardPrimitive::removeType(id,removeAll);
  bool after = hasType(id);
  WEvent * e = NEW WEventCardChangeType(this,id,before,after);
  GameObserver * go = GameObserver::GetInstance();
  if(go) go->receiveEvent(e);
  else SAFE_DELETE(e);
  return result;
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
  return currentZone;
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

int MTGCardInstance::regenerate(){
  if (has(Constants::CANTREGEN)) return 0;
  return ++regenerateTokens;
}

int MTGCardInstance::triggerRegenerate(){
  if (! regenerateTokens) return 0;
  if (has(Constants::CANTREGEN)) return 0;
  regenerateTokens--;
  tap();
  life = toughness;
  initAttackersDefensers();
  if (life < 1) return 0;  //regeneration didn't work (wither ?)
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
  preventable = 0;
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
  if (originalOwner  == newController) return this;
  MTGCardInstance * copy = originalOwner->game->putInZone(this, originalOwner->game->inPlay, newController->game->inPlay);
  copy->summoningSickness = 1;
  return copy;
}

Player * MTGCardInstance::controller(){
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
  if (opponent->cantBeBlockedBy(this)) return 0;
  if (opponent->basicAbilities[Constants::UNBLOCKABLE]) return 0;
  if (opponent->basicAbilities[Constants::ONEBLOCKER] && opponent->blocked) return 0;
  if (opponent->basicAbilities[Constants::FEAR] && !(hasType(Subtypes::TYPE_ARTIFACT) || hasColor(Constants::MTG_COLOR_BLACK))) return 0;

  //intimidate
  if (opponent->basicAbilities[Constants::INTIMIDATE] && !(hasType(Subtypes::TYPE_ARTIFACT))){
    int canblock = 0;
    for (int i = Constants::MTG_COLOR_GREEN; i <= Constants::MTG_COLOR_WHITE; ++i){
      if(hasColor(i) && opponent->hasColor(i)){
        canblock = 1;
        break;
      }
    }
    if (!canblock) return 0;
  }

  if (opponent->basicAbilities[Constants::FLYING] && !( basicAbilities[Constants::FLYING] || basicAbilities[Constants::REACH])) return 0;
  //Can block only creatures with flying if has cloud
  if (basicAbilities[Constants::CLOUD] && !( opponent->basicAbilities[Constants::FLYING])) return 0;
  // If opponent has shadow and a creature does not have either shadow or reachshadow it cannot be blocked
  if (opponent->basicAbilities[Constants::SHADOW] && !( basicAbilities[Constants::SHADOW] || basicAbilities[Constants::REACHSHADOW])) return 0;
  // If opponent does not have shadow and a creature has shadow it cannot be blocked
  if (!opponent->basicAbilities[Constants::SHADOW] && basicAbilities[Constants::SHADOW]) return 0;
  if (opponent->basicAbilities[Constants::HORSEMANSHIP] && !basicAbilities[Constants::HORSEMANSHIP]) return 0;
  if (opponent->basicAbilities[Constants::SWAMPWALK] && controller()->game->inPlay->hasType("swamp")) return 0;
  if (opponent->basicAbilities[Constants::FORESTWALK] && controller()->game->inPlay->hasType("forest")) return 0;
  if (opponent->basicAbilities[Constants::ISLANDWALK] && controller()->game->inPlay->hasType("island")) return 0;
  if (opponent->basicAbilities[Constants::MOUNTAINWALK] && controller()->game->inPlay->hasType("mountain")) return 0;
  if (opponent->basicAbilities[Constants::PLAINSWALK] && controller()->game->inPlay->hasType("plains")) return 0;
  return 1;
}

JQuad * MTGCardInstance::getIcon(){
 return resources.RetrieveCard(this,CACHE_THUMB);
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

int MTGCardInstance::setAttacker(int value){
  Targetable * previousTarget = NULL;
  Targetable * target = NULL;
  Player * p  = controller()->opponent();
  if (value) target = p;
  if (attacker) previousTarget = p;
  attacker = value;
  WEvent * e = NEW WEventCreatureAttacker(this,previousTarget, target);
  GameObserver * go = GameObserver::GetInstance();
  if(go) go->receiveEvent(e);
  else SAFE_DELETE(e);
  return 1;
}

int MTGCardInstance::toggleAttacker(){
  if (!attacker){
    if (!basicAbilities[Constants::VIGILANCE]) tap();
    setAttacker(1);
    return 1;
  }else{
      untap();
      setAttacker(0);
      return 1;
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

int MTGCardInstance::raiseBlockerRankOrder(MTGCardInstance * blocker){
  list<MTGCardInstance *>::iterator it1 = find(blockers.begin(), blockers.end(), blocker);
  list<MTGCardInstance *>::iterator it2 = it1;
  if (blockers.begin() == it2) ++it2; else --it2;

  std::iter_swap(it1,it2);
  WEvent* e = NEW WEventCreatureBlockerRank(*it1,*it2,this);
  GameObserver * go = GameObserver::GetInstance();
  if(go) go->receiveEvent(e);
  else SAFE_DELETE(e);
  //delete(e);
  return 1;
}

int MTGCardInstance::getDefenserRank(MTGCardInstance * blocker){
  int result = 0;
  for(list<MTGCardInstance *>::iterator it1 = blockers.begin(); it1 != blockers.end(); ++it1){
    result++;
    if ((*it1) == blocker) return result;
  }
  return 0;
};

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
  if (attacker){
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
  }else if (defenser){
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


int MTGCardInstance::addProtection(TargetChooser * tc){
  tc->targetter = NULL;
  protections.push_back(tc);
  return protections.size();
}

int MTGCardInstance::removeProtection(TargetChooser * tc, int erase){
  for (size_t i = 0; i < protections.size() ; i++){
    if (protections[i] == tc){
      if (erase) delete (protections[i]);
      protections.erase(protections.begin()+i);
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
  for (size_t i = 0; i < protections.size() ; i++){
    if (protections[i]->canTarget(card)) 
      return 1;
  }
  return 0;
}


int MTGCardInstance::addCantBeBlockedBy(TargetChooser * tc){
  cantBeBlockedBys.push_back(tc);
  return cantBeBlockedBys.size();
}

int MTGCardInstance::removeCantBeBlockedBy(TargetChooser * tc, int erase){
  for (size_t i = 0; i < cantBeBlockedBys.size() ; i++){
    if (cantBeBlockedBys[i] == tc){
      if (erase) delete (cantBeBlockedBys[i]);
      cantBeBlockedBys.erase(cantBeBlockedBys.begin()+i);
      return 1;
    }
  }
  return 0;
}

int MTGCardInstance::cantBeBlockedBy(MTGCardInstance * card){
  for (size_t i = 0; i < cantBeBlockedBys.size() ; i++){
    if (cantBeBlockedBys[i]->canTarget(card)) 
      return 1;
  }
  return 0;
}



/* Choose a sound sample to associate to that card */
JSample * MTGCardInstance::getSample(){
  JSample * js;

  if(sample.size())
    return resources.RetrieveSample(sample);

  for (int i = types.size()-1; i>0; i--){
    string type = Subtypes::subtypesList->find(types[i]);
    type = type + ".wav";
    js = resources.RetrieveSample(type);
    if (js){
      sample = string(type);
      return js;
    }
  }

  for(map<int,int>::const_iterator it = basicAbilities.begin(); it != basicAbilities.end(); ++it){
    int i = it->first;
    if (!basicAbilities[i]) continue;
    string type = Constants::MTGBasicAbilities[i];
    type = type + ".wav";
    js = resources.RetrieveSample(type);
    if (js){
      sample = string(type);
      return js;
    }
  }

  string type = Subtypes::subtypesList->find(types[0]);
  type = type + ".wav";
  js = resources.RetrieveSample(type);
  if (js){
    sample = string(type);
    return js;
  }

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
      if (has(Constants::FIRSTSTRIKE) && !has(Constants::DOUBLESTRIKE)) return 0; else return MAX(0, power);
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
