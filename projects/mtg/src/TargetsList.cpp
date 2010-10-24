#include "PrecompiledHeader.h"

#include "TargetsList.h"
#include "Player.h"
#include "MTGCardInstance.h"
#include "Damage.h"
#include "ActionStack.h"

TargetsList::TargetsList(){
  cursor = 0;
}

TargetsList::TargetsList(Targetable * _targets[], int nbtargets){
  for (int i = 0; i < nbtargets; i++){
    targets[i] = _targets[i];
  }
  cursor = nbtargets;
}

int TargetsList::addTarget(Targetable * target){
  if (!alreadyHasTarget(target)){
    targets[cursor] = target;
    cursor++;
    return 1;
  }
  return 0;

}


int TargetsList::alreadyHasTarget(Targetable * target){
  for (int i=0; i<cursor; i++){
    if (targets[i] == target) return 1;
  }
  return 0;
}

int TargetsList::removeTarget(Targetable * target){
  for (int i=0; i<cursor; i++){
    if (targets[i] == target) {
      targets[i] = targets[cursor];
      targets[cursor] = NULL;
      cursor--;
    }
  }

  return 0;
}


int TargetsList::toggleTarget(Targetable * target){
  if (alreadyHasTarget(target)){

    return removeTarget(target);
  }else{

    return addTarget(target);
  }
}


Targetable * TargetsList::getNextTarget(Targetable * previous , int type){
  int found = 0;
  if (!previous) found = 1;
  for (int i = 0; i < cursor; i++){
    if (found && (type == -1 || targets[i]->typeAsTarget() == type)){
      return (targets[i]);
    }
    if (targets[i] == previous) found = 1;
  }
  return NULL;
}

MTGCardInstance * TargetsList::getNextCardTarget(MTGCardInstance * previous){
  return ((MTGCardInstance *)getNextTarget(previous, TARGET_CARD));
}


Player * TargetsList::getNextPlayerTarget(Player * previous){
  return ((Player *)getNextTarget(previous, TARGET_PLAYER));
}


Interruptible * TargetsList::getNextInterruptible(Interruptible * previous, int type){
  int found = 0;
  if (!previous) found = 1;
  for (int i = 0; i < cursor; i++){
    if (found && targets[i]->typeAsTarget() == TARGET_STACKACTION){
      Interruptible * action = (Interruptible *) targets[i];
      if (action->type==type){
	return action;
      }
    }
    if (targets[i] == previous) found = 1;
  }
  return NULL;
}

Spell * TargetsList::getNextSpellTarget(Spell * previous){
  Spell * spell = (Spell *) getNextInterruptible(previous, ACTION_SPELL);
  return spell;
}

//How about DAMAGESTacks ??
Damage * TargetsList::getNextDamageTarget(Damage * previous){
  Damage * damage = (Damage * ) getNextInterruptible(previous, ACTION_DAMAGE);
  return damage;
}

Damageable * TargetsList::getNextDamageableTarget(Damageable * previous){
  int found = 0;
  if (!previous) found = 1;
  for (int i = 0; i < cursor; i++){

    if (targets[i]->typeAsTarget() == TARGET_PLAYER){
      if (found){
	return ((Player *) targets[i]);
      }else{
	if ((Player *)targets[i] == previous) found = 1;
      }
    }else if(targets[i]->typeAsTarget() == TARGET_CARD){
      if (found){
	return ((MTGCardInstance *) targets[i]);
      }else{
	if ((MTGCardInstance *)targets[i] == previous) found = 1;
      }
    }
  }
  return NULL;
}
