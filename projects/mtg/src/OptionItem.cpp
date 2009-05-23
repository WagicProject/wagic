#include "../include/OptionItem.h"
#include "../include/GameApp.h"
#include <JGE.h>
#include "../include/GameOptions.h"
#include "../include/Translate.h"

OptionItem::OptionItem(string _id, string _displayValue, int _maxValue, int _increment):JGuiObject(0){
  id = _id;
  maxValue = _maxValue;
  increment = _increment;
  displayValue = _(_displayValue);
  value = GameOptions::GetInstance()->values[id].getIntValue();
  hasFocus = 0;
  x = 0;
  y = 0;
}

OptionItem::~OptionItem(){
  //TODO
}

void OptionItem::setData(){
  GameOptions::GetInstance()->values[id] = GameOption(value);
}

void OptionItem::Render(){
  JLBFont * mFont = GameApp::CommonRes->GetJLBFont("graphics/f3");
  if (hasFocus){
    mFont->SetColor(ARGB(255,255,255,0));
  }else{
    mFont->SetColor(ARGB(255,255,255,255));
  }
  JRenderer * renderer = JRenderer::GetInstance();
  renderer->FillRoundRect(x-5,y-2,SCREEN_WIDTH -x - 5,20,2,ARGB(150,50,50,50));
  mFont->DrawString(displayValue.c_str(),x,y);
  char buf[512];
  sprintf(buf, "%i", value);
  mFont->DrawString(buf,SCREEN_WIDTH -10 ,y,JGETEXT_RIGHT);
}

void OptionItem::Update(float dt){
  JGE * mEngine = JGE::GetInstance();
  if (hasFocus){
    if (mEngine->GetButtonClick(PSP_CTRL_CIRCLE)) updateValue();
  }
}



void OptionItem::Entering(){
  hasFocus = true;
}
bool OptionItem::Leaving(){
  hasFocus = false;
  return true;
}


OptionItem * options[20];
int nbitems;
OptionsList::OptionsList(){
  nbitems = 0;
  current = -1;
}
OptionsList::~OptionsList(){
  for (int i = 0 ; i < nbitems; i++){
    SAFE_DELETE(options[i]);
  }
}

void OptionsList::Add(OptionItem * item){
  if (nbitems < 20){
    options[nbitems] = item;
    item->x = 10;
    item->y = 20 + 30*nbitems;
    nbitems++;
    if (current < 0){
      current = 0;
      options[0]->Entering();
    }

  }
}

void OptionsList::Render(){
  if (!nbitems){
    JLBFont * mFont = GameApp::CommonRes->GetJLBFont("graphics/f3");
    mFont->DrawString("NO OPTIONS AVAILABLE",SCREEN_WIDTH/2, 5, JGETEXT_RIGHT);
  }
  for (int i = 0 ; i < nbitems; i++){
    options[i]->Render();
  }
}

void OptionsList::save(){
  for (int i = 0; i < nbitems; i++){
    options[i]->setData();
  }
  GameOptions::GetInstance()->save();
}

void OptionsList::Update(float dt){
  JGE * mEngine = JGE::GetInstance();
  if (mEngine->GetButtonClick(PSP_CTRL_UP))
    {
      if (current > 0){
	options[current]->Leaving();
	current--;
	options[current]->Entering();
      }
    }
  else if (mEngine->GetButtonClick(PSP_CTRL_DOWN))
    {
      if (current < nbitems -1){
	options[current]->Leaving();
	current++;
	options[current]->Entering();
      }
    }
  for (int i = 0 ; i < nbitems; i++){
    options[i]->Update(dt);
  }
}

ostream& OptionItem::toString(ostream& out) const
{
  return out << "OptionItem ::: displayValue : " << displayValue
	     << " ; id : " << id
	     << " ; value : " << value
	     << " ; hasFocus : " << hasFocus
	     << " ; maxValue : " << maxValue
	     << " ; increment : " << increment
	     << " ; x,y : " << x << "," << y;
}
