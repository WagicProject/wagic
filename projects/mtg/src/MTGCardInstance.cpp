/*---------------------------------------------
Card Instance
Instance of a given MTGCard in the game
Although there is only one MTGCard of each type, there can be as much Instances of it as needed in the game
--------------------------------------------
*/
#include "../include/debug.h"
#include "../include/MTGCardInstance.h"
#include "../include/CardDescriptor.h"
#include "../include/Counters.h"

MTGCardInstance::MTGCardInstance(): MTGCard(), Damageable(0){
	LOG("==Creating MTGCardInstance==");
initMTGCI();
LOG("==Creating MTGCardInstance Successful==");
}
MTGCardInstance::MTGCardInstance(MTGCard * card, MTGPlayerCards * _belongs_to): MTGCard(card), Damageable(card->getToughness()){
	LOG("==Creating MTGCardInstance==");
	initMTGCI();
	model = card;
	lifeOrig = life;
  belongs_to=_belongs_to;
	initAttackersDefensers();
	life=toughness;
	LOG("==Creating MTGCardInstance Successful==");

}

MTGCardInstance::~MTGCardInstance(){
	LOG("==Deleting MTGCardInstance==");
	SAFE_DELETE(blockers);
	SAFE_DELETE(counters);
	LOG("==Deleting MTGCardInstance Succesfull==");
}
void MTGCardInstance::initMTGCI(){
	model=NULL;
	lifeOrig = 0;
	doDamageTest = 0;
  belongs_to=NULL;
  tapped = 0;
	blockers = NEW Blockers();
	untapping = 0;
	summoningSickness = 0;
	target = NULL;
	nbprotections = 0;
	type_as_damageable = DAMAGEABLE_MTGCARDINSTANCE;
	banding = NULL;
	owner = NULL;
	changedZoneRecently = 0;
	counters = NEW Counters(this);
}


void MTGCardInstance::addType(int type){
	  types[nb_types] = type;
		nb_types++;
}

Blockers * MTGCardInstance::getBlockers(){
	return blockers;
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
		#if defined (WIN32) || defined (LINUX)
char    buf[4096], *p = buf;
sprintf(buf,"After Damage Test, life is %i for %s \n",life,model->getName());
OutputDebugString(buf);
#endif
	doDamageTest = 0;
	if (life <=0 && isInPlay()){
		GameObserver * game = GameObserver::GetInstance();
		game->mLayers->stackLayer()->addPutInGraveyard(this);
		return 1;
	}
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

JQuad * MTGCardInstance::getIcon(){
	return getThumb();
}

int MTGCardInstance::has(int basicAbility){
	return basicAbilities[basicAbility];
}





//Taps the card
void MTGCardInstance::tap(){
  tapped = 1;
}

void MTGCardInstance::setUntapping(){
	untapping = 1;
}

int MTGCardInstance::isUntapping(){
	return untapping;
}

//Untaps the card
void MTGCardInstance::untap(){
	if (untapping){
		tapped = 0;
		untapping = 0;
	}
}

//Tells if the card is tapped or not
int MTGCardInstance::isTapped(){
	return tapped;
}

void MTGCardInstance::resetAllDamage(){
  //for (int i=0;i<nb_damages;i++){
    //   delete damages[i];
  //}
  nb_damages = 0;
}

void MTGCardInstance::regenerate(){
	tapped = 1;
	life = toughness;
	initAttackersDefensers();
}


int MTGCardInstance::initAttackersDefensers(){
	attacker = 0;
	defenser = NULL;
	banding = NULL;
	return 1;
}

//Function to call to remove all damages, etc to a card (generally at the end of the turn)
int MTGCardInstance::cleanup(){
	initAttackersDefensers();
	life=toughness;
	GameObserver * game = GameObserver::GetInstance();
	if (!game || game->currentPlayer == controller()) summoningSickness = 0;
	return 1;
}

/* Summoning Sickness
 * 212.3f A creaturefs activated ability with the tap symbol or the untap symbol in its activation cost
 * canft be played unless the creature has been under its controllerfs control since the start of his or
 * her most recent turn. A creature canft attack unless it has been under its controllerfs control
 * since the start of his or her most recent turn. This rule is informally called the gsummoning
 * sicknessh rule. Ignore this rule for creatures with haste (see rule 502.5).
 */
int MTGCardInstance::hasSummoningSickness(){
	if (!summoningSickness) return 0;
	if (basicAbilities[HASTE]) return 0;
	if (!isACreature()) return 0;
	return 1;
}

int MTGCardInstance::changeController(Player * newController){
	Player * originalOwner = controller();
	if (originalOwner  == newController) return 0;
	originalOwner->game->inPlay->removeCard(this);
	newController->game->inPlay->addCard(this);
	summoningSickness = 1;
	return 1;
}

//Reset the card parameters
int MTGCardInstance::reset(){
	cleanup();
	tapped=0;
	SAFE_DELETE(counters);
	counters = NEW Counters(this);
	return 1;
}

Player * MTGCardInstance::controller(){
	GameObserver * game = GameObserver::GetInstance();
	if (!game) return NULL;
	for (int i = 0; i < 2; i++){
		if (game->players[i]->game->inPlay->hasCard(this)) return game->players[i];
		if (game->players[i]->game->stack->hasCard(this)) return game->players[i];
		if (game->players[i]->game->graveyard->hasCard(this)) return game->players[i];
	}
	return NULL;
}

int MTGCardInstance::canAttack(){
	if (!hasSummoningSickness() && !tapped && isACreature() && basicAbilities[DEFENSER] !=1)
		return 1;
	return 0;
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
	if (!tapped && isACreature())return 1;
	if (!basicAbilities[SHADOW]&& isACreature()) return 1; // Try to add shadow
	return 0;
}


int MTGCardInstance::canBlock(MTGCardInstance * opponent){
	if (!canBlock()) return 0;
	if (!opponent) return 1;
	if (!opponent->isAttacker()) return 0;
	
	// Comprehensive rule 502.7f : If a creature with protection attacks, it can't be blocked by creatures that have the stated quality.
	if (opponent->protectedAgainst(this)) return 0; 

	if (opponent->basicAbilities[UNBLOCKABLE]) return 0;
	if (opponent->basicAbilities[FEAR] && !(hasColor(MTG_COLOR_ARTIFACT) || hasColor(MTG_COLOR_BLACK))) return 0;
	if (opponent->basicAbilities[FLYING] && !( basicAbilities[FLYING] || basicAbilities[REACH])) return 0;
	if (opponent->basicAbilities[SHADOW] && !( basicAbilities[SHADOW] || basicAbilities[REACHSHADOW])) return 0;
	if (opponent->basicAbilities[SWAMPWALK] && controller()->game->inPlay->hasType("swamp")) return 0;
	if (opponent->basicAbilities[FORESTWALK] && controller()->game->inPlay->hasType("forest")) return 0;
	if (opponent->basicAbilities[ISLANDWALK] && controller()->game->inPlay->hasType("island")) return 0;
	if (opponent->basicAbilities[MOUNTAINWALK] && controller()->game->inPlay->hasType("mountain")) return 0;
	if (opponent->basicAbilities[PLAINSWALK] && controller()->game->inPlay->hasType("plains")) return 0;
	return 1;
}


MTGCardInstance * MTGCardInstance::getNextPartner(){
	MTGInPlay * inplay = controller()->game->inPlay;
	MTGCardInstance * bandingPartner = inplay->getNextAttacker(banding);
	while (bandingPartner){
		if (basicAbilities[BANDING] || bandingPartner->basicAbilities[BANDING]) return bandingPartner;
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

int MTGCardInstance::toggleAttacker(){
	//TODO more controls ?
	if (canAttack()){
		if (!attacker){
			attacker = 1;
			tapped = 1;
			return 1;
		}else{
			MTGCardInstance * bandingPartner = getNextPartner();
			if (bandingPartner){
				if (banding) unband();
				if (!bandingPartner->banding) bandingPartner->banding = bandingPartner;
				banding = bandingPartner->banding;
				return 1;
			}else{
				attacker = 0;
				tapped = 0;
				return 1;
			}
		}
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

int MTGCardInstance::toggleDefenser(MTGCardInstance * opponent){
	if (canBlock()){
		if (canBlock(opponent)){
			defenser = opponent;
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
	for (int i = 0; i < nbprotections ; i++){
		if (protections[i]->match(card)) return 1;
	}
	return 0;
}
