#include "../include/config.h"
#include "../include/Damage.h"
#include "../include/MTGCardInstance.h"
#include "../include/Counters.h"
#include "../include/WEvent.h"

Damage::Damage(int id, MTGCardInstance * _source, Damageable * _target): Interruptible(id){
  init(_source, _target, _source->getPower());
}

Damage::Damage(int id, MTGCardInstance * _source, Damageable * _target, int _damage): Interruptible(id){
  init(_source, _target, _damage);
}

void Damage::init(MTGCardInstance * _source, Damageable * _target, int _damage){
  target = _target;
  source = _source;


  if (_damage < 0) _damage = 0; //Negative damages cannot happen
  damage = _damage;
  mHeight = 40;
  type = ACTION_DAMAGE;
}

int Damage::resolve(){
  if (damage <0) damage = 0; //Negative damages cannot happen
  state = RESOLVED_OK;
  if (target->type_as_damageable == DAMAGEABLE_MTGCARDINSTANCE){
    MTGCardInstance * _target = (MTGCardInstance *)target;
    if ((_target)->protectedAgainst(source)) damage = 0;
    // Damage for WITHER on creatures

    if (!damage){
      state = RESOLVED_NOK;
      return 0;
    }
    if (source->has(Constants::WITHER)){
      for (int i = 0; i < damage; i++){
	      _target->counters->addCounter(-1, -1);
      }
      return 1;
    }
    _target->doDamageTest = 1;
  }

  int a = target->dealDamage(damage);

  //Send Damage event to listeners
  WEventDamage * e = NEW WEventDamage(this);
  GameObserver::GetInstance()->mLayers->actionLayer()->receiveEvent(e);
  delete e;

  return a;
}

void Damage::Render(){
  JLBFont * mFont = GameApp::CommonRes->GetJLBFont(Constants::MAIN_FONT);
  mFont->SetBase(0);
  mFont->SetScale(DEFAULT_MAIN_FONT_SCALE);
  char buffer[200];
  sprintf(buffer, "Does %i damage to", damage);
  mFont->DrawString(buffer, x + 20 , y, JGETEXT_LEFT);
  JRenderer * renderer = JRenderer::GetInstance();
  JQuad * quad = source->getThumb();
  if (quad){
    float scale = 30 / quad->mHeight;
    renderer->RenderQuad(quad, x  , y , 0,scale,scale);
  }else{
    mFont->DrawString(source->getName(),x,y-15);
  }
  quad = target->getIcon();
  if (quad){
    float scale = 30 / quad->mHeight;
    renderer->RenderQuad(quad, x + 150  , y , 0,scale,scale);
  }else{
    if (target->type_as_damageable == DAMAGEABLE_MTGCARDINSTANCE)
        mFont->DrawString(((MTGCardInstance *)target)->getName(),x+120,y);
  }

}

ostream& Damage::toString(ostream& out) const
{
  out << "Damage ::: target : " << target << " ; damage " << damage;
  return out;
}

DamageStack::DamageStack(int id, GameObserver * _game):GuiLayer(id, _game), Interruptible(id){
  currentState = -1;
  type = ACTION_DAMAGES;
}

int DamageStack::CombatDamages(){
  CombatDamages(1);
  CombatDamages(0);
  return 1;
}

int DamageStack::CombatDamages(int strike){
  mHeight = 0;
  MTGInPlay * attackers = game->currentPlayer->game->inPlay;
  MTGInPlay * defensers = game->opponent()->game->inPlay;

  MTGCardInstance * attacker = attackers->getNextAttacker(NULL);
  while (attacker != NULL){
    int nbdefensers = defensers->nbDefensers(attacker);
    if ((!strike && !attacker->has(Constants::FIRSTSTRIKE)) || (strike && attacker->has(Constants::FIRSTSTRIKE)) || attacker->has(Constants::DOUBLESTRIKE)){
      if (nbdefensers == 0){
	Damage * damage = NEW Damage (mCount, attacker, game->opponent());
	Add(damage);
      }else if (nbdefensers == 1){
	Damage * damage = NEW Damage (mCount, attacker, defensers->getNextDefenser(NULL, attacker));
	Add(damage);
      }else{
	//TODO Fetch list of defensers and allow user to choose targets
	Damage * damage = NEW Damage (mCount, attacker, defensers->getNextDefenser(NULL, attacker));
	Add(damage);
      }
    }
    MTGCardInstance * defenser = defensers->getNextDefenser(NULL, attacker);
    while (defenser != NULL){
      if ((!strike && !defenser->has(Constants::FIRSTSTRIKE)) || (strike && defenser->has(Constants::FIRSTSTRIKE)) || defenser->has(Constants::DOUBLESTRIKE)){
	Damage * damage = NEW Damage (mCount,defenser, attacker);
	Add(damage);
      }
      defenser = defensers->getNextDefenser(defenser, attacker);
    }
    attacker = attackers->getNextAttacker(attacker);
  }

  for (int i = 0; i < mCount; i++){
    Damage * damage = (Damage*)mObjects[i];
    mHeight += damage->mHeight;
  }

  return mCount;
}

int DamageStack::resolve(){
  for (int i = mCount-1; i>= 0; i--){
    Damage * damage = (Damage*)mObjects[i];
    if (damage->state == NOT_RESOLVED) damage->resolve();
    //damage->resolve();
  }
  for (int i = mCount-1; i>= 0; i--){
    Damage * damage = (Damage*)mObjects[i];
    if (damage->state == RESOLVED_OK) damage->target->afterDamage();
    //damage->target->afterDamage();
  }
  return 1;
}

void DamageStack::Render(){
  int currenty = y;
  for (int i= 0; i < mCount; i++){
    Damage * damage = (Damage*)mObjects[i];
    if (damage->state == NOT_RESOLVED){
      damage->x = x;
      damage->y = currenty;
      currenty += damage->mHeight;
      damage->Render();
    }
  }
}

ostream& DamageStack::toString(ostream& out) const
{
  return (out << "DamageStack ::: currentState : " << currentState);
}
