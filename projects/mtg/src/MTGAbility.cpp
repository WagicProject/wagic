#include "../include/debug.h"
#include "../include/MTGAbility.h"
#include "../include/ManaCost.h"
#include "../include/MTGGameZones.h"
#include "../include/AllAbilities.h"
#include "../include/Damage.h"
#include "../include/TargetChooser.h"
#include "../include/CardGui.h"
#include "../include/MTGDeck.h"

int AbilityFactory::destroyAllFromTypeInPlay(const char * type, MTGCardInstance * source, int bury){
	GameObserver * game = GameObserver::GetInstance();
	for (int i = 0; i < 2 ; i++){
		for (int j = 0; j < game->players[i]->game->inPlay->nb_cards; j++){
			MTGCardInstance * current =  game->players[i]->game->inPlay->cards[j];
			if (current->hasType(type)){
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

int AbilityFactory::destroyAllFromColorInPlay(int color, MTGCardInstance * source, int bury){
	GameObserver * game = GameObserver::GetInstance();
	for (int i = 0; i < 2 ; i++){
		for (int j = 0; j < game->players[i]->game->inPlay->nb_cards; j++){
			MTGCardInstance * current =  game->players[i]->game->inPlay->cards[j];
			if (current->hasColor(color)){
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
					Spell * spell = NEW Spell(card);
					p->game->putInZone(card,  zone, p->game->stack);
					spell->resolve();
					delete spell;
					return 1;
}


Trigger * AbilityFactory::parseTrigger(string magicText){
	int found = magicText.find("@");
	if (found == string::npos) return NULL;

	//Next Time...
	found = magicText.find("next");
	if (found != string::npos){
		for (int i = 0; i < NB_MTG_PHASES; i++){
			found = magicText.find(MTGPhaseCodeNames[i]);
			if (found != string::npos){
				return NEW TriggerNextPhase(i);
			}
		}
	}

	return NULL;
}

//Some basic functionalities that can be added automatically in the text file
int AbilityFactory::magicText(int id, Spell * spell, MTGCardInstance * card){
	int dryMode = 0;
	if (!spell) dryMode = 1;
	GameObserver * game = GameObserver::GetInstance();
	if (!card) card = spell->source;
	MTGCardInstance * target = card->target;
	if (!target) target = card;
	string magicText = card->magicText;
	if (card->alias && magicText.size() == 0){
		//An awful way to get access to the aliasedcard
		magicText = GameObserver::GetInstance()->players[0]->game->collection->getCardById(card->alias)->magicText;
	}
	string s;
	int size = magicText.size();
	if (size == 0) return 0;
	unsigned int found;
	int result = id;
	

	while (magicText.size()){
		found = magicText.find("\n");
		if (found != string::npos){ 
				s = magicText.substr(0,found);
				magicText = magicText.substr(found+1);
		}else{
			s = magicText;
			magicText = "";
		}
#if defined (WIN32) || defined (LINUX)
		char buf[4096];
		sprintf(buf, "AUTO ACTION: %s\n", s.c_str());
	OutputDebugString(buf);
#endif

		TargetChooser * tc = NULL;
		int doTap = 0;
		string lordType = "";

		Trigger * trigger = parseTrigger(s);
		//Dirty way to remove the trigger text (could get in the way)
		if (trigger){
			found = s.find(":");
			s = s.substr(found+1);
		}

		//Tap in the cost ?
		if (s.find("{t}") != string::npos) doTap = 1;

		//Target Abilities
		found = s.find("target(");
		if (found != string::npos){
			int end = s.find(")");
			string target = s.substr(found + 7,end - found - 7);
			TargetChooserFactory tcf;
			tc = tcf.createTargetChooser(target, card);

		}

		//Lord
		found = s.find("lord(");
		if (found != string::npos){
			if (dryMode) return BAKA_EFFECT_GOOD;
			unsigned int end = s.find(")", found+5);
			if (end != string::npos){ 
				lordType = s.substr(found+5,end-found-5).c_str();
			}
		}

		//Champion. Very basic, needs to be improved !
		found = s.find("champion(name:");
		if (found != string::npos){
			if (dryMode) return BAKA_EFFECT_GOOD;
			unsigned int end = s.find(")", found+14);
			if (end != string::npos){ 
				string type = s.substr(found+14,end-found-14).c_str();
				game->addObserver(NEW APlagueRats(id,card,type.c_str()));
				result++;
				continue;
			}
		}
	
		//Untapper (Ley Druid...)
		found = s.find("untap");
		if (found != string::npos){
			if (dryMode) return BAKA_EFFECT_GOOD;
			ManaCost * cost = ManaCost::parseManaCost(s);
			if (tc){
				game->addObserver(NEW AUntaper(id, card, cost, tc));
			}else{
				target->tapped = 0;
			}

			result++;
			continue;
		}

		//Tapper (icy manipulator)
		found = s.find("tap");
		if (found != string::npos){
			if (dryMode) return BAKA_EFFECT_GOOD;
			ManaCost * cost = ManaCost::parseManaCost(s);
			if (tc){
				game->addObserver(NEW ATapper(id, card, cost, tc));
			}else{
				target->tapped = 1;
			}

			result++;
			continue;
		}	

		//Regeneration
		found = s.find("}:regenerate");
		if (found != string::npos){
					if (dryMode) return BAKA_EFFECT_GOOD;
					ManaCost * cost = ManaCost::parseManaCost(s);

					if (lordType.size() > 0){
						game->addObserver(NEW ALord(id,card,lordType.c_str(),0,0,-1,cost));
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
		//Bury
		found = s.find("bury");
		if (found != string::npos){ 
					if (dryMode) return BAKA_EFFECT_BAD;
					if (tc){
						game->addObserver(NEW ABurier(id, card,tc));
					}else{
						target->controller()->game->putInGraveyard(target);
					}
					result++;
					continue;
		}
		//Destroy
		found = s.find("destroy");
		if (found != string::npos){ 
					if (dryMode) return BAKA_EFFECT_BAD;
					if (tc){
						game->addObserver(NEW ADestroyer(id, card,tc));
					}else{
						game->mLayers->stackLayer()->addPutInGraveyard(target);
					}
					result++;
					continue;
		}

		//Damage
		found = s.find("damage");
		if (found != string::npos){ 
					unsigned int start = s.find(":",found);
					unsigned int end = s.find(" ",start);
					int damage;
					ManaCost * cost = ManaCost::parseManaCost(s);
					if (end != string::npos){ 
						damage = atoi(s.substr(start+1,end-start-1).c_str());
					}else{
						damage = atoi(s.substr(start+1).c_str());
					}
					if (dryMode) return BAKA_EFFECT_BAD;
					if (tc){
						game->addObserver(NEW ADamager(id, card, cost, damage, tc,doTap));
					}else{
						delete cost;
						game->mLayers->stackLayer()->addDamage(card,spell->getNextDamageableTarget(), damage);
					}
					result++;
					continue;
		}

		//gain/lose life
		found = s.find("life");
		if (found != string::npos){ 
			unsigned int start = s.find(":",found);
			unsigned int end = s.find(" ",start);
			int life;
			ManaCost * cost = ManaCost::parseManaCost(s);
			if (end != string::npos){ 
				life = atoi(s.substr(start+1,end-start-1).c_str());
			}else{
				life = atoi(s.substr(start+1).c_str());
			}
			if (dryMode) return BAKA_EFFECT_GOOD;
			if (tc){
				//TODO ?
			}else{
				if (cost->getConvertedCost() == 0 && !doTap){
					delete cost;
					card->controller()->life+=life;
				}else{
					//TODO;
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
					ManaCost * cost = ManaCost::parseManaCost(s);
					if (end != string::npos){ 
						nbcards = atoi(s.substr(start+1,end-start-1).c_str());
					}else{
						nbcards = atoi(s.substr(start+1).c_str());
					}
					if (dryMode) return BAKA_EFFECT_GOOD;
					if (trigger){
						DrawEvent * action = NEW DrawEvent(card->controller(),nbcards);
						game->addObserver(NEW GenericTriggeredAbility(id, card,trigger,action));
					}else{
						if (tc){
							//TODO ?
						}else{
							if (cost->getConvertedCost() == 0){
								delete cost;
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
		found = s.find("/");
		if (found != string::npos){ 
					unsigned int start = s.find(":");
					if (start == string::npos) start = -1;
					int power = atoi(s.substr(start+1,size-found).c_str());
					unsigned int end = s.find(" ",start);
					int toughness;
					if (end != string::npos){ 
						toughness = atoi(s.substr(found+1,end-found-1).c_str());
					}else{
						toughness = atoi(s.substr(found+1).c_str());
					}
					if (dryMode){
						if (power >=0 && toughness >= 0 ) return BAKA_EFFECT_GOOD;
						return BAKA_EFFECT_BAD;
					}
					int limit = 0;
					unsigned int limit_str = s.find("limit:");
					if (limit_str != string::npos){
						limit = atoi(s.substr(limit_str+6).c_str());
					}		 
					ManaCost * cost = ManaCost::parseManaCost(s);

					if (lordType.size() > 0){
						game->addObserver(NEW ALord(id,card,lordType.c_str(),power,toughness));
					}else{
						if(tc){
							game->addObserver(NEW ATargetterPowerToughnessModifierUntilEOT(id, card,power,toughness, cost, tc));
						}else{
							if (cost->getConvertedCost() == 0){
								delete cost;
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
					result++;
					continue;
		}

		//Mana Producer
		found = s.find("add");
		if (found != string::npos){ 
			if (dryMode) return BAKA_EFFECT_GOOD;
			ManaCost * cost = ManaCost::parseManaCost(s.substr(0,found));
			ManaCost * output = ManaCost::parseManaCost(s.substr(found));
			if (cost->getConvertedCost()){
				game->addObserver(NEW AManaProducer(id, target, output, cost));
			}else{
				delete cost;
				if (doTap){
					game->addObserver(NEW AManaProducer(id, target, output));
				}else{
					card->controller()->getManaPool()->add(output);
					delete output;
				}

			}
			result++;
			continue;
		}

		//Gain/loose Ability
		for (int j = 0; j < NB_BASIC_ABILITIES; j++){
			found = s.find(MTGBasicAbilities[j]);
			if (found!= string::npos){
				int modifier = 1;
				if (found > 0 && s[found-1] == '-') modifier = 0;
				if (dryMode){
					if (j == DEFENDER){
						if (modifier == 1) return BAKA_EFFECT_BAD;
						return BAKA_EFFECT_GOOD;
					}else{
						if (modifier == 1) return BAKA_EFFECT_GOOD;
						return BAKA_EFFECT_BAD;
					}
				}
					ManaCost * cost = ManaCost::parseManaCost(s);

					if (lordType.size() > 0){
						game->addObserver(NEW ALord(id,card,lordType.c_str(),0,0,j));
					}else{

						if (tc){
							game->addObserver(NEW ABasicAbilityModifierUntilEOT(id, card, j, cost,tc, modifier));
						}else{
							if (cost->getConvertedCost() == 0){
								delete cost;
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
	return result;


}

void AbilityFactory::addAbilities(int _id, Spell * spell){
	MTGCardInstance * card = spell->source;
	if (spell->cursor==1) card->target =  spell->getNextCardTarget();
	_id = magicText(_id, spell); 
	int putSourceInGraveyard = 0; //For spells that are not already InstantAbilities;


	GameObserver * game = GameObserver::GetInstance(); 
  int id = card->model->getId();
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
	case 106525: //Ascendant Evincar
		{
			game->addObserver(NEW AColorLord(_id, card,MTG_COLOR_BLACK,-1,1,1));
			game->addObserver(NEW AColorLord(_id + 1, card,0,MTG_COLOR_BLACK,-1,-1));
			break;
		}

	case 1096: //Basalt Monolith
		{
			int cost[] = {MTG_COLOR_ARTIFACT, 3};
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
			game->addObserver( NEW ALifeZoneLink(_id ,card, MTG_PHASE_UPKEEP, 4));
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
			int cost[] = {MTG_COLOR_ARTIFACT, 1};
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
				card->target->setColor(MTG_COLOR_RED, 1);
			}else{
				Spell * starget = spell->getNextSpellTarget();
				starget->source->setColor(MTG_COLOR_RED, 1);
			}
			break;
		}
	case 1335: //Circle of protection : black
		{
			game->addObserver(NEW ACircleOfProtection( _id,card, MTG_COLOR_BLACK));
			break;
		}
	case 1336: //Circle of protection : blue
		{
			game->addObserver(NEW ACircleOfProtection( _id,card, MTG_COLOR_BLUE));
			break;
		}
	case 1337: //Circle of protection : green
		{
			game->addObserver(NEW ACircleOfProtection( _id,card, MTG_COLOR_GREEN));
			break;
		}
	case 1338: //Circle of protection : red
		{
			game->addObserver(NEW ACircleOfProtection( _id,card, MTG_COLOR_RED));
			break;
		}
	case 1339: //Circle of protection : white
		{
			game->addObserver(NEW ACircleOfProtection( _id,card, MTG_COLOR_WHITE));
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
			int cost[] = {MTG_COLOR_BLUE, 1};
			ASpellCastLife* ability = NEW ASpellCastLife(_id, card, MTG_COLOR_WHITE,NEW ManaCost(cost,1) , 1);
			game->addObserver(ability);
			break;
		}
	case 1151: //Deathgrip
		{
			int _cost[] = {MTG_COLOR_BLACK, 2};
			game->addObserver(NEW ASpellCounterEnchantment(_id, card, NEW ManaCost(_cost, 1),MTG_COLOR_GREEN));
			break;
		}
	case 1152: //Deathlace
		{
			if (card->target){
				card->target->setColor(MTG_COLOR_BLACK, 1);
			}else{
				Spell * starget = spell->getNextSpellTarget();
				starget->source->setColor(MTG_COLOR_BLACK, 1);
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
			int cost[] = {MTG_COLOR_ARTIFACT, 1};
			ASpellCastLife* ability = NEW ASpellCastLife(_id, card, MTG_COLOR_RED,NEW ManaCost(cost,1) , 1);
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
			int cost[] = {MTG_COLOR_ARTIFACT, 1};
			ASpellCastLife* ability = NEW ASpellCastLife(_id, card, MTG_COLOR_WHITE,NEW ManaCost(cost,1) , 1);
			game->addObserver(ability);
			break;
		}
	case 1115: //Ivory Tower
		{
			game->addObserver(NEW ALifeZoneLink(_id ,card, MTG_PHASE_UPKEEP, 4, 1, 1));
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
			int _cost[] = {MTG_COLOR_GREEN, 2};
			game->addObserver(NEW ASpellCounterEnchantment(_id, card, NEW ManaCost(_cost, 1),MTG_COLOR_BLACK));
			break;
		}
	case 1257: //Lifelace
		{
			if (card->target){
				card->target->setColor(MTG_COLOR_GREEN, 1);
			}else{
				Spell * starget = spell->getNextSpellTarget();
				starget->source->setColor(MTG_COLOR_GREEN, 1);
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
			int output[] = {MTG_COLOR_ARTIFACT, 3};
			game->addObserver(NEW AManaProducer(_id,card,NEW ManaCost(output,1)));
			int cost[] = {MTG_COLOR_ARTIFACT, 4};
			game->addObserver(NEW AUntapManaBlocker(_id+1, card, NEW ManaCost(cost,1)));
			game->addObserver(NEW ARegularLifeModifierAura(_id+2, card, card, MTG_PHASE_DRAW, -1, 1));
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
				card->target->setColor(MTG_COLOR_WHITE, 1);
			}else{
				Spell * starget = spell->getNextSpellTarget();
				starget->source->setColor(MTG_COLOR_WHITE, 1);
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
			game->addObserver( NEW ALifeZoneLink(_id ,card, MTG_PHASE_UPKEEP, -3));
			break;
		}
	case 1140: //Throne of bones
		{
			int cost[] = {MTG_COLOR_ARTIFACT, 1};
			ASpellCastLife* ability = NEW ASpellCastLife(_id, card, MTG_COLOR_BLACK,NEW ManaCost(cost,1) , 1);
			game->addObserver(ability);
			break;
		}
	case 1142: //Wooden Sphere
		{
			int cost[] = {MTG_COLOR_ARTIFACT, 1};
			ASpellCastLife* ability = NEW ASpellCastLife(_id, card, MTG_COLOR_GREEN,NEW ManaCost(cost,1) , 1);
			game->addObserver(ability);
			break;
		}
	case 1143: //Animate Dead
		{
			game->addObserver(NEW AAnimateDead(_id, card, card->target));
			break;
		}
	case 1144: //Bad moon
		{
			game->addObserver(NEW ABadMoon(_id,card));
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
	case 1170: //Nightmare
		{
			game->addObserver(NEW ANightmare(_id, card));
			break;
		}
	case 1171: //Paralysis
		{
			int cost[] = {MTG_COLOR_ARTIFACT, 4};
			game->addObserver(NEW AUntapManaBlocker(_id, card,card->target, NEW ManaCost(cost,1)));
			card->target->tapped = 1;
			break;
		}
	case 1172: //Pestilence
		{
			game->addObserver(NEW APestilence(_id, card));
			break;
		}
	/*case 1173: //Plague Rats
		{
			game->addObserver(NEW APlagueRats(_id, card, "Plague Rats"));
			break;
		}
	*/
	case 1174: //Raise Dead
		{
			MTGPlayerCards * zones = game->currentlyActing()->game;
			zones->putInZone(card->target,zones->graveyard,zones->hand);
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
			game->addObserver(NEW ARegularLifeModifierAura(_id, card, card->target, MTG_PHASE_UPKEEP, -1));
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
			game->addObserver(NEW ATakeControlAura(_id, card, card->target));
			break;
		}

	case 1200 : //Feedback
		{
			game->addObserver(NEW AWanderlust(_id, card, card->target));
			break;
		}
	case 129601: //Icy Manipulator
		{
			int cost[] = {MTG_COLOR_ARTIFACT, 1};
			TypeTargetChooser * tc = new TypeTargetChooser("artifact",card);
			tc->addType("land");
			tc->addType("creature");
			game->addObserver(NEW ATapper(_id,card,NEW ManaCost(cost, 1),tc));
			break;
		}

	case 1203: //Island Fish
		{
			int cost[] = {MTG_COLOR_BLUE, 3};
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
	case 1315: //Sedge Troll
		{
			game->addObserver( NEW ASedgeTroll(_id, card));
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
			game->addObserver(NEW APowerToughnessModifierRegularCounter(_id, card, card->target, MTG_PHASE_UPKEEP, -1, -1));
			break;
		}
	case 1229: //Unsummon
		{
			MTGPlayerCards * zones = card->target->controller()->game;
			zones->putInZone(card->target,zones->inPlay,zones->hand);
			break;
			
		}
	case 1235: //Aspect of Wolf
		{
			game->addObserver(NEW AAspectOfWolf(_id, card, card->target));
			break;
		}
	case 1236: //Birds of Paradise
		{
			for (int i = MTG_COLOR_GREEN; i <= MTG_COLOR_WHITE; i++){
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
					if (current->basicAbilities[FLYING] && current->isACreature()){
						game->mLayers->stackLayer()->addDamage(card, current, x);
					}
				}
			}
			break;
		}
	case 1262: //Regeneration
		{
			int cost[] = {MTG_COLOR_GREEN, 1};
			game->addObserver(NEW AStandardRegenerate(_id,card,card->target,NEW ManaCost(cost,1)));
			break;
		}
	case 1263: //Regrowth
		{
			MTGPlayerCards * zones = game->currentlyActing()->game;
			zones->putInZone(card->target,zones->graveyard,zones->hand);
			break;
		}
	case 1266: //stream of life
		{
			int x = spell->cost->getConvertedCost() - 1; //TODO Improve that !
			spell->getNextPlayerTarget()->life += x;
			break;
		}
	case 1270: //tranquility
		{
			destroyAllFromTypeInPlay("enchantment", card);
			break;
		}
	case 1271: //Tsunami
		{
			destroyAllFromTypeInPlay("island", card);
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
	case 1278: //Web
		{
			game->addObserver(NEW APowerToughnessModifier(_id, card, card->target, 0,2));
			game->addObserver(NEW ABasicAbilityModifier(_id + 1, card, card->target, REACH));
			break;
		}
	case 1280: //Atog
		{
			game->addObserver(NEW AAtog(_id, card));
			break;
		}
	case 1285: //Dwarven Warriors{
		{
			CreatureTargetChooser * tc = NEW CreatureTargetChooser(card);
			tc->maxpower = 2;
			game->addObserver(NEW ABasicAbilityModifierUntilEOT(_id, card, UNBLOCKABLE, NULL,tc));
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
					if (!current->basicAbilities[FLYING] && current->isACreature()){
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
	case 1293: //FlashFires
		{
			destroyAllFromTypeInPlay("plains", card);
			break;
		}
	case 1301: // Keldon Warlord
		{
			game->addObserver(NEW AKeldonWarlord(_id, card));
			break;
		}
	case 1302: //Kird Ape
		{
			game->addObserver(NEW AKirdApe(_id, card));
			break;
		}
	case 1309: //Orcish Artillery
		{
			game->addObserver(NEW AOrcishArtillery(_id, card));
			break;
		}
	case 1310: //Orcish Oriflame
		{
			game->addObserver(NEW AOrcishOriflame(_id, card));
			break;
		}
	case 1317: //ShatterStorm
		{
			destroyAllFromTypeInPlay("artifact", card, 1);
			break;
		}
	case 1326: //Wheel of fortune
		{
			for (int i = 0; i < 2; i++){
				MTGLibrary * library = game->players[i]->game->library;
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
	case 1328: //Armageddon
		{
			destroyAllFromTypeInPlay("land", card);
			break;
		}
	case 1331: //Black Ward
		{
			game->addObserver(NEW AProtectionFrom( _id,card, card->target, MTG_COLOR_BLACK));
			break;
		}
	case 1333: //Blue  Ward
		{
			game->addObserver(NEW AProtectionFrom( _id,card, card->target, MTG_COLOR_BLUE));
			break;
		}
	case 1334: //Castle
		{
			game->addObserver(NEW ACastle(_id,card));
			break;
		}
	case 1238: //Cockatrice
		{
			game->addObserver(NEW AOldSchoolDeathtouch(_id,card));
			break;
		}
	case 1341: //Crusade:
		{
			game->addObserver(NEW ABadMoon(_id,card, MTG_COLOR_WHITE));
			break;

		}
	case 1346: //Green Ward
		{
			game->addObserver(NEW AProtectionFrom( _id,card, card->target, MTG_COLOR_GREEN));
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
			game->addObserver(NEW AProtectionFrom( _id,card, card->target, MTG_COLOR_RED));
			break;
		}
	case 1360: //Resurrection
		{
			Player * p = card->controller();
			AbilityFactory af;
			af.putInPlayFromZone(card->target, p->game->graveyard,  p);
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

	case 1367: //Sword to Plowshares
		{
			card->target->controller()->life+= card->target->power;
			card->target->controller()->game->inPlay->removeCard(card->target);
			break;
		}
	case 1182: //Terror
		{
			if (card->target->hasColor(MTG_COLOR_BLACK) || card->target->hasSubtype("artifact")){
			}else{
				card->target->controller()->game->putInGraveyard(card->target);
			}
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
				card->target->setColor(MTG_COLOR_BLUE, 1);
			}else{
				Spell * starget = spell->getNextSpellTarget();
				starget->source->setColor(MTG_COLOR_BLUE, 1);
			}
			break;
		}
	case 1371: //White Ward
		{
			game->addObserver(NEW AProtectionFrom( _id,card, card->target, MTG_COLOR_WHITE));
			break;
		}
	case 1372: //Wrath of God
		{
			destroyAllFromTypeInPlay("creature", card); //TODO -> bury !!!
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

	case 1818: //Tivadar's Crusade
		{
			destroyAllFromTypeInPlay("goblin", card);
			break;
		}	

//Addons Legends
	case 1470: //Acid Rain
		{
			destroyAllFromTypeInPlay("forest", card);
			break;
		}
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
	case 2631: //Jokulhaups
		{
			destroyAllFromTypeInPlay("artifact", card);
			destroyAllFromTypeInPlay("creature", card);
			destroyAllFromTypeInPlay("land", card);
			break;
		}

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
			if (card->target->hasColor(MTG_COLOR_BLACK)){
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
			game->currentlyActing()->getManaPool()->add(MTG_COLOR_BLACK, mana);
			break;
		}
	case 2606: //Anarchy
		{
			destroyAllFromColorInPlay(MTG_COLOR_WHITE, card);
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
			game->currentlyActing()->getManaPool()->add(MTG_COLOR_ARTIFACT, spoil);
			game->currentlyActing()->life+= spoil;
			break;
		}
	case 2435: //Whalebone Glider
		{
			int cost[] = {MTG_COLOR_ARTIFACT,2};
			CreatureTargetChooser * tc = NEW CreatureTargetChooser(card);
			tc->maxpower = 3;
			game->addObserver(NEW ABasicAbilityModifierUntilEOT(_id, card, FLYING, NEW ManaCost(cost,1),tc));
			break;	
		}
	case 2393: //Aegis of the Meek work but work also for 0/1 creatures... :D
		{
			int cost[] = {MTG_COLOR_ARTIFACT,1};
			CreatureTargetChooser * tc = NEW CreatureTargetChooser(card);
			tc->maxpower = 1;
			tc->maxtoughness =1;
			game->addObserver(NEW ATargetterPowerToughnessModifierUntilEOT(id, card, 1,2, NEW ManaCost(cost,1),tc));
			break;	
		}
	case 2703: // Lost Order of Jarkeld
		{
			game->addObserver(NEW ALostOrderofJarkeld(_id, card));
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
	if (card->basicAbilities[LIFELINK]){
		ALifeLink * ability = NEW ALifeLink(_id, card);
		game->addObserver(ability);
	}

	for (int i=PROTECTIONGREEN; i <= PROTECTIONWHITE; i++){
		if (card->basicAbilities[i]){
			game->addObserver(NEW AProtectionFrom(_id, card, card, i - PROTECTIONGREEN + MTG_COLOR_GREEN));
		}
	}

	if (card->basicAbilities[EXALTED]){
		game->addObserver(NEW AExalted(_id, card));
	}
	
	// Tested works the first r10 did not function because of the mistake in the array of the definition 
	if (card->basicAbilities[FORESTHOME]){
		game->addObserver(NEW AStrongLandLinkCreature(_id, card, "forest"));
	}
	if (card->basicAbilities[ISLANDHOME]){
		game->addObserver(NEW AStrongLandLinkCreature(_id, card, "island"));
	}
	if (card->basicAbilities[MOUNTAINHOME]){
		game->addObserver(NEW AStrongLandLinkCreature(_id, card,"moutain"));
	}
	if (card->basicAbilities[SWAMPHOME]){
		game->addObserver(NEW AStrongLandLinkCreature(_id, card,"swamp"));
	}
	if (card->basicAbilities[PLAINSHOME]){
		game->addObserver(NEW AStrongLandLinkCreature(_id, card,"plains"));
	}

	// New Abilities Flanking and Rampage

	if (card->basicAbilities [RAMPAGE1]){
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
		zones->putInGraveyard(card);
	}
}

MTGAbility::MTGAbility(int id, MTGCardInstance * card):ActionElement(id){
	game = GameObserver::GetInstance();
  source = card;
	target = card;
}

MTGAbility::MTGAbility(int id, MTGCardInstance * _source,Damageable * _target ):ActionElement(id){
	game = GameObserver::GetInstance();
  source = _source;
	target = _target;
}

MTGAbility::~MTGAbility(){
  
}

//returns 1 if this ability needs to be removed from the list of active abilities
int MTGAbility::testDestroy(){
  if (!game->isInPlay(source)){
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

ActivatedAbility::ActivatedAbility(int id, MTGCardInstance * card, ManaCost * _cost, int _playerturnonly,int tap):MTGAbility(id,card), cost(_cost), playerturnonly(_playerturnonly), needsTapping(tap){
}


int ActivatedAbility::isReactingToClick(MTGCardInstance * card){
		Player * player = game->currentPlayer;
		if (!playerturnonly) player = game->currentlyActing();
		if (card == source && (!cost || player->getManaPool()->canAfford(cost)) && source->controller()==player && (!needsTapping || (!source->isTapped() && !source->hasSummoningSickness())) && player==game->currentlyActing())
			return 1;
		return 0;
}

int ActivatedAbility::reactToClick(MTGCardInstance * card){
	if (!isReactingToClick(card)) return 0;
	if (needsTapping) source->tapped = 1;
	if (cost) game->currentlyActing()->getManaPool()->pay(cost);
	fireAbility();
	return 1;

}

int ActivatedAbility::reactToTargetClick(Targetable * object){
	if (!isReactingToTargetClick(object)) return 0;
	if (needsTapping) source->tapped = 1;
	if (cost) game->currentlyActing()->getManaPool()->pay(cost);
	fireAbility();
	return 1;

}


ActivatedAbility::~ActivatedAbility(){
	if (cost) delete cost;
}

//

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
			waitingForAnswer = 0;
			ActivatedAbility::reactToClick(source);
		}
	}
}

int TargetAbility::reactToTargetClick(Targetable * object){
	if (object->typeAsTarget() == TARGET_CARD) return reactToClick((MTGCardInstance *)object);
	if (waitingForAnswer){
		tc->toggleTarget(object);
		return 1;
	}
	return 0;
}


int TargetAbility::reactToClick(MTGCardInstance * card){
	if (!waitingForAnswer) {
		if (isReactingToClick(card)){
			waitingForAnswer = 1;
			tc->initTargets();
		}
	}else{
		if (card == source){
			if (tc->targetsReadyCheck() == TARGET_OK){
				waitingForAnswer = 0;
				ActivatedAbility::reactToClick(source);
			}
		}else{
			tc->toggleTarget(card); 
		}
	}
	return 1;
}

void TargetAbility::Render(){
	//TODO
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
		if (newPhase != currentPhase && newPhase == MTG_PHASE_UNTAP) return 1;
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
				MTGGameZone * zones[] = {p->game->inPlay};
				for (int k = 0; k < 1; k++){
					MTGGameZone * zone = zones[k];
					if (zone->hasCard(card)){
						doDelete = 0;
						break;
					}
				}
			}
			if (doDelete || !canBeInList(card)){
#if defined (WIN32) || defined (LINUX)
OutputDebugString("DELETE FRO LISTMAINTAINER\n");
#endif
				cards.erase(card);
				removed(card);
			}
		}
		for (int i = 0; i < 2; i++){
			Player * p = game->players[i];
			MTGGameZone * zones[] = {p->game->inPlay};
			for (int k = 0; k < 1; k++){
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
		map<MTGCardInstance *,bool>::iterator it;

		for ( it=cards.begin() ; it != cards.end(); it++ ){
			removed((*it).first);
		}
		cards.clear();
		return 1;
	}


	/* An attempt to globalize triggered abilities as much as possible */

	MTGAbilityBasicFeatures::MTGAbilityBasicFeatures(){
		game = GameObserver::GetInstance();
	}
	MTGAbilityBasicFeatures::MTGAbilityBasicFeatures(MTGCardInstance * _source, Damageable * _target):source(_source),target(_target){
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
		newPhase = -1;
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




	DrawEvent::DrawEvent(Player * _player, int _nbcards):TriggeredEvent(),player(_player),nbcards(_nbcards){
	}

	int DrawEvent::resolve(){
		game->mLayers->stackLayer()->addDraw(player,nbcards);
		return nbcards;
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

		t->init(_source,_target);
		te->init(_source,_target);
		if (dc) dc->init(_source,_target);
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
