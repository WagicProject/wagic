#ifndef _GAME_OPTIONS_H_
#define _GAME_OPTIONS_H_

#include <map>
#include <string>
using std::map;
using std::string;
#include <JGE.h>
#include "../include/SimplePad.h"
#include "../include/GameApp.h"

#define GLOBAL_SETTINGS RESPATH"/settings/options.txt"
#define PLAYER_SAVEFILE "data.dat"
#define PLAYER_SETTINGS "options.txt"
#define PLAYER_COLLECTION "collection.dat"

#define INVALID_OPTION -1

//Note to self-- provide some form of options initialization.

class Options {
public:
  friend class GameSettings;
  enum {
    //Values /must/ match ordering in optionNames, or everything loads wrong.
    //Profile settings    
    ACTIVE_THEME,
    ACTIVE_MODE,
    MUSICVOLUME,
    SFXVOLUME,
    DIFFICULTY,
    OSD,
    CLOSEDHAND,
    HANDDIRECTION,
    REVERSETRIGGERS,
    DISABLECARDS,
    INTERRUPT_SECONDS,
    //My interrupts    
    INTERRUPTMYSPELLS,
    INTERRUPTMYABILITIES,
    //Other interrupts
    INTERRUPT_BEFOREBEGIN,
    INTERRUPT_UNTAP,
    INTERRUPT_UPKEEP,
    INTERRUPT_DRAW,
    INTERRUPT_FIRSTMAIN,
    INTERRUPT_BEGINCOMBAT,
    INTERRUPT_ATTACKERS,
    INTERRUPT_BLOCKERS,
    INTERRUPT_DAMAGE,
    INTERRUPT_ENDCOMBAT,
    INTERRUPT_SECONDMAIN,
    INTERRUPT_ENDTURN,
    INTERRUPT_CLEANUP,
    INTERRUPT_AFTEREND,
    //Global settings
    ACTIVE_PROFILE,
    DIFFICULTY_MODE_UNLOCKED,
    MOMIR_MODE_UNLOCKED,
    EVILTWIN_MODE_UNLOCKED,
    RANDOMDECK_MODE_UNLOCKED,    
    CACHESIZE,
    //Theme metrics. These will be phased out fairly soon.
    THEME_METRICS, //Start of theme metrics.
    LOADING_TC = THEME_METRICS,
    STATS_TC,
    SCROLLER_TC,
    SCROLLER_FC,
    MAINMENU_TC,
    POPUP_MENU_FC,
    POPUP_MENU_TC,
    POPUP_MENU_TCH,
    MSG_FAIL_TC,
    OPTION_ITEM_FC,
    OPTION_ITEM_TC,
    OPTION_ITEM_TCH,
    OPTION_HEADER_FC,
    OPTION_HEADER_TC,
    OPTION_SCROLLBAR_FC,
    OPTION_SCROLLBAR_FCH,  
    OPTION_TAB_FC,
    OPTION_TAB_FCH,  
    OPTION_TAB_TC,
    OPTION_TAB_TCH,  
    OPTION_TEXT_TC,
    OPTION_TEXT_FC,
    KEY_TC,
    KEY_TCH,  
    KEY_FC,
    KEY_FCH,
    KEYPAD_FC, 
    KEYPAD_FCH, 
    KEYPAD_TC,

    LAST_NAMED, //Any option after this does not look up in optionNames.
    SET_UNLOCKS = LAST_NAMED + 1, //For sets.
  };
  
  static int optionSet(int setID);
  static int optionInterrupt(int gamePhase);

  static int getID(string name);
  static string getName(int option);

private:
  static const char* optionNames[];
};

struct Metrics {
  //*_TC is text-color, *_TCH is highlighted text color
  //*_FC is fill-color, *_FCH is highlighted fill color
  //*_B and *_BH are for secondary text/fill colors, if needed
  //*_X, *_Y, *_W, *_H are x, y, width and height.
  enum {
    LOADING_TC = Options::THEME_METRICS,
    STATS_TC,
    SCROLLER_TC,
    SCROLLER_FC,
    MAINMENU_TC,
    POPUP_MENU_FC,
    POPUP_MENU_TC,
    POPUP_MENU_TCH,
    MSG_FAIL_TC,
    OPTION_ITEM_FC,
    OPTION_ITEM_TC,
    OPTION_ITEM_TCH,
    OPTION_HEADER_FC,
    OPTION_HEADER_TC,
    OPTION_SCROLLBAR_FC,
    OPTION_SCROLLBAR_FCH,  
    OPTION_TAB_FC,
    OPTION_TAB_FCH,  
    OPTION_TAB_TC,
    OPTION_TAB_TCH,  
    OPTION_TEXT_TC,
    OPTION_TEXT_FC,
    KEY_TC,
    KEY_TCH,  
    KEY_FC,
    KEY_FCH,
    KEYPAD_FC, 
    KEYPAD_FCH, 
    KEYPAD_TC,
  };
};

class GameOption {
public:
  int number;
  string str;
  //All calls to asColor should include a fallback color for people without a theme.
  PIXEL_TYPE asColor(PIXEL_TYPE fallback = ARGB(255,255,255,255));
  bool isDefault(); //Returns true when  number is 0 abd string is "" or "default"
  GameOption(int value = 0);
  GameOption(string);
  GameOption(int, string);
};

struct EnumDefinition {
  int findIndex(int value);

  typedef pair<int, string> assoc;
  vector<assoc> values;
};

class GameOptions {
 public:
  string mFilename;
  int save();
  int load();

  GameOption& operator[](int);
  GameOptions(string filename);
  ~GameOptions();

 private:
  bool load_option(int id, string input); //Sends an option to the proper reader.
  bool read_default(int id, string input);
  bool read_enum(int id, string input, EnumDefinition * def);

  bool save_option(std::ofstream * file, int id, string name, GameOption * opt); //Sends an option to the proper writer.
  bool write_default(std::ofstream * file, string name, GameOption * opt);
  bool write_enum(std::ofstream * file, string name, GameOption * opt, EnumDefinition * def);

  map<int,GameOption> values;
};

class GameSettings{
public:
  friend class GameApp;
  GameSettings();
  ~GameSettings();
  int save();

  SimplePad * keypadStart(string input, string * _dest = NULL, bool _cancel=true, bool _numpad=false, int _x = SCREEN_WIDTH/2, int _y = SCREEN_HEIGHT/2);
  string keypadFinish();
  void keypadShutdown();
  void keypadTitle(string set);
  bool keypadActive() {if(keypad) return keypad->isActive(); return false;};
  void keypadUpdate(float dt) {if(keypad) keypad->Update(dt);};
  void keypadRender() {if(keypad) keypad->Render();};
  

  //These return a filepath accurate to the current mode/profile/theme, and can
  //optionally fallback to a file within a certain directory. 
  //The sanity=false option returns the adjusted path even if the file doesn't exist.
  string profileFile(string filename="", string fallback="", bool sanity=false,bool relative=false);

  void reloadProfile(bool images = true); //Reloads profile using current options[ACTIVE_PROFILE]
  void checkProfile();  //Confirms that a profile is loaded and contains a collection.
  void createUsersFirstDeck(int setId); 

  GameOption& operator[](int);
  GameOptions* profileOptions;
  GameOptions* globalOptions;
  GameOptions* themeOptions;

  static GameOption invalid_option;

private:
  GameApp * theGame;  
  SimplePad * keypad;
};

extern GameSettings options;

#endif
