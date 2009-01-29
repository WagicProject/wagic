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

GameOption::GameOption(int _value){
  value = _value;
}

int GameOption::getIntValue(){
  return value;
}

GameOptions* GameOptions::mInstance = NULL;

GameOptions * GameOptions::GetInstance(){
  if (mInstance == NULL)
    mInstance = NEW GameOptions();
  return mInstance;
}

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
      sprintf(writer,"%s=%d\n", it->first.c_str(), it->second.getIntValue());
      file<<writer;
    }
    file.close();
  }
  return 1;
}


GameOptions::~GameOptions(){
}

void GameOptions::Destroy(){
  if (mInstance){
    delete mInstance;
    mInstance = NULL;
  }
}
