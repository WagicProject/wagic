#include "../include/config.h"
#include "../include/OptionItem.h"
#include "../include/GameApp.h"
#include <JGE.h>
#include "../include/GameOptions.h"
#include "../include/PlayerData.h"
#include "../include/Translate.h"
#include <dirent.h>
#include <stdlib.h>
#include <algorithm>

//Option Item

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

ostream& OptionItem::toString(ostream& out) const{
return out << "OptionItem ::: displayValue : " << displayValue
	     << " ; id : " << id
	     << " ; hasFocus : " << hasFocus
	     << " ; x,y : " << x << "," << y;
}

OptionItem::OptionItem( string _id,  string _displayValue) {
 id = _id;
 displayValue = _(_displayValue);
 canSelect=true;
 hasFocus=false;
 width = SCREEN_WIDTH;
 height = 20;
}

//Option Integer 

void OptionInteger::Render(){
  JLBFont * mFont = GameApp::CommonRes->GetJLBFont("graphics/f3");
  if (hasFocus){
    mFont->SetColor(options[Metrics::OPTION_ITEM_TCH].asColor(ARGB(255,255,255,0)));
  }else{
    mFont->SetColor(options[Metrics::OPTION_ITEM_TC].asColor());
  }
  JRenderer * renderer = JRenderer::GetInstance();
  renderer->FillRoundRect(x-5,y-2,width-x-5,height,2,options[Metrics::OPTION_ITEM_FC].asColor(ARGB(150,50,50,50)));
  mFont->DrawString(displayValue.c_str(),x,y);
  char buf[512];
  if (maxValue == 1){
    if (value){
      sprintf(buf, _("Yes").c_str());
    }else{
      sprintf(buf,_("No").c_str());
    }
  }else{
    sprintf(buf, "%i", value);
  }
  mFont->DrawString(buf,width -10 ,y,JGETEXT_RIGHT);
}

OptionInteger::OptionInteger(string _id, string _displayValue, int _maxValue, int _increment): OptionItem(_id, _displayValue){
 
  maxValue = _maxValue;
  increment = _increment;
  value = ::options[id].number;
  hasFocus = false;
  x = 0;
  y = 0;
}

void OptionInteger::setData(){
  if(id != "") 
    options[id] = GameOption(value);
}


ostream& OptionInteger::toString(ostream& out) const{
  return out << "OptionItem ::: displayValue : " << displayValue
	     << " ; id : " << id
	     << " ; value : " << value
	     << " ; hasFocus : " << hasFocus
	     << " ; maxValue : " << maxValue
	     << " ; increment : " << increment
	     << " ; x,y : " << x << "," << y;
}

//Option Select

void OptionSelect::initSelections(){
  //Find currently active bit in the list.
    for(int i=0;i<selections.size();i++)
    {
      if(selections[i] == options[id].str)
        value = i;
    }
}

void OptionSelect::Render(){
  JLBFont * mFont = GameApp::CommonRes->GetJLBFont("graphics/f3");
  if (hasFocus){
    mFont->SetColor(options[Metrics::OPTION_ITEM_TCH].asColor(ARGB(255,255,255,0)));
  }else{
    mFont->SetColor(options[Metrics::OPTION_ITEM_TC].asColor());
  }

  JRenderer * renderer = JRenderer::GetInstance();
  renderer->FillRoundRect(x-5,y-2,width-x-5,height,2,options[Metrics::OPTION_ITEM_FC].asColor(ARGB(150,50,50,50)));
  mFont->DrawString(displayValue.c_str(),x,y);

  if(value >= 0 && value < selections.size())
    mFont->DrawString(selections[value].c_str(),width-10,y,JGETEXT_RIGHT);
  else
   mFont->DrawString("Unset",width-10,y,JGETEXT_RIGHT);
}

void OptionSelect::setData()
{
  if(id == "") return;
  if(value >= 0 && value < selections.size())
    options[id] = GameOption(selections[value]);
}

void OptionSelect::addSelection(string s)
{
  selections.push_back(s);
}


ostream& OptionSelect::toString(ostream& out) const
{
  return out << "OptionItem ::: displayValue : " << displayValue
	     << " ; id : " << id
	     << " ; value : " << value
	     << " ; hasFocus : " << hasFocus
	     << " ; x,y : " << x << "," << y;
}

//OptionHeader

void OptionHeader::Render(){
  JLBFont * mFont = GameApp::CommonRes->GetJLBFont("graphics/f3");
  mFont->SetColor(options[Metrics::OPTION_HEADER_TC].asColor());
 
  JRenderer * renderer = JRenderer::GetInstance();
  renderer->FillRoundRect(x-5,y-2,width-x-5,height,2,options[Metrics::OPTION_HEADER_FC].asColor(ARGB(150,80,80,80)));
  mFont->DrawString(displayValue.c_str(),width/2,y,JGETEXT_CENTER);
}

void OptionText::Render(){
  JLBFont * mFont = GameApp::CommonRes->GetJLBFont("graphics/f3");
  mFont->SetScale(.8);
  mFont->SetColor(options[Metrics::OPTION_TEXT_TC].asColor());
 
  JRenderer * renderer = JRenderer::GetInstance();
  renderer->FillRoundRect(x-5,y-2,width-x-5,height,2,options[Metrics::OPTION_TEXT_FC].asColor(ARGB(150,80,80,80)));
  mFont->DrawString(displayValue.c_str(),x,y,JGETEXT_LEFT); 
  mFont->SetScale(1);
}

//OptionProfile

OptionProfile::~OptionProfile(){
  SAFE_DELETE(mAvatarTex);
  SAFE_DELETE(mAvatar);
}

int OptionProfile::Submode(){
  if(value != initialValue && bCheck){
     bCheck=false; //Just about to check it!
     return OPTIONS_SUBMODE_PROFILE;
  }  
  return OPTIONS_SUBMODE_NORMAL;
}

void OptionProfile::addSelection(string s){
  OptionDirectory::addSelection(s);

  //Check how many options... if 1, we're not selectable.
  if(selections.size() > 1)
    canSelect = true;
  else
    canSelect = false;
}

OptionProfile::OptionProfile(GameApp * _app): OptionDirectory(RESPATH"/profiles",Options::ACTIVE_PROFILE, "Profile"){
  app = _app;
  height=100;
  addSelection("Default");
  sort(selections.begin(),selections.end());
  initSelections();
  mAvatarTex=NULL;
  mAvatar=NULL;
  hasFocus=false;
  populate();
  bCheck=false;
};

void OptionProfile::updateValue(){
 value++; 
 if (value > selections.size() - 1 || value < 0)
   value=0;
 
  populate();
}

void OptionProfile::populate(){ 

 JRenderer * renderer = JRenderer::GetInstance();
 string temp = options[Options::ACTIVE_PROFILE].str;
 if(value < 0 || value >= selections.size()){ //TODO fail gracefully.
   return;
 }
 
 options[Options::ACTIVE_PROFILE].str = selections[value];
 
 SAFE_DELETE(mAvatarTex);
 mAvatarTex = JRenderer::GetInstance()->LoadTexture(options.profileFile("avatar.jpg","",true,true).c_str(), false);  
 if (mAvatarTex) {
   SAFE_DELETE(mAvatar);   
   mAvatar = NEW JQuad(mAvatarTex, 0, 0, 35, 50);
   renderer->BindTexture(mAvatarTex); //Prevents font corruption.
 }
 else
   mAvatar = NULL;

 options.checkProfile();
 PlayerData * pdata = NEW PlayerData(app->collection);

 options[Options::ACTIVE_PROFILE] = temp;
 
 char buf[512];
 sprintf(buf,"Credits: %i\nCards: %i",pdata->credits,pdata->collection->totalCards());
 preview = buf;


 SAFE_DELETE(pdata);
}

void OptionProfile::Render(){ 
  JRenderer * renderer = JRenderer::GetInstance();
  JLBFont * mFont = GameApp::CommonRes->GetJLBFont("graphics/f3");
  mFont->SetScale(1);
  int spacing = 2+(int)mFont->GetHeight();

  //Draw faux option. Not highlighted if we've only one.
  if (hasFocus && selections.size() > 1){
    mFont->SetColor(options[Metrics::OPTION_ITEM_TCH].asColor(ARGB(255,255,255,0)));
    renderer->FillRoundRect(x-5,y-2,width-x-5,20,2,options[Metrics::OPTION_HEADER_FC].asColor(ARGB(150,80,80,80)));
    mFont->DrawString("Change Profile",SCREEN_WIDTH/2,y,JGETEXT_CENTER);
  }else{
    mFont->SetColor(options[Metrics::OPTION_ITEM_TC].asColor());
    renderer->FillRoundRect(x-5,y-2,width-x-5,20,2,options[Metrics::OPTION_HEADER_FC].asColor(ARGB(150,80,80,80)));
    mFont->DrawString("Profile",SCREEN_WIDTH/2,y,JGETEXT_CENTER);
  }


  //Draw preview box.
  renderer->FillRoundRect(x-5,y-2+25,width-x-5,height-25,2,options[Metrics::OPTION_ITEM_FC].asColor(ARGB(150,50,50,50)));
  
  int pX, pY;
  pX = x;
  pY = y+30;
  if(mAvatar){
    renderer->RenderQuad(mAvatar,x,pY);
    pX += 40;
  }

  mFont->SetColor(options[Metrics::OPTION_TEXT_TC].asColor()); 
  mFont->DrawString(selections[value].c_str(),pX,pY,JGETEXT_LEFT);
  mFont->SetScale(.8);
  mFont->DrawString(preview.c_str(),pX,pY+spacing,JGETEXT_LEFT); 
  mFont->SetScale(1);
}
void OptionProfile::Update(float dt){
  JGE * mEngine = JGE::GetInstance();
  if (hasFocus){
    if (mEngine->GetButtonClick(PSP_CTRL_CIRCLE)) updateValue();
  }
}

void OptionProfile::Entering(){
  JLBFont * menuFont = GameApp::CommonRes->GetJLBFont(Constants::MENU_FONT);
  hasFocus = true;
  bCheck = false; 
  initialValue = value;
}
bool OptionProfile::Leaving(){

  //Choice must be confirmed.
  if(value != initialValue){
    bCheck = true;
    return false;
  }  
  hasFocus = false;
  return true;
}

void OptionProfile::cancelSubmode()
{
  if(initialValue < 0 || initialValue >= selections.size())
    return;

    options[Options::ACTIVE_PROFILE] = selections[initialValue];
    value = initialValue;
    populate();
}
void OptionProfile::acceptSubmode()
{
  if(value < 0 || value >= selections.size())
    return;

    options[Options::ACTIVE_PROFILE] = selections[value];
    initialValue = value;
    populate(); 
}

//OptionDirectory
void OptionDirectory::Reload(){
  DIR *mDip;
  struct dirent *mDit;
  char buf[4096];
  mDip = opendir(root.c_str());

  while ((mDit = readdir(mDip))){
    if(mDit->d_name[0] != '.'){    
      sprintf(buf,"%s/%s",root.c_str(),mDit->d_name);
      std::ifstream file(buf);
      if(file){
        file.close();
        continue;
      }
      if(find(selections.begin(),selections.end(),mDit->d_name) == selections.end())
        addSelection(mDit->d_name);
    }     
  }

  closedir(mDip);
  mDip = NULL;
  initSelections();
}

OptionDirectory::OptionDirectory(string _root, string _id, string _displayValue): OptionSelect(_id, _displayValue){
  DIR *mDip;
  struct dirent *mDit;
  char buf[4096];
  root = _root;
  mDip = opendir(root.c_str());

  while ((mDit = readdir(mDip))){
    if(mDit->d_name[0] != '.'){    
      sprintf(buf,"%s/%s",root.c_str(),mDit->d_name);
      std::ifstream file(buf);
      if(file){
        file.close();
        continue;
      }
      addSelection(mDit->d_name);
    }     
  }

  closedir(mDip);
  mDip = NULL;
  initSelections();
}

OptionsList::OptionsList(string name){
  sectionName = name;
  failMsg = "NO OPTIONS AVAILABLE";
  nbitems = 0;
  current = -1;
}
OptionsList::~OptionsList(){
  for (int i = 0 ; i < nbitems; i++){
    SAFE_DELETE(listItems[i]);
  }
}

void OptionsList::Add(OptionItem * item){
  if (nbitems < MAX_OPTION_ITEMS){
    listItems[nbitems] = item;
    nbitems++;
  }
}
bool OptionsList::Leaving(){
  if(current >= 0 && current < nbitems)
    return listItems[current]->Leaving();

  return true;
}
void OptionsList::Entering(){  
  //Try to force a selectable option.
  if(current == -1){
    for (int i = 0 ; i < nbitems; i++){
      if(listItems[i]->canSelect) {
        current = i;
        listItems[current]->Entering();
        break;
      }
    }
  }

  if(current >= 0 && current < nbitems)
    listItems[current]->Entering();

  return;
}
void OptionsList::Render(){
  JRenderer * renderer = JRenderer::GetInstance();

  int width = SCREEN_WIDTH;
  int listHeight=40; 
  int listSelectable=0;
  int adjustedCurrent=0;
  int start = 0, nowPos = 0, vHeight=0;

  
  //List is empty.
  if (!nbitems && failMsg != ""){
    JLBFont * mFont = GameApp::CommonRes->GetJLBFont("graphics/f3");
    mFont->SetColor(options[Metrics::MSG_FAIL_TC].asColor(ARGB(255,155,155,155)));
    mFont->DrawString(failMsg.c_str(),SCREEN_WIDTH/2, 40, JGETEXT_RIGHT);
    return;
  }

  //Force a selectable option.
  if(current == -1){
    for (int i = 0 ; i < nbitems; i++){
      if(listItems[i]->canSelect) {
        current = i;
        listItems[current]->Entering();
        break;
      }
    }
  }
  //Find out how large our list is.
  for (int pos=0;pos < nbitems; pos++){
    listHeight+=listItems[pos]->height+5;
    if(listItems[pos]->canSelect){
      listSelectable++;
      if(pos < current) adjustedCurrent++;
    }
  }



  //Always fill screen
  if(listHeight > SCREEN_HEIGHT)
  {
    width -= 10;    
    for (start=current;start > 0; start--)    {
      vHeight += listItems[start]->height+5;
      if(vHeight >= (SCREEN_HEIGHT-60)/2)
        break;
    }
    vHeight = 0;
    for (nowPos=nbitems;nowPos > 1; nowPos--)  
      vHeight += listItems[nowPos-1]->height+5;

    if(vHeight <= SCREEN_HEIGHT-40 && nowPos < start)
      start = nowPos;

  }

  vHeight = 0;
  nowPos = 40;

  //Render items.
  if(start >= 0)
  {
    for (int pos=0;pos < nbitems; pos++){
      if(pos < start){
        vHeight += listItems[pos]->height + 5;
        continue;
      }
      listItems[pos]->x = 10; 
      listItems[pos]->y = nowPos;
      listItems[pos]->width = width;
      nowPos += listItems[pos]->height + 5;
      listItems[pos]->Render();
      if(nowPos > SCREEN_HEIGHT) 
        break;
    }

    //Draw scrollbar
    if(listHeight > SCREEN_HEIGHT && listSelectable > 1){
      int barPosition = 35+((float)adjustedCurrent/listSelectable)*(SCREEN_HEIGHT-40);
      int barLength = (SCREEN_HEIGHT-40) / listSelectable;
      if(barLength < 4) barLength = 4;
      width = (SCREEN_WIDTH-(width-5))/2; //Find center of blank space by options.
      renderer->FillRect(SCREEN_WIDTH-width-1,39,2,SCREEN_HEIGHT-42,
        options[Metrics::OPTION_SCROLLBAR_FC].asColor(ARGB(150,150,150,150)));
      renderer->FillRoundRect(SCREEN_WIDTH-width-4,barPosition,5,barLength,2,
        options[Metrics::OPTION_SCROLLBAR_FCH].asColor(ARGB(255,255,255,255)));
    }
  }
}

void OptionsList::save(){
  for (int i = 0; i < nbitems; i++){
    listItems[i]->setData();
  }
  ::options.save();
}

void OptionsList::Update(float dt){
  JGE * mEngine = JGE::GetInstance();
  int potential = current;
  
  if (mEngine->GetButtonClick(PSP_CTRL_UP))
  {
    if (potential > 0){
      potential--;
      while(potential > 0 && listItems[potential]->canSelect == false)
        potential--;
      if(potential < 0 || !listItems[potential]->canSelect)
        potential = -1;
      else if(listItems[current]->Leaving()){
          current = potential;
          listItems[current]->Entering();
      }
    }
  }
  else if (mEngine->GetButtonClick(PSP_CTRL_DOWN))
  { 
    if (potential < nbitems-1){
      potential++;
      while(potential < nbitems-1 && listItems[potential]->canSelect == false)
        potential++;
      if(potential == nbitems || !listItems[potential]->canSelect)
        potential = -1;
      else if(potential != current && listItems[current]->Leaving()){
          current = potential;
          listItems[current]->Entering();
      }
    }
  }
  for (int i = 0 ; i < nbitems; i++){
    listItems[i]->Update(dt);
  }
}


void OptionsMenu::Add(OptionsList * tab){
  if (nbitems < MAX_OPTION_TABS){
    tabs[nbitems] = tab;
    nbitems++;
    if (current < 0){
      current = 0;
    }

  }
}

void OptionsMenu::Render(){
  JLBFont * mFont = GameApp::CommonRes->GetJLBFont("graphics/f3");
  
  if (!nbitems){
    mFont->DrawString("NO OPTIONS AVAILABLE",SCREEN_WIDTH/2, 5, JGETEXT_RIGHT);
    return;
  }

  JRenderer * renderer = JRenderer::GetInstance();
  
  int offset = 0;
  for(int i=0;i<nbitems;i++){
    int w = mFont->GetStringWidth(tabs[i]->sectionName.c_str());
    if(i == current){
      mFont->SetColor(options[Metrics::OPTION_TAB_TCH].asColor());
      renderer->FillRoundRect(offset+5,5,w + 5,25,2,options[Metrics::OPTION_TAB_FCH].asColor(ARGB(150,150,150,150)));
    }
    else{
      mFont->SetColor(options[Metrics::OPTION_TAB_TC].asColor(ARGB(255,155,155,155)));
      renderer->FillRoundRect(offset+5,5,w + 5,25,2,options[Metrics::OPTION_TAB_FC].asColor(ARGB(150,50,50,50)));
    }
    mFont->DrawString(tabs[i]->sectionName.c_str(),offset+10,10);
    offset += w + 10 + 2;
  }

  if(current > -1 && current < nbitems && tabs[current])
   tabs[current]->Render();
}

void OptionsMenu::Update(float dt){
  JGE * mEngine = JGE::GetInstance();
  if(current < 0 || current >= nbitems)   
    return;
  
  //We use the shoulder buttons to switch tabs, if we've got them.
  if (mEngine->GetButtonClick(PSP_CTRL_LTRIGGER))
    {
      if (current > 0)
      {
        if(tabs[current]->Leaving()){
          current--;
          tabs[current]->Entering();
        }
      }
    }
  else if (mEngine->GetButtonClick(PSP_CTRL_RTRIGGER))
    {
      if (current < nbitems -1)
      {
        if(tabs[current]->Leaving()){
	        current++;
          tabs[current]->Entering();
        }
      }
    }
    
  tabs[current]->Update(dt);
}

OptionsMenu::OptionsMenu(){
  nbitems=0;
  current=0;
  for(int x=0;x<MAX_OPTION_TABS;x++)
    tabs[x] = NULL;
}

OptionsMenu::~OptionsMenu(){
  for(int x=0;x<MAX_OPTION_TABS;x++)
      SAFE_DELETE(tabs[x]);
}

void OptionsMenu::save(){
  for(int x=0;x<MAX_OPTION_TABS;x++)
    if(tabs[x] != NULL) 
       tabs[x]->save();
}

bool OptionsMenu::isTab(string name){
  if(current <0 || current >= nbitems)
    return false;
  else if(tabs[current]->sectionName == name)
    return true;

  return false;
};

int OptionsMenu::Submode()
{
 if(current <0 || current >= nbitems)
    return OPTIONS_SUBMODE_NORMAL;

 return tabs[current]->Submode();
}

void OptionsMenu::acceptSubmode()
{
 if(current > -1 && current < nbitems)
    tabs[current]->acceptSubmode();
}
void OptionsMenu::reloadValues()
{
 if(current > -1 && current < nbitems)
    tabs[current]->reloadValues();
}

void OptionsMenu::cancelSubmode()
{
 if(current > -1 && current < nbitems)
    tabs[current]->cancelSubmode();
}


int OptionsList::Submode()
{
 if(current <0 || current >= nbitems)
    return OPTIONS_SUBMODE_NORMAL;

 return listItems[current]->Submode();
}

void OptionsList::reloadValues()
{
 for(int i=0;i<nbitems;i++) {
   if(listItems[i] != NULL)
     listItems[i]->Reload();
 }
}
void OptionsList::acceptSubmode()
{
 if(current > -1 && current < nbitems)
    listItems[current]->acceptSubmode();
}

void OptionsList::cancelSubmode()
{
 if(current > -1 && current < nbitems)
    listItems[current]->cancelSubmode();
}

//OptionString

void OptionString::Render(){

  JLBFont * mFont = GameApp::CommonRes->GetJLBFont("graphics/f3");
  if (hasFocus){
    mFont->SetColor(options[Metrics::OPTION_ITEM_TCH].asColor(ARGB(255,255,255,0)));
  }else{
    mFont->SetColor(options[Metrics::OPTION_ITEM_TC].asColor());
  }
  JRenderer * renderer = JRenderer::GetInstance();
  renderer->FillRoundRect(x-5,y-2,width-x-5,height,2,options[Metrics::OPTION_ITEM_FC].asColor(ARGB(150,50,50,50)));

  if(!bShowValue){
   mFont->DrawString(displayValue.c_str(),(x+width)/2,y,JGETEXT_CENTER);
  }
  else{
   mFont->DrawString(displayValue.c_str(),x,y);
   int w = mFont->GetStringWidth(value.c_str()-10);
   mFont->DrawString(value.c_str(),width - w,y,JGETEXT_RIGHT);
  }
}

void OptionString::setData(){
  if(id != "") 
  options[id] = GameOption(value);
}
void OptionString::updateValue(){
    options.keypadStart(value,&value);
    options.keypadTitle(displayValue);
}

void OptionNewProfile::updateValue(){
    options.keypadStart("",&value);
    options.keypadTitle(displayValue);
}

void OptionNewProfile::Update(float dt){
  if(value != ""){
    string temp;
    temp = options[Options::ACTIVE_PROFILE].str;
    value = options.keypadFinish();
    if(value == "")
      return;
    
    if(temp != value){
    options[Options::ACTIVE_PROFILE] = value;
    options.checkProfile();
    }
    value = "";
    bChanged = true;
  }
  OptionItem::Update(dt);
}

int OptionNewProfile::Submode(){
  if(bChanged){
     bChanged=false; //Just about to check it!
     return OPTIONS_SUBMODE_RELOAD;
  }  
  return OPTIONS_SUBMODE_NORMAL;
}
OptionString::OptionString(string _id, string _displayValue): OptionItem(_id, _displayValue)
{
  bShowValue=true;
  if(_id != "")
    value=options[_id].str;
};

ostream& OptionString::toString(ostream& out) const{
return out << "OptionString ::: displayValue : " << displayValue
	     << " ; id : " << id
       << " ; value : " << value
	     << " ; hasFocus : " << hasFocus
	     << " ; x,y : " << x << "," << y;
}