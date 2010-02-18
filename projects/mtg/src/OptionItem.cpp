#include "../include/config.h"
#include "../include/OptionItem.h"
#include <JGE.h>
#include "../include/PlayerData.h"
#include "../include/Translate.h"
#include "../include/Subtypes.h"
#include "../include/TranslateKeys.h"
#include <dirent.h>
#include <stdlib.h>
#include <algorithm>

//OptionItem
OptionItem::OptionItem( int _id,  string _displayValue): WGuiItem(_displayValue) {
 id = _id;
 mFocus=false;
}

//OptionInteger
void OptionInteger::Render(){
  JLBFont * mFont = resources.GetJLBFont(Constants::OPTION_FONT);
  mFont->SetColor(getColor(WGuiColor::TEXT));
  JRenderer * renderer = JRenderer::GetInstance();

  mFont->DrawString(_(displayValue).c_str(),x,y);
  char buf[512];
  if (maxValue == 1){
    if (value)
      sprintf(buf, "%s", _("Yes").c_str());
    else
      sprintf(buf, "%s", _("No").c_str());
  }else{
    if(value == defValue && strDefault.size())
      sprintf(buf, "%s", _(strDefault).c_str());
    else
      sprintf(buf, "%i", value);
  }
  mFont->DrawString(buf,width -10 ,y,JGETEXT_RIGHT);
}

OptionInteger::OptionInteger(int _id, string _displayValue, int _maxValue, int _increment, int _defV, string _sDef, int _minValue): OptionItem(_id, _displayValue){
  defValue = _defV;
  strDefault = _sDef;
  maxValue = _maxValue;
  minValue = _minValue;
  increment = _increment;
  value = ::options[id].number;
  x = 0;
  y = 0;
}

void OptionInteger::setData(){
  if(id != INVALID_OPTION)
    options[id] = GameOption(value);
}

//Option Select
void OptionSelect::initSelections(){
  //Find currently active bit in the list.
    for(size_t i=0;i<selections.size();i++){
      if(selections[i] == options[id].str)
        value = i;
    }
}

void OptionSelect::Entering(JButton key){
  OptionItem::Entering(key);
  prior_value = value;
}

void OptionSelect::Render(){
  JLBFont * mFont = resources.GetJLBFont(Constants::OPTION_FONT);
  mFont->SetColor(getColor(WGuiColor::TEXT));

  JRenderer * renderer = JRenderer::GetInstance();
  mFont->DrawString(_(displayValue).c_str(),x,y);

  if (value < selections.size())
    mFont->DrawString(_(selections[value]).c_str(),x+width-10,y,JGETEXT_RIGHT);
  else
    mFont->DrawString(_("Unset").c_str(),x+width-10,y,JGETEXT_RIGHT);
}

void OptionSelect::setData(){
  if(id == INVALID_OPTION) return;

  if (value < selections.size())
    options[id] = GameOption(selections[value]);
}
bool OptionSelect::Selectable(){
  return (selections.size() > 1);
}

void OptionSelect::addSelection(string s){
  selections.push_back(s);
}

//OptionProfile
const string OptionProfile::DIRTESTER = "collection.dat";
OptionProfile::OptionProfile(GameApp * _app, JGuiListener * jgl) : OptionDirectory(RESPATH"/profiles", Options::ACTIVE_PROFILE, "Profile", DIRTESTER){
  app = _app;
  listener = jgl;
  height=60;
  addSelection("Default");
  sort(selections.begin(),selections.end());
  mFocus = false;
  initSelections();
  populate();
};

void OptionProfile::addSelection(string s){
  OptionDirectory::addSelection(s);

  //Check how many options... if 1, we're not selectable.
  if(selections.size() > 1)
    canSelect = true;
  else
    canSelect = false;

}
void OptionProfile::updateValue(){
 value++;
 if (value > selections.size() - 1)
   value=0;

  populate();
}

void OptionProfile::Reload(){
  OptionDirectory::Reload();
  populate();
}
void OptionProfile::populate(){
 string temp = options[Options::ACTIVE_PROFILE].str;
 if (value >= selections.size()){ //TODO fail gracefully.
   return;
 }
 options[Options::ACTIVE_PROFILE].str = selections[value];
 PlayerData * pdata = NEW PlayerData(app->collection);

 int unlocked = 0, sets = setlist.size();
 std::ifstream file(options.profileFile(PLAYER_SETTINGS).c_str());
 std::string s;
  if(file){
    while(std::getline(file,s)){
      if(s.substr(0, 9) == "unlocked_")
        unlocked++;
    }
    file.close();
  }

 options[Options::ACTIVE_PROFILE] = temp;

 char buf[512], format[512];
 sprintf(format,"%s\n%s\n%s\n",_("Credits: %i").c_str(),_("Cards: %i").c_str(),_("Sets: %i (of %i)").c_str());
 sprintf(buf,format,pdata->credits,pdata->collection->totalCards(),unlocked,sets);
 preview = buf;

 SAFE_DELETE(pdata);
}

void OptionProfile::Render(){
  JRenderer * renderer = JRenderer::GetInstance();
  JLBFont * mFont = resources.GetJLBFont(Constants::OPTION_FONT);
  mFont->SetScale(1);
  int spacing = 2+(int)mFont->GetHeight();

  int pX, pY;
  pX = x;
  pY = y;
  char buf[512];
  if(selections[value] == "Default")
    sprintf(buf,"player/avatar.jpg");
  else
    sprintf(buf,"profiles/%s/avatar.jpg",selections[value].c_str());

  JQuad * mAvatar = resources.RetrieveQuad(buf,0,0,0,0,"temporary",RETRIEVE_NORMAL,TEXTURE_SUB_EXACT);

  if(mAvatar){
    renderer->RenderQuad(mAvatar,x,pY);
    pX += 40;
  }

  mFont->SetColor(getColor(WGuiColor::TEXT_HEADER));
  mFont->DrawString(selections[value].c_str(),pX,pY,JGETEXT_LEFT);
  mFont->SetScale(.8);
  mFont->SetColor(getColor(WGuiColor::TEXT_BODY));
  mFont->DrawString(preview.c_str(),pX,pY+spacing,JGETEXT_LEFT);
  mFont->SetScale(1);

}
void OptionProfile::Entering(JButton key){
  mFocus = true;
  initialValue = value;
}

void OptionProfile::confirmChange(bool confirmed){
  if (initialValue >= selections.size())
   return;

  int result;

  if(confirmed)    result = value;
  else             result = initialValue;

  options[Options::ACTIVE_PROFILE] = selections[result];
  value = result;

  populate();
  if(listener && confirmed){
    listener->ButtonPressed(-102,5);
    initialValue = value;
  }
  return;
}
//OptionLanguage
OptionLanguage::OptionLanguage(string _displayValue) : OptionSelect(Options::LANG,_displayValue)
{
  Reload();
  initSelections();
};

void OptionLanguage::setData(){
  if(id == INVALID_OPTION) return;

  if (value < selections.size()){
    options[id] = GameOption(actual_data[value]);
    Translator::EndInstance();
    Translator::GetInstance()->init();
    Translator::GetInstance()->tempValues.clear();
  }
}
void OptionLanguage::confirmChange(bool confirmed){
  if(!confirmed)
    value = prior_value;
  else{
    setData();
    if(Changed()){
      options[id] = GameOption(actual_data[value]);
      Translator::EndInstance();
      Translator::GetInstance()->init();
      Translator::GetInstance()->tempValues.clear();
    }
    prior_value = value;
  }
}
void OptionLanguage::Reload(){
  struct dirent *mDit;
  DIR *mDip;

  mDip = opendir("Res/lang");

  while ((mDit = readdir(mDip))){
    string filename = "Res/lang/";
    filename += mDit->d_name;
    std::ifstream file(filename.c_str());
    string s;
    string lang;
    if(file){
      if(std::getline(file,s)){
        if (!s.size()){
          lang = "";
        }else{
          if (s[s.size()-1] == '\r')
            s.erase(s.size()-1); //Handle DOS files
          size_t found = s.find("#LANG:");
          if (found != 0) lang = "";
          else lang = s.substr(6);
        }
      }
      file.close();
    }

    if (lang.size()){
      string filen = mDit->d_name;
      addSelection(filen.substr(0,filen.size()-4),lang);
    }
  }
  closedir(mDip);
  initSelections();
}
void OptionLanguage::addSelection(string s,string show){
  selections.push_back(show);
  actual_data.push_back(s);
}
void OptionLanguage::initSelections(){
  //Find currently active bit in the list.
    for(size_t i=0;i<actual_data.size();i++){
      if(actual_data[i] == options[id].str)
        value = i;
    }
}
bool OptionLanguage::Visible(){
  if(selections.size() > 1)
    return true;
  return false;
}
bool OptionLanguage::Selectable(){
  if(selections.size() > 1)
    return true;
  return false;
}
//OptionDirectory
void OptionDirectory::Reload(){
  DIR *mDip;
  struct dirent *mDit;
  char buf[PATH_MAX];
  mDip = opendir(root.c_str());

  if (!mDip) return;

  while ((mDit = readdir(mDip))){
    sprintf(buf,"%s/%s/%s", root.c_str(), mDit->d_name, type.c_str());
    std::ifstream file(buf);
    if (!file) continue;
    file.close();
    if (find(selections.begin(), selections.end(), mDit->d_name) == selections.end())
      addSelection(mDit->d_name);
  }

  closedir(mDip);
  mDip = NULL;
  initSelections();
}

OptionDirectory::OptionDirectory(string root, int id, string displayValue, string type): OptionSelect(id, displayValue), root(root), type(type){
  DIR *mDip;
  struct dirent *mDit;
  char buf[PATH_MAX];

  mDip = opendir(root.c_str());
  if(!mDip) return;

  while ((mDit = readdir(mDip))){
    sprintf(buf,"%s/%s/%s", root.c_str(), mDit->d_name, type.c_str());
    std::ifstream file(buf);
    if (!file) continue;
    file.close();
    addSelection(mDit->d_name);
  }

  closedir(mDip);
  mDip = NULL;
  initSelections();
}

const string OptionTheme::DIRTESTER = "preview.png";
OptionTheme::OptionTheme() : OptionDirectory(RESPATH"/themes", Options::ACTIVE_THEME, "Current Theme", DIRTESTER){
  addSelection("Default");
  sort(selections.begin(),selections.end());
  initSelections();
  mFocus=false;
  bChecked = false;
}
JQuad * OptionTheme::getImage(){
  char buf[512];
  string val = selections[value];
  if(val == "Default")
    sprintf(buf,"graphics/preview.png");
  else
    sprintf(buf,"themes/%s/preview.png",val.c_str());

  return resources.RetrieveQuad(buf,0,0,0,0,"temporary",RETRIEVE_NORMAL,TEXTURE_SUB_EXACT);
}

float OptionTheme::getHeight(){
  return 130;
};
void OptionTheme::updateValue(){
  OptionDirectory::updateValue();
  bChecked = false;
}

void OptionTheme::Render(){
  JRenderer * renderer = JRenderer::GetInstance();
  JQuad * q = getImage();
  JLBFont * mFont = resources.GetJLBFont(Constants::OPTION_FONT);
  mFont->SetColor(getColor(WGuiColor::TEXT_HEADER));
  char buf[512];
  if(!bChecked){
    author = "";
    bChecked = true;
    if(selections[value] == "Default")
      sprintf(buf,RESPATH"/graphics/themeinfo.txt");
    else
      sprintf(buf,RESPATH"/themes/%s/themeinfo.txt",selections[value].c_str());
    std::ifstream file(buf);
    if(file){
      string temp;
      std::getline(file,temp);
      for(unsigned int x=0;x<17,x<temp.size();x++){
        if(isprint(temp[x])) //Clear stuff that breaks mFont->DrawString, cuts to 16 chars.
          author += temp[x];
      }
      file.close();
    }
  }
  sprintf(buf,_("Theme: %s").c_str(),selections[value].c_str());

  if(q){
    float scale = 128 / q->mHeight;
    renderer->RenderQuad(q,x, y,0,scale,scale);
  }

  mFont->DrawString(buf,x,y);
  if(bChecked && author.size()){
    mFont->SetColor(getColor(WGuiColor::TEXT_BODY));
     mFont->SetScale(.8);
    float hi = mFont->GetHeight();
    sprintf(buf,_("Artist: %s").c_str(),author.c_str());
    mFont->DrawString(buf,x,y+getHeight()-hi);
    mFont->SetScale(1);
  }
}

bool OptionTheme::Visible(){
  if(selections.size() <= 1)
    return false;

  return true;
}

void OptionTheme::confirmChange(bool confirmed){
  bChecked = false;
  if(!confirmed)
    value = prior_value;
  else{
    setData();
    resources.Refresh(); //Update images
    prior_value = value;
  }
}

OptionKey::OptionKey(GameStateOptions* g, LocalKeySym from, JButton to) : WGuiItem(""), from(from), to(to), grabbed(false), g(g), btnMenu(NULL) {}

void OptionKey::Update(float dt) { if (btnMenu) btnMenu->Update(dt); }
void OptionKey::Render() {
  JLBFont * mFont = resources.GetJLBFont(Constants::OPTION_FONT);
  mFont->SetColor(getColor(WGuiColor::TEXT));
  JRenderer * renderer = JRenderer::GetInstance();

  if (LOCAL_KEY_NONE == from)
    {
      string msg = _("New binding...");
      mFont->DrawString(msg, (SCREEN_WIDTH - mFont->GetStringWidth(msg.c_str())) / 2, y + 2);
    }
  else
    {
      const KeyRep& rep = translateKey(from);
      if (rep.icon)
        renderer->RenderQuad(rep.icon, x + 2, y + 2);
      else
        mFont->DrawString(rep.text, x + 4, y + 2, JGETEXT_LEFT);
      const KeyRep& rep2 = translateKey(to);
      if (rep2.icon)
        renderer->RenderQuad(rep2.icon, x + 2, y + 2);
      else
        mFont->DrawString(rep2.text, width - 4, y + 2, JGETEXT_RIGHT);
    }
}
bool OptionKey::CheckUserInput(JButton key) {
  if (btnMenu)
    return btnMenu->CheckUserInput(key);
  if (JGE_BTN_OK == key) {
      grabbed = true;
      g->GrabKeyboard(this);
      return true;
    }
  return false;
}

static JButton btnList[] = {JGE_BTN_MENU, JGE_BTN_CTRL,   JGE_BTN_RIGHT,
                            JGE_BTN_LEFT, JGE_BTN_UP,     JGE_BTN_DOWN,
                            JGE_BTN_OK,   JGE_BTN_CANCEL, JGE_BTN_PRI,
                            JGE_BTN_SEC,  JGE_BTN_PREV,   JGE_BTN_NEXT,
                            JGE_BTN_NONE};
void OptionKey::KeyPressed(LocalKeySym key) {
  from = key;
  g->UngrabKeyboard(this);
  grabbed = false;

  btnMenu = NEW SimpleMenu(0, this, Constants::MENU_FONT, 80, 10);
  for (int i = sizeof(btnList) / sizeof(btnList[0]) - 1; i >= 0; --i)
    {
      const KeyRep& rep = translateKey(btnList[i]);
      btnMenu->Add(i, rep.text.c_str());
    }
}
bool OptionKey::isModal() { return grabbed; }
void OptionKey::Overlay()
{
  JRenderer * renderer = JRenderer::GetInstance();
  JLBFont * mFont = resources.GetJLBFont(Constants::OPTION_FONT);
  mFont->SetColor(ARGB(255, 0, 0, 0));
  if (grabbed) {
    static const int x = 30, y = 45;
    renderer->FillRoundRect(x, y, SCREEN_WIDTH - 2*x, 50, 2, ARGB(200, 200, 200, 255));
    string msg = _("Press a key to associate.");
    mFont->DrawString(msg, (SCREEN_WIDTH - mFont->GetStringWidth(msg.c_str())) / 2, y + 20);
  }
  else if (btnMenu)
    btnMenu->Render();
}
void OptionKey::ButtonPressed(int controllerId, int controlId)
{
  to = btnList[controlId];
  SAFE_DELETE(btnMenu);
  btnMenu = NULL;
}
bool OptionKey::Visible() { return JGE_BTN_NONE != to || LOCAL_KEY_NONE == from || btnMenu != NULL; }
bool OptionKey::Selectable() { return JGE_BTN_NONE != to || LOCAL_KEY_NONE == from || btnMenu != NULL; }
