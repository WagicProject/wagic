#include "../include/config.h"
#include "../include/GameOptions.h"
#include "../include/PlayerData.h"

#include <string.h>
#include <stdio.h>

PlayerData::PlayerData(){
  init();
}
PlayerData::PlayerData(MTGAllCards * allcards){
  init();

  //COLLECTION
  if (allcards) collection = NEW MTGDeck(options.profileFile(PLAYER_COLLECTION).c_str(), allcards);
}

void PlayerData::init() {
  collection = NULL;

  //CREDITS
  credits = 3000; //Default value

  std::ifstream file(options.profileFile(PLAYER_SAVEFILE).c_str());
  std::string s;
  if(file){
    if(std::getline(file,s)){
      credits = atoi(s.c_str());
    }else{
      //TODO error management
    }

    //Story Saves
    while(std::getline(file,s)){
      if (!s.size()) 
        continue;
      if (s[s.size()-1] == '\r') s.erase(s.size()-1); //Handle DOS files
      if (s.size() && s[0] == '#')
         continue;
      size_t i = s.find_first_of("=");
      if (i == string::npos)
        continue;

      string key = s.substr(0,i);
      string value = s.substr(i+1);
      if (key.size() < 3)
        continue;

      if(key[0] != 's')
        continue;
      key = key.substr(2);
      storySaves[key]=value;
    }
    file.close();
  }

  taskList = NEW TaskList(options.profileFile(PLAYER_TASKS).c_str());
}

int PlayerData::save(){
  std::ofstream file(options.profileFile(PLAYER_SAVEFILE).c_str());
  char writer[512];
  if (file){
    sprintf(writer,"%i\n", credits);
    file<<writer;

    //Story Saves
    for (map<string,string>::iterator it =storySaves.begin(); it !=storySaves.end(); ++it){
      sprintf(writer,"s %s=%s\n", it->first.c_str(),it->second.c_str());
      file << writer;
    }

    file.close();
  }
  if (collection) collection->save();
  taskList->save();
  return 1;
}

PlayerData::~PlayerData(){
  SAFE_DELETE(collection);
  SAFE_DELETE(taskList);
}
