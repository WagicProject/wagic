#ifndef _GAME_OPTIONS_H_
#define _GAME_OPTIONS_H_

#include <map>
#include <string>
using std::map;
using std::string;
#include <JGE.h>
#include <time.h>
#include "../include/SimplePad.h"
#include "../include/GameApp.h"

#define GLOBAL_SETTINGS RESPATH"/settings/options.txt"
#define PLAYER_SAVEFILE "data.dat"
#define PLAYER_SETTINGS "options.txt"
#define PLAYER_COLLECTION "collection.dat"
#define PLAYER_TASKS "tasks.dat"
#define SECRET_PROFILE "Maxglee"

#define INVALID_OPTION -1

class Options {
public:
  friend class GameSettings;
  enum {
    //Global settings
    ACTIVE_PROFILE,
    LANG,
    LAST_GLOBAL = LANG, //This must be the value above, to keep ordering.
    //Values /must/ match ordering in optionNames, or everything loads wrong.
    //Profile settings        
    ACTIVE_THEME,
    ACTIVE_MODE,
    MUSICVOLUME,
    SFXVOLUME,
    DIFFICULTY,
    CHEATMODE,
    OSD,
    CLOSEDHAND,
    HANDDIRECTION,
    MANADISPLAY,
    REVERSETRIGGERS,
    DISABLECARDS,
    MAX_GRADE,
    ECON_DIFFICULTY,
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
    BEGIN_AWARDS, //Options after this use the GameOptionAward struct, which includes a timestamp.
    DIFFICULTY_MODE_UNLOCKED = BEGIN_AWARDS,
    MOMIR_MODE_UNLOCKED,
    EVILTWIN_MODE_UNLOCKED,
    RANDOMDECK_MODE_UNLOCKED,    
    AWARD_COLLECTOR,    
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

class GameOption {
public:
  virtual ~GameOption() {};
  int number;
  string str;
  //All calls to asColor should include a fallback color for people without a theme.
  PIXEL_TYPE asColor(PIXEL_TYPE fallback = ARGB(255,255,255,255));

  virtual bool isDefault(); //Returns true when number is 0 and string is "" or "Default"
  virtual string menuStr(); //The string we'll use for GameStateOptions.
  virtual bool write(std::ofstream * file, string name);
  virtual bool read(string input);

  GameOption(int value = 0);
  GameOption(string);
  GameOption(int, string);
};

struct EnumDefinition {
  int findIndex(int value);

  typedef pair<int, string> assoc;
  vector<assoc> values;
};

class GameOptionEnum: public GameOption {
public:
  virtual string menuStr();
  virtual bool write(std::ofstream * file, string name);
  virtual bool read(string input);
  EnumDefinition * def;
};

class GameOptionAward: public GameOption {
public:
  GameOptionAward();
  virtual string menuStr();
  virtual bool write(std::ofstream * file, string name);
  virtual bool read(string input);
  virtual bool giveAward(); //Returns false if already awarded
  virtual bool isViewed();
  virtual void setViewed(bool v = true) {viewed = v;};
private:
  time_t achieved; //When was it awarded?
  bool viewed;     //Flag it as "New!" or not.  
};

class OptionVolume: public EnumDefinition{
public:
  enum { MUTE = 0, MAX = 100 };
  static EnumDefinition * getInstance() {return &mDef;};
private:
  OptionVolume();
  static OptionVolume mDef;
};
class OptionClosedHand: public EnumDefinition {
public:
  enum { INVISIBLE = 0, VISIBLE = 1 };
  static EnumDefinition * getInstance() {return &mDef;};
private:  
  OptionClosedHand();
  static OptionClosedHand mDef;
};
class OptionHandDirection: public EnumDefinition {
public:
  enum { VERTICAL = 0, HORIZONTAL = 1};
  static EnumDefinition * getInstance() {return &mDef;};
private:
  OptionHandDirection();
  static OptionHandDirection mDef;
};
class OptionManaDisplay: public EnumDefinition {
public:
  enum { DYNAMIC = 0, STATIC = 1, BOTH = 2};
  static EnumDefinition * getInstance() {return &mDef;};
private:
  OptionManaDisplay();
  static OptionManaDisplay mDef;
};

class OptionMaxGrade: public EnumDefinition {
public:
  static EnumDefinition * getInstance() {return &mDef;};
private:
  OptionMaxGrade();
  static OptionMaxGrade mDef;
};

class OptionEconDifficulty: public EnumDefinition {
public:
  static EnumDefinition * getInstance() {return &mDef;};
private:
  OptionEconDifficulty();
  static OptionEconDifficulty mDef;
};

class OptionDifficulty: public EnumDefinition {
public:
  enum { NORMAL = 0, HARD = 1, HARDER = 2, EVIL = 3};
  static EnumDefinition * getInstance() {return &mDef;};
private:
  OptionDifficulty();
  static OptionDifficulty mDef;
};

class GameOptions {
 public:
  string mFilename;
  int save();
  int load();

  GameOption * get(int);
  GameOption& operator[](int);
  GameOptions(string filename);
  ~GameOptions();

 private:
  vector<GameOption*> values;
  vector<string> unknown;
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
  
  bool newAward();

  //These return a filepath accurate to the current mode/profile/theme, and can
  //optionally fallback to a file within a certain directory. 
  //The sanity=false option returns the adjusted path even if the file doesn't exist.
  string profileFile(string filename="", string fallback="", bool sanity=false,bool relative=false);

  void reloadProfile(bool images = true); //Reloads profile using current options[ACTIVE_PROFILE]
  void checkProfile();  //Confirms that a profile is loaded and contains a collection.
  void createUsersFirstDeck(int setId); 

  GameOption * get(int);
  GameOption& operator[](int);

  GameOptions* profileOptions;
  GameOptions* globalOptions;

  static GameOption invalid_option;

private:
  GameApp * theGame;  
  SimplePad * keypad;
};

extern GameSettings options;

#endif
