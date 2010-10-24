#include "PrecompiledHeader.h"

#include "ReplacementEffects.h"
#include "MTGCardInstance.h"
#include "TargetChooser.h"



REDamagePrevention::REDamagePrevention(MTGAbility * source, TargetChooser *tcSource, TargetChooser *tcTarget, int damage, bool oneShot, int typeOfDamage):source(source), tcSource(tcSource), tcTarget(tcTarget), damage(damage), oneShot(oneShot), typeOfDamage(typeOfDamage){
}

WEvent * REDamagePrevention::replace (WEvent *event){
  if (!event) return event;
  if (!damage) return event;
  WEventDamage * e = dynamic_cast<WEventDamage*>(event);
  if (!e) return event;
  Damage *d = e->damage;
  if (d->typeOfDamage != typeOfDamage && typeOfDamage != DAMAGE_ALL_TYPES) return event;
  if ((!tcSource || tcSource->canTarget(d->source)) &&
      (!tcTarget || tcTarget->canTarget(d->target))
    ){
    if (damage == -1){
      d->damage = 0;
      delete event;
      if (oneShot) damage = 0;
      return NULL;
    }
    if (damage >= d->damage){
      damage-= d->damage;
      d->damage = 0;
      delete event;
      return NULL;
    }
    d->damage -= damage;
    damage = 0;
    delete event;
    WEventDamage* newEvent = NEW WEventDamage(d);
    return newEvent;
  }
  return event;
}
REDamagePrevention::~REDamagePrevention(){
  SAFE_DELETE(tcSource);
  SAFE_DELETE(tcTarget);
}

ReplacementEffects::ReplacementEffects(){}

WEvent * ReplacementEffects::replace(WEvent *e){
    list<ReplacementEffect *>::iterator it;

    for ( it=modifiers.begin() ; it != modifiers.end(); it++ ){
      ReplacementEffect *re = *it;
      WEvent * newEvent = re->replace(e);
      if (!newEvent) return NULL;
      if (newEvent != e) return replace(newEvent);
    }
    return e;
}

int ReplacementEffects::add(ReplacementEffect * re){
  modifiers.push_back(re);
  return 1;
}
 
int ReplacementEffects::remove (ReplacementEffect *re){
  modifiers.remove(re);
  return 1;
}

ReplacementEffects::~ReplacementEffects(){
  list<ReplacementEffect *>::iterator it;
    for ( it=modifiers.begin() ; it != modifiers.end(); it++ ){
      ReplacementEffect *re = *it;
      delete(re);
    }
    modifiers.clear();
}
