#ifndef _CARDS_H_
#define _CARDS_H_

#include "MTGAbility.h"
#include "GroupOfCards.h"
#include "ManaCost.h"
#include "CardDescriptor.h"
#include "AIPlayer.h"
#include "CardDisplay.h"
#include "Subtypes.h"
#include "CardGui.h"

#include <JGui.h>
#include <hge/hgeparticle.h>


#include <map>
using std::map;

/*
Generic classes
*/


//Drawer, allows to draw a card for a cost:

class ADrawer:public ActivatedAbility{
public:
	int nbcards;
	ADrawer(int _id, MTGCardInstance * card,ManaCost * _cost, int _nbcards = 1, int _tap = 1):ActivatedAbility(_id, card,_cost,0,_tap),nbcards(_nbcards){
	}

	int resolve(){
		game->mLayers->stackLayer()->addDraw(source->controller(),nbcards);
		return 1;
	}
};


//Destroyer. TargetAbility
class ADestroyer:public TargetAbility{
public:
	int bury;
	ADestroyer(int _id, MTGCardInstance * _source, TargetChooser * _tc = NULL, int _bury = 0):TargetAbility(_id,_source, tc),bury(_bury){
		if (!tc) tc = NEW CreatureTargetChooser();
	}

	int resolve(){
		MTGCardInstance * _target = tc->getNextCardTarget();  
		if(_target){
			if (bury){
				_target->controller()->game->putInGraveyard(_target);
			}else{
				game->mLayers->stackLayer()->addPutInGraveyard(_target);
			}
			return 1;
		}
		return 0;
	}
	
};

//Destroyer. TargetAbility
class ABurier:public ADestroyer{
public:
	ABurier(int _id, MTGCardInstance * _source, TargetChooser * _tc = NULL):ADestroyer(_id,_source, tc,1){
	}	
};


/*Changes one of the basic abilities of target
source : spell
target : spell target (creature)
modifier : 1 to add the ability, 0 to remove it
_ability : Id of the ability, as described in mtgdefinitions
*/
class ABasicAbilityModifier:public MTGAbility{
public:
	int modifier;
	int ability;
	int value_before_modification;
	ABasicAbilityModifier(int _id, MTGCardInstance * _source, MTGCardInstance * _target, int _ability, int _modifier = 1): MTGAbility(_id,_source,_target),modifier(_modifier),ability(_ability){
		value_before_modification = ((MTGCardInstance * )target)->basicAbilities[ability];
		((MTGCardInstance * )target)->basicAbilities[ability]=modifier;
	}

	int destroy(){
		if (((MTGCardInstance * )target)->basicAbilities[ability] == modifier){
			((MTGCardInstance * )target)->basicAbilities[ability] = value_before_modification;
			return 1;
		}else{
			//BUG !!!
			return 0;
		}
	}
};

//Modifies an	ability until end of turn. Needs a target
class ABasicAbilityModifierUntilEOT:public TargetAbility{
	public:
	MTGCardInstance * mTargets[50];
	int nbTargets;
	int modifier;
	int stateBeforeActivation[50];
	int ability;
	ABasicAbilityModifierUntilEOT(int _id, MTGCardInstance * _source, int _ability, ManaCost * _cost, TargetChooser * _tc = NULL, int _modifier = 1): TargetAbility(_id,_source,_cost),ability(_ability), modifier(_modifier){
		nbTargets = 0;
		tc = _tc;
		if (!tc) tc = NEW CreatureTargetChooser(_source);
	}

	void Update(float dt){
		if (newPhase != currentPhase && newPhase == MTG_PHASE_UNTAP){
			for (int i = 0; i < nbTargets; i++){
				MTGCardInstance * mTarget = mTargets[i];
				if(mTarget && mTarget->basicAbilities[ability]){
					mTarget->basicAbilities[ability] = stateBeforeActivation[i];
				}
			}
			nbTargets = 0;
		}
		TargetAbility::Update(dt);
	}


	int resolve(){
		MTGCardInstance * mTarget = tc->getNextCardTarget();
		if (mTarget){
			mTargets[nbTargets] = mTarget;
			stateBeforeActivation[nbTargets] = mTarget->basicAbilities[ability];
			mTarget->basicAbilities[ability] = modifier;
			nbTargets++;
		}
		return 1;
	}
	

};

/*Instants that modifies a basic ability until end of turn */
class  AInstantBasicAbilityModifierUntilEOT: public InstantAbility{
public:
	int stateBeforeActivation;
	int ability;
	AInstantBasicAbilityModifierUntilEOT(int _id, MTGCardInstance * _source, MTGCardInstance * _target, int _ability, int value):InstantAbility(_id, _source, _target),ability(_ability){
		stateBeforeActivation = _target->basicAbilities[ability];
		_target->basicAbilities[ability] = value;	
	}

	int destroy(){
		((MTGCardInstance *)target)->basicAbilities[ability] = stateBeforeActivation;
		return 1;
	}

};

//Alteration of Ability until of turn (Aura)
class ABasicAbilityAuraModifierUntilEOT: public ActivatedAbility{
public:
	int stateBeforeActivation;
	int ability;
	int value;
	ABasicAbilityAuraModifierUntilEOT(int _id, MTGCardInstance * _source, MTGCardInstance * _target, ManaCost * _cost, int _ability, int _value = 1):ActivatedAbility(_id,_source, _cost, 0,0), ability(_ability), value(_value){
		target = _target;
		stateBeforeActivation = _target->basicAbilities[ability];
	}
	
	void Update(float dt){
		if (newPhase != currentPhase && newPhase == MTG_PHASE_UNTAP){
			MTGCardInstance * _target = (MTGCardInstance *) target;
			_target->basicAbilities[ability] = stateBeforeActivation;	
		}
		ActivatedAbility::Update(dt);
	}

	int resolve(){
		MTGCardInstance * _target = (MTGCardInstance *) target;
		stateBeforeActivation = _target->basicAbilities[ability];
		_target->basicAbilities[ability] = value;	
		return 1;
	}

};



/*Gives life each time a spell matching CardDescriptor's criteria are match . Optionnal manacost*/
class ASpellCastLife:public MTGAbility{
public:
	CardDescriptor trigger;
	ManaCost * cost;
	int life;
	MTGCardInstance * lastUsedOn;
		MTGCardInstance * lastChecked;
	ASpellCastLife(int id, MTGCardInstance * _source, CardDescriptor  _trigger, ManaCost * _cost, int _life): MTGAbility(id, _source), trigger(_trigger), cost(_cost), life(_life){
	}
	ASpellCastLife(int id, MTGCardInstance * _source, int color, ManaCost * _cost, int _life): MTGAbility(id, _source), cost(_cost), life(_life){
		trigger.setColor(color);
	}

	int isReactingToClick(MTGCardInstance *  _card){
		if (_card == source && game->currentlyActing()->game->inPlay->hasCard(source)){
			if (game->currentlyActing()->getManaPool()->canAfford(cost)){
				Interruptible * laststackitem = game->mLayers->stackLayer()->_(-1);
				if (laststackitem && laststackitem->type == ACTION_SPELL){
					Spell * spell = (Spell*)laststackitem;
					if (spell->source != lastUsedOn && trigger.match(spell->source)){
						lastChecked = spell->source;
						return 1;
					}
				}
			}
		}
		return 0;
	}

	int reactToClick(MTGCardInstance *  _card){
		if (!isReactingToClick( _card)) return 0;
		game->currentlyActing()->getManaPool()->pay(cost);
		game->currentlyActing()->life+=life;
		lastUsedOn = lastChecked;
		return 1;
	}

};

//Allows to untap at any moment for an amount of mana
class AUnBlocker:public MTGAbility{
public:
	ManaCost * cost;
	AUnBlocker(int id, MTGCardInstance * _source, MTGCardInstance * _target, ManaCost * _cost):MTGAbility(id, _source, _target), cost(_cost){
	}


	int isReactingToClick(MTGCardInstance *  _card){
		if (_card == target && game->currentlyActing()->game->inPlay->hasCard(source) && (MTGCardInstance *) _card->isTapped()){
			if (game->currentlyActing()->getManaPool()->canAfford(cost)){
				return 1;
			}
		}
		return 0;
	}

	int reactToClick(MTGCardInstance *  _card){
		if (!isReactingToClick( _card)) return 0;
		game->currentlyActing()->getManaPool()->pay(cost);
		_card->untap();
		return 1;
	}
};

//Allows to untap target card once per turn for a manaCost
class AUntaperOnceDuringTurn:public AUnBlocker{
public:
	int untappedThisTurn;
	int onlyPlayerTurn;
	AUntaperOnceDuringTurn(int id, MTGCardInstance * _source, MTGCardInstance * _target, ManaCost * _cost, int _onlyPlayerTurn = 1):AUnBlocker(id, _source, _target, _cost){
		onlyPlayerTurn = _onlyPlayerTurn;
		untappedThisTurn = 0;
	}

	void Update(float dt){
		if (newPhase != currentPhase && newPhase == MTG_PHASE_UNTAP) untappedThisTurn = 0;
		AUnBlocker::Update(dt);
	}

	int isReactingToClick(MTGCardInstance * card){
		if (onlyPlayerTurn && game->currentPlayer!=source->controller()) return 0;
		if (untappedThisTurn) return 0;
		return AUnBlocker::isReactingToClick(card);
	}

	int reactToClick(MTGCardInstance * card){
		untappedThisTurn = 1;
		return AUnBlocker::reactToClick(card);
	}
};

//Alteration of Power and Toughness  (enchantments)
class APowerToughnessModifier: public MTGAbility{
public:
		int power, toughness;
	  APowerToughnessModifier(int id, MTGCardInstance * _source, MTGCardInstance * _target, int _power, int _toughness):MTGAbility(id,_source,_target),power(_power),toughness(_toughness){
				_target->power += power;
				_target->addToToughness(toughness);
		}

	int destroy(){
		((MTGCardInstance *)target)->power -= power;
		((MTGCardInstance *)target)->addToToughness(-toughness);
		return 1;
	}
};

// Permanent life alteration evry turn of the target's controller. Useful only for unstable mutation currently
class APowerToughnessModifierRegularCounter:public MTGAbility{
public:
	int power, toughness;
	int phase;
	APowerToughnessModifierRegularCounter(int id, MTGCardInstance * _source, MTGCardInstance * _target, int _phase, int _power, int _toughness):MTGAbility(id,_source,_target),power(_power),toughness(_toughness), phase(_phase){
	}

	void Update(float dt){
		if (newPhase !=currentPhase && newPhase==phase && game->currentPlayer==((MTGCardInstance *)target)->controller()){
			((MTGCardInstance *)target)->power += power;
			((MTGCardInstance *)target)->addToToughness(toughness);
		}
	}

};


//Alteration of Power and Toughness until end of turn (TargetAbility)
// Gives +n/+m until end of turn to any card that's a target
class ATargetterPowerToughnessModifierUntilEOT: public TargetAbility{
public:
		MTGCardInstance * mTargets[50];
		int nbTargets;
		int power, toughness;

	  ATargetterPowerToughnessModifierUntilEOT(int _id, MTGCardInstance * _source, int _power, int _toughness,  ManaCost * _cost, TargetChooser * _tc = NULL):TargetAbility(_id,_source,_tc,_cost,0),power(_power),toughness(_toughness){
			if (!tc) tc = NEW CreatureTargetChooser(_source);
			nbTargets = 0;
		}


	void Update(float dt){
		if (newPhase != currentPhase && newPhase == MTG_PHASE_UNTAP){
			for (int i = 0; i < nbTargets; i++){
				MTGCardInstance * mTarget = mTargets[i];
				if(mTarget){
					mTarget->power-=power;
					mTarget->addToToughness(-toughness);
				}
			}
			nbTargets = 0;
		}
		TargetAbility::Update(dt);
	}


	int resolve(){
		MTGCardInstance * mTarget = tc->getNextCardTarget();
		if (mTarget){
			mTargets[nbTargets] = mTarget;
			mTarget->power+= power;
			mTarget->addToToughness(toughness);
			nbTargets++;
		}
		return 1;
	}

};



//Alteration of Power and Toughness until end of turn (Aura)
class APowerToughnessModifierUntilEndOfTurn: public MTGAbility{
public:
		int power, toughness;
		int counters;
		int maxcounters;
		ManaCost * cost;
	  APowerToughnessModifierUntilEndOfTurn(int id, MTGCardInstance * _source, MTGCardInstance * _target, int _power, int _toughness,  ManaCost * _cost, int _maxcounters = 0):MTGAbility(id,_source,_target),power(_power),toughness(_toughness),maxcounters(_maxcounters), cost(_cost){
			counters = 0;
		}

	void Update(float dt){
		if (newPhase != currentPhase && newPhase == MTG_PHASE_UNTAP){
			while(counters){
				((MTGCardInstance *)target)->power -= power;
				((MTGCardInstance *)target)->addToToughness(-toughness);
				counters--;
			}
		}
	}

	int isReactingToClick(MTGCardInstance *  _card){
		if (_card == source && (!maxcounters || counters < maxcounters) && game->currentlyActing()->game->inPlay->hasCard(source)){
			if (game->currentlyActing()->getManaPool()->canAfford(cost)){
				return 1;
			}
		}
		return 0;
	}

	int reactToClick(MTGCardInstance *  _card){
		if (!isReactingToClick( _card)) return 0;
		game->currentlyActing()->getManaPool()->pay(cost);
		((MTGCardInstance *)target)->power += power;
		((MTGCardInstance *)target)->addToToughness(toughness);
		counters++;
		return 1;
	}
};


//Alteration of Power and toughness until end of turn (instant)
class  AInstantPowerToughnessModifierUntilEOT: public InstantAbility{
public:
	int power, toughness;
	AInstantPowerToughnessModifierUntilEOT(int _id, MTGCardInstance * _source, MTGCardInstance * _target, int _power, int _toughness): InstantAbility(_id, _source, _target), power(_power), toughness(_toughness){
	}

	int resolve(){
		((MTGCardInstance *)target)->power +=power;
		((MTGCardInstance *)target)->addToToughness(toughness);
		return 1;
	}

	int destroy(){
		((MTGCardInstance *)target)->power -=power;
		((MTGCardInstance *)target)->addToToughness(-toughness);
		return 1;
	}

};
//Untap Blockers with simple Mana Mechanism
class AUntapManaBlocker: public Blocker{
public:
	AUntapManaBlocker(int id, MTGCardInstance * card, ManaCost * _cost):Blocker(id, card, _cost){
	}

	AUntapManaBlocker(int id, MTGCardInstance * card, MTGCardInstance * _target, ManaCost * _cost):Blocker(id, card,_target, _cost){
	}
};

/* Spell Counters (Enchantment) for a mana cost */
//LifeForce
class ASpellCounterEnchantment:public TargetAbility{
public:

	ASpellCounterEnchantment(int _id, MTGCardInstance * _source, ManaCost * _cost,int color = -1, int _tap = 0):TargetAbility(_id,_source,NEW SpellTargetChooser(_source,color),_cost,0,_tap){
	}

	int resolve(){
		Spell * _target = tc->getNextSpellTarget();  
		if(_target){
			game->mLayers->stackLayer()->Fizzle(_target);
			return 1;
		}
		return 0;
	}
	
};

/*Mana Producers (lands)
//These have a reactToClick function, and therefore two manaProducers on the same card conflict with each other
//That means the player has to choose one. although that is perfect for cards such as birds of paradise or badlands, 
other solutions need to be provided for abilities that add mana (ex: mana flare)
*/
class AManaProducer: public MTGAbility{
protected:
	ManaCost * cost;
	ManaCost * output;
	string menutext;
	float x0,y0,x1,y1,x,y;
	float animation;
	Player * controller;

	hgeParticleSystem * mParticleSys;
public:
	AManaProducer(int id, MTGCardInstance * card, ManaCost * _output, ManaCost * _cost = NULL ):MTGAbility(id, card){
		LOG("==Creating ManaProducer Object");
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

		if (landColor == MTG_COLOR_RED){
			mParticleSys = NEW hgeParticleSystem("graphics/manared.psi",GameApp::CommonRes->GetQuad("particles"));
		}else if (landColor == MTG_COLOR_BLUE){
			mParticleSys = NEW hgeParticleSystem("graphics/manablue.psi", GameApp::CommonRes->GetQuad("particles"));
		}else if (landColor == MTG_COLOR_GREEN){
			mParticleSys = NEW hgeParticleSystem("graphics/managreen.psi", GameApp::CommonRes->GetQuad("particles"));
		}else if (landColor == MTG_COLOR_BLACK){
			mParticleSys = NEW hgeParticleSystem("graphics/manablack.psi", GameApp::CommonRes->GetQuad("particles"));
		}else if (landColor == MTG_COLOR_WHITE){
			mParticleSys = NEW hgeParticleSystem("graphics/manawhite.psi", GameApp::CommonRes->GetQuad("particles"));
		}else{
			mParticleSys = NEW hgeParticleSystem("graphics/mana.psi", GameApp::CommonRes->GetQuad("particles"));
		}

	

		LOG("==ManaProducer Object Creation successful !");
	}

	void Update(float dt){
		if (mParticleSys) mParticleSys->Update(dt);
		if (animation){
			x = (1.f - animation)*x1 + animation * x0;
			y = (1.f - animation)*y1 + animation * y0;
			if (mParticleSys) mParticleSys->MoveTo(x, y);
			if (mParticleSys && animation == 1.f) mParticleSys->Fire();
			animation -= 4 *dt;
			if (animation < 0){
				animation = 0;
				controller->getManaPool()->add(output);
				if (mParticleSys) mParticleSys->Stop();
			}
		}

	}

	void Render(){
		JRenderer * renderer = JRenderer::GetInstance();
		if (animation){
				renderer->SetTexBlend(BLEND_SRC_ALPHA, BLEND_ONE);
				if (mParticleSys) mParticleSys->Render();
				// set normal blending
				renderer->SetTexBlend(BLEND_SRC_ALPHA, BLEND_ONE_MINUS_SRC_ALPHA);
		}

	}

	int isReactingToClick(MTGCardInstance *  _card){
		int result = 0;
		if (_card == source && !source->isTapped()  && game->currentlyActing()->game->inPlay->hasCard(source) && (source->hasType("land") || !source->hasSummoningSickness()) ){
			if (!cost || game->currentlyActing()->getManaPool()->canAfford(cost)) result =  1;
		}
		return result;
	}

	int reactToClick(MTGCardInstance *  _card){
		if (!isReactingToClick( _card)) return 0;
		source->tapped = 1;
		if (cost) GameObserver::GetInstance()->currentlyActing()->getManaPool()->pay(cost);
		animation = 1.f;
		CardGui * cardg = game->mLayers->playLayer()->getByCard(source);
		if (cardg){
			x0 = cardg->x + 15;
			y0 = cardg->y + 20;
		}
		controller = source->controller();
		return 1;
	}

	const char * getMenuText(){
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
					case MTG_COLOR_RED:
						menutext.append("red");
						break;
					case MTG_COLOR_BLUE:
						menutext.append("blue");
						break;
					case MTG_COLOR_GREEN:
						menutext.append("green");
						break;
					case MTG_COLOR_WHITE:
						menutext.append("white");
						break;
					case MTG_COLOR_BLACK:
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

	int testDestroy(){
		if (animation >0) return 0;
		return MTGAbility::testDestroy();
	}

	~AManaProducer(){
		LOG("==Destroying ManaProducer Object");
		if (cost) delete cost;
		SAFE_DELETE(output);
		if (mParticleSys) delete mParticleSys;
		LOG("==Destroying ManaProducer Object Successful!");
	}

};


/* Lifelink Ability */
class ALifeLink:public MTGAbility{
public:
	int nbdamagesthisturn;
	Damage * lastDamage;
	ALifeLink(int _id, MTGCardInstance * _source):MTGAbility(_id, _source){
		nbdamagesthisturn = 0;
		lastDamage = NULL;
	}

	void Update(float dt){
		ActionStack * as = game->mLayers->stackLayer();
		int totaldamages = as->count(ACTION_DAMAGE,RESOLVED_OK);
		if ( totaldamages > nbdamagesthisturn){
			Damage * damage = ((Damage * )as->getNext(lastDamage,ACTION_DAMAGE, RESOLVED_OK));
			while(damage){
				lastDamage = damage;			
				if (damage->source == source){
					source->controller()->life+= damage->damage;
				}
				damage = ((Damage * )as->getNext(lastDamage,ACTION_DAMAGE, RESOLVED_OK));
			}
		}else if (totaldamages ==0){
			lastDamage = NULL;
		}
		nbdamagesthisturn = totaldamages;
	}

};


//Circle of Protections
class ACircleOfProtection: public TargetAbility{
public:
	ACircleOfProtection(int _id, MTGCardInstance * source, int _color):TargetAbility(_id,source,NEW DamageTargetChooser(source,_color),NEW ManaCost(),0,0){
		cost->add(MTG_COLOR_ARTIFACT,1);
	}

	int resolve(){
		Damage * damage = tc->getNextDamageTarget();
		if (!damage) return 0;
		game->mLayers->stackLayer()->Fizzle(damage);
		return 1;
	}
};

//Basic regeneration mechanism for a Mana cost
class AStandardRegenerate:public ActivatedAbility{
public:
	AStandardRegenerate(int _id, MTGCardInstance * _source, MTGCardInstance * _target, ManaCost * _cost):ActivatedAbility(_id,_source,_cost,0,0){
		target = _target;
	}

	int resolve(){
		MTGCardInstance * _target = (MTGCardInstance *)target;
		_target->regenerate();
		PutInGraveyard * action = ((PutInGraveyard *) game->mLayers->stackLayer()->getNext(NULL,ACTION_PUTINGRAVEYARD,NOT_RESOLVED));
		while(action){
#if defined (WIN32) || defined (LINUX)
				OutputDebugString("Fizzling due to regenerate! \n");
#endif
			if (action->card == _target){
				game->mLayers->stackLayer()->Fizzle(action);
			}
			action = ((PutInGraveyard *) game->mLayers->stackLayer()->getNext(action,ACTION_PUTINGRAVEYARD,NOT_RESOLVED));
		}
		return 1;
	}
	
};

/*Gives protection to a target */
class AProtectionFrom:public MTGAbility{
public:
	CardDescriptor * cd;
	void initProtection(){
		((MTGCardInstance *)target)->addProtection(cd);
	}

	AProtectionFrom(int _id, MTGCardInstance * _source, MTGCardInstance * _target, CardDescriptor * _cd):MTGAbility(_id, _source, _target),cd(_cd){
		initProtection();
	}
	AProtectionFrom(int _id, MTGCardInstance * _source, MTGCardInstance * _target, int color):MTGAbility(_id, _source, _target){
		cd = NEW CardDescriptor();
		cd->colors[color] = 1;
		initProtection();
	}

	int destroy(){
		((MTGCardInstance *)target)->removeProtection(cd);
		return 1;
	}

};

//Aura Enchantments that provide controller of target life or damages at a given phase of their turn
class ARegularLifeModifierAura:public MTGAbility{
public:
	int life;
	int phase;
	int onlyIfTargetTapped;
	ARegularLifeModifierAura(int id, MTGCardInstance * _source, MTGCardInstance * _target, int _phase, int _life, int _onlyIfTargetTapped=0):MTGAbility(id,_source,_target),life(_life), phase(_phase),onlyIfTargetTapped(_onlyIfTargetTapped){
	}

	void Update(float dt){
		if (newPhase !=currentPhase && newPhase==phase && game->currentPlayer==((MTGCardInstance *)target)->controller()){
			if (!onlyIfTargetTapped || ((MTGCardInstance *)target)->tapped){
				if (life > 0){
					game->currentPlayer->life+=life;
				}else{
					game->mLayers->stackLayer()->addDamage(source, game->currentPlayer, -life);
				}
			}
		}
	}
};


//ExaltedAbility (Shards of Alara)
class AExalted:public ListMaintainerAbility{
public:
	int power, toughness;
	MTGCardInstance * luckyWinner; 
	AExalted(int _id, MTGCardInstance * _source, int _power = 1, int _toughness = 1):ListMaintainerAbility(_id, _source),power(_power),toughness(_toughness){
		luckyWinner = NULL;
	}


	int canBeInList(MTGCardInstance * card){
		if (card->isAttacker() && game->currentPlayer == source->controller()) return 1;
		return 0;
	}

	int added(MTGCardInstance * card){	
		if(cards.size() == 1){
			luckyWinner = cards.begin()->first;
			luckyWinner->addToToughness(toughness);
			luckyWinner->power+=power;
		}else if (cards.size() == 2){
			luckyWinner->addToToughness(-toughness);
			luckyWinner->power-=power;
		}
		return 1;
	}

	int removed(MTGCardInstance * card){
		if(cards.size() == 1){
			luckyWinner = cards.begin()->first;
			luckyWinner->addToToughness(toughness);
			luckyWinner->power+=power;
		}else if (cards.size() == 0){
			luckyWinner->addToToughness(-toughness);
			luckyWinner->power-=power;
		}
		return 1;
	}

};


//ExaltedAbility for basic abilities (Shards of Alara)
class AExaltedAbility:public ListMaintainerAbility{
public:
	int ability;
	MTGCardInstance * luckyWinner;
	AExaltedAbility(int _id, MTGCardInstance * _source, int _ability):ListMaintainerAbility(_id, _source),ability(_ability){
		luckyWinner = NULL;
	}


	int canBeInList(MTGCardInstance * card){
		if (card->isAttacker() && game->currentPlayer == source->controller()) return 1;
		return 0;
	}

	int added(MTGCardInstance * card){
		luckyWinner = cards.begin()->first;
		if(cards.size() == 1){
			luckyWinner->basicAbilities[ability]+=1;
		}else if (cards.size() == 2){
			luckyWinner->basicAbilities[ability]-=1;
		}
		return 1;
	}

	int removed(MTGCardInstance * card){
		if(cards.size() == 1){
			luckyWinner->basicAbilities[ability]+=1;
		}else if (cards.size() == 0){
			luckyWinner->basicAbilities[ability]-=1;
		}
		return 1;
	}

};


//Converts lands to creatures (Kormus bell, Living lands)
class AConvertLandToCreatures:public ListMaintainerAbility{
public:
	int type;
	int power, toughness;
	AConvertLandToCreatures(int _id, MTGCardInstance * _source, const char * _type, int _power = 1, int _toughness = 1):ListMaintainerAbility(_id, _source),power(_power),toughness(_toughness){
		type = Subtypes::subtypesList->Add(_type);
	}


	int canBeInList(MTGCardInstance * card){
		if (card->hasType(type)) return 1;
		return 0;
	}

	int added(MTGCardInstance * card){
		card->power = 1;
		card->setToughness(1);
		card->setSubtype("creature");
		return 1;
	}

	int removed(MTGCardInstance * card){
		card->removeType("creature");
		return 1;
	}

};

//Lords (Merfolk lord...) give power and toughness to OTHER creatures of their type, they can give them special abilities, regeneration 
class ALord:public ListMaintainerAbility{
public:
	string type;
	int power, toughness;
	int ability;
	ManaCost * regenCost;
	map<MTGCardInstance *, MTGAbility *> regenerations;
	ALord(int _id, MTGCardInstance * card, const char * _type, int _power = 0 , int _toughness = 0, int _ability = -1, ManaCost * _regenCost = NULL):ListMaintainerAbility(_id,card){
		type = _type;
		power = _power;
		toughness = _toughness;
		ability = _ability;
		regenCost = _regenCost;
	}

	int canBeInList(MTGCardInstance * card){
		if (card!=source && card->isACreature() && card->hasSubtype(type)) return 1;
		return 0;
	}

	int added(MTGCardInstance * card){
		card->power += power;
		card->addToToughness(toughness);
		if (ability != -1) card->basicAbilities[ability] +=1;
		if (regenCost){
			AStandardRegenerate * regen = NEW AStandardRegenerate(0, card, card, regenCost);
			regenerations[card] = regen;
			game->addObserver(regen);
		}
		return 1;
	}

	int removed(MTGCardInstance * card){
		card->power -= power;
		card->addToToughness(-toughness);
		if (ability != -1 && card->basicAbilities[ability]) card->basicAbilities[ability] -=1;
		if (regenCost){
			if(regenerations.find(card) != regenerations.end()){
				if (game->isInPlay(card)) game->removeObserver(regenerations[card]);
				regenerations.erase(card);
			}
		}
		return 1;
	}

};

//Lords (Merfolk lord...) give power and toughness to OTHER creatures of a given color, they can give them special abilities, regeneration 
class AColorLord:public ListMaintainerAbility{
public:
	int color;
	int notcolor;
	int power, toughness;
	int ability;
	ManaCost * regenCost;
	map<MTGCardInstance *, MTGAbility *> regenerations;
	AColorLord(int _id, MTGCardInstance * card, int _color, int _notcolor = -1, int _power = 0 , int _toughness = 0, int _ability = -1, ManaCost * _regenCost = NULL):ListMaintainerAbility(_id,card){
		color = _color;
		notcolor = _notcolor;
		power = _power;
		toughness = _toughness;
		ability = _ability;
		regenCost = _regenCost;
	}

	int canBeInList(MTGCardInstance * card){
		if (notcolor > -1){
			if (card!=source && card->isACreature() && !card->hasColor(color)) return 1;
		}else{
			if (card!=source && card->isACreature() && card->hasColor(color)) return 1;
		}
		return 0;
	}

	int added(MTGCardInstance * card){
		card->power += power;
		card->addToToughness(toughness);
		if (ability != -1) card->basicAbilities[ability] +=1;
		if (regenCost){
			AStandardRegenerate * regen = NEW AStandardRegenerate(0, card, card, regenCost);
			regenerations[card] = regen;
			game->addObserver(regen);
		}
		return 1;
	}

	int removed(MTGCardInstance * card){
		card->power -= power;
		card->addToToughness(-toughness);
		if (ability != -1 && card->basicAbilities[ability]) card->basicAbilities[ability] -=1;
		if (regenCost){
			if(regenerations.find(card) != regenerations.end()){
				game->removeObserver(regenerations[card]);
				regenerations.erase(card);
			}
		}
		return 1;
	}

};


/* Standard Damager, can choose a NEW target each time the price is paid */
class ADamager:public TargetAbility{
public:
	int damage;
	 ADamager(int id, MTGCardInstance * card, ManaCost * _cost, int _damage, TargetChooser * _tc = NULL, int _tap = 1):TargetAbility(id,card, _tc, _cost,0,_tap),damage(_damage){
		if (!tc) tc = NEW DamageableTargetChooser(card);
	}
	int resolve(){
		Damageable * _target = tc->getNextDamageableTarget();
		GameObserver::GetInstance()->mLayers->stackLayer()->addDamage(source,_target, damage);
		return 1;
	}


};

/* Can tap a target for a cost */
class ATapper:public TargetAbility{
public:
	int damage;
	 ATapper(int id, MTGCardInstance * card, ManaCost * _cost, TargetChooser * _chooser):TargetAbility(id,card, _chooser, _cost){
	}

	int resolve(){
		MTGCardInstance * _target = tc->getNextCardTarget();
		if (_target){
			_target->tapped = true;
		}
		return 1;
	}

};

// Add life of gives damage if a given zone has more or less than [condition] cards at the beginning of [phase]
//Ex : the rack, ivory tower...
class ALifeZoneLink:public MTGAbility{
public:
	int phase;
	int condition;
	int life;
	int controller;
	int nbcards;
	MTGGameZone * zone;
	ALifeZoneLink(int _id ,MTGCardInstance * card, int _phase, int _condition, int _life = -1, int _controller = 0, MTGGameZone * _zone = NULL):MTGAbility(_id, card){
		phase = _phase;
		condition = _condition;
		controller = _controller;
		life = _life;
		zone = _zone;
		if (zone == NULL){
			if (controller){
				zone = game->currentPlayer->game->hand;
			}else{
				zone = game->opponent()->game->hand;
			}
		}
	}

	void Update(float dt){
		if (newPhase != currentPhase && newPhase == phase){
			if ((controller && game->currentPlayer == source->controller()) ||(!controller && game->currentPlayer != source->controller()) ){
				if ((condition < 0 && zone->nb_cards < - condition) ||(condition >0 && zone->nb_cards > condition)){
					int diff = zone->nb_cards - condition;
					if (condition < 0) diff = - condition - zone->nb_cards;	
					if (life > 0){
						game->currentPlayer->life+=life*diff;
					}else{
						game->mLayers->stackLayer()->addDamage(source,game->currentPlayer,-life*diff);
					}
				}
			}	
		}
	}
};

//Creatures that cannot attack if opponent has not a given type of land, and die if controller has not this type of land
//Ex : pirate ship...
class AStrongLandLinkCreature: public MTGAbility{
public:
	char land[20];
	AStrongLandLinkCreature(int _id, MTGCardInstance * _source, const char * _land):MTGAbility(_id, _source){
		sprintf(land,"%s",_land);
	}

	void Update(float dt){
		if (source->isAttacker()){
			if (!game->opponent()->game->inPlay->hasType(land)){
				source->attacker=0;
				source->tapped = 0;
				//TODO Improve, there can be race conditions here
			}
		}
		Player * player = source->controller();
		if(!player->game->inPlay->hasType(land)){
			player->game->putInGraveyard(source);
		}
	}
};

//Steal control of a target
class AControlStealAura: public MTGAbility{
public:
	Player  * originalController;
	AControlStealAura(int _id , MTGCardInstance * _source, MTGCardInstance * _target):MTGAbility(_id, _source, _target){
		originalController = _target->controller();
		_target->changeController(game->currentlyActing());
	}

	int destroy(){
	MTGCardInstance * _target = (MTGCardInstance *) target;
		if (_target->controller()->game->inPlay->hasCard(_target)){ //if the target is still in game -> spell was destroyed
			_target->changeController(originalController);		
		}
		return 1;
	}
	//TODO put it back into owners's graveyard if needed...
};

//Ability to untap a target
class AUntaper:public TargetAbility{
public:
	AUntaper(int _id, MTGCardInstance * card, ManaCost * _manacost, TargetChooser * _tc):TargetAbility(_id,card,_tc,_manacost){
	}

	int resolve(){
		tc->getNextCardTarget()->tapped = 0;
		return 1;
	}

};


//Same as StealControl Aura ???? Obsolete ?
class ATakeControlAura:public MTGAbility{
public:
	Player * previousController;
	ATakeControlAura(int _id, MTGCardInstance * _source, MTGCardInstance * _target):MTGAbility(_id, _source,_target){
		previousController = _target->controller();
		previousController->game->putInZone(_target, previousController->game->inPlay, source->controller()->game->inPlay);

	}

	int destroy(){
		MTGCardInstance * _target = (MTGCardInstance *) target;
		if (_target->controller()->game->inPlay->hasCard(_target)){
			_target->controller()->game->putInZone(_target, _target->controller()->game->inPlay, previousController->game->inPlay);
		}
		return 1;
	}

};

//Creatures that kill their blockers
//Ex : Cockatrice
class AOldSchoolDeathtouch:public MTGAbility{
public:
	MTGCardInstance * opponents[20]; 
	int nbOpponents;
	AOldSchoolDeathtouch(int _id, MTGCardInstance * _source):MTGAbility(_id, _source){
		nbOpponents = 0;
	}

	void Update(float dt){
		if (newPhase != currentPhase){
			if( newPhase == MTG_PHASE_COMBATDAMAGE){
				nbOpponents = 0;
				MTGCardInstance * opponent = source->getNextOpponent();
				while (opponent && !opponent->hasSubtype("wall")){
					opponents[nbOpponents] = opponent;
					nbOpponents ++;
					opponent = source->getNextOpponent(opponent);
				}
			}else if (newPhase == MTG_PHASE_COMBATEND){
				for (int i = 0; i < nbOpponents ; i++){
					game->mLayers->stackLayer()->addPutInGraveyard(opponents[i]);
				}
			}
		}
	}

	int testDestroy(){
		if(!game->isInPlay(source) && currentPhase != MTG_PHASE_UNTAP){
				return 0;
		}else{
			return MTGAbility::testDestroy();
		}
	}
};


//Converts a card to a creature (Aura)
class AConvertToCreatureAura:public MTGAbility{
public:
	AConvertToCreatureAura(int _id, MTGCardInstance * _source, MTGCardInstance * _target, int _power, int _toughness):MTGAbility(_id, _source, _target){
		_target->setSubtype("creature");
		_target->power = _power;
		_target->toughness = _toughness;
		_target->life = _toughness;
		//_target->afterDamage();
		_target->doDamageTest = 1;
	}

	int destroy(){
		MTGCardInstance * _target = (MTGCardInstance *) target;
		_target->removeType("creature");
		return 1;
	}
};

/*
Specific Classes
*/

// 1092 Specific to Aladdin's Lamp
class AAladdinsLamp: public TargetAbility{
 public:
	CardDisplay cd;
	int nbcards;
	int init;
	AAladdinsLamp(int _id, MTGCardInstance * card):TargetAbility(_id,card){
		cost = NEW ManaCost();
		cost->x();
		cd = CardDisplay(1,game,SCREEN_WIDTH/2, SCREEN_HEIGHT/2,NULL);
		MTGGameZone * zones[] = {game->currentPlayer->game->library};
		tc = NEW TargetZoneChooser(zones,1,source);
		nbcards = 0;
		init = 0;
	}

	void Update(float dt){
		if (waitingForAnswer){
			if (!init){
				cd.resetObjects();
				int wished = game->currentlyActing()->getManaPool()->getConvertedCost();
				game->currentlyActing()->getManaPool()->pay(cost);
				nbcards = 0;
				MTGGameZone * library = game->currentlyActing()->game->library;
				while (nbcards < wished && nbcards < library->nb_cards){
					cd.AddCard(library->cards[library->nb_cards - 1 - nbcards]);
					nbcards++;
				}
				init = 1;
			}
			cd.Update(dt);
			cd.CheckUserInput(dt);
		}
	}

	void Render(float dt){
		if (waitingForAnswer){
			cd.Render();
		}
	}


	int fireAbility(){
		source->tapped = 1;
		MTGLibrary * library = game->currentlyActing()->game->library;
		library->removeCard(tc->getNextCardTarget());
		library->shuffleTopToBottom(nbcards - 1 );
		library->addCard(tc->getNextCardTarget());
		init = 0;
		return 1;
	}

	int resolve(){return 1;};
		

};




//Ankh of Mishra
class AAnkhOfMishra: public MTGAbility{
public:
	int playerLands[2];
	AAnkhOfMishra(int id, MTGCardInstance * _source):MTGAbility(id, _source){
		for (int i=0; i< 2; i++){
			playerLands[i] = GameObserver::GetInstance()->players[i]->game->inPlay->countByType("land");
		}
	}

	void Update(float dt){
		for (int i=0; i < 2; i++){
			int lands = GameObserver::GetInstance()->players[i]->game->inPlay->countByType("land");
			while (lands > playerLands[i]){
				GameObserver::GetInstance()->mLayers->stackLayer()->addDamage(source,GameObserver::GetInstance()->players[i], 2);
				playerLands[i]++;
			}
		}
	}
};



// Armageddon Clock
class AArmageddonClock:public MTGAbility{
public:
	int counters;
	ManaCost cost;
	AArmageddonClock(int id, MTGCardInstance * _source):MTGAbility(id, _source){
		counters = 0;
		int _cost[] = {MTG_COLOR_ARTIFACT, 4};
		cost = ManaCost(_cost,1);
	}

	void Update(float dt){
		if (newPhase != currentPhase){
			if (newPhase == MTG_PHASE_UPKEEP && game->currentPlayer->game->inPlay->hasCard(source)){
				counters ++;
			}else if (newPhase == MTG_PHASE_DRAW && counters > 0 && game->currentPlayer->game->inPlay->hasCard(source)){ //End of upkeep = beginning of draw
				GameObserver::GetInstance()->mLayers->stackLayer()->addDamage(source,GameObserver::GetInstance()->players[0], counters);
				GameObserver::GetInstance()->mLayers->stackLayer()->addDamage(source,GameObserver::GetInstance()->players[1], counters);
			}
		}
	}
	int isReactingToClick(MTGCardInstance *   _card){
		if (counters > 0 && _card == source && currentPhase == MTG_PHASE_UPKEEP){
			if (game->currentlyActing()->getManaPool()->canAfford( & cost)){
				return 1;
			}
		}
		return 0;
	}

	int reactToClick(MTGCardInstance * _card){
		if (!isReactingToClick( _card)) return 0;
		game->currentlyActing()->getManaPool()->pay(& cost);
		counters --;
		return 1;
	}
};


//Black Vise
class ABlackVise: public MTGAbility{
public:
	int nbcards;
	ABlackVise(int id, MTGCardInstance * _source):MTGAbility(id, _source){
		nbcards = game->opponent()->game->hand->nb_cards;
	}

	void Update(float dt){
		if (newPhase == MTG_PHASE_UPKEEP && GameObserver::GetInstance()->opponent()->game->inPlay->hasCard(source)){
			nbcards = game->currentPlayer->game->hand->nb_cards;
		}
		if (newPhase != currentPhase && newPhase == MTG_PHASE_DRAW && GameObserver::GetInstance()->opponent()->game->inPlay->hasCard(source)){ 
			if ( nbcards > 4) game->mLayers->stackLayer()->addDamage(source,game->currentPlayer, nbcards - 4);
		}
	}
};


//Channel
class AChannel:public ActivatedAbility{
public:

	AChannel(int _id, MTGCardInstance * card):ActivatedAbility(_id, card,0,0,0){
	}

	int isReactingToClick(PlayGuiObject * object){
		if (object->type == GUI_AVATAR){
			Player * player = ((GuiAvatar *)object)->player;
			if (player == source->controller()) return 1;
		}
		return 0;
	}
	
	int resolve(){
		source->controller()->life--;
		source->controller()->getManaPool()->add(MTG_COLOR_ARTIFACT, 1);
		return 1;
	}

	int testDestroy(){
		if (newPhase != currentPhase && newPhase == MTG_PHASE_UNTAP) return 1;
		currentPhase = newPhase;
		return 0;
	}
};


// Clockwork Beast
class AClockworkBeast:public MTGAbility{
public:
	int counters;
	ManaCost cost;
	AClockworkBeast(int id, MTGCardInstance * _source):MTGAbility(id, _source){
		counters = 7;
		((MTGCardInstance *)target)->power+=7;
		int _cost[] = {MTG_COLOR_ARTIFACT, 1};
		cost = ManaCost(_cost,1);
	}

	void Update(float dt){
		if (newPhase != currentPhase && newPhase == MTG_PHASE_COMBATEND){
			if (((MTGCardInstance *)source)->isAttacker() || ((MTGCardInstance *)source)->isDefenser()){
				counters--;
				((MTGCardInstance *)target)->power-=1;
			}
		}
	}
	int isReactingToClick(MTGCardInstance *  _card){
		if (counters < 7  && _card == source && currentPhase == MTG_PHASE_UPKEEP && game->currentPlayer->game->inPlay->hasCard(source)){
			if (game->currentlyActing()->getManaPool()->canAfford( & cost)){
				return 1;
			}
		}
		return 0;
	}

	int reactToClick(MTGCardInstance * _card){
		if (!isReactingToClick( _card)) return 0;
		game->currentlyActing()->getManaPool()->pay(& cost);
		counters ++;
		((MTGCardInstance *)target)->power++;
		((MTGCardInstance *)target)->tapped = 1;
		return 1;
	}
};

//1102: Conservator
class AConservator: public MTGAbility{
public:
	int canprevent;
	ManaCost cost;
	AConservator(int _id, MTGCardInstance * _source):MTGAbility(_id, _source){
		canprevent = 0;
		int _cost[] = {MTG_COLOR_ARTIFACT, 2};
		cost = ManaCost(_cost, 1);
	}

	int alterDamage(Damage * damage){
		if (canprevent && damage->target == source->controller()){
			if (damage->damage >= canprevent){
				damage->damage-=canprevent;
				canprevent = 0;
			}else{
				canprevent-=damage->damage;
				damage->damage = 0;
			}	
		}
		return 1;
	}
	int alterDamage(){
		if (canprevent){
			ActionStack * stack = game->mLayers->stackLayer();
			for (int i = stack->mCount-1; i>=0; i--){
				if (!canprevent) return 1;
				Interruptible * current = ((Interruptible *)stack->mObjects[i]);
				if (current->type == ACTION_DAMAGE && current->state==NOT_RESOLVED){
					Damage * damage = (Damage *)current;
					alterDamage(damage);
				}else if (current->type == ACTION_DAMAGES && current->state == NOT_RESOLVED){
					DamageStack * damages = (DamageStack *)current;
					for (int j = damages->mCount-1;j >=0; j--){
						alterDamage(((Damage *)damages->mObjects[j]));
					}
				}
			}
		}
		return 1;
	}

	void Update(float dt){
		alterDamage();
	}

	int isReactingToClick(MTGCardInstance *  _card){
		if ( _card == source && game->currentlyActing()->game->inPlay->hasCard(source) && !_card->isTapped()){
			if (game->currentlyActing()->getManaPool()->canAfford( & cost)){
				return 1;
			}
		}
		return 0;
	}

	int reactToClick(MTGCardInstance * _card){
		if (!isReactingToClick( _card)) return 0;
		game->currentlyActing()->getManaPool()->pay(& cost);
		source->tapped = 1;
		canprevent = 2;
		alterDamage();
		return 1;
	}

};


//Creature bond
class ACreatureBond:public TriggeredAbility{
public:
	int resolved;
	ACreatureBond(int _id, MTGCardInstance * _source, MTGCardInstance * _target):TriggeredAbility(_id,_source,_target){
		resolved = 1;
	}

	int trigger(){
		MTGCardInstance * _target = (MTGCardInstance *) target;
		for (int i = 0; i < 2; i++){
			if (game->players[i]->game->graveyard->hasCard(_target)) return 1;
		}
		return 0;
	}

	int resolve(){
		MTGCardInstance * _target = (MTGCardInstance *) target;
		game->mLayers->stackLayer()->addDamage(source,_target->controller(),_target->toughness);
		resolved = 1;
		return 1;
	}

	int testDestroy(){
		MTGCardInstance * _target = (MTGCardInstance *)target;
		if(_target->controller()->game->graveyard->hasCard(_target) && !resolved){
				return 0;
		}else{
			return TriggeredAbility::testDestroy();
		}
	}
};

//1105: Dingus Egg
class ADingusEgg: public MTGAbility{
public:
	int playerLands[2];
	ADingusEgg(int id, MTGCardInstance * _source):MTGAbility(id, _source){
		for (int i=0; i< 2; i++){
			playerLands[i] = GameObserver::GetInstance()->players[i]->game->inPlay->countByType("land");
		}
	}

	void Update(float dt){
		for (int i=0; i < 2; i++){
			int lands = GameObserver::GetInstance()->players[i]->game->inPlay->countByType("land");
			while (lands < playerLands[i]){
				GameObserver::GetInstance()->mLayers->stackLayer()->addDamage(source,GameObserver::GetInstance()->players[i], 2);
				playerLands[i]--;
			}
		}
	}
};



//1106 DisruptingScepter
class ADisruptingScepter:public TargetAbility{
public:
	ADisruptingScepter(int id, MTGCardInstance * _source):TargetAbility(id,_source){
		MTGGameZone * zones[] = {GameObserver::GetInstance()->opponent()->game->hand};
		tc = NEW TargetZoneChooser(zones,1,_source);
		int _cost[] = {MTG_COLOR_ARTIFACT, 3};
		cost = NEW ManaCost(_cost,1);
	}

	void Update(float dt){
		if (game->opponent()->isAI()){
			if(waitingForAnswer){
				MTGCardInstance * card = ((AIPlayer *)game->opponent())->chooseCard(tc, source);
				if (card) tc->toggleTarget(card);
				if (!card || tc->targetsReadyCheck() == TARGET_OK) waitingForAnswer = 0;
			}
			TargetAbility::Update(dt);
		}else{
			TargetAbility::Update(dt);
		}
	}

	int resolve(){
		game->opponent()->game->putInGraveyard(tc->getNextCardTarget());
		return 1;
	}
	

};


//1108 Ebony Horse
class AEbonyHorse:public TargetAbility{
public:

	AEbonyHorse(int _id, MTGCardInstance * _source):TargetAbility(_id,_source, NEW CreatureTargetChooser()){
		int _cost[] = {MTG_COLOR_ARTIFACT, 2};
		cost = NEW ManaCost(_cost,1);
	}

	int resolve(){
		tc->getNextCardTarget()->attacker =  0;
		return 1;
	}
	
};

//1345 Farmstead
class AFarmstead:public ActivatedAbility{
public:
	AFarmstead(int _id, MTGCardInstance * source, MTGCardInstance * _target):ActivatedAbility(_id, source,0,1,0){
		int _cost[] = {MTG_COLOR_WHITE, 2};
		cost = NEW ManaCost(_cost,1);
		target = _target;
	}

	int isReactingToClick(MTGCardInstance * card){
		if (!ActivatedAbility::isReactingToClick(card)) return 0;
		if (currentPhase == MTG_PHASE_UPKEEP) return 1;
		return 0;
	}

	int resolve(){
		source->controller()->life++;
		return 1;
	}

};

//1109 flying Carpet
class AFlyingCarpet:public ABasicAbilityModifierUntilEOT{

	public:
	AFlyingCarpet(int _id, MTGCardInstance * _source): ABasicAbilityModifierUntilEOT(_id,_source,FLYING, NEW ManaCost()){
		cost->add(MTG_COLOR_ARTIFACT,2);
	}

	void Update(float dt){
		ABasicAbilityModifierUntilEOT::Update(dt);
	
		if (nbTargets){
			MTGCardInstance * mTarget = mTargets[0];
			for (int i = 0; i < 2; i++){
				if(game->players[i]->game->graveyard->hasCard(mTarget)){
					game->players[i]->game->putInGraveyard(source);
					mTarget = NULL;
				}
			}
		}
	}



	int destroy(){
		if (!nbTargets) return 0;
		MTGCardInstance * mTarget = mTargets[0];
		if (mTarget && mTarget->basicAbilities[FLYING]){
				mTarget->basicAbilities[FLYING] = stateBeforeActivation[0];
				mTarget = NULL;
			return 1;
		}else{
			//BUG !!!
			return 0;
		}
	}

};



//1110 Glasses of Urza
class AGlassesOfUrza:public MTGAbility{
public:
	CardDisplay * display;
	AGlassesOfUrza(int _id, MTGCardInstance * _source):MTGAbility(_id, _source){
		display = NEW CardDisplay(0, game,SCREEN_WIDTH/2, SCREEN_HEIGHT/2,NULL);
	}

	void Update(float dt){
		if(modal){
			display->Update(dt);
		}
	}

	void CheckUserInput(float dt){
		if (modal){
			display->CheckUserInput(dt);
			JGE * mEngine = JGE::GetInstance();
			if (mEngine->GetButtonClick(PSP_CTRL_CROSS)){
				modal = 0;
			}
		}
	}

	void Render(float dt){
		if (modal){
			display->Render();
		}

	}
	int isReactingToClick(MTGCardInstance *  card){
		if ( card == source){
		if (game->currentlyActing()->game->isInPlay(card) && !source->isTapped()){
				return 1;
			}
		}
		return 0;
	}

	int reactToClick(MTGCardInstance * card){
		if (!isReactingToClick(card)) return 0;
		source->tapped = 1;
		modal = 1;
		return 1;
	}

};

//1112 Howling Mine
class AHowlingMine:public MTGAbility{
public:
	AHowlingMine(int _id, MTGCardInstance * _source):MTGAbility(_id, _source){}

	void Update(float dt){
		if (newPhase != currentPhase && newPhase == MTG_PHASE_DRAW && !source->tapped){
			game->mLayers->stackLayer()->addDraw(game->currentPlayer);
		}
	}
};

//1118 Jandors Sandlebag
class AJandorsSandlebag:public TargetAbility{
public:
	AJandorsSandlebag(int _id, MTGCardInstance * card):TargetAbility(_id, card){
		int _cost[] = {MTG_COLOR_ARTIFACT, 3};
		cost = NEW ManaCost(_cost,1);
		tc = NEW CreatureTargetChooser();
	}

	int resolve(){
		MTGCardInstance * card = tc->getNextCardTarget();
		if (card->tapped){
			card->tapped = 0;
			return 1;
		}
	return 0;
	}
};

//1119 Jayemdae Tome
class AJayemdaeTome:public ActivatedAbility{
public:
	AJayemdaeTome(int _id, MTGCardInstance * card):ActivatedAbility(_id, card){
		int _cost[] = {MTG_COLOR_ARTIFACT, 4};
		cost = NEW ManaCost(_cost,1);
	}

	int resolve(){
		game->mLayers->stackLayer()->addDraw(source->controller());
		return 1;
	}
};


//1205 Lifetap
class ALifetap:public MTGAbility{
public:
	int nbforeststapped;

	int countForestsTapped(){
		int result = 0;
		MTGInPlay * inplay = source->controller()->opponent()->game->inPlay;
		for (int i = 0; i < inplay->nb_cards; i++){
			MTGCardInstance * card = inplay->cards[i];
			if (card->tapped && card->hasType("forest")) result++;
		}
		return result;
	}

	ALifetap(int _id, MTGCardInstance * source):MTGAbility(_id, source){
		nbforeststapped = countForestsTapped();
	}

	void Update(float dt){
		int newcount = countForestsTapped();
		for (int i=0; i < newcount - nbforeststapped; i++){
			source->controller()->life++;
		}
		nbforeststapped = newcount;
	}

};


//Living Artifact
class ALivingArtifact:public MTGAbility{
public:
	int usedThisTurn;
	int counters;
	Damage * latest;
	ALivingArtifact(int _id, MTGCardInstance * _source, MTGCardInstance * _target):MTGAbility(_id,_source,_target){
		usedThisTurn = 0;
		counters = 0;
		latest = NULL;
	}

	void Update(float dt){
		if (newPhase != currentPhase && newPhase == MTG_PHASE_UNTAP) usedThisTurn = 0;
		Damage * damage = ((Damage *)game->mLayers->stackLayer()->getNext(latest,ACTION_DAMAGE,RESOLVED_OK));
		while (damage){
			if (damage->target == source->controller()){
				counters += damage->damage;
			}
			latest = damage;
			damage = ((Damage *)game->mLayers->stackLayer()->getNext(damage,ACTION_DAMAGE,RESOLVED_OK));
		}
	}

	int isReactingtoclick(MTGCardInstance * card){
		if (currentPhase == MTG_PHASE_UPKEEP && card == source && game->currentPlayer == source->controller() && counters && !usedThisTurn){
			return 1;
		}
		return 0;
	}

	int reactToClick(MTGCardInstance * card){
		source->controller()->life+=1;
		counters--;
		usedThisTurn = 1;
		return 1;
	}

};

//Lord of the Pit
class ALordOfThePit: public TargetAbility{
public:
	int paidThisTurn;
	ALordOfThePit(int _id, MTGCardInstance * source):TargetAbility(_id, source, NEW CreatureTargetChooser(),0,1,0){
		paidThisTurn = 1;
	}

	void Update(float dt){
		if (newPhase != currentPhase && source->controller() == game->currentPlayer){
			if (newPhase == MTG_PHASE_UNTAP){
				paidThisTurn = 0;
			}else if( newPhase == MTG_PHASE_UPKEEP + 1 && !paidThisTurn){
				game->mLayers->stackLayer()->addDamage(source,source->controller(), 7);
			}
		}
		TargetAbility::Update(dt);
	}

	int isReactingToClick(MTGCardInstance * card){
		if (currentPhase != MTG_PHASE_UPKEEP || paidThisTurn) return 0;
		return TargetAbility::isReactingToClick(card);
	}

	int resolve(){
		MTGCardInstance * card = tc->getNextCardTarget();
		if (card && card != source && card->controller() == source->controller()){
			card->controller()->game->putInGraveyard(card);
			paidThisTurn = 1;
			return 1;
		}
		return 0;
	}

};
//1143 Animate Dead 
class AAnimateDead:public MTGAbility{
public:
	AAnimateDead(int _id, MTGCardInstance * _source, MTGCardInstance * _target):MTGAbility(_id, _source, _target){
		MTGCardInstance * card =  _target;
		card->power--;
		card->life = card->toughness;
		//Put the card in play again, with all its abilities !
		//AbilityFactory af;
		Spell * spell = NEW Spell(card);
		//af.addAbilities(game->mLayers->actionLayer()->getMaxId(), spell);
		source->controller()->game->putInZone(card,  _target->controller()->game->graveyard, source->controller()->game->stack);
		spell->resolve();
		delete spell;
	}

	int destroy(){
		MTGCardInstance * card = (MTGCardInstance *) target;
		card->power++;
		return 1;
	}
};

//1144 Bad Moon, 1341 Crusade
class ABadMoon:public ListMaintainerAbility{
public:	
	int color;
	ABadMoon(int _id, MTGCardInstance * _source, int _color = MTG_COLOR_BLACK):ListMaintainerAbility(_id, _source),color(_color){
	}

	int canBeInList(MTGCardInstance * card){
		if (card->isACreature() && card->hasColor(color)) return 1;
		return 0;
	}

	int added(MTGCardInstance * card){
		card->power += 1;
		card->addToToughness(1);
		return 1;
	}

	int removed(MTGCardInstance * card){
		card->power -= 1;
		card->addToToughness(-1);
		return 1;
	}

};


//1159 Erg Raiders
class AErgRaiders:public MTGAbility{
public:
	int init;
	int dealDamage;
	AErgRaiders(int _id, MTGCardInstance * _source):MTGAbility(_id, _source){
		init = 0;
		dealDamage = 0;
	}

	void Update(float dt){
		if (newPhase != currentPhase){
			Player * controller =  source->controller();
			if (newPhase == MTG_PHASE_COMBATDAMAGE && game->currentPlayer == controller){
				if (!source->isAttacker() && init){
					dealDamage = 1;
				}
			}else if (newPhase == MTG_PHASE_UNTAP && game->currentPlayer != controller){
				if (dealDamage){
					game->mLayers->stackLayer()->addDamage(source, controller,2);
				}
				init = 1;
				dealDamage = 0;
			}
		}
		
	}
};

//Fastbond
class AFastbond:public TriggeredAbility{
public:
	int alreadyPlayedALand;
	AFastbond(int _id, MTGCardInstance * card):TriggeredAbility(_id, card){
		alreadyPlayedALand = 0;
	}

	void Update(float dt){
		if (newPhase!=currentPhase && newPhase == MTG_PHASE_UNTAP){
			alreadyPlayedALand = 0;
		}
		TriggeredAbility::Update(dt);
	}

	int trigger(){
		if(source->controller()->canPutLandsIntoPlay==0) return 1;
		return 0;
	}

	int resolve(){
		source->controller()->canPutLandsIntoPlay = 1;
		if (alreadyPlayedALand){
			game->mLayers->stackLayer()->addDamage(source, source->controller(), 1);
		}
		alreadyPlayedALand = 1;
		return 1;
	}
};



//1165 Hypnotic Specter
class AHypnoticSpecter:public MTGAbility{
public:
	int nbdamagesthisturn[2];
	AHypnoticSpecter(int _id, MTGCardInstance * _source):MTGAbility(_id, _source){
		currentPhase = -1;
		for (int i = 0; i < 2; i++){
			nbdamagesthisturn[i] = 0;
		}
	}

	void Update(float dt){
		if (newPhase != currentPhase && newPhase == MTG_PHASE_UNTAP){
			for (int i = 0; i < 2; i++){
				nbdamagesthisturn[i] = 0;
			}
		}

		ActionStack * as = game->mLayers->stackLayer();
		int nbdamages[2];
		for (int i = 0; i < 2; i++){
			nbdamages[i] = 0;
		}

		Damage * current = ((Damage *)as->getNext(NULL,ACTION_DAMAGE,RESOLVED_OK));
		while(current){
			if (current->source == source){
				for (int j=0; j < 2; j++){
					if(current->target == game->players[j]) nbdamages[j]++;
				}			
			}
			current = ((Damage *)as->getNext(current,ACTION_DAMAGE,RESOLVED_OK));

		}

		for (int i = 0; i < 2; i++){
			while(nbdamages[i] > nbdamagesthisturn[i]){
				nbdamagesthisturn[i]++;
				game->players[i]->game->discardRandom(game->players[i]->game->hand);
			}
		}

	
	}

};

//1117 Jandor's Ring
class AJandorsRing:public ActivatedAbility{
public:
	AJandorsRing(int _id, MTGCardInstance * _source):ActivatedAbility(_id,_source, NEW ManaCost()){
		cost->add(MTG_COLOR_ARTIFACT, 2);
	}

	int isReactingToClick(MTGCardInstance * card){
		if (!source->controller()->game->hand->hasCard(source->controller()->game->library->lastCardDrawn)) return 0;
		return ActivatedAbility::isReactingToClick(card);
	}

	int resolve(){
		source->controller()->game->putInGraveyard(source->controller()->game->library->lastCardDrawn);
		game->mLayers->stackLayer()->addDraw(source->controller());
		return 1;
	}

};


//Kudzu.
//What happens when there are no targets ???
class AKudzu: public TargetAbility{
public:
	int previouslyTapped;
	AKudzu(int _id, MTGCardInstance * card, MTGCardInstance * _target):TargetAbility(_id,card, NEW TypeTargetChooser("land",card)){
		tc->toggleTarget(_target);
		target = _target;
		previouslyTapped = 0;
		if (_target->tapped) previouslyTapped = 1;
	}


	void Update(float dt){
		MTGCardInstance * _target = (MTGCardInstance *)target;
		if (!_target->tapped){
			previouslyTapped = 0;
		}else if (!previouslyTapped){
#if defined (WIN32) || defined (LINUX)
		OutputDebugString("Kudzu Strikes !\n");
#endif
			MTGCardInstance * _target = (MTGCardInstance *)target;
			_target->controller()->game->putInGraveyard(_target);
			reactToClick(source); // ????
		}
		TargetAbility::Update(dt);
	}

	int isReactingToClick(MTGCardInstance * card){
		MTGCardInstance * _target = (MTGCardInstance *)target;
		if (card == source && (!_target || !_target->isInPlay())){
#if defined (WIN32) || defined (LINUX)
		OutputDebugString("Kudzu Reacts to click !\n");
#endif
			return 1;
		}
		return 0;
	}

/*
	int reactToClick(MTGCardInstance * card){
		if (!waitingForAnswer) {
		}else{
			tc->toggleTarget(card); 
		}
		return 1;
	}
	*/

	int resolve(){
		target = tc->getNextCardTarget();
		source->target = (MTGCardInstance *) target;
		previouslyTapped = 0;
		if (source->target->tapped) previouslyTapped = 1;
		return 1;
	}

	int testDestroy(){
		GameObserver * g = GameObserver::GetInstance();
		int stillLandsInPlay = 0;
		for (int i = 0; i < 2; i++){
			if (game->players[i]->game->inPlay->hasType("land")) stillLandsInPlay = 1;
		}
		if (!stillLandsInPlay){
			source->controller()->game->putInGraveyard(source);
			return 1;
		}

		if (!game->isInPlay(source)){
			return 1;
		}

		return 0;
	}


};

//Millstone
class AMillstone:public TargetAbility{
public:
	AMillstone(int _id, MTGCardInstance * card):TargetAbility(_id,card, NEW PlayerTargetChooser(), NEW ManaCost()){
		cost->add(MTG_COLOR_ARTIFACT, 2);
	}

	int resolve(){
		Player * player = tc->getNextPlayerTarget();
		if (!player) return 0;
		MTGLibrary * library = player->game->library;
		for (int i = 0; i < 2; i++){
			if (library->nb_cards)
				player->game->putInZone(library->cards[library->nb_cards-1],library, player->game->graveyard);
		}
		return 1;
	}

};

//1170: Nightmare
class ANightmare:public ListMaintainerAbility{
public:	
	ANightmare(int _id, MTGCardInstance * _source):ListMaintainerAbility(_id, _source){
	}

	int canBeInList(MTGCardInstance * card){
		if (source->controller()->game->inPlay->hasCard(card) && card->hasType("swamp") ) return 1;
		return 0;
	}

	int added(MTGCardInstance * card){
		source->power += 1;
		source->addToToughness(1);
		return 1;
	}

	int removed(MTGCardInstance * card){
		source->power -= 1;
		source->addToToughness(-1);
		return 1;
	}

};



//1172 Pestilence
class APestilence: public ActivatedAbility{
public:
	APestilence(int _id, MTGCardInstance * card):ActivatedAbility(_id, card, NEW ManaCost(), 0,0){
		cost->add(MTG_COLOR_BLACK, 1);
	}
	
	void Update(float dt){
		if (newPhase !=currentPhase && newPhase == MTG_PHASE_EOT){
			if (!game->players[0]->game->inPlay->hasType("creature") && !game->players[1]->game->inPlay->hasType("creature")){
				source->controller()->game->putInGraveyard(source);
			}
		}
	}

	int resolve(){
		for (int i = 0; i < 2 ; i++){
			MTGInPlay * inplay = game->players[i]->game->inPlay;
			for (int j = inplay->nb_cards - 1 ; j >=0; j--){
				if (inplay->cards[j]->isACreature()) game->mLayers->stackLayer()->addDamage(source,inplay->cards[j],1);
			}
			game->mLayers->stackLayer()->addDamage(source,game->players[i],1);
		}
		return 1;
	}

};

//Plague Rats and similar. Power and toughness equal to number of cards that share a name
class APlagueRats:public ListMaintainerAbility{
public:
	string name;
	APlagueRats(int _id, MTGCardInstance * _source, const char * _name):ListMaintainerAbility(_id,_source){
		name = _name;
		std::transform(name.begin(), name.end(), name.begin(),::tolower );
	}

	int canBeInList(MTGCardInstance * card){
		if (card == source) return 0;
		string compared = card->name;
		std::transform( compared.begin(), compared.end(), compared.begin(),::tolower );
		if (name.compare(compared) == 0) return 1;
		return 0;
	}

	int added(MTGCardInstance * card){
		source->power += 1;
		source->addToToughness(1);
		return 1;
	}

	int removed(MTGCardInstance * card){
		source->power -= 1;
		source->addToToughness(-1);
		return 1;
	}

};

//Power Leak
class APowerLeak:public TriggeredAbility{
public:
	int damagesToDealThisTurn;
	ManaCost cost;
	APowerLeak(int _id, MTGCardInstance * _source, MTGCardInstance * _target):TriggeredAbility(_id, _source, _target){
		cost.add(MTG_COLOR_ARTIFACT, 1);
		damagesToDealThisTurn = 0;
	}

	void Update(float dt){
		MTGCardInstance * _target  = (MTGCardInstance *) target;
		if (newPhase != currentPhase && newPhase == MTG_PHASE_UPKEEP && _target->controller() == game->currentPlayer){
			damagesToDealThisTurn = 2;
		}
		TriggeredAbility::Update(dt);
	}

	int isReactingToClick(MTGCardInstance * card){
		MTGCardInstance * _target = (MTGCardInstance *) target;
		if (damagesToDealThisTurn && currentPhase == MTG_PHASE_UPKEEP && card==source && _target->controller() == game->currentPlayer){
			if (game->currentPlayer->getManaPool()->canAfford(& cost)) return 1;
		}
		return 0;
	}

	int reactToclick(MTGCardInstance * card){
		game->currentPlayer->getManaPool()->pay( & cost);
		damagesToDealThisTurn--;
		return 1;
	}

	int trigger(){
		MTGCardInstance * _target = (MTGCardInstance *) target;
		if (newPhase != currentPhase && newPhase == MTG_PHASE_DRAW && _target->controller() == game->currentPlayer){
			if (damagesToDealThisTurn) return 1;
		}
		return 0;
	}

	int resolve(){
		MTGCardInstance * _target = (MTGCardInstance *) target;
		game->mLayers->stackLayer()->addDamage(source,_target->controller(), damagesToDealThisTurn);
		return 1;
	}
};

//Power Surge
class APowerSurge:public TriggeredAbility{
public: 
		int totalLands;
		APowerSurge(int _id, MTGCardInstance * _source):TriggeredAbility(_id,_source){
			totalLands = 0;
		}

	int trigger(){
		if (newPhase != currentPhase && newPhase == MTG_PHASE_EOT){ 
			//That's ugly but untapped land at the beginning of the turn are opponent's untapped lands at the end of the turn
			totalLands = 0;
			MTGInPlay * inPlay = game->opponent()->game->inPlay;
			for (int i = 0; i < inPlay->nb_cards; i++){
				MTGCardInstance * card = inPlay->cards[i];
				if (!card->tapped && card->hasType("land")){
					totalLands++;
				}
			}
		}
		if (newPhase != currentPhase && newPhase == MTG_PHASE_UPKEEP && totalLands){
			return 1;
		}
		return 0;
	}

	int resolve(){
		if (totalLands) game->mLayers->stackLayer()->addDamage(source,game->currentPlayer,totalLands);
		totalLands = 0;
		return 1;
	}
};

//1175 Royal Assassin
class ARoyalAssassin:public TargetAbility{
public:

	ARoyalAssassin(int _id, MTGCardInstance * _source):TargetAbility(_id,_source, NEW CreatureTargetChooser()){
	}

	int resolve(){
		MTGCardInstance * _target = tc->getNextCardTarget();  
		if(_target && _target->tapped){
			_target->controller()->game->putInGraveyard(_target);
			return 1;
		}
		return 0;
	}
	
};


//1176 Sacrifice
class ASacrifice:public InstantAbility{
public:
	ASacrifice(int _id, MTGCardInstance * _source, MTGCardInstance * _target):InstantAbility(_id, _source){
		target = _target;
	}

	int resolve(){
		MTGCardInstance * _target = (MTGCardInstance *) target;
		if (_target->isInPlay()){
			game->currentlyActing()->game->putInGraveyard(_target);
			int x = _target->getManaCost()->getConvertedCost();
			game->currentlyActing()->getManaPool()->add(MTG_COLOR_BLACK, x);
		}
		return 1;
	}

};

//1178 Scavenging Ghoul
class AScavengingGhoul:public MTGAbility{
public:
	int counters;
	AScavengingGhoul(int _id, MTGCardInstance * _source, MTGCardInstance * _target):MTGAbility(_id, _source, _target){
		counters = 0;
	}


	void Update(float dt){
		//TODO
	}

	int isReactingToClick(MTGCardInstance *  _card){
		if (counters > 0 && _card == source && game->currentlyActing()->game->inPlay->hasCard(source)){
				return 1;
		}
		return 0;
	}

	int reactToClick(MTGCardInstance *  _card){
		if (!isReactingToClick( _card)) return 0;
		counters--;
		source->regenerate(); 
		return 1;
	}
	
};

//1218 Psychic Venom
class APsychicVenom:public MTGAbility{
public:
	int tapped;
	APsychicVenom(int _id, MTGCardInstance * _source, MTGCardInstance * _target):MTGAbility(_id, _source,_target){
		tapped = _target->tapped;
	}

	void Update(float dt){
		MTGCardInstance*  _target = (MTGCardInstance* )target;
		int newState = _target->isTapped();
		if (newState != tapped && newState == 1){
			game->mLayers->stackLayer()->addDamage(source,_target->controller(),2);
		}
		tapped = newState;
	}
};


//1221 Serendib Efreet
class ASerendibEfreet:public MTGAbility{
public:
	ASerendibEfreet(int _id, MTGCardInstance * _source):MTGAbility(_id, _source){
	}

	void Update(float dt){
		if (newPhase == MTG_PHASE_UPKEEP && newPhase != currentPhase && game->currentPlayer == source->controller()){
			game->mLayers->stackLayer()->addDamage(source,game->currentPlayer,1);
		}
	}
};


//1235 Aspect of Wolf
class AAspectOfWolf:public ListMaintainerAbility{
public:	
	int color;
	AAspectOfWolf(int _id, MTGCardInstance * _source, MTGCardInstance * _target):ListMaintainerAbility(_id, _source, _target){
	}

	int canBeInList(MTGCardInstance * card){

		if (card->controller() == source->controller() &&  card->hasType("forest")) return 1;
		return 0;
	}

	int added(MTGCardInstance * card){
		MTGCardInstance * _target = (MTGCardInstance *) target;
		int size = cards.size();
		if (size % 2 == 0){
			_target->power += 1;
		}else{
			_target->addToToughness(1);
		}
		return 1;
	}

	int removed(MTGCardInstance * card){
		MTGCardInstance * _target = (MTGCardInstance *) target;
		int size = cards.size();
		if (size % 2 == 1){
			_target->power -= 1;
		}else{
			_target->addToToughness(-1);
		}
		return 1;
	}

};

//1276 Wanderlust, 1148 Cursed Lands
class AWanderlust:public TriggeredAbility{
public:
	AWanderlust(int _id, MTGCardInstance * _source, MTGCardInstance * _target):TriggeredAbility(_id,_source, _target){}

	int trigger(){
		if (newPhase != currentPhase && newPhase == MTG_PHASE_UPKEEP && ((MTGCardInstance *) target)->controller()==game->currentPlayer){
			return 1;
		}
		return 0;
	}

	int resolve(){
		game->mLayers->stackLayer()->addDamage(source,((MTGCardInstance *) target)->controller(),1);
		return 1;
	}
};

//1280 Atog
class AAtog:public TargetAbility{
public:
	Player * currentController;
	int counters;
	AAtog(int _id, MTGCardInstance * _source):TargetAbility(_id, _source,NULL, NULL, 0,0){
		currentController = source->controller();
		MTGGameZone * zones[] = {currentController->game->inPlay};  
		tc = NEW TypeTargetChooser("artifact", zones, 1, source);
		counters = 0;
	}
	
	void Update(float dt){
		if (newPhase != currentPhase && newPhase == MTG_PHASE_UNTAP){
			for (int i = 0; i < counters; i++){
				source->power-=2;
				source->addToToughness(-2);
			}
			counters = 0;
		}
		TargetAbility::Update(dt);
		Player * newController = source->controller();
		if (newController != currentController){
			delete tc;
			MTGGameZone * zones[] = {newController->game->inPlay};  //In case Atog's controller changes
			tc = NEW TypeTargetChooser("artifact", zones, 1, source);
			currentController = newController;
		}
	}

	int resolve(){
			tc->getNextCardTarget()->controller()->game->putInGraveyard(tc->getNextCardTarget());
			source->power+=2;
			source->addToToughness(2);
			counters ++;
			return 1;
	}
};




//1284 Dragon Whelp
class ADragonWhelp: public APowerToughnessModifierUntilEndOfTurn{
public:
	ADragonWhelp(int id, MTGCardInstance * card):APowerToughnessModifierUntilEndOfTurn(id, card, card, 1, 0, NEW ManaCost()){
		cost->add(MTG_COLOR_RED, 1);
	}

		void Update(float dt){
			if (newPhase != currentPhase && newPhase == MTG_PHASE_UNTAP && counters > 3){
				source->controller()->game->putInGraveyard(source);
			}
			APowerToughnessModifierUntilEndOfTurn::Update(dt);		
		}
	
};

//1288 EarthBind
class AEarthbind:public ABasicAbilityModifier{
public:	
	AEarthbind(int _id, MTGCardInstance * _source, MTGCardInstance * _target):ABasicAbilityModifier(_id,_source,_target,FLYING,0){
		if (value_before_modification) game->mLayers->stackLayer()->addDamage(source,target,2);
	}
};

//1291 Fireball
class AFireball:public InstantAbility{
public:
	AFireball(int _id, MTGCardInstance * card, Spell * spell, int x):InstantAbility(_id, card){
		int nbtargets = spell->cursor;
		int totaldamage = x+1-nbtargets;
		int individualdamage  = totaldamage / nbtargets;
		Damageable * _target = spell->getNextDamageableTarget();
		while(_target){
			game->mLayers->stackLayer()->addDamage(source,_target,individualdamage);
			_target = spell->getNextDamageableTarget(_target);
		}
	}
};

//1245 ForceOfNature
class AForceOfNature:public ActivatedAbility{
public:
	int dealDamageThisTurn;
	AForceOfNature(int _id, MTGCardInstance * card):ActivatedAbility(_id,card, NEW ManaCost(),1,0){
		dealDamageThisTurn = 0;
		cost->add(MTG_COLOR_GREEN,4);
	}

	void Update(float dt){
		if (newPhase !=currentPhase){
			if (newPhase == MTG_PHASE_UNTAP){
				dealDamageThisTurn = 1;
			}else if (newPhase == MTG_PHASE_DRAW && dealDamageThisTurn && game->currentPlayer==source->controller() ){
				game->mLayers->stackLayer()->addDamage(source,source->controller(),8);
			}
		}
		ActivatedAbility::Update(dt);
	}

	int isReactingToClick(MTGCardInstance * card){
		return (dealDamageThisTurn && currentPhase == MTG_PHASE_UPKEEP && ActivatedAbility::isReactingToClick(card));
	}

	int resolve(){
		dealDamageThisTurn = 0;
		return 1;
	}
};

//1301 KeldonWarlord
class AKeldonWarlord:public ListMaintainerAbility{
public:	
	AKeldonWarlord(int _id, MTGCardInstance * _source):ListMaintainerAbility(_id, _source){
	}

	int canBeInList(MTGCardInstance * card){
		if (source->controller()->game->inPlay->hasCard(card) && card->isACreature() && !card->hasType("wall") ) return 1;
		return 0;
	}

	int added(MTGCardInstance * card){
		source->power += 1;
		source->addToToughness(1);
		return 1;
	}

	int removed(MTGCardInstance * card){
		source->power -= 1;
		source->addToToughness(-1);
		return 1;
	}

};

//1302 : Kird Ape
class AKirdApe:public MTGAbility{
public:
	int init;
	AKirdApe(int _id, MTGCardInstance * _source):MTGAbility(_id, _source){
		init = 0;
	}

	void Update(float dt){
		if (source->controller()->game->inPlay->hasType("forest")){
			if(!init){
				init = 1;
				source->power+=1;
				source->addToToughness(2);
			}
		}else{
			if (init){
				init = 0;
				source->power-=1;
				source->addToToughness(-2);
			}
		}
	}
};

//1309 Orcish Artilery
class AOrcishArtillery: public ADamager{
public:
	AOrcishArtillery(int _id,MTGCardInstance * card): ADamager(_id, card, NEW ManaCost(), 2){
	}

	int resolve(){
		ADamager::resolve();
		game->mLayers->stackLayer()->addDamage(source,source->controller(), 3);
		return 1;
	}
	
};

//1310 Orcish Oriflame
class AOrcishOriflame:public ListMaintainerAbility{
public:	
	int color;
	AOrcishOriflame(int _id, MTGCardInstance * _source):ListMaintainerAbility(_id, _source){
	}

	int canBeInList(MTGCardInstance * card){
		if (source->controller() == game->currentPlayer && game->currentPlayer->game->inPlay->hasCard(card) && card->attacker) return 1;
		return 0;
	}

	int added(MTGCardInstance * card){
		card->power += 1;
		return 1;
	}

	int removed(MTGCardInstance * card){
		card->power -= 1;
		return 1;
	}

};

//1334 Castle
class ACastle:public ListMaintainerAbility{
public:
	ACastle(int _id, MTGCardInstance * _source):ListMaintainerAbility(_id, _source){
	}

	int canBeInList(MTGCardInstance * card){
		if (source->controller()->game->inPlay->hasCard(card) && card->isACreature() && !card->isAttacker() && !card->tapped) return 1;
		return 0;
	}

	int added(MTGCardInstance * card){
		card->addToToughness(2);
		return 1;
	}

	int removed(MTGCardInstance * card){
		card->addToToughness(-2);
		return 1;
	}
};


//1351 Island Sanctuary
class AIslandSanctuary:public MTGAbility{
public:
	int initThisTurn;
	AIslandSanctuary(int _id, MTGCardInstance * _source):MTGAbility(_id, _source){
		initThisTurn = 0;
	}
	
	void Update(float dt){
		if (currentPhase == MTG_PHASE_UNTAP && game->currentPlayer == source->controller()) initThisTurn = 0;

		if (initThisTurn && currentPhase == MTG_PHASE_COMBATATTACKERS && game->currentPlayer != source->controller()){
			MTGGameZone *  zone = game->currentPlayer->game->inPlay;
			for (int i = 0; i < zone->nb_cards; i++){
				MTGCardInstance * card =  zone->cards[i];
				if (card->isAttacker() && !card->basicAbilities[FLYING] && !card->basicAbilities[ISLANDWALK]) card->attacker=0;
			}
		}
	}

	int isReactingToClick(MTGCardInstance * card){
		if (card==source && game->currentPlayer == card->controller() && currentPhase == MTG_PHASE_DRAW){
			Interruptible * action = game->mLayers->stackLayer()->_(-1);
			if (action->type == ACTION_DRAW) return 1;
		}
		return 0;
	}


	int reactToClick(MTGCardInstance * card){
		if (!isReactingToClick(card)) return 0;
		game->mLayers->stackLayer()->Remove(game->mLayers->stackLayer()->_(-1));
		initThisTurn = 1;
		return 1;
	}
};

//1352 Karma
class AKarma: public TriggeredAbility{
public:
	AKarma(int _id, MTGCardInstance * _source):TriggeredAbility(_id, _source){
	}

	int trigger(){
		if (newPhase != currentPhase && newPhase == MTG_PHASE_UPKEEP) return 1;
		return 0;
	}

	int resolve(){
		int totaldamage = 0;
		MTGGameZone *  zone = game->currentPlayer->game->inPlay;
		for (int i = 0; i < zone->nb_cards; i++){
			if (zone->cards[i]->hasType("swamp")) totaldamage++;;
		}
		if (totaldamage) game->mLayers->stackLayer()->addDamage(source,game->currentPlayer, totaldamage);
		return 1;
	}
};

//1355 Norther Paladin
class ANorthernPaladin:public TargetAbility{
public:
	ANorthernPaladin(int _id, MTGCardInstance * card):TargetAbility(_id, card){
		int _cost[] = {MTG_COLOR_WHITE, 2};
		cost = NEW ManaCost(_cost,1);
		tc = NEW TargetChooser();
	}

	int resolve(){
		MTGCardInstance * card = tc->getNextCardTarget();
		if (card->hasColor(MTG_COLOR_BLACK)){
			card->controller()->game->putInGraveyard(card);
			return 1;
		}
	return 0;
	}


};

//Sedge Troll
class ASedgeTroll:public MTGAbility{
public:
	int init;
	ASedgeTroll(int _id, MTGCardInstance * _source):MTGAbility(_id, _source){
		init = 0;
	}

	void Update(float dt){
		if (source->controller()->game->inPlay->hasType("swamp")){
			if(!init){
				init = 1;
				source->power+=1;
				source->addToToughness(1);
			}
		}else{
			if (init){
				init = 0;
				source->power-=1;
				source->addToToughness(-1);
			}
		}
	}
};

//Soul Net
class ASoulNet:public ActivatedAbility{
public:
	PutInGraveyard * latest;
	PutInGraveyard * newDead;
	ASoulNet(int _id, MTGCardInstance * card):ActivatedAbility(_id, card,0,0,0){
		int _cost[] = {MTG_COLOR_ARTIFACT, 1};
		cost = NEW ManaCost(_cost,1);
		latest = ((PutInGraveyard *) GameObserver::GetInstance()->mLayers->stackLayer()->getPrevious(NULL,ACTION_PUTINGRAVEYARD,RESOLVED_OK));
		newDead = latest;
	}

	int isReactingToClick(MTGCardInstance * card){
		newDead = ((PutInGraveyard *) GameObserver::GetInstance()->mLayers->stackLayer()->getPrevious(NULL,ACTION_PUTINGRAVEYARD,RESOLVED_OK));
		if (newDead && newDead != latest && newDead->card->isACreature())
			return ActivatedAbility::isReactingToClick(card);
		return 0;
	}
		int resolve(){
			latest = newDead;
			source->controller()->life++;
			return 1;
		}
};

//Sunglasses of Urza
class ASunglassesOfUrza:public ActivatedAbility{
public:
	ASunglassesOfUrza(int _id, MTGCardInstance * card):ActivatedAbility(_id, card,NEW ManaCost(),0,0){
		cost->add(MTG_COLOR_WHITE, 1);
	}


	int resolve(){
		source->controller()->getManaPool()->add(MTG_COLOR_RED, 1);
		return 1;
	}
};



//--------------Addon Abra------------------
//ShieldOfTheAge
class AShieldOfTheAge: public TargetAbility{
public:
	AShieldOfTheAge(int _id, MTGCardInstance * card):TargetAbility(_id,card,NEW DamageTargetChooser(card,_id),NEW ManaCost(),0,0){
		cost->add(MTG_COLOR_ARTIFACT,2);
	}

	int resolve(){
		Damage * damage = tc->getNextDamageTarget();
		if (!damage) return 0;
		game->mLayers->stackLayer()->Fizzle(damage);
		return 1;
	}
};

//2593 Thoughtleech
class AThoughtleech:public MTGAbility{
public:
	int nbIslandstapped;

	int countIslandsTapped(){
		int result = 0;
		MTGInPlay * inplay = source->controller()->opponent()->game->inPlay;
		for (int i = 0; i < inplay->nb_cards; i++){
			MTGCardInstance * card = inplay->cards[i];
			if (card->tapped && card->hasType("island")) result++;
		}
		return result;
	}

	AThoughtleech(int _id, MTGCardInstance * source):MTGAbility(_id, source){
		nbIslandstapped = countIslandsTapped();
	}

	void Update(float dt){
		int newcount = countIslandsTapped();
		for (int i=0; i < newcount - nbIslandstapped; i++){
			source->controller()->life++;
		}
		nbIslandstapped = newcount;
	}

};

//Minion of Leshrac
class AMinionofLeshrac: public TargetAbility{
public:
	int paidThisTurn;
	AMinionofLeshrac(int _id, MTGCardInstance * source):TargetAbility(_id, source, NEW CreatureTargetChooser(),0,1,0){
		paidThisTurn = 1;
	}

	void Update(float dt){
		if (newPhase != currentPhase && source->controller() == game->currentPlayer){
			if (newPhase == MTG_PHASE_UNTAP){
				paidThisTurn = 0;
			}else if( newPhase == MTG_PHASE_UPKEEP + 1 && !paidThisTurn){
				game->mLayers->stackLayer()->addDamage(source,source->controller(), 5);
				source->tapped = 1;
			}
		}
		TargetAbility::Update(dt);
	}

	int isReactingToClick(MTGCardInstance * card){
		if (currentPhase != MTG_PHASE_UPKEEP || paidThisTurn) return 0;
		return TargetAbility::isReactingToClick(card);
	}

	int resolve(){
		MTGCardInstance * card = tc->getNextCardTarget();
		if (card && card != source && card->controller() == source->controller()){
			card->controller()->game->putInGraveyard(card);
			paidThisTurn = 1;
			return 1;
		}
		return 0;
	}

};

//2703 Lost Order of Jarkeld
class ALostOrderofJarkeld:public ListMaintainerAbility{
public:	
	ALostOrderofJarkeld(int _id, MTGCardInstance * _source):ListMaintainerAbility(_id, _source){
	}

	int canBeInList(MTGCardInstance * card){
		if (card==source || (game->currentPlayer->game->inPlay->hasCard(card) && card->isACreature()) ) return 1;
		return 0;
	}

	int added(MTGCardInstance * card){
		source->power += 1;
		source->addToToughness(1);
		return 1;
	}

	int removed(MTGCardInstance * card){
		source->power -= 1;
		source->addToToughness(-1);
		return 1;
	}

};


#endif
