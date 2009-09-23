#include "../include/config.h"
#include "../include/utils.h"
#include "../include/MTGDeck.h"
#include "../include/GameOptions.h"
#include "../include/OptionItem.h"
#include <iostream>
#include <fstream>
#include <string>
#include <stdlib.h>
#include <JGE.h>

const char * Options::optionNames[] = {
//Options set on a per-profile basis
  "Theme",
  "Mode",
  "musicVolume",
  "sfxVolume",
  "difficulty",
  "plasmaEffect",
  "displayOSD",
  "closed_hand",
  "hand_direction",
  "interruptSeconds",
  "interruptMySpells",
  "interruptMyAbilities",
//General interrupts
  "interruptBeforeBegin",
  "interruptUntap",
  "interruptUpkeep",
  "interruptDraw",
  "interruptFirstMain",
  "interruptBeginCombat",
  "interruptAttackers",
  "interruptBlockers",
  "interruptDamage",
  "interruptEndCombat",
  "interruptSecondMain",
  "interruptEndTurn",
  "interruptCleanup",
  "interruptAfterEnd",
//Global options
  "_gProfile",
  "_gprx_handler",
  "_gprx_rimom",
  "_gprx_eviltwin",
  "_gprx_rnddeck",
  "_gcacheSize",
//Theme metrics
  "_tLoadingTC",
  "_tStatsTC",
  "_tScrollerTC",
  "_tScrollerFC",
  "_tMainMenuTC",
  "_tPopupMenuFC",
  "_tPopupMenuTC",
  "_tPopupMenuTCH",
  "_tMsgFailTC",
  "_tOptionItemFC",
  "_tOptionItemTC",
  "_tOptionItemTCH",
  "_tOptionHeaderFC",
  "_tOptionHeaderTC",
  "_tOptionScrollbarFC",
  "_tOptionScrollbarFCH",  
  "_tOptionHeaderFC",
  "_tOptionHeaderFCH",  
  "_tOptionTabTC",
  "_tOptionHeaderTCH",  
  "_tOptionTextTC",
  "_tOptionTextFC",    
  "_tKeyTC",
  "_tKeyTCH",
  "_tKeyFC",  
  "_tKeyFCH", 
  "_tKeypadFC",  
  "_tKeypadFCH",  
  "_tKeypadTC"
};
int Options::getID(string name){
  if(!name.size())
    INVALID_OPTION;

  std::transform(name.begin(),name.end(),name.begin(),::tolower);

  //Is it a named option?
  for(int x = 0; x < LAST_NAMED; x++){
    string lower = Options::optionNames[x];
    std::transform(lower.begin(),lower.end(),lower.begin(),::tolower);

    if(lower == name)
      return x;
  }

  //Is it an unlocked set?
  string setname = name.substr(strlen("unlocked_"));
  if(MtgSets::SetsList){
    int unlocked = MtgSets::SetsList->find(setname);
    if(unlocked != -1)
      return Options::optionSet(unlocked);  
  }

  //Failure.
  return INVALID_OPTION;
}

string Options::getName(int option){
  //Invalid options
  if(option < 0)
    return "";

  //Standard named options
  if(option < LAST_NAMED)
    return optionNames[option];
  
  //Unlocked sets.
  if(MtgSets::SetsList){
    int setID = option - SET_UNLOCKS;
    if(setID >= 0 && setID < MtgSets::SetsList->nb_items){
      char buf[512];
      sprintf(buf,"unlocked_%s",MtgSets::SetsList->values[setID].c_str());
      return buf;
    }
  }

  //Failed.
  return "";
}

int Options::optionSet(int setID){
  //Sanity check if possible
  if(setID < 0 || (MtgSets::SetsList && setID > MtgSets::SetsList->nb_items))
    return INVALID_OPTION;

  return SET_UNLOCKS + setID;  
}

int Options::optionInterrupt(int gamePhase){
  //Huge, nearly illegible switch block spread out to improve readability.
  switch(gamePhase){
      case Constants::MTG_PHASE_BEFORE_BEGIN:
        return INTERRUPT_BEFOREBEGIN;

      case Constants::MTG_PHASE_UNTAP:
        return INTERRUPT_UNTAP;

      case Constants::MTG_PHASE_UPKEEP:
        return INTERRUPT_UPKEEP;

      case Constants::MTG_PHASE_DRAW:
        return INTERRUPT_DRAW;

      case Constants::MTG_PHASE_FIRSTMAIN:
        return INTERRUPT_FIRSTMAIN;

      case Constants::MTG_PHASE_COMBATBEGIN:
        return INTERRUPT_BEGINCOMBAT;

      case Constants::MTG_PHASE_COMBATATTACKERS:
        return INTERRUPT_ATTACKERS;

      case Constants::MTG_PHASE_COMBATBLOCKERS:
        return INTERRUPT_BLOCKERS;

      case Constants::MTG_PHASE_COMBATDAMAGE:
        return INTERRUPT_DAMAGE;

      case Constants::MTG_PHASE_COMBATEND:
        return INTERRUPT_ENDCOMBAT;

      case Constants::MTG_PHASE_SECONDMAIN:
        return INTERRUPT_SECONDMAIN;

      case Constants::MTG_PHASE_ENDOFTURN:
        return INTERRUPT_ENDTURN;

      case Constants::MTG_PHASE_CLEANUP:
        return INTERRUPT_CLEANUP;

      case Constants::MTG_PHASE_AFTER_EOT:
        return INTERRUPT_AFTEREND;
  }

  return INVALID_OPTION;
}

//Theme metrics
/*
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
const string Metrics::KEYPAD_TC = "_tKeypadTC";  */


GameOption::GameOption(int value) : number(value){}
GameOption::GameOption(string value) : number(0), str(value) {}
GameOption::GameOption(int num, string str) : number(num), str(str) {}

bool GameOption::isDefault(){
  string test = str;
  std::transform(test.begin(),test.end(),test.begin(),::tolower);
  
  if(!test.size() || test == "default")
    return true;

  return false;
}

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
      color[subpixel] = (unsigned char) atoi(temp.c_str());
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
    color[subpixel] = (unsigned char) atoi(temp.c_str());
  if(subpixel == 2)
    color[3] = 255;

  return ARGB(color[3],color[0],color[1],color[2]);  
}

GameOptions::GameOptions(string filename){
  mFilename = filename;
  load();
}

bool GameOptions::read_default(int id, string input){
  bool bNumeric = true;
  
  if(!input.size()){
    values[id] = GameOption(0);
    return true; //Default reader doesn't care about invalid formatting.
  }

  //Is it a number?
  for(size_t x=0;x<input.size();x++) {
    if(!isdigit(input[x])) {
      bNumeric = false;
      break;
    }
  }

  if(bNumeric)
    values[id] = GameOption(atoi(input.c_str()));
  else
    values[id] = GameOption(input);

  return true;
}
bool GameOptions::read_enum(int id, string input, EnumDefinition * def){
  if(!def) 
    return false;

  std::transform(input.begin(),input.end(),input.begin(),::tolower);

  vector<EnumDefinition::assoc>::iterator it;
  for(it=def->values.begin();it != def->values.end();it++){
    if(it->second == input){
      values[id] = GameOption(it->first);
      return true;
    }
  }
  
  return false;
}
bool GameOptions::load_option(int id, string input){
  switch(id){
   case Options::HANDDIRECTION:
     return read_enum(id, input, OptionHandDirection::getDefinition());
   case Options::CLOSEDHAND:
     return read_enum(id, input, OptionClosedHand::getDefinition());
   default:
     return read_default(id, input);
  }

  return false;
}

int GameOptions::load(){
  std::ifstream file(mFilename.c_str());
  std::string s;
  
  if(file){
    while(std::getline(file,s)){
      int found =s.find("=");
      string name = s.substr(0,found);
      string val = s.substr(found+1);
      int id = Options::getID(name);      
      if(id == INVALID_OPTION)
        continue;

      load_option(id,val);
    }
    file.close();
  }
  return 1;
}

bool GameOptions::save_option(std::ofstream * file, int id, string name, GameOption * opt){
  if(!opt)
    return false;

  switch(id){
   case Options::HANDDIRECTION:
     return write_enum(file, name, opt, OptionHandDirection::getDefinition());
   case Options::CLOSEDHAND:
     return write_enum(file, name, opt, OptionClosedHand::getDefinition());
   default:
     return write_default(file, name, opt);
  }

  return false;
}

bool GameOptions::write_default(std::ofstream * file, string name, GameOption * opt){
  char writer[1024];

  if(!file || !opt)
    return false;

   if(opt->str ==""){
     	if(opt->number == 0) //This is absolutely default. No need to write it.
	      return true;

      //It's a number!
      sprintf(writer,"%s=%d\n", name.c_str(), opt->number);
	  }
	  else
	    sprintf(writer,"%s=%s\n", name.c_str(), opt->str.c_str());
  
  (*file)<<writer;
  return true;
}

bool GameOptions::write_enum(std::ofstream * file, string name, GameOption * opt, EnumDefinition * def){

  if(!file || !def || !opt)
    return false;

  if(opt->number < 0 || opt->number >= (int) def->values.size())
    return false;

  char writer[1024];
  sprintf(writer,"%s=%s\n", name.c_str(), def->values[opt->number].second.c_str());

  (*file)<<writer;
  return true;
}

int GameOptions::save(){
  std::ofstream file(mFilename.c_str());
  if (file){
    map<int, GameOption>::iterator it;
    for ( it=values.begin() ; it != values.end(); it++ ){
      
      //Check that this is a valid option.
      string name = Options::getName(it->first);
      if(!name.size())
        continue;

      //Save it.
      save_option(&file, it->first, name, &it->second);
    }
      file.close();
  }
  return 1;
}

GameOption& GameOptions::operator[](int optionID) {
  return values[optionID];
}

GameOptions::~GameOptions(){
}

GameSettings options;

GameSettings::GameSettings()
{
  //Load global options
  globalOptions = NEW GameOptions(GLOBAL_SETTINGS);

  //reloadProfile should be called for the rest.
  theGame = NULL;
  profileOptions = NULL;
  themeOptions = NULL;
}

GameSettings::~GameSettings(){
  if(globalOptions)
    globalOptions->save();

  if(profileOptions)
    profileOptions->save();

  SAFE_DELETE(globalOptions);
  SAFE_DELETE(profileOptions);
  SAFE_DELETE(themeOptions);
  SAFE_DELETE(keypad);
  SAFE_DELETE(OptionHandDirection::definition);
  SAFE_DELETE(OptionClosedHand::definition);
}

GameOption GameSettings::invalid_option = GameOption(0);

GameOption& GameSettings::operator[](int optionID){
  string option_name = Options::getName(optionID);
  
  //Last chance sanity checking.
  if(!option_name.size()){
    OutputDebugString("Error: Accessing invalid option.\n");
    invalid_option.number = 0;
    invalid_option.str = "";
    return invalid_option;  
  }

  if(option_name.size() > 2){
   if(option_name[0] == '_' && option_name[1] == 't')
    return (*themeOptions)[optionID];
   else if(option_name[0] == '_' && option_name[1] == 'g')
    return (*globalOptions)[optionID];
  }

  return (*profileOptions)[optionID];
}


int GameSettings::save(){
  if(globalOptions)
    globalOptions->save();

  if(profileOptions){
    //Force our directories to exist.
    MAKEDIR(RESPATH"/profiles");
    string temp = profileFile("","",false,false);
    MAKEDIR(temp.c_str()); 
    temp+="/stats";
    MAKEDIR(temp.c_str()); 
    temp = profileFile(PLAYER_SETTINGS,"",false);

    profileOptions->save();
  }

  checkProfile();

  return 1;
}

string GameSettings::profileFile(string filename, string fallback,bool sanity, bool relative)
{
  char buf[512];
  string profile = (*this)[Options::ACTIVE_PROFILE].str;

  if(!(*this)[Options::ACTIVE_PROFILE].isDefault())  {
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

void GameSettings::reloadProfile(bool images){
    SAFE_DELETE(profileOptions);
    SAFE_DELETE(themeOptions);
    checkProfile();
    if(images)
      resources.Refresh(); //Update images
}

void GameSettings::checkProfile(){
    char buf[512];

    //If it doesn't exist, load current profile.
    if(!profileOptions)
      profileOptions = NEW GameOptions(profileFile(PLAYER_SETTINGS,"",false));
    
    //Load theme options
    if(!themeOptions){
      if(!profileOptions || (*profileOptions)[Options::ACTIVE_THEME].isDefault())
       sprintf(buf,RESPATH"/graphics/metrics.txt");
      else
       sprintf(buf,RESPATH"/themes/%s/metrics.txt",(*profileOptions)[Options::ACTIVE_THEME].str.c_str());
      
      themeOptions = NEW GameOptions(buf); 
    }

    //Validation of collection, etc, only happens if the game is up.
    if(theGame == NULL || theGame->collection == NULL)
      return; 

    string pcFile = profileFile(PLAYER_COLLECTION,"",false);
    if(!pcFile.size() || !fileExists(pcFile.c_str()))
    {
      //If we had any default settings, we'd set them here.
      
      //Find the set for which we have the most variety
      int setId = 0;
      int maxcards = 0;
      for (int i=0; i< MtgSets::SetsList->nb_items; i++){
        int value = theGame->collection->countBySet(i);
        if (value > maxcards){
          maxcards = value;
          setId = i;
        }
      }

      //Make the proper directories
      if(profileOptions){
        //Force our directories to exist.
        MAKEDIR(RESPATH"/profiles");
        string temp = profileFile("","",false,false);
        MAKEDIR(temp.c_str()); 
        temp+="/stats";
        MAKEDIR(temp.c_str()); 
        temp = profileFile(PLAYER_SETTINGS,"",false);

        profileOptions->save();
      }

      //Save this set as "unlocked"
      (*profileOptions)[Options::optionSet(setId)]=1;
      profileOptions->save();

      //Give the player their first deck
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

  MTGDeck *mCollection = NEW MTGDeck(options.profileFile(PLAYER_COLLECTION,"",false).c_str(), theGame->collection);
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

int EnumDefinition::findIndex(int value){
  vector<assoc>::iterator it;
  for(it = values.begin();it!=values.end();it++){
    if(it->first == value)
      return it - values.begin();
  }

  return 0; //Default!
}
