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

struct Options {
  static const string MUSICVOLUME;
  static const string SFXVOLUME;
  static const string DIFFICULTY_MODE_UNLOCKED;
  static const string MOMIR_MODE_UNLOCKED;
  static const string EVILTWIN_MODE_UNLOCKED;
  static const string RANDOMDECK_MODE_UNLOCKED;
  static const string DIFFICULTY;
  static const string CACHESIZE;
  static const string PLASMAEFFECT;
  static const string INTERRUPT_SECONDS;
  static const string INTERRUPTMYSPELLS;
  static const string INTERRUPTMYABILITIES;
  static const string OSD;
  static const string ACTIVE_PROFILE;
  static const string ACTIVE_THEME;
  static const string ACTIVE_MODE;
  static const string HANDMODE;
};

struct Metrics {
  //*_TC is text-color, *_TCH is highlighted text color
  //*_FC is fill-color, *_FCH is highlighted fill color
  //*_B and *_BH are for secondary text/fill colors, if needed
  //*_X, *_Y, *_W, *_H are x, y, width and height.
  static const string LOADING_TC;
  static const string STATS_TC;
  static const string SCROLLER_TC;
  static const string SCROLLER_FC;
  static const string MAINMENU_TC;
  static const string POPUP_MENU_FC;
  static const string POPUP_MENU_TC;
  static const string POPUP_MENU_TCH;
  static const string MSG_FAIL_TC;
  static const string OPTION_ITEM_FC;
  static const string OPTION_ITEM_TC;
  static const string OPTION_ITEM_TCH;
  static const string OPTION_HEADER_FC;
  static const string OPTION_HEADER_TC;
  static const string OPTION_SCROLLBAR_FC;
  static const string OPTION_SCROLLBAR_FCH;  
  static const string OPTION_TAB_FC;
  static const string OPTION_TAB_FCH;  
  static const string OPTION_TAB_TC;
  static const string OPTION_TAB_TCH;  
  static const string OPTION_TEXT_TC;
  static const string OPTION_TEXT_FC;
  static const string KEY_TC;
  static const string KEY_TCH;  
  static const string KEY_FC;
  static const string KEY_FCH;
  static const string KEYPAD_FC; 
  static const string KEYPAD_FCH; 
  static const string KEYPAD_TC;
};

class GameOption {
public:
  int number;
  string str;
  //All calls to asColor should include a fallback color for people without a theme.
  PIXEL_TYPE asColor(PIXEL_TYPE fallback = ARGB(255,255,255,255));
  bool isDefault(); //Returns true when  number is 0 abd string is "" or "default"
  GameOption(int value = 0);
  GameOption(string value);
};


class GameOptions {
 public:
  string mFilename;
  int save();
  int load();

  static const char * phaseInterrupts[];
  GameOption& operator[](string);
  GameOptions(string filename);
  ~GameOptions();

 private:
  map<string,GameOption> values;
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

  GameOption& operator[](string);
  GameOptions* profileOptions;
  GameOptions* globalOptions;
  GameOptions* themeOptions;

private:
  GameApp * theGame;  
  SimplePad * keypad;
};

extern GameSettings options;

#endif
