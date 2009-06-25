#include "../include/WEvent.h"
#include "../include/MTGCardInstance.h"
#include "../include/MTGGameZones.h"
#include "../include/Damage.h"
#include "../include/PhaseRing.h"


WEvent::WEvent(int _type){
  type=_type;
}

WEventZoneChange::WEventZoneChange(MTGCardInstance * _card, MTGGameZone * _from, MTGGameZone *_to):WEvent(CHANGE_ZONE){
  card = _card;
  from = _from;
  to = _to;
}

WEventDamage::WEventDamage(Damage *_damage):WEvent(DAMAGE){
  damage = _damage;
}

WEventPhaseChange::WEventPhaseChange(Phase * _from, Phase * _to):WEvent(CHANGE_PHASE){
  from = _from;
  to = _to;
}

WEventCardTap::WEventCardTap(MTGCardInstance * _card, int _before, int _after):WEvent(){
  card = _card;
  before = _before;
  after = _after;
}