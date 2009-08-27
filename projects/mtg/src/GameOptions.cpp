#include "../include/config.h"
#include "../include/utils.h"
#include "../include/MTGDeck.h"
#include "../include/GameOptions.h"
#include <iostream>
#include <fstream>
#include <string>
#include <stdlib.h>
#include <JGE.h>

const char* GameOptions::phaseInterrupts[] = {
	"interrupt ---",
	"interrupt Untap",
	"interrupt Upkeep",
	"interrupt Draw",
	"interrupt Main phase 1",
	"interrupt Combat begins",
	"interrupt Attackers",
	"interrupt Blockers",
	"interrupt Combat damage",
	"interrupt Combat ends",
	"interrupt Main phase 2",
	"interrupt End of turn",
	"interrupt Cleanup",
	"interrupt ---"
};
//Profile options
const string Options::MUSICVOLUME = "musicVolume";
const string Options::SFXVOLUME = "sfxVolume";
const string Options::DIFFICULTY = "difficulty";
const string Options::PLASMAEFFECT = "plasmaEffect";
const string Options::INTERRUPT_SECONDS = "interruptSeconds";
const string Options::INTERRUPTMYSPELLS = "interruptMySpells";
const string Options::INTERRUPTMYABILITIES = "interruptMyAbilities";
const string Options::OSD = "displayOSD";
const string Options::ACTIVE_THEME = "Theme";
const string Options::ACTIVE_MODE = "Mode";
//Global options
const string Options::ACTIVE_PROFILE = "_gProfile";
const string Options::DIFFICULTY_MODE_UNLOCKED = "_gprx_handler"; //huhu
const string Options::MOMIR_MODE_UNLOCKED = "_gprx_rimom"; //haha
const string Options::EVILTWIN_MODE_UNLOCKED = "_gprx_eviltwin";
const string Options::RANDOMDECK_MODE_UNLOCKED = "_gprx_rnddeck";
const string Options::CACHESIZE = "_gcacheSize";
//Theme metrics
const string Metrics::LOADING_TC = "_tLoadingTC";
const string Metrics::STATS_TC = "_tStatsTC";
const string Metrics::SCROLLER_TC = "_tScrollerTC";
const string Metrics::SCROLLER_FC = "_tScrollerFC";
const string Metrics::MAINMENU_TC = "_tMainMenuTC";
const string Metrics::POPUP_MENU_FC = "_tPopupMenuFC";
const string Metrics::POPUP_MENU_TC = "_tPopupMenuTC";
const string Metrics::POPUP_MENU_TCH = "_tPopupMenuTCH";
const string Metrics::MSG_FAIL_TC = "_tMsgFailTC";
const string Metrics::OPTION_ITEM_FC = "_tOptionItemFC";
const string Metrics::OPTION_ITEM_TC = "_tOptionItemTC";
const string Metrics::OPTION_ITEM_TCH = "_tOptionItemTCH";
const string Metrics::OPTION_HEADER_FC = "_tOptionHeaderFC";
const string Metrics::OPTION_HEADER_TC = "_tOptionHeaderTC";
const string Metrics::OPTION_SCROLLBAR_FC = "_tOptionScrollbarFC";
const string Metrics::OPTION_SCROLLBAR_FCH = "_tOptionScrollbarFCH";  
const string Metrics::OPTION_TAB_FC = "_tOptionHeaderFC";
const string Metrics::OPTION_TAB_FCH = "_tOptionHeaderFCH";  
const string Metrics::OPTION_TAB_TC = "_tOptionTabTC";
const string Metrics::OPTION_TAB_TCH = "_tOptionHeaderTCH";  
const string Metrics::OPTION_TEXT_TC = "_tOptionTextTC";
const string Metrics::OPTION_TEXT_FC = "_tOptionTextFC";    
const string Metrics::KEY_TC = "_tKeyTC";
const string Metrics::KEY_TCH = "_tKeyTCH";
const string Metrics::KEY_FC = "_tKeyFC";  
const string Metrics::KEY_FCH = "_tKeyFCH"; 
const string Metrics::KEYPAD_FC = "_tKeypadFC";  
const string Metrics::KEYPAD_FCH = "_tKeypadFCH";  
const string Metrics::KEYPAD_TC = "_tKeypadTC";  


GameOption::GameOption(int value) : number(value){}
GameOption::GameOption(string value) : str(value){}

PIXEL_TYPE GameOption::asColor(PIXEL_TYPE fallback)
{
  unsigned char color[4];
  string temp;
  int subpixel=0;

  //The absolute shortest a color could be is 5 characters: "0,0,0" (implicit 255 alpha)
  if(str.length() < 5)
    return fallback;

  for(size_t i=0;i<str.length();i++)  {
    if(isspace(str[i]))
      continue;
    if(str[i] == ','){
      if(temp == "")
        return fallback;
      color[subpixel] = atoi(temp.c_str());
      temp = "";
      subpixel++;
      continue;
    }
    else if(!isdigit(str[i]))
      return fallback;
    if(subpixel > 3)
      return fallback;
    temp += str[i];
  }

  if(temp != "")
    color[subpixel] = atoi(temp.c_str());
  if(subpixel == 2)
    color[3] = 255;

  return ARGB(color[3],color[0],color[1],color[2]);  
}

GameOptions::GameOptions(string filename){
  mFilename = filename;
  load();
}
int GameOptions::load(){
  std::ifstream file(mFilename.c_str());
  std::string s;
  
  if(file){
    while(std::getline(file,s)){
      int found =s.find("=");
      bool bnumber = true; 
      string name = s.substr(0,found);
      string val = s.substr(found+1);
      for(size_t x=0;x<val.size();x++) {
        if(!isdigit(val[x])) {
          bnumber = false;
          break;
        }
      }
      if(bnumber)
        values[name] = GameOption(atoi(val.c_str()));
      else
        values[name] = GameOption(val);
    }
    file.close();
  }
  return 1;
}

int GameOptions::save(){
  std::ofstream file(mFilename.c_str());
  char writer[1024];
  if (file){
    map<string, GameOption>::iterator it;
    for ( it=values.begin() ; it != values.end(); it++ ){
		if(it->second.str ==""){
		  sprintf(writer,"%s=%d\n", it->first.c_str(), it->second.number);
		  if(it->second.number==0)
		    continue;
		}
		else
		  sprintf(writer,"%s=%s\n", it->first.c_str(), it->second.str.c_str());
      file<<writer;
    }
    file.close();
  }
  return 1;
}

GameOption& GameOptions::operator[](string option_name) {
  return values[option_name];
}

GameOptions::~GameOptions(){
}

GameSettings options;

GameSettings::GameSettings()
{
  //Load global options
  globalOptions = NEW GameOptions(GLOBAL_SETTINGS);

  //Load profile options. 
  char buf[512];
  string temp = (*globalOptions)[Options::ACTIVE_PROFILE.substr(2)].str;
  if(temp == "")
   temp = "Default";
  (*globalOptions)[Options::ACTIVE_PROFILE.substr(2)].str = temp;

  profileOptions = NULL;
  checkProfile();

  //Force a theme.
  temp = (*profileOptions)[Options::ACTIVE_THEME].str;
  if(temp == ""){
    temp = "Default";
   (*profileOptions)[Options::ACTIVE_THEME].str = "Default";
  }

  //Load theme options
  if(temp == "Default")
   sprintf(buf,RESPATH"/graphics/metrics.txt");
  else{
   sprintf(buf,RESPATH"/themes/%s/",temp.c_str());
   MAKEDIR(buf); 
   sprintf(buf,RESPATH"/themes/%s/metrics.txt",temp.c_str());
  }

  themeOptions = NEW GameOptions(buf);  
}

GameSettings::~GameSettings(){
  if(globalOptions)
    globalOptions->save();

  if(profileOptions)
    profileOptions->save();

  SAFE_DELETE(globalOptions);
  SAFE_DELETE(profileOptions);
   SAFE_DELETE(themeOptions);
}

GameOption& GameSettings::operator[](string option_name){
  if(option_name.size() > 2){
   if(option_name[0] == '_' && option_name[1] == 't')
    return (*themeOptions)[option_name.substr(2)];
   else if(option_name[0] == '_' && option_name[1] == 'g')
    return (*globalOptions)[option_name.substr(2)];
  }

  return (*profileOptions)[option_name];
}

int GameSettings::save(){
  if(globalOptions)
    globalOptions->save();

  if(profileOptions)
    profileOptions->save();

  checkProfile();

  return 1;
}

string GameSettings::profileFile(string filename, string fallback,bool sanity, bool relative)
{
  char buf[512];
  string profile =(*this)[Options::ACTIVE_PROFILE].str;

  if(profile != "" && profile != "Default")  {
     //No file, return root of profile directory
     if(filename == ""){ 
       sprintf(buf,"%sprofiles/%s",( relative ? "" : RESPATH"/" ),profile.c_str());
       return buf;
     }
     //Return file
     sprintf(buf,RESPATH"/profiles/%s/%s",profile.c_str(),filename.c_str());
     if(fileExists(buf)){
        if(relative)
          sprintf(buf,"profiles/%s/%s",profile.c_str(),filename.c_str());
        return buf;
     }
  }
  else{
    //Use the default directory.
    sprintf(buf,"%splayer%s%s",(relative ? "" : RESPATH"/"),(filename == "" ? "" : "/"), filename.c_str());
    return buf;
  }
  
  //Don't fallback if sanity checking is disabled..
  if(!sanity){
    sprintf(buf,"%sprofiles/%s%s%s",(relative ? "" : RESPATH"/"),profile.c_str(),(filename == "" ? "" : "/"), filename.c_str());
    return buf;
  }

  //No fallback directory. This is often a crash.
  if(fallback == "")
      return "";

  sprintf(buf,"%s%s%s%s",(relative ? "" : RESPATH"/"),fallback.c_str(),(filename == "" ? "" : "/"), filename.c_str());
  return buf;
}



void GameSettings::checkProfile(){
    //Load current profile's options. Doesn't save prior set.
    if(profileOptions != NULL)
      SAFE_DELETE(profileOptions);

    //Force our directories to exist.
    MAKEDIR(RESPATH"/profiles");
    string temp = profileFile("","",false,false);
    MAKEDIR(temp.c_str()); 
    temp+="/stats";
    MAKEDIR(temp.c_str()); 
    temp = profileFile(PLAYER_SETTINGS,"",false);
    profileOptions = NEW GameOptions(temp);

    //Validation of collection, etc, only happens if the game is up.
    if(theGame == NULL || theGame->collection == NULL)
      return; 
    
    if(profileFile(PLAYER_COLLECTION) == "")
    {
      //If we had any default settings, we'd set them here.
      
      //Give the player cards from the set for which we have the most variety
      int setId = 0;
      int maxcards = 0;
      for (int i=0; i< MtgSets::SetsList->nb_items; i++){
        int value = theGame->collection->countBySet(i);
        if (value > maxcards){
          maxcards = value;
          setId = i;
        }
      }
      //Save this set as "unlocked"
      char buffer[4096];
      string s = MtgSets::SetsList->values[setId];
      sprintf(buffer,"unlocked_%s", s.c_str());
      (*profileOptions)[buffer]=1;
      profileOptions->save();
      createUsersFirstDeck(setId);
    }
}

void GameSettings::createUsersFirstDeck(int setId){
#if defined (WIN32) || defined (LINUX)
  char buf[4096];
  sprintf(buf, "setID: %i", setId);
  OutputDebugString(buf);
#endif

  if(theGame == NULL || theGame->collection == NULL)
    return;

  MTGDeck *mCollection = NEW MTGDeck(options.profileFile(PLAYER_COLLECTION,"",false).c_str(), &cache, theGame->collection);
  //10 lands of each
  int sets[] = {setId};
  if (!mCollection->addRandomCards(10, sets,1, Constants::RARITY_L,"Forest")){
    mCollection->addRandomCards(10, 0,0,Constants::RARITY_L,"Forest");
  }
  if (!mCollection->addRandomCards(10, sets,1,Constants::RARITY_L,"Plains")){
    mCollection->addRandomCards(10, 0,0,Constants::RARITY_L,"Plains");
  }
  if (!mCollection->addRandomCards(10, sets,1,Constants::RARITY_L,"Swamp")){
    mCollection->addRandomCards(10, 0,0,Constants::RARITY_L,"Swamp");
  }
  if (!mCollection->addRandomCards(10, sets,1,Constants::RARITY_L,"Mountain")){
    mCollection->addRandomCards(10, 0,0,Constants::RARITY_L,"Mountain");
  }
  if (!mCollection->addRandomCards(10, sets,1,Constants::RARITY_L,"Island")){
    mCollection->addRandomCards(10, 0,0,Constants::RARITY_L,"Island");
  }


#if defined (WIN32) || defined (LINUX)
  OutputDebugString("1\n");
#endif

  //Starter Deck
  mCollection->addRandomCards(3, sets,1,Constants::RARITY_R,NULL);
  mCollection->addRandomCards(9, sets,1,Constants::RARITY_U,NULL);
  mCollection->addRandomCards(48, sets,1,Constants::RARITY_C,NULL);

#if defined (WIN32) || defined (LINUX)
  OutputDebugString("2\n");
#endif
  //Boosters
  for (int i = 0; i< 2; i++){
    mCollection->addRandomCards(1, sets,1,Constants::RARITY_R);
    mCollection->addRandomCards(3, sets,1,Constants::RARITY_U);
    mCollection->addRandomCards(11, sets,1,Constants::RARITY_C);
  }
  mCollection->save();
  SAFE_DELETE(mCollection);
}
void GameSettings::keypadTitle(string set){
  if(keypad != NULL)
    keypad->title = set;
}
SimplePad * GameSettings::keypadStart(string input, string * _dest,bool _cancel, bool _numpad, int _x,int _y ){
  if(keypad == NULL)
    keypad = NEW SimplePad();
  keypad->bShowCancel = _cancel;
  keypad->bShowNumpad = _numpad;
  keypad->mX = _x;
  keypad->mY = _y;
  keypad->Start(input,_dest);
  return keypad;
}

string GameSettings::keypadFinish(){
  if(keypad == NULL)
    return "";
  return keypad->Finish();
}

void GameSettings::keypadShutdown(){
  SAFE_DELETE(keypad);
}