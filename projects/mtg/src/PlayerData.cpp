#include "../include/config.h"
#include "../include/PlayerData.h"

#include <string.h>
#include <stdio.h>

PlayerData::PlayerData(MTGAllCards * allcards){
  //CREDITS
  credits = 3000; //Default value
  std::ifstream file(PLAYER_SAVEFILE);
  std::string s;
  if(file){
    if(std::getline(file,s)){
      credits = atoi(s.c_str());
    }else{
      //TODO error management
    }
    file.close();
  }

  //COLLECTION
  collection = NEW MTGDeck(RESPATH"/player/collection.dat", allcards->mCache , allcards);
}


int PlayerData::save(){
  std::ofstream file(PLAYER_SAVEFILE);
  char writer[64];
  if (file){
    sprintf(writer,"%i\n", credits);
    file<<writer;
    file.close();
  }
  collection->save();
  return 1;
}

PlayerData::~PlayerData(){
  SAFE_DELETE(collection);
}
