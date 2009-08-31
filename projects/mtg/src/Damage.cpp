#include "../include/config.h"
#include "../include/Damage.h"
#include "../include/MTGCardInstance.h"
#include "../include/Counters.h"
#include "../include/WEvent.h"
#include "../include/Translate.h"
#include "../include/TexturesCache.h"

Damage::Damage(MTGCardInstance * source, Damageable * target) {
  init(source, target, source->getPower());
}

Damage::Damage(MTGCardInstance * source, Damageable * target, int damage) {
  init(source, target, damage);
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

  GameObserver * g = GameObserver::GetInstance();
  WEvent * e = NEW WEventDamage(this);
  e = g->replacementEffects->replace(e);

  int a = 0;
  if (damage) a = target->dealDamage(damage);

  //Send (Damage/Replaced effect) event to listeners

  g->receiveEvent(e);
  //SAFE_DELETE(e);

  return a;
}

void Damage::Render(){
  JLBFont * mFont = GameApp::CommonRes->GetJLBFont(Constants::MAIN_FONT);
  mFont->SetBase(0);
  mFont->SetScale(DEFAULT_MAIN_FONT_SCALE);
  char buffer[200];
  sprintf(buffer, _("Deals %i damage to").c_str(), damage);
  mFont->DrawString(buffer, x + 20 , y, JGETEXT_LEFT);
  JRenderer * renderer = JRenderer::GetInstance();
  JQuad * quad = cache.getThumb(source);
  if (quad){
    float scale = 30 / quad->mHeight;
    renderer->RenderQuad(quad, x  , y , 0,scale,scale);
  }else{
    mFont->DrawString(_(source->getName()).c_str(),x,y-15);
  }
  quad = target->getIcon();
  if (quad){
    float scale = 30 / quad->mHeight;
    renderer->RenderQuad(quad, x + 150  , y , 0,scale,scale);
  }else{
    if (target->type_as_damageable == DAMAGEABLE_MTGCARDINSTANCE)
      mFont->DrawString(_(((MTGCardInstance *)target)->getName()).c_str(),x+120,y);
  }

}

ostream& Damage::toString(ostream& out) const
{
  out << "Damage ::: target : " << target << " ; damage " << damage;
  return out;
}

DamageStack::DamageStack(GameObserver* game) : game(game){
  currentState = -1;
  type = ACTION_DAMAGES;
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
