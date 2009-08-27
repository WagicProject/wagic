#include "../include/config.h"
#include "../include/DamagerDamaged.h"

/*
Temporary objects that store the damages dealt to/from creatures during the combat phase
*/


DamagerDamaged::DamagerDamaged(MTGCardInstance* card, Player * _damageSelecter, bool _hasFocus) : card(card){
  mCount = 0;
  damageSelecter = _damageSelecter;
  damageToDeal = card->power;
}

DamagerDamaged::~DamagerDamaged(){
  for (int i = 0; i < mCount; i++){
    SAFE_DELETE(damages[i]);

  }
}

int DamagerDamaged::sumDamages(){
  int total = 0;
  for (int i = 0; i < mCount; i++){
    total += damages[i]->damage;
  }
  return total;
}

int DamagerDamaged::hasLethalDamage(){
  if (sumDamages() >= card->life) return 1;
  return 0;
}

int DamagerDamaged::dealOneDamage(DamagerDamaged * target){
  if (!damageToDeal) return 0;
  damageToDeal--;
#if defined (WIN32) || defined (LINUX)
  char buf[4096];
  sprintf(buf, "==========\n%s can still deal %i damages\n=============\n", card->getName().c_str(), damageToDeal);
  OutputDebugString(buf);
#endif
  return target->addDamage(1, this);
}

int DamagerDamaged::addDamage(int damage, DamagerDamaged * source){
  for (int i = 0; i < mCount; i++){
    if (damages[i]->source == source->card){
      damages[i]->damage+= damage;
      return damage;
    }
  }
  damages[mCount] = NEW Damage(mCount, source->card, this->card,damage);
  mCount++;
  return damage;
}

int DamagerDamaged::removeDamagesTo(DamagerDamaged * target){
  damageToDeal+= target->removeDamagesFrom(this);
  return 1;
}

int DamagerDamaged::removeDamagesFrom(DamagerDamaged * source){
  for (int i = 0; i < mCount; i++){
    if (damages[i]->source == source->card){
      int damage = damages[i]->damage;
      SAFE_DELETE(damages[i]);
      damages[i] = damages[mCount-1];
      mCount--;
      return damage;
    }
  }
  return 0;
}

void DamagerDamaged::Render(Player * currentPlayer){
  JLBFont * mFont = GameApp::CommonRes->GetJLBFont(Constants::MAIN_FONT);
  mFont->SetBase(0);
  //  CardGui::Render();
  
  /*
  char buf[4096];
  if (currentPlayer != damageSelecter){
    if (hasLethalDamage()){
      mFont->DrawString("X",x,y);
    }
    mFont->SetColor(ARGB(255,255,0,0));
    sprintf(buf, "%i", sumDamages());
    mFont->DrawString(buf,x+5, y+5);
  }else{
    mFont->SetColor(ARGB(255,0,0,255));
    sprintf(buf, "%i", damageToDeal);
    mFont->DrawString(buf,x+5, y+5);
  }
  mFont->SetColor(ARGB(255,255,255,255));
  */
}
