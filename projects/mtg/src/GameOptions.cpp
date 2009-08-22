#include "../include/config.h"
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
const string Options::MUSICVOLUME = "musicVolume";
const string Options::SFXVOLUME = "sfxVolume";
const string Options::DIFFICULTY_MODE_UNLOCKED = "prx_handler"; //huhu
const string Options::MOMIR_MODE_UNLOCKED = "prx_rimom"; //haha
const string Options::DIFFICULTY = "difficulty";
const string Options::CACHESIZE = "cacheSize";
const string Options::PLASMAEFFECT = "plasmaEffect";
const string Options::INTERRUPT_SECONDS = "interruptSeconds";
const string Options::INTERRUPTMYSPELLS = "interruptMySpells";
const string Options::INTERRUPTMYABILITIES = "interruptMyAbilities";
const string Options::EVILTWIN_MODE_UNLOCKED = "prx_eviltwin";
const string Options::RANDOMDECK_MODE_UNLOCKED = "prx_rnddeck";
const string Options::OSD = "displayOSD";



GameOption::GameOption(int value) : number(value){}
GameOption::GameOption(string value) : str(value){}

GameOptions::GameOptions(){
  load();
}

int GameOptions::load(){
  std::ifstream file(OPTIONS_SAVEFILE);
  std::string s;
  if(file){
    while(std::getline(file,s)){
      int found =s.find("=");
      string name = s.substr(0,found);
      values[name] = GameOption(atoi(s.substr(found+1).c_str()));
    }
    file.close();
  }
  return 1;
}

int GameOptions::save(){
  std::ofstream file(OPTIONS_SAVEFILE);
  char writer[1024];
  if (file){
    map<string, GameOption>::iterator it;
    for ( it=values.begin() ; it != values.end(); it++ ){
      sprintf(writer,"%s=%d\n", it->first.c_str(), it->second.number);
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

GameOptions options;
