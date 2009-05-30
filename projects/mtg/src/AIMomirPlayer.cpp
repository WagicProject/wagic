#include "../include/config.h"
#include "../include/AIMomirPlayer.h"
#include "../include/CardDescriptor.h"
#include "../include/DamageResolverLayer.h"
#include "../include/DamagerDamaged.h"
#include "../include/AIStats.h"
#include "../include/AllAbilities.h"


MTGAbility * AIMomirPlayer::momirAbility = NULL;

AIMomirPlayer::AIMomirPlayer(MTGPlayerCards * _deck, char * file, char * avatarFile): AIPlayerBaka(_deck,file, avatarFile){
  momirAbility = NULL;
  agressivity = 100;
}

int AIMomirPlayer::getEfficiency(AIAction * action){


int efficiency = AIPlayerBaka::getEfficiency(action);

GameObserver * g = GameObserver::GetInstance();
if (g->getCurrentGamePhase() < Constants::MTG_PHASE_FIRSTMAIN) return 0;
 return efficiency;
}

MTGAbility * AIMomirPlayer::getMomirAbility(){
   if (momirAbility) return momirAbility;

   GameObserver * g = GameObserver::GetInstance();
   momirAbility = g->mLayers->actionLayer()->getAbility(MTGAbility::MOMIR);
   return momirAbility;
}

int AIMomirPlayer::momir(){
 if (!game->hand->nb_cards) return 0; //nothing to discard :/
 int result = 0;
 int opponentCreatures = getCreaturesInfo(opponent(), INFO_NBCREATURES);
 int myCreatures = getCreaturesInfo(this, INFO_NBCREATURES );
 getPotentialMana();
 int converted = potentialMana->getConvertedCost();
 int efficiency = 100;
 int chance = 1 + (rand() % 100);
 if (converted == 5 &&  myCreatures > opponentCreatures && game->hand->nb_cards<4) efficiency = 5 ; //Strategy: skip 5 drop
 if (converted == 7 &&  myCreatures > opponentCreatures && game->hand->nb_cards<2) efficiency = 50; //Strategy: 7 drops have bad upkeep costs and the AI doesn't handle those right now...
 if (converted > 8 ) converted = 8;
 if (converted == 8) efficiency = 100 - (myCreatures-opponentCreatures);

 if (efficiency >= chance){
   int _cost[] = {Constants::MTG_COLOR_ARTIFACT,converted};
   ManaCost * cost = NEW ManaCost(_cost);
   MTGAbility * ability = getMomirAbility();
   MTGCardInstance * card = game->hand->cards[0];
   if (ability->isReactingToClick(card,cost)){
     tapLandsForMana(potentialMana,cost);
     AIAction * a = NEW AIAction(ability,card);
     clickstream.push(a);
    result = 1;
   }
   delete cost;
 }
 return result;
}

int AIMomirPlayer::computeActions(){
//Part of the strategy goes here. When should we put a land into play ?
/*
Another gift from Alex Majlaton on my first day playing Momir, and it has served me well ever since. It goes a little something like this: (a) if you are on the play, hit your Two through Four, skip your Five, and then hit all the way to Eight; (b) if you are on the draw and your opponent skips his One, you make Two through Eight; (c) if you are on the draw and your opponent hits a One, you match him drop-for-drop for the rest of the game.

You skip your Five on the play because it is the weakest drop. There are plenty of serviceable guys there, but very few bombs compared to other drops
the general rule is this: if you want to get to Eight, you have to skip two drops on the play and one drop on the draw.
*/
  GameObserver * g = GameObserver::GetInstance();
  Player * p = g->currentPlayer;
  if (!(g->currentlyActing() == this)) return 0;
  if (chooseTarget()) return 1;
  int currentGamePhase = g->getCurrentGamePhase();
  if (g->isInterrupting == this){ // interrupting
    selectAbility();
    return 1;
  }else if (p == this && g->mLayers->stackLayer()->count(0,NOT_RESOLVED) == 0){ //standard actions
    CardDescriptor cd;
    MTGCardInstance * card = NULL;
    //No mana, try to get some
    getPotentialMana();

    switch(currentGamePhase){
    case Constants::MTG_PHASE_FIRSTMAIN:
      if (canPutLandsIntoPlay && (potentialMana->getConvertedCost() <8 || game->hand->nb_cards > 1) ){
	//Attempt to put land into play
	cd.init();
	cd.setColor(Constants::MTG_COLOR_LAND);
	card = cd.match(game->hand);
	if (card){
	  MTGAbility * putIntoPlay = g->mLayers->actionLayer()->getAbility(MTGAbility::PUT_INTO_PLAY);
	  AIAction * a = NEW AIAction(putIntoPlay,card); //TODO putinplay action
	  clickstream.push(a);
	  return 1;
	}
      }
      momir();
      return 1;
      break;
    case Constants::MTG_PHASE_SECONDMAIN:
      selectAbility();
      return 1;
      break;
    default:
      return AIPlayerBaka::computeActions();
      break;
    }
  }
  return AIPlayerBaka::computeActions();
}

/*
int AIPlayerBaka::computeActions(){
  GameObserver * g = GameObserver::GetInstance();
  Player * p = g->currentPlayer;
  if (!(g->currentlyActing() == this)) return 0;
  if (chooseTarget()) return 1;
  int currentGamePhase = g->getCurrentGamePhase();
  if (g->isInterrupting == this){ // interrupting
    selectAbility();
    return 1;
  }else if (p == this && g->mLayers->stackLayer()->count(0,NOT_RESOLVED) == 0){ //standard actions
    CardDescriptor cd;
    MTGCardInstance * card = NULL;
    switch(currentGamePhase){
    case Constants::MTG_PHASE_FIRSTMAIN:
    case Constants::MTG_PHASE_SECONDMAIN:
      if (canPutLandsIntoPlay){
	      //Attempt to put land into play
	      cd.init();
	      cd.setColor(Constants::MTG_COLOR_LAND);
	      card = cd.match(game->hand);
	      if (card){
          AIAction * a = NEW AIAction(card);
	        clickstream.push(a);
          return 1;
	      }
      }

	    //No mana, try to get some
	    getPotentialMana();
	    if (potentialMana->getConvertedCost() > 0){


	      //look for the most expensive creature we can afford
	      nextCardToPlay = FindCardToPlay(potentialMana, "creature");
	      //Let's Try an enchantment maybe ?
	      if (!nextCardToPlay) nextCardToPlay = FindCardToPlay(potentialMana, "enchantment");
	      if (!nextCardToPlay) nextCardToPlay = FindCardToPlay(potentialMana, "artifact");
	      if (!nextCardToPlay) nextCardToPlay = FindCardToPlay(potentialMana, "instant");
	      if (!nextCardToPlay) nextCardToPlay = FindCardToPlay(potentialMana, "sorcery");
	      if (nextCardToPlay){
#if defined (WIN32) || defined (LINUX)
          char buffe[4096];
	        sprintf(buffe, "Putting Card Into Play: %s", nextCardToPlay->getName());
	        OutputDebugString(buffe);
#endif

	        tapLandsForMana(potentialMana,nextCardToPlay->getManaCost());
          AIAction * a = NEW AIAction(nextCardToPlay);
	        clickstream.push(a);
          return 1;
        }else{
          selectAbility();
        }
      }else{
        selectAbility();
      }
      break;
    case Constants::MTG_PHASE_COMBATATTACKERS:
      chooseAttackers();
      break;
    default:
      selectAbility();
      break;
    }
  }else{
    switch(currentGamePhase){
    case Constants::MTG_PHASE_COMBATBLOCKERS:
      chooseBlockers();
      break;
    default:
      break;
    }
    return 1;
  }
  return 1;
};
*/
