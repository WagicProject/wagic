#ifndef _GAME_OPTIONS_H_
#define _GAME_OPTIONS_H_

#include <map>
#include <string>
using std::map;
using std::string;

#define OPTIONS_MUSICVOLUME "musicVolume"
#define OPTIONS_SFXVOLUME "sfxVolume"

#define OPTIONS_DIFFICULTY_MODE_UNLOCKED "prx_handler" //huhu
#define OPTIONS_MOMIR_MODE_UNLOCKED "prx_rimom" //haha
#define OPTIONS_EVILTWIN_MODE_UNLOCKED "prx_eviltwin"
#define OPTIONS_RANDOMDECK_MODE_UNLOCKED "prx_rnddeck"

#define OPTIONS_DIFFICULTY "difficulty"
#define OPTIONS_CACHESIZE "cacheSize"
#define OPTIONS_PLASMAEFFECT "plasmaEffect"
#define OPTIONS_INTERRUPTMYSPELLS "interruptMySpells"
#define OPTIONS_INTERRUPTMYABILITIES "interruptMyAbilities"
#define OPTIONS_OSD "displayOSD"

// WALDORF - added
#define OPTIONS_INTERRUPT_SECONDS "interruptSeconds"                         


#define OPTIONS_SAVEFILE RESPATH"/settings/options.txt"

class GameOption {
public:
  int value;
  string svalue;
  int getIntValue();
  GameOption(int _value = 0);
};


class GameOptions {
 public:
  map<string,GameOption> values;
  static GameOptions * GetInstance();
  static void Destroy();
  int save();
  int load();
  static const char * phaseInterrupts[]; 

 private:
  GameOptions();
  ~GameOptions();
  static GameOptions* mInstance;

  static map <string,int> optionsTypes;


};

#endif
