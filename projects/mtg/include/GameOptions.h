#ifndef _GAME_OPTIONS_H_
#define _GAME_OPTIONS_H_


#define MAX_OPTIONS 50
#define OPTIONS_MUSICVOLUME 0
#define OPTIONS_INTERRUPTATENDOFPHASE_OFFSET 1
#define OPTIONS_SAVEFILE "Res/settings/options.txt"
class GameOptions {
 public:
  int values[MAX_OPTIONS];
  static GameOptions * GetInstance();
  static void Destroy();
  int save();
  int load();

 private:
  GameOptions();
  ~GameOptions();
  static GameOptions* mInstance;

};

#endif
