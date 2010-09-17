#include "../include/DeckMetaData.h"
#include "../include/DeckStats.h"
#include "../include/MTGDeck.h"
#include "../include/config.h"
#include "../include/utils.h"

//Possible improvements:
//Merge this with DeckStats
//Have this class handle all the Meta Data rather than relying on MTGDeck. Then MTGDeck would have a MetaData object...


DeckMetaDataList * DeckMetaDataList::decksMetaData = NEW DeckMetaDataList();

DeckMetaData::DeckMetaData(){
  
}

DeckMetaData::DeckMetaData(string filename, Player * statsPlayer){
  load(filename);
}


void DeckMetaData::loadStatsForPlayer( Player * statsPlayer, string deckStatsFileName )
{
    DeckStats * stats = DeckStats::GetInstance();
    if ( statsPlayer )
    {
      stats->load(statsPlayer);
      DeckStat * opponentDeckStats = stats->getDeckStat(deckStatsFileName);
      if ( opponentDeckStats )
      {
        percentVictories = stats->percentVictories(deckStatsFileName);
        victories = opponentDeckStats->victories;
        nbGamesPlayed = opponentDeckStats->nbgames;
        if (percentVictories < 34){
          difficulty = HARD;
        }else if (percentVictories < 67){
          difficulty = NORMAL;
        }else{
          difficulty = EASY;
        }
      }
    }
    else
    {
      if(fileExists(deckStatsFileName.c_str())){
        stats->load(deckStatsFileName.c_str());
        nbGamesPlayed = stats->nbGames();
        percentVictories = stats->percentVictories();
      }
    }
}
      

string DeckMetaData::getDescription()
{
    char deckDesc[512];
    string difficultyString = "";
    switch( difficulty )
    {
        case HARD: 
            difficultyString = "Hard";
            break;
        case EASY:
            difficultyString = "Easy";
            break;
    }
    if ( nbGamesPlayed > 0 && difficultyString != "")    
        sprintf(deckDesc, "Difficulty: %s\nVictory %%: %i\nGames Played: %i\n\n%s", difficultyString.c_str(), percentVictories, nbGamesPlayed, desc.c_str() );
    else if ( nbGamesPlayed > 0 )
        sprintf(deckDesc, "Victory %%: %i\nGames Played: %i\n\n%s", percentVictories, nbGamesPlayed, desc.c_str() );
    else
        return desc.c_str();
    return deckDesc;
}
      
void DeckMetaData::load(string filename){
  MTGDeck * mtgd = NEW MTGDeck(filename.c_str(),NULL,1);
  name = trim( mtgd->meta_name );
  desc =  trim( mtgd->meta_desc );
  deckid = atoi( (filename.substr( filename.find("deck") + 4, filename.find(".txt") )).c_str() );
  
  delete(mtgd);
}


DeckMetaDataList::~DeckMetaDataList(){
  for(map<string,DeckMetaData *>::iterator it = values.begin(); it != values.end(); ++it){
    SAFE_DELETE(it->second);
  }
  values.clear();
}

void DeckMetaDataList::invalidate(string filename){
   map<string,DeckMetaData *>::iterator it = values.find(filename);
   if (it !=values.end()){
     SAFE_DELETE(it->second);
     values.erase(it);
   }
}


DeckMetaData * DeckMetaDataList::get(string filename, Player * statsPlayer){
  map<string,DeckMetaData *>::iterator it = values.find(filename);
  if (it ==values.end()){
    if (fileExists(filename.c_str())) {
      values[filename] = NEW DeckMetaData(filename, statsPlayer);
    }
  }

   return values[filename]; //this creates a NULL entry if the file does not exist
}


