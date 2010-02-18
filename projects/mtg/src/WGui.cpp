#include "../include/config.h"
#include "../include/OptionItem.h"
#include "../include/PlayerData.h"
#include "../include/Translate.h"
#include <algorithm>
#include <hge/hgedistort.h>

//WGuiBase
PIXEL_TYPE WGuiBase::getColor(int type){
  switch(type){
    case WGuiColor::TEXT_BODY:
    case WGuiColor::SCROLLBUTTON:
      return ARGB(255,255,255,255);
    case WGuiColor::SCROLLBAR:
      return ARGB(150,50,50,50);
    case WGuiColor::BACK_HEADER:
      return ARGB(150,80,80,80);
    default:
      if(type < WGuiColor::BACK){
        if(hasFocus())
          return ARGB(255,255,255,0);
        else
          return ARGB(255,255,255,255);
      }
      else
        return ARGB(150,50,50,50);
  }
  return ARGB(150,50,50,50);
}


void WGuiBase::renderBack(WGuiBase * it){
  if(!it) return; 
  WDecoStyled * styled = dynamic_cast<WDecoStyled*>(it);
  if(styled)
    styled->renderBack(styled->getDecorated());
  else
    subBack(it);
}


//WGuiItem
void WGuiItem::Entering(JButton key){
  mFocus = true;
} 
float WGuiItem::minWidth(){
  JLBFont * mFont = resources.GetJLBFont(Constants::OPTION_FONT);
  return mFont->GetStringWidth(_(displayValue).c_str())+4;
}
float WGuiItem::minHeight(){
  JLBFont * mFont = resources.GetJLBFont(Constants::OPTION_FONT);
  return mFont->GetHeight();
}

bool WGuiItem::Leaving(JButton key){
  mFocus = false;
  return true;
}

void WGuiItem::Render(){
  JRenderer * renderer = JRenderer::GetInstance();
  JLBFont * mFont = resources.GetJLBFont(Constants::OPTION_FONT);
  DWORD oldcolor = mFont->GetColor();
  mFont->SetColor(getColor(WGuiColor::TEXT));
  float fH = (height-mFont->GetHeight())/2;
  string trans = _(displayValue);
  float fW = mFont->GetStringWidth(trans.c_str());
  float boxW = getWidth();
  float oldS = mFont->GetScale();
  if(fW > boxW){
    mFont->SetScale(boxW/fW);
  }
  mFont->DrawString(trans,x+(width/2),y+fH,JGETEXT_CENTER);
  mFont->SetScale(oldS);
  mFont->SetColor(oldcolor);
}

WGuiItem::WGuiItem(string _display, u8 _mF){
 mFlags=_mF;
 displayValue = _display;
 mFocus = false;
 width=SCREEN_WIDTH;
 height=20;
 x=0;
 y=0;
}

string WGuiItem::_(string input){
  if(mFlags & WGuiItem::NO_TRANSLATE)
    return input;
  return ::_(input);
}
bool WGuiItem::CheckUserInput(JButton key){
  if(mFocus && key == JGE_BTN_OK){
    updateValue();
    return true;
  }
  return false;
}

//WDecoStyled
void WDecoStyled::subBack(WGuiBase * item){
  if(!item)
    return;
  JRenderer * renderer = JRenderer::GetInstance();
  if(mStyle & DS_STYLE_BACKLESS)
    return;
  //TODO: if(mStyle & DS_STYLE_EDGED) Draw the edged box ala SimpleMenu
  else{ //Draw standard style
    WGuiSplit * split = dynamic_cast<WGuiSplit*>(item);
    if(split && split->left->Visible() && split->right->Visible()){
      if(split->left)
        renderer->FillRoundRect(split->left->getX()-2,split->getY()-2,split->left->getWidth()-6,split->getHeight(),2,split->left->getColor(WGuiColor::BACK));
      if(split->right)
        renderer->FillRoundRect(split->right->getX()-2,split->getY()-2,split->right->getWidth(),split->getHeight(),2,split->right->getColor(WGuiColor::BACK));
    }
    else{
      renderer->FillRoundRect(item->getX()-2,item->getY()-2,item->getWidth(),item->getHeight(),2,getColor(WGuiColor::BACK));
    }
  }
  
}

PIXEL_TYPE WDecoStyled::getColor(int type){
  switch(type){
    case WGuiColor::BACK:
    case WGuiColor::BACK_HEADER:
      if(mStyle & DS_COLOR_DARK)
        return ARGB(150,35,35,35);
      else if(mStyle & DS_COLOR_BRIGHT)
        return ARGB(150,80,80,80);
      else if(mStyle & DS_STYLE_ALERT)
        return ARGB(150,120,80,80);
      else
        return ARGB(150,50,50,50);        
    default:
      return WGuiBase::getColor(type);
  }
  return ARGB(150,50,50,50);
}

//WGuiHeader
void WGuiHeader::Render(){
  JLBFont * mFont = resources.GetJLBFont(Constants::OPTION_FONT);
  mFont->SetColor(getColor(WGuiColor::TEXT));
 
  JRenderer * renderer = JRenderer::GetInstance();
  mFont->DrawString(_(displayValue).c_str(),x+width/2,y,JGETEXT_CENTER);
}


bool WGuiMenu::Leaving(JButton key){
  int nbitems = (int) items.size();
  if(key == buttonNext && currentItem < nbitems-1)
    return false;
  else if(key == buttonPrev && currentItem > 0)
    return false;

  if(currentItem >= 0 && currentItem < nbitems)
    if(!items[currentItem]->Leaving(key))
      return false;

  mFocus = false;
  return true;
}
void WGuiMenu::Entering(JButton key){
  mFocus = true;

  //Try to force a selectable option.
  if(currentItem == -1){
    for (size_t i = 0 ; i < items.size(); i++){
      if(items[i]->Selectable()) {
        currentItem = i;
        break;
      }
    }
  }

  if(currentItem >= 0 && currentItem < (int) items.size())
    items[currentItem]->Entering(key);
  return;
}

void WGuiMenu::subBack(WGuiBase * item){
  if(!item)
    return;
  JRenderer * renderer = JRenderer::GetInstance();

  WGuiSplit * split = dynamic_cast<WGuiSplit*>(item);
  if(split && split->left->Visible() && split->right->Visible()){
    if(split->left)
      subBack(split->left);//renderer->FillRoundRect(split->left->getX()-2,split->getY()-2,split->left->getWidth()-6,split->getHeight(),2,split->left->getColor(WGuiColor::BACK));
    if(split->right)
      subBack(split->right);//renderer->FillRoundRect(split->right->getX()-2,split->getY()-2,split->right->getWidth(),split->getHeight(),2,split->right->getColor(WGuiColor::BACK));
  }
  else
    renderer->FillRoundRect(item->getX(),item->getY(),item->getWidth()-4,item->getHeight()-2,2,item->getColor(WGuiColor::BACK));

}
//WGuiList
WGuiList::WGuiList(string name, WSyncable * syncme): WGuiMenu(JGE_BTN_DOWN, JGE_BTN_UP, false, syncme){
  failMsg = "NO OPTIONS AVAILABLE";
  width = SCREEN_WIDTH-10;
  height = SCREEN_HEIGHT-10;
  y = 5;
  x = 5;
  mFocus = false;
  sync = syncme;
  displayValue = name;
}
void WGuiList::confirmChange(bool confirmed){
  for(size_t x=0;x<items.size();x++){
    if(!items[x])
      continue;
    items[x]->confirmChange(confirmed);
  }
}
void WGuiList::Render(){
  JRenderer * renderer = JRenderer::GetInstance();
  int listHeight=40; 
  int listSelectable=0;
  int adjustedCurrent=0;
  int start = 0, nowPos = 0, vHeight=0;
  int nbitems = (int) items.size();

  //List is empty.
  if (!items.size() && failMsg != ""){
    JLBFont * mFont = resources.GetJLBFont(Constants::OPTION_FONT);
    mFont->SetColor(getColor(WGuiColor::TEXT_FAIL));
    mFont->DrawString(_(failMsg).c_str(),x+width/2, y, JGETEXT_RIGHT);
    return;
  }

  //Force a selectable option.
  if(currentItem == -1){
    for (int i = 0 ; i < nbitems; i++){
      if(items[i]->Selectable()) {
        currentItem = i;
        if(hasFocus())
          items[currentItem]->Entering(JGE_BTN_NONE);
        break;
      }
    }
  }
  //Find out how large our list is, with all items and margin.
  for (int pos=0;pos < nbitems; pos++){
    listHeight+=items[pos]->getHeight()+1; //What does the +1 do exactly ?
    if(items[pos]->Selectable()){
      listSelectable++;
      if(pos < currentItem) adjustedCurrent++;
    }
  }

  //Always fill screen
  if(listHeight > SCREEN_HEIGHT)
  { 
    for (start=currentItem;start > 0; start--){
      if(!items[start]->Visible())
        continue;

      vHeight += items[start]->getHeight()+5;
      if(vHeight >= (SCREEN_HEIGHT-60)/2)
        break;
    }
    vHeight = 0;
	if(start >= 0)
    for (nowPos=nbitems;nowPos > 1; nowPos--){
       if(!items[start]->Visible())
        continue;
      vHeight += items[nowPos-1]->getHeight()+5;
    }

    if(vHeight <= SCREEN_HEIGHT-40 && nowPos < start)
      start = nowPos;
  }

  vHeight = 0;
  nowPos = 0;

  //Render items.
  if(start >= 0)
  {
    //Render current underlay.
    if(currentItem >= 0 && currentItem < nbitems && items[currentItem]->Visible())
      items[currentItem]->Underlay();

    for (int pos=0;pos < nbitems; pos++){
      if(!items[pos]->Visible())
        continue;

      if(pos < start){
        vHeight += items[pos]->getHeight() + 5;
        continue;
      }

      items[pos]->setY(y+nowPos);
      items[pos]->setX(x);
      if(listHeight > SCREEN_HEIGHT && listSelectable > 1)
        items[pos]->setWidth(width-10);
      else
        items[pos]->setWidth(width);
      nowPos += items[pos]->getHeight() + 5;
      renderBack(items[pos]);
      items[pos]->Render();
      if(nowPos > SCREEN_HEIGHT) //Stop displaying things once we reach the bottom of the screen.
        break;
    }

    //Draw scrollbar
    if(listHeight > SCREEN_HEIGHT && listSelectable > 1){
      int barPosition = y-5+((float)adjustedCurrent/listSelectable)*(SCREEN_HEIGHT-y);
      int barLength = (SCREEN_HEIGHT-y) / listSelectable;
      if(barLength < 4) barLength = 4;
      renderer->FillRect(x+width-2,y-1,2,SCREEN_HEIGHT-y,
        getColor(WGuiColor::SCROLLBAR));
      renderer->FillRoundRect(x+width-5,barPosition,5,barLength,2,
        getColor(WGuiColor::SCROLLBUTTON));
    }

    //Render current overlay.
    if(currentItem >= 0 && currentItem < nbitems && items[currentItem]->Visible())
      items[currentItem]->Overlay();
    }
}

void WGuiList::setData(){
  for (size_t i = 0; i < items.size(); i++){
    items[i]->setData();
  }
}

void WGuiList::ButtonPressed(int controllerId, int controlId){
  WGuiBase * it;

  if(!(it = Current()))
    return;

  it->ButtonPressed(controllerId,controlId);
}


string WDecoEnum::lookupVal(int value){

  if(edef == NULL){
    int id = getId();
    if(id != INVALID_ID){
      GameOptionEnum * goEnum = dynamic_cast<GameOptionEnum*>(options.get(getId()));
      if(goEnum)
        edef = goEnum->def;
    }
  }

  if(edef){
    int idx = edef->findIndex(value);
    if(idx != INVALID_ID)
      return edef->values[idx].second;
  }

  char buf[32];
  sprintf(buf,"%d",value);
  return buf;
}

void WDecoEnum::Render()
{
  JLBFont * mFont = resources.GetJLBFont(Constants::OPTION_FONT);
  mFont->SetColor(getColor(WGuiColor::TEXT));
  JRenderer * renderer = JRenderer::GetInstance();
  mFont->DrawString(_(getDisplay()).c_str(),getX(),getY());
  OptionInteger* opt = dynamic_cast<OptionInteger*>(it);
  if(opt)
    mFont->DrawString(_(lookupVal(opt->value)).c_str(), getWidth() -10, getY(), JGETEXT_RIGHT);
}

WDecoEnum::WDecoEnum(WGuiBase * _it, EnumDefinition *_edef) : WGuiDeco(_it) {edef = _edef;}
//WDecoCheat
WDecoCheat::WDecoCheat(WGuiBase * _it): WGuiDeco(_it){
  bVisible = (options[Options::ACTIVE_PROFILE].str == SECRET_PROFILE);
}
void WDecoCheat::Reload(){
  bVisible = (options[Options::ACTIVE_PROFILE].str == SECRET_PROFILE);
}
bool WDecoCheat::Visible(){
  if(bVisible && it && it->Visible())
    return true;
  return false;
}
bool WDecoCheat::Selectable(){
  if(!it || !Visible())
    return false;
  return it->Selectable();
}
//WDecoConfirm

WDecoConfirm::WDecoConfirm(JGuiListener * _listener, WGuiBase * _it): WGuiDeco(_it){
  listener = _listener;
  confirm = "Confirm";
  cancel = "Cancel";
  confirmMenu = NULL;
  bModal = false;
  mState = OP_CONFIRMED;
}

WDecoConfirm::~WDecoConfirm(){
  SAFE_DELETE(confirmMenu);
}

void WDecoConfirm::Entering(JButton key){
  setFocus(true);

  if(it)
    it->Entering(key);

  SAFE_DELETE(confirmMenu);
  mState = OP_CONFIRMED; 
  confirmMenu = NEW SimpleMenu(444, listener,Constants::MENU_FONT, 50,170);
  confirmMenu->Add(1,confirm.c_str());
  confirmMenu->Add(2,cancel.c_str());
}

bool WDecoConfirm::isModal(){
  if(bModal || (it && it->isModal()))
    return true;

  return false;
}

void WDecoConfirm::setModal(bool val){
  bModal = val;
}
void WDecoConfirm::setData(){
  if(!it)
    return;

  it->setData();
}

bool WDecoConfirm::Leaving(JButton key){
  if(!it)
    return true;

  //Choice must be confirmed.
    if(mState == OP_UNCONFIRMED){
        if(!isModal())
          setModal(true); 
        if(!it->Changed())
          mState = OP_CONFIRMED;
        else
          mState = OP_CONFIRMING;
    }
    
    if(mState == OP_CONFIRMED && it->Leaving(key)){
      setFocus(false);
      setModal(false);
      SAFE_DELETE(confirmMenu);
      return true;
    }
  
  return false;
}
bool WDecoConfirm::CheckUserInput(JButton key){
  if(hasFocus()){
    if (mState == OP_CONFIRMED && key == JGE_BTN_OK)
      mState = OP_UNCONFIRMED;
    
    if (mState != OP_CONFIRMING && it){
      if(it->CheckUserInput(key))
        return true;
    } else if(confirmMenu && confirmMenu->CheckUserInput(key))
      return true;
  }
  return false;
}

void WDecoConfirm::Update(float dt){
  if (hasFocus()){
    if (it && mState != OP_CONFIRMING)
      it->Update(dt);
    else
      confirmMenu->Update(dt);
  }
}

void WDecoConfirm::Overlay(){
  if (confirmMenu && mState == OP_CONFIRMING)
    confirmMenu->Render();

  if(it)
    it->Overlay();
}

void WDecoConfirm::ButtonPressed(int controllerId, int controlId){
  if(controllerId == 444){
    setModal(false); 
    switch(controlId){
      case 1:
        mState = OP_CONFIRMED;      
        if(it)
          it->confirmChange(true);
        break;
      case 2:
        mState = OP_CONFIRMED;
        if(it)
          it->confirmChange(false);
        break;
    }
  }
  else
    it->ButtonPressed(controllerId,controlId);
}

WGuiButton::WGuiButton( WGuiBase* _it, int _controller, int _control, JGuiListener * jgl): WGuiDeco(_it) {
   control = _control;
   controller = _controller;
   mListener = jgl;
}

void WGuiButton::updateValue(){
  if(mListener)
    mListener->ButtonPressed(controller, control);
}

bool WGuiButton::CheckUserInput(JButton key){
  if (hasFocus() && key == JGE_BTN_OK){
      updateValue();
      return true;
  }
  return false;
}

PIXEL_TYPE WGuiButton::getColor(int type){
  if(type == WGuiColor::BACK && hasFocus())
    return it->getColor(WGuiColor::BACK_HEADER);
  return it->getColor(type);
};

WGuiSplit::WGuiSplit(WGuiBase* _left, WGuiBase* _right) : WGuiItem("") {
  right = _right;
  left = _left;
  bRight = false;
  percentRight = 0.5f;
  if(!left->Selectable())
    bRight = true;
}
WGuiSplit::~WGuiSplit(){
  SAFE_DELETE(left);
  SAFE_DELETE(right);
}

void WGuiSplit::setData(){
  left->setData();
  right->setData();
}
void WGuiSplit::setX(float _x){
  x = _x;
  left->setX(x);
  right->setX(x+(1-percentRight)*width);
}
void WGuiSplit::setY(float _y){
  y = _y;
  left->setY(y);
  right->setY(y);
}
void WGuiSplit::setWidth(float _w){
  width = _w;
  if(right->Visible())
    left->setWidth((1-percentRight)*width);
  else
    left->setWidth(width);

  right->setWidth(percentRight*width);
}
void WGuiSplit::setHeight(float _h){
  left->setHeight(_h);
  right->setHeight(_h);
  height = _h;
}
float WGuiSplit::getHeight(){
  float lH, rH;
  lH = left->getHeight();
  rH = right->getHeight();
  if(lH > rH)
    return lH;

  return rH;
}

void WGuiSplit::Render(){
  if(right->Visible())
    right->Render();
  if(left->Visible())
    left->Render();
}

bool WGuiSplit::isModal(){
  if(bRight)
    return right->isModal();
  
  return left->isModal();
}
void WGuiSplit::setModal(bool val){
  if(bRight)
    return right->setModal(val);
  
  return left->setModal(val);
}

bool WGuiSplit::CheckUserInput(JButton key){
    if(hasFocus()){
      if (!bRight){
        if(left->CheckUserInput(key))
          return true;
        if(key == JGE_BTN_RIGHT && !isModal() 
          && right->Selectable() && left->Leaving(JGE_BTN_RIGHT)){
          bRight = !bRight;
          right->Entering(JGE_BTN_RIGHT);
          return true;
        }
      }
      else 
      {
        if(right->CheckUserInput(key))
          return true;
        if (key == JGE_BTN_LEFT && !isModal() 
          && left->Selectable() && right->Leaving(JGE_BTN_LEFT)){
          bRight = !bRight;
          left->Entering(JGE_BTN_LEFT);
          return true;
        }
      }
    }
    
  return false;
}

void WGuiSplit::Update(float dt){
  if(bRight)
    right->Update(dt);
  else
    left->Update(dt);
}

void WGuiSplit::Entering(JButton key){
  mFocus = true;
  if(bRight)
    right->Entering(key);
  else
    left->Entering(key);
}
bool WGuiSplit::Leaving(JButton key){

   if(bRight){
     if(right->Leaving(key)){
      mFocus = false;
      return true;
     }
   }
   else{
     if(left->Leaving(key)){
      mFocus = false;
      return true;
     }
   }

   return false;
}
void WGuiSplit::Overlay(){
  if(bRight)
    right->Overlay();
  else
    left->Overlay();
}
void WGuiSplit::Underlay(){
  if(bRight)
    right->Underlay();
  else
    left->Underlay();
}
void WGuiSplit::ButtonPressed(int controllerId, int controlId)
{
  if(bRight)
    right->ButtonPressed(controllerId, controlId);
  else
    left->ButtonPressed(controllerId, controlId);
}
void WGuiSplit::Reload(){
  left->Reload();
  right->Reload();
}
void WGuiSplit::confirmChange(bool confirmed){
  right->confirmChange(confirmed);
  left->confirmChange(confirmed);
}

//WGuiMenu
WGuiMenu::WGuiMenu(JButton next = JGE_BTN_RIGHT, JButton prev = JGE_BTN_LEFT, bool m, WSyncable * syncme): WGuiItem(""){
  buttonNext = next;
  buttonPrev = prev;
  currentItem = -1;
  mDPad = m;
  sync = syncme;
}
WGuiMenu::~WGuiMenu(){
  for(vector<WGuiBase*>::iterator it = items.begin();it!=items.end();it++)
    SAFE_DELETE(*it);
}
void WGuiMenu::setData(){
  for(vector<WGuiBase*>::iterator it = items.begin();it!=items.end();it++)
    (*it)->setData();
};

void WGuiMenu::Reload(){
 for(vector<WGuiBase*>::iterator it = items.begin();it!=items.end();it++)
    (*it)->Reload();
};

void WGuiMenu::Render(){
  for(vector<WGuiBase*>::iterator it = items.begin();it!=items.end();it++)
    (*it)->Render();
}
void WGuiMenu::confirmChange(bool confirmed){
  for(vector<WGuiBase*>::iterator it = items.begin();it!=items.end();it++)
    (*it)->confirmChange(confirmed);
}

void WGuiMenu::ButtonPressed(int controllerId, int controlId){
  WGuiBase * it = Current();
  if(!it) return;
  it->ButtonPressed(controllerId,controlId);
}

WGuiBase * WGuiMenu::Current(){
  if(currentItem >= 0 && currentItem < (int) items.size())
    return items[currentItem];
  return NULL;
}
void WGuiMenu::Add(WGuiBase * it){
  if(it)
    items.push_back(it);
}
bool WGuiMenu::CheckUserInput(JButton key){
  bool kidModal = false;
  bool handledInput = false;
  int nbitems = (int) items.size();
  JGE * mEngine = JGE::GetInstance();

  if(!mEngine->GetButtonState(held)) //Key isn't held down.
    held = JGE_BTN_NONE;

  if(currentItem >= 0 && currentItem < nbitems)
    kidModal = items[currentItem]->isModal();

  if(!kidModal && hasFocus()){
    if (isButtonDir(key,-1)){
     held = buttonPrev;
     duration = 0;
     if(prevItem())
       return true;
    } 
    else if(held == buttonPrev && duration > 1){
     duration = .92;
     if(prevItem())
       return true;
    } 
    else if (isButtonDir(key,1)){
     held = buttonNext;
     duration = 0;
     if(nextItem())
       return true;
    }
    else if(held == buttonNext && duration > 1){
      duration = .92;      
     if(nextItem())
       return true;
    }
  }

  if(currentItem >= 0 && currentItem < nbitems)
    return items[currentItem]->CheckUserInput(key);

  return false;
}
void WGuiMenu::syncMove(){
  if(!sync)
    return;
  int i = currentItem - sync->getPos();

  while(i < 0 && sync->prev())
    i = currentItem - sync->getPos();
  while(i > 0 && sync->next())
    i = currentItem - sync->getPos();
}
bool WGuiMenu::isButtonDir(JButton key, int dir){
  if(!mDPad)
    return ((dir > 0 && key == buttonNext) || (dir <= 0 && key == buttonPrev));
  
  if(dir <= 0){
    switch(buttonPrev){
      case JGE_BTN_LEFT:
        if(key == JGE_BTN_UP)
          return true;
        break;
      case JGE_BTN_UP:
        if(key == JGE_BTN_LEFT)
          return true;
        break;
    default: ; // Nothing
    }
    return (key == buttonPrev);
  }else {
    switch(buttonNext){
      case JGE_BTN_RIGHT:
        if(key == JGE_BTN_DOWN)
          return true;
        break;
      case JGE_BTN_DOWN:
        if(key == JGE_BTN_RIGHT)
          return true;
        break;
    default: ; // Nothing
    }
    return (key == buttonNext);
  }
}
void WGuiMenu::Update(float dt){
  int nbitems = (int) items.size();
  JGE * mEngine = JGE::GetInstance();

  if(held)
    duration += dt;
  
  if(currentItem >= 0 && currentItem < nbitems)
    items[currentItem]->Update(dt);

  for (int i = 0 ; i < nbitems; i++){
    if(i != currentItem)
      items[i]->Update(dt);
  }
}

bool WGuiMenu::nextItem(){
 int potential = currentItem;
 int nbitems = (int) items.size();
 if(nbitems < 2)
    return false;

 WGuiBase * now = NULL;
 if(currentItem < nbitems && currentItem > -1)
   now = items[currentItem];

   if (potential < nbitems-1)
      potential++;
   else 
      potential = 0;

    while(potential < nbitems-1 && items[potential]->Selectable() == false)
      potential++;
    if(potential == nbitems || !items[potential]->Selectable())
      potential = -1;
    else if(potential != currentItem && (!now || now->Leaving(buttonNext))){
        currentItem = potential;
        items[currentItem]->Entering(buttonNext);
        
        if(sync)
          syncMove();
        return true;
    }
  
  if(sync)
    syncMove();
  return false;
}

bool WGuiMenu::prevItem(){
 int potential = currentItem;
 WGuiBase * now = NULL;
 int nbitems = (int) items.size();
 if(nbitems < 2)
    return false;

 if(currentItem < (int)items.size() && currentItem > -1)
  now = items[currentItem];

 if (potential > 0)
   potential--;
 else
   potential = nbitems-1;

 while(potential > 0 && items[potential]->Selectable() == false)
   potential--;
 if(potential < 0 || !items[potential]->Selectable())
   potential = -1;
 else if(potential != currentItem && (!now || now->Leaving(buttonNext))){
   currentItem = potential;
   items[currentItem]->Entering(buttonPrev);
   if(sync)
     syncMove();
   return true;
  }
  if(sync)
    syncMove();
  return false;
}

void WGuiMenu::setModal(bool val){
  WGuiBase* c = Current();
  if(c)
    c->setModal(val);
}
bool WGuiMenu::isModal(){
  WGuiBase* c = Current();
  if(c)
    return c->isModal();

  return false;
}
//WGuiTabMenu
void WGuiTabMenu::Add(WGuiBase * it){
  if (it){
    it->setY(it->getY()+35);
    it->setHeight(it->getHeight()-35);
    WGuiMenu::Add(it);
  }
}

void WGuiTabMenu::Render(){
  JLBFont * mFont = resources.GetJLBFont(Constants::OPTION_FONT);
  JRenderer * renderer = JRenderer::GetInstance();

  if (!items.size())
    return;

  int offset = x;
  mFont->SetScale(0.8);  
  for(vector<WGuiBase*>::iterator it = items.begin();it!=items.end();it++){
    int w = mFont->GetStringWidth(_((*it)->getDisplay()).c_str());
    mFont->SetColor((*it)->getColor(WGuiColor::TEXT_TAB));
    renderer->FillRoundRect(offset+5,5,w + 5,25,2,(*it)->getColor(WGuiColor::BACK_TAB));
    mFont->DrawString(_((*it)->getDisplay()).c_str(),offset+10,10);
    offset += w + 10 + 2;
  }
  mFont->SetScale(1);

  WGuiBase * c = Current();
  if(c)
    c->Render();
}

void WGuiTabMenu::save(){
  confirmChange(true);
  setData();
  ::options.save();
}


//WGuiAward
void WGuiAward::Overlay(){
  JRenderer * r = JRenderer::GetInstance();
  JLBFont * mFont = resources.GetJLBFont(Constants::OPTION_FONT);
  mFont->SetScale(.8);
  mFont->SetColor(getColor(WGuiColor::TEXT));

  string s = details;
  if(s.size()){
    float fW = mFont->GetStringWidth(s.c_str());
    float fH = mFont->GetHeight();

    if(fH < 16)
      fH = 18;
    JQuad * button = resources.RetrieveQuad("iconspsp.png", (float)4*32, 0, 32, 32,"",RETRIEVE_NORMAL);

    r->FillRoundRect(5,10,fW+32,fH+2,2,getColor(WGuiColor::BACK));
    if(button)
      r->RenderQuad(button, 10,12,0,.5,.5);
    mFont->DrawString(::_(s),30,16);
  }

  mFont->SetScale(1);
}
void WGuiAward::Underlay(){
  char buf[1024];
  JRenderer * r = JRenderer::GetInstance();
  JQuad * trophy = NULL;

  string n = Options::getName(id);
  if(n.size()){
    sprintf(buf,"trophy_%s.png",n.c_str()); //Trophy specific to the award
    trophy = resources.RetrieveTempQuad(buf); //Themed version...
  }
  
  if(!trophy && id >= Options::SET_UNLOCKS){
    trophy = resources.RetrieveTempQuad("trophy_set.png");  //TODO FIXME: Should look in set dir too.
  }

  if(!trophy) //Fallback to basic trophy image.
   trophy = resources.RetrieveTempQuad("trophy.png");
  
  if(trophy){
      r->RenderQuad(trophy, 0, SCREEN_HEIGHT-trophy->mHeight);
  }

}
void WGuiAward::Render(){
  GameOptionAward * goa = dynamic_cast<GameOptionAward*>(&options[id]);
  
  if(!goa)
    return;

  JRenderer * renderer = JRenderer::GetInstance();
  JLBFont * mFont = resources.GetJLBFont(Constants::OPTION_FONT);
  mFont->SetScale(1);
  mFont->SetColor(getColor(WGuiColor::TEXT));

  float myX = x;
  float myY = y;
  float fH = mFont->GetHeight();
  float fM = fH / 5; //Font Margin is 20% font height

  myX += fM;
    renderer->FillRoundRect(x-fM/2,y-fM,getWidth()-fM,fH-fM,fM,getColor(WGuiColor::BACK_TAB));
  mFont->DrawString(::_(displayValue).c_str(),myX,myY,JGETEXT_LEFT);

  myY += fH + 3*fM;
  mFont->SetScale(.75);
  fH = mFont->GetHeight();
  if(text.size()){
    mFont->DrawString(_(text.c_str()),myX,myY,JGETEXT_LEFT);
    myY+=fH + fM;
  }
  string s = goa->menuStr();
  if(s.size()){
    mFont->DrawString(s.c_str(),myX,myY,JGETEXT_LEFT);
    myY+=fH + fM;
  }
  setHeight(myY-y);
  mFont->SetScale(1);
}

WGuiAward::WGuiAward(int _id, string name, string _text, string _details): WGuiItem(name){
  id = _id;
  text = _text;
  height = 60;
  details = _details;
}
WGuiAward::~WGuiAward(){
  GameOptionAward * goa = dynamic_cast<GameOptionAward*>(&options[id]);
  if(goa) 
    goa->setViewed(true);
}
bool WGuiAward::Visible(){
  //WGuiAward is only visible when it's tied to an already acchieved award.
  GameOptionAward * goa = dynamic_cast<GameOptionAward*>(&options[id]);
  if(!goa || !goa->number)
    return false; 
  return true;
}

//WGuiImage
WGuiImage::WGuiImage(WDataSource * wds, float _w, float _h, int _margin): WGuiItem("") {
  imgW = _w;
  imgH = _h;
  margin = _margin;
  source = wds;
}

void WGuiImage::imageScale(float _w, float _h){
  imgH = _h;
  imgW = _w;
}

float WGuiImage::getHeight(){
  JQuad * q = NULL;

  if(imgH == 0 ){
    if(source && (q = source->getImage())) //Intentional assignment.
      return MAX(height,q->mHeight+(2*margin));
  }
    
  return MAX(height,imgH+(2*margin));
}

void WGuiImage::Render(){
  if(!source)
    return;

  JRenderer * renderer = JRenderer::GetInstance();
  JQuad * q = source->getImage();
  if(q){
    float xS = 1, yS = 1;
    if(imgH != 0 && q->mHeight != 0)
      yS = imgH / q->mHeight;
    if(imgW != 0 && q->mWidth != 0)
      xS = imgW / q->mWidth;

    renderer->RenderQuad(q,x+margin, y+margin,0,xS,yS);
  }
}

WGuiCardImage::WGuiCardImage(WDataSource * wds, bool _thumb) : WGuiImage(wds){
  bThumb = _thumb;
};
void WGuiCardImage::Render(){

  JRenderer * renderer = JRenderer::GetInstance();

  MTGCard * c = NULL;
  Pos p(x+margin, y+margin, 1,0,255);

  if(!source || (c = source->getCard(mOffset.getPos())) == NULL){ //No card, use card back.
      JQuad * q;      
    if(bThumb){
      q = resources.GetQuad("back_thumb");
#if defined WIN32 || defined LINUX
      if(!q)
        q = resources.GetQuad("back");
#endif
      }
    else
      q = resources.GetQuad("back");
    float scale = p.actZ * 257.f / q->mHeight;
    renderer->RenderQuad(q,p.x,p.y,0,scale,scale);
  }else{ //Have card.
    if(bThumb){ //Thumbnail.
      JQuad * q = NULL;
      if(!options[Options::DISABLECARDS].number){
        q = source->getThumb(mOffset.getPos());
#if defined WIN32 || defined LINUX
        if(!q)
          q = source->getImage(mOffset.getPos());
#endif
      }
      if(!q && (q = CardGui::alternateThumbQuad(c)) == NULL)
        return; //TODO Some kind of error image.
      renderer->RenderQuad(q,p.x,p.y);
    }
    else{ //Normal card.
      JQuad * q = source->getImage(mOffset.getPos());
      if(!q || options[Options::DISABLECARDS].number)
        CardGui::alternateRender(c,p);
      else
        CardGui::RenderBig(c,p);
    }
  }
}

//WGuiCardDistort
WGuiCardDistort::WGuiCardDistort(WDataSource * wds, bool _thumb, WDataSource * _distort): WGuiCardImage(wds,_thumb){
  mesh = NEW hgeDistortionMesh(2,2);
  distortSrc = NULL;
}
WGuiCardDistort::~WGuiCardDistort(){
  SAFE_DELETE(mesh);
}

void WGuiCardDistort::Render(){
  JRenderer * renderer = JRenderer::GetInstance();
  JQuad * q = NULL;

  if(distortSrc) {
    WDistort * dt = distortSrc->getDistort(mOffset.getPos());
    if(dt)
      xy = *dt;
  }
  
  if(!source){
    //Default to back.
    if(bThumb){
      q = resources.GetQuad("back_thumb");
#if defined WIN32 || defined LINUX
      if(!q)
        q = resources.GetQuad("back");
#endif
    }
    else
      q = resources.GetQuad("back");
  }else {
    MTGCard * c = source->getCard(mOffset.getPos());
    if(!c)
      return; 
    
    if(bThumb){
      q = source->getThumb(mOffset.getPos());
#if defined WIN32 || defined LINUX
      if(!q)
        q = source->getImage(mOffset.getPos());
#endif
      if(!q || options[Options::DISABLECARDS].number)
        q = CardGui::alternateThumbQuad(c);
    }
    else {
      q = source->getImage(mOffset.getPos());
      if(!q || options[Options::DISABLECARDS].number)
        q = CardGui::alternateThumbQuad(c); //TODO alternateX should render to texture.
    }
  }
  if(!q)
    return;
  mesh->SetTexture(q->mTex);
  float x0,y0,w0,h0;
  q->GetTextureRect(&x0,&y0,&w0,&h0);
  mesh->SetTextureRect(x0,y0,w0,h0);
  mesh->Clear(ARGB(0xFF,0xFF,0xFF,0xFF));
  mesh->SetDisplacement(0, 0, xy[0],xy[1], HGEDISP_NODE);
  mesh->SetDisplacement(1, 0, xy[2] - w0,xy[3], HGEDISP_NODE);
  mesh->SetDisplacement(0, 1,xy[4],xy[5]-h0, HGEDISP_NODE);
  mesh->SetDisplacement(1, 1, xy[6]-w0,xy[7]-h0, HGEDISP_NODE);
  if(hasFocus()){
    mesh->SetColor(1,1,ARGB(255,200,200,200));
    mesh->SetColor(0,1,ARGB(255,200,200,200));
    mesh->SetColor(1,0,ARGB(255,200,200,200));
    mesh->SetColor(0,0,ARGB(255,255,255,200));
  }else{
    mesh->SetColor(1,1,ARGB(255,100,100,100));
    mesh->SetColor(0,1,ARGB(255,100,100,100));
    mesh->SetColor(1,0,ARGB(255,100,100,100));
    mesh->SetColor(0,0,ARGB(255,200,200,200));
  }
  mesh->Render(0,0);

}

//WDistort
WDistort::WDistort(){
  for(int i=0;i<8;i++)
    xy[i] = 0;
}

float & WDistort::operator[](int p){
  return xy[p];
}

WDistort::WDistort(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4){
  xy[0] = x1;  xy[1] = y1;
  xy[2] = x2;  xy[3] = y2;
  xy[4] = x3;  xy[5] = y3;
  xy[6] = x4;  xy[7] = y4;
}

//WGuiListRow

void WGuiListRow::Render(){
  JRenderer * renderer = JRenderer::GetInstance();
  int listHeight=40; 
  int listSelectable=0;
  int adjustedCurrent=0;
  int start = 0, nowPos = 0, vHeight=0;
  int nbitems = (int) items.size();

  //List is empty.
  if (!items.size() && failMsg != ""){
    JLBFont * mFont = resources.GetJLBFont(Constants::OPTION_FONT);
    mFont->SetColor(getColor(WGuiColor::TEXT_FAIL));
    mFont->DrawString(_(failMsg).c_str(),x+width/2, y, JGETEXT_RIGHT);
    return;
  }

  //Force a selectable option.
  if(currentItem == -1){
    for (int i = 0 ; i < nbitems; i++){
      if(items[i]->Selectable()) {
        currentItem = i;
        if(hasFocus())
          items[currentItem]->Entering(JGE_BTN_NONE);
        break;
      }
    }
  }

  vHeight = 0;
  nowPos = 4;
  float nowVPos = 4;
  float tallestRow = 0;
  int numRows = 1;
  float cTallest = 0;

  //Render items.
  if(start >= 0)
  {
    //Render current underlay.
    if(currentItem >= 0 && currentItem < nbitems && items[currentItem]->Visible())
      items[currentItem]->Underlay();

    for (int pos=0;pos < nbitems; pos++){
      if(!items[pos]->Visible())
        continue;

      items[pos]->setX(x+nowPos);
      items[pos]->setY(y+nowVPos);
      //items[pos]->setWidth(width/nbitems);
      items[pos]->setWidth(items[pos]->minWidth());
      float temp = items[pos]->getHeight() + 3;
      if(temp > tallestRow)
        tallestRow = temp;
      if(temp > cTallest)
        cTallest = temp;
      nowPos += items[pos]->getWidth() + 5;
      renderBack(items[pos]);
      if(x+nowPos+items[pos]->getWidth()+10 > SCREEN_WIDTH){
        nowPos = 0 + 20; //Indent newlines.
        nowVPos += cTallest;
        cTallest = 0;
        numRows++;
      }
      items[pos]->Render();
      if(nowVPos > SCREEN_HEIGHT)
        break;
    }

    //Render current overlay.
    if(currentItem >= 0 && currentItem < nbitems && items[currentItem]->Visible())
      items[currentItem]->Overlay();
    }
    setHeight(tallestRow*numRows+10);
}
WGuiListRow::WGuiListRow(string n, WSyncable * s) : WGuiList(n,s) {
  buttonNext = JGE_BTN_RIGHT;
  buttonPrev = JGE_BTN_LEFT;
  width = SCREEN_WIDTH;
  height = 20;
}
//WGuiFilterUI
bool WGuiFilters::Finish(bool emptyset){
  bFinished = true;
  string src;
  if(source){
    src = getCode();
    source->clearFilters();
    if(src.size()){
      WCFilterFactory * wc = WCFilterFactory::GetInstance();
      WCardFilter * f = wc->Construct(src);
      if(recolorTo > -1 && recolorTo < Constants::MTG_NB_COLORS){
        f = NEW WCFilterAND(f,NEW WCFilterColor(recolorTo));
      }
      source->addFilter(f);
    }else {
      if(recolorTo > -1 && recolorTo < Constants::MTG_NB_COLORS){
        WCardFilter * f = NEW WCFilterColor(recolorTo);
        source->addFilter(f);
      }
    }
    if(!source->Size() && !emptyset){
      source->clearFilters(); //TODO: Pop a "No results found" warning
    }
  }
  return true;
}
void WGuiFilters::ButtonPressed(int controllerId, int controlId){
  if(controllerId == -102){
    if(controlId == -10){
      WGuiListRow * wgl= NEW WGuiListRow("");
      wgl->Add(NEW WGuiFilterItem(this));
      list->Add(wgl);
    }else if(controlId == -11){
      Finish();
    }
    else{
      if(source)
        source->clearFilters();
      SAFE_DELETE(list);
      buildList();
    }
  return;
  }else{
    if(list != NULL)
      list->ButtonPressed(controllerId,controlId);
  }
}
void WGuiFilters::buildList(){
  list = NEW WGuiList("");
  WGuiButton * l = NEW WGuiButton(NEW WGuiItem("Add Filter"),-102,-10,this);
  WGuiButton * r = NEW WGuiButton(NEW WGuiItem("Done"),-102,-11,this);
  WGuiButton * mid = NEW WGuiButton(NEW WGuiItem("Clear"),-102,-66,this);
  WGuiSplit * sub = NEW WGuiSplit(mid,r);
  WGuiSplit * wgs = NEW WGuiSplit(l,sub);
  subMenu = NULL; 
  list->Add(NEW WGuiHeader(displayValue));
  list->Add(wgs);
  list->Entering(JGE_BTN_NONE);
}
WGuiFilters::WGuiFilters(string header, WSrcCards * src) : WGuiItem(header) {
  bFinished = false;
  source = src;
  recolorTo = -1;
  buildList();
}
void  WGuiFilters::recolorFilter(int color){
  recolorTo = color;
}
string WGuiFilters::getCode(){
  if(!list)
    return "";
  string res;
  vector<WGuiBase*>::iterator row, col;
  for(row=list->items.begin();row!=list->items.end();row++){
    WGuiList * wgl =  dynamic_cast<WGuiList*>(*row);
    if(wgl){
      if(res.size())
        res += "|(";
      else
        res += "(";
      for(col=wgl->items.begin();col!=wgl->items.end();col++){    
        WGuiFilterItem * fi =  dynamic_cast<WGuiFilterItem*>(*col);
        if(fi){
          string gc = fi->getCode();
          if(res.size() && gc.size() && res[res.size()-1] != '(')
            res += "&";
          res += gc;
        }
      }
      res += ")";
    }
  }
  return res;
}
void WGuiFilters::Update(float dt){
   if(subMenu && !subMenu->closed) 
     subMenu->Update(dt); 
  if(list){
    list->Update(dt);
    WGuiList * wgl = dynamic_cast<WGuiList*>(list->Current());
    if(!wgl)
      return;
    vector<WGuiBase*>::iterator it;
    bool bDeleted=false;
    for(it=wgl->items.begin();it!=wgl->items.end();it++){
        WGuiFilterItem * wgfi = dynamic_cast<WGuiFilterItem*>(*it);
        if(!wgfi || wgfi->mState != WGuiFilterItem::STATE_REMOVE) continue;
        SAFE_DELETE(*it);
        it = wgl->items.erase(it);
        bDeleted = true;
    }
    if(bDeleted)
     wgl->Entering(JGE_BTN_NONE);
  }
}
void WGuiFilters::Entering(JButton key){
  bFinished = false;
  WGuiItem::Entering(key);
}
void WGuiFilters::Render(){
  if(!list) return; //Hurrah for paranoia.
  JRenderer * r = JRenderer::GetInstance();
  float tX, tY;
  tX = getX();
  tY = getY();
  r->FillRect(0,0,SCREEN_WIDTH,SCREEN_HEIGHT,ARGB(128,0,0,0));
  list->setX(tX); list->setY(tY);
  list->Render();
  
  if(subMenu && !subMenu->closed) 
    subMenu->Render();
}
bool WGuiFilters::CheckUserInput(JButton key){
  if(subMenu && !subMenu->closed && subMenu->CheckUserInput(key))
    return true;
  if(key == JGE_BTN_SEC){//|| key == JGE_BTN_MENU){
    //TODO Pop up a "Are you sure?" dialog.
    return true;
  }
  if(list){
    return list->CheckUserInput(key);
  }
  return WGuiItem::CheckUserInput(key);
}
WGuiFilters::~WGuiFilters(){
  SAFE_DELETE(list);
  SAFE_DELETE(subMenu); 
}
void WGuiFilters::addColumn(){
  if(!list)
    return;
  WGuiList * wgl = dynamic_cast<WGuiList*>(list->Current());
  if(wgl){
    wgl->currentItem = wgl->items.size()-1;
    wgl->Add(NEW WGuiFilterItem(this));
  }
}
bool WGuiFilters::isAvailableCode(string code){
if(!list) return false;
  WGuiList * wgl = dynamic_cast<WGuiList*>(list->Current());
  if(wgl){
    vector<WGuiBase*>::iterator it;
    for(it=wgl->items.begin();it!=wgl->items.end();it++){
      WGuiFilterItem * wgfi = dynamic_cast<WGuiFilterItem*>(*it);
      if(!wgfi || wgfi->mState != WGuiFilterItem::STATE_FINISHED) continue;
      if(!wgfi->mCode.size()) continue;
      if(wgfi->mCode == code)
        return false;
    }
    return true;
  }
  return false; //For some reason, we don't have any rows?
}
bool WGuiFilters::isAvailable(int type){
 if(!list) return false;
 int colors = 0, ma = 0;
  WGuiList * wgl = dynamic_cast<WGuiList*>(list->Current());
  if(wgl){
    vector<WGuiBase*>::iterator it;
    for(it=wgl->items.begin();it!=wgl->items.end();it++){
      WGuiFilterItem * wgfi = dynamic_cast<WGuiFilterItem*>(*it);
      if(!wgfi || wgfi->mState != WGuiFilterItem::STATE_FINISHED) continue;
      switch(type){
        case WGuiFilterItem::FILTER_BASIC:
          return true;
        case WGuiFilterItem::FILTER_PRODUCE:
          if(wgfi->filterType == type)
            ma++;
          break;
        case WGuiFilterItem::FILTER_COLOR:
          if(wgfi->filterType == type)
            colors++;
          break;
        default:
          if(wgfi->filterType == type)
            return false;
      }
    }
    if(colors >= 5)
      return false;
    if(ma >= 5)
      return false;
    return true;
  }
  return false; //For some reason, we don't have any rows?
}
void WGuiFilters::clearArgs(){
  tempArgs.clear();
}
void WGuiFilters::addArg(string display, string code){
  if(!subMenu || !isAvailableCode(code))
    return;
  subMenu->Add((int)tempArgs.size(),display.c_str());
  tempArgs.push_back(pair<string,string>(display,code));
}

//WGuiFilterItem
WGuiFilterItem::WGuiFilterItem(WGuiFilters * parent): WGuiItem("Cards..."){
  filterType = -1; filterVal = -1; mState = 0;
  mParent = parent;  mNew = true;
};
void WGuiFilterItem::updateValue(){
  bool delMenu = true;
  char buf_name[512];
  char buf_code[512];
  if(!mParent)
    return;
  switch(mState){
    case STATE_CANCEL:
      mState = STATE_UNSET;
      break;
    case STATE_FINISHED:
    case STATE_UNSET:
      SAFE_DELETE(mParent->subMenu);
      mState = STATE_CHOOSE_TYPE;
      mParent->subMenu = NEW SimpleMenu(-1234,this,Constants::MENU_FONT,20,20,"Filter By...",10);
      if(mParent->isAvailable(FILTER_SET)){
        mParent->subMenu->Add(FILTER_SET,"Set");
        delMenu = false;
      }
      if(mParent->isAvailable(FILTER_COLOR)){
        mParent->subMenu->Add(FILTER_COLOR,"Color");
        delMenu = false;
      }
      if(mParent->isAvailable(FILTER_TYPE)){
        mParent->subMenu->Add(FILTER_TYPE,"Type");
        delMenu = false;
      }
      if(mParent->isAvailable(FILTER_RARITY)){
        mParent->subMenu->Add(FILTER_RARITY,"Rarity");
        delMenu = false;
      }
      if(mParent->isAvailable(FILTER_CMC)){
        mParent->subMenu->Add(FILTER_CMC,"Mana Cost");
        delMenu = false;
      }
      if(mParent->isAvailable(FILTER_BASIC)){
        mParent->subMenu->Add(FILTER_BASIC,"Basic Ability");
        delMenu = false;
      }
      if(mParent->isAvailable(FILTER_PRODUCE)){
        mParent->subMenu->Add(FILTER_PRODUCE,"Mana Ability");
        delMenu = false;
      }
      if(mParent->isAvailable(FILTER_POWER)){
        mParent->subMenu->Add(FILTER_POWER,"Power");
        delMenu = false;
      }
      if(mParent->isAvailable(FILTER_TOUGH)){
        mParent->subMenu->Add(FILTER_TOUGH,"Toughness");
        delMenu = false;
      }
      if(mParent->isAvailable(FILTER_ALPHA)){
        mParent->subMenu->Add(FILTER_ALPHA,"First Letter");
        delMenu = false;
      }
      if(!mNew)
        mParent->subMenu->Add(-2,"Remove");
      mParent->subMenu->Add(-1,"Cancel");
      if(delMenu){
        SAFE_DELETE(mParent->subMenu);
        mState = STATE_FINISHED;
      }
      break;
    case STATE_CHOOSE_TYPE:
      SAFE_DELETE(mParent->subMenu);
      mParent->clearArgs();
      mState = STATE_CHOOSE_VAL;
      mParent->subMenu = NEW SimpleMenu(-1234,this,Constants::MENU_FONT,20,20,"Filter:");
      if(filterType == FILTER_TYPE){
        mParent->addArg("Artifact","t:Artifact;");
        mParent->addArg("Artifact Creature","t:Artifact;&t:Creature;");
        mParent->addArg("Creature","t:Creature;");
        mParent->addArg("Enchantment","t:Enchantment;");
        mParent->addArg("Instant","t:Instant;");
        mParent->addArg("Land","t:Land;");
        mParent->addArg("Legendary","t:Legendary;");
        mParent->addArg("Sorcery","t:Sorcery;");        
      }else if(filterType == FILTER_RARITY){
        mParent->addArg("Mythic","r:m;");
        mParent->addArg("Rare","r:r;");
        mParent->addArg("Uncommon","r:u;");
        mParent->addArg("Common","r:c;");
      }else if(filterType == FILTER_CMC){
        for(int i=0;i<20;i++){
          sprintf(buf_code,"cmc:%i;",i);
          sprintf(buf_name,"%i Mana",i);
          mParent->addArg(buf_name,buf_code);
        }
      }else if(filterType == FILTER_POWER){
        for(int i=0;i<14;i++){
          sprintf(buf_code,"pow:%i;",i);
          sprintf(buf_name,"%i power",i);
          mParent->addArg(buf_name,buf_code);
        }
      }else if(filterType == FILTER_TOUGH){
        for(int i=0;i<14;i++){
          sprintf(buf_code,"tgh:%i;",i);
          sprintf(buf_name,"%i toughness",i);
          mParent->addArg(buf_name,buf_code);
        }
      }else if(filterType == FILTER_COLOR){
        mParent->addArg("White","c:w;");
        mParent->addArg("Blue","c:u;");
        mParent->addArg("Black","c:b;");
        mParent->addArg("Red","c:r;");
        mParent->addArg("Green","c:g;");
        mParent->addArg("Exclusively White","xc:w;");
        mParent->addArg("Exclusively Blue","xc:u;");
        mParent->addArg("Exclusively Black","xc:b;");
        mParent->addArg("Exclusively Red","xc:r;");
        mParent->addArg("Exclusively Green","xc:g;");
      }else if(filterType == FILTER_PRODUCE){
        mParent->addArg("White mana abiltity","ma:w;");
        mParent->addArg("Blue mana abiltity","ma:u;");
        mParent->addArg("Black mana abiltity","ma:b;");
        mParent->addArg("Red mana abiltity","ma:r;");
        mParent->addArg("Green mana abiltity","ma:g;");
        mParent->addArg("Colorless mana abiltity","ma:x;");
      }else if(filterType == FILTER_BASIC){
        char buf[512];
        for(int i=0;i<Constants::NB_BASIC_ABILITIES;i++){
          string s = Constants::MTGBasicAbilities[i];
          sprintf(buf,"a:%s;",s.c_str());
          s[0] = toupper(s[0]);
          mParent->addArg(s,buf);
        }
      }else if(filterType == FILTER_SET){
        char buf[512];
        for(int i=0;i<setlist.size();i++){
          if(options[Options::optionSet(i)].number == 0)
            continue;
          sprintf(buf,"s:%s;",setlist[i].c_str());
          mParent->addArg((setlist.getInfo(i))->getName(),buf);
        }
      }else if(filterType == FILTER_ALPHA){
        char buf[24], pretty[16];
        for(char c='a';c<='z';c++){
          sprintf(buf,"alpha:%c;",c);
          sprintf(pretty,"Letter %c",toupper(c));
          mParent->addArg(pretty,buf);
        }
        mParent->addArg("Digit","alpha:#;");
      }
      mParent->subMenu->Add(-1,"Cancel");
    break;
    case STATE_CHOOSE_VAL:
      mState = STATE_FINISHED;
      if(mNew && mParent)
        mParent->addColumn();
      mNew = false;
      if(filterVal > -1 && filterVal < (int) mParent->tempArgs.size()){
        displayValue = mParent->tempArgs[filterVal].first;
        mCode = mParent->tempArgs[filterVal].second;
      }
      SAFE_DELETE(mParent->subMenu);
      break;
  }
}
void WGuiFilterItem::ButtonPressed(int controllerId, int controlId){
  if(!mParent) return;

  switch(mState){
    case STATE_CHOOSE_TYPE:
      if(controlId == -1){
        mParent->subMenu->Close();
        mState = STATE_CANCEL; return;
      }else if(controlId == -2){
        mParent->subMenu->Close();
        mState = STATE_REMOVE; return;
      }
      filterType = controlId;
      break;
    case STATE_CHOOSE_VAL:
      if(controlId == -1){
        mParent->subMenu->Close();
        mState = STATE_UNSET; return;
      }else if(controlId == -2){
        mParent->subMenu->Close();
        mState = STATE_REMOVE; return;
      }
      filterVal = controlId;
      break;
  }
  updateValue();
}

bool WGuiFilterItem::isModal(){
  switch(mState){
    case STATE_UNSET:
    case STATE_REMOVE:
    case STATE_CANCEL:
    case STATE_FINISHED:
      return false;
  }
  return true;
}

string WGuiFilterItem::getCode(){
  if(mState != STATE_FINISHED || !mCode.size())
    return "";
  return mCode;
}

WGuiKeyBinder::WGuiKeyBinder(string name, GameStateOptions* parent) : WGuiList(name), parent(parent), modal(false) {
  JGE* j = JGE::GetInstance();
  JGE::keybindings_it start = j->KeyBindings_begin(), end = j->KeyBindings_end();

  u32 y = 40;
  for (JGE::keybindings_it it = start; it != end; ++it)
    Add(NEW OptionKey(parent, it->first, it->second));
}
bool WGuiKeyBinder::isModal() { return modal; }
bool WGuiKeyBinder::CheckUserInput(JButton key)
{
  switch (key) {
    case JGE_BTN_OK: items[currentItem]->CheckUserInput(key); break;
    default:
      return WGuiList::CheckUserInput(key);
    }
  return false;
}
