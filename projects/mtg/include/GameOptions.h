#ifndef _GAME_OPTIONS_H_
#define _GAME_OPTIONS_H_

#include <map>
#include <string>
using std::map;
using std::string;

#define OPTIONS_SAVEFILE RESPATH"/settings/options.txt"

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
};

class GameOption {
public:
  int number;
  string str;
  GameOption(int value = 0);
  GameOption(string value);
};


class GameOptions {
 public:
  int save();
  int load();
  static const char * phaseInterrupts[];
  GameOption& operator[](string);
  GameOptions();
  ~GameOptions();

 private:
  static map <string,int> optionsTypes;
  map<string,GameOption> values;
};

extern GameOptions options;

#endif
