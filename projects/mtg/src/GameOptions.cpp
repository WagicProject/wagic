#include "../include/debug.h"
#include "../include/GameOptions.h"
#include <iostream>
#include <fstream>
#include <string>
#include <stdlib.h>

GameOptions* GameOptions::mInstance = NULL;

GameOptions * GameOptions::GetInstance(){
  if (mInstance == NULL)
    mInstance = NEW GameOptions();
  return mInstance;
}

GameOptions::GameOptions(){
  for(int i = 0; i < MAX_OPTIONS; i++){
    values[i] = 0;
  }
  load();
}

int GameOptions::load(){
  std::ifstream file(OPTIONS_SAVEFILE);
  std::string s;
  if(file){
    for (int i = 0; i < MAX_OPTIONS; i++){
      if(std::getline(file,s)){
	values[i] = atoi(s.c_str());
      }else{
	//TODO error management
      }
    }
    file.close();
  }
  return 1;
}

int GameOptions::save(){
  std::ofstream file(OPTIONS_SAVEFILE);
  char writer[10];
  if (file){
    for (int i = 0; i < MAX_OPTIONS; i++){
      sprintf(writer,"%i\n", values[i]);
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
