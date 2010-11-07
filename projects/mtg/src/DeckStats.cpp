#include "PrecompiledHeader.h"

#include "DeckStats.h"
#include "Player.h"
#include "GameObserver.h"

DeckStats * DeckStats::mInstance = NULL;

int DeckStat::percentVictories(){
  if (nbgames == 0) return 50;
  return (100 * victories / nbgames);
}

DeckStats * DeckStats::GetInstance(){
  if (!mInstance){
    mInstance = NEW DeckStats();
    
  }
  return mInstance;
}

void DeckStats::cleanStats(){
  map<string,DeckStat *>::iterator it;
  for (it = stats.begin(); it != stats.end(); it++){
    SAFE_DELETE(it->second);
  }
  
  stats.clear();
}

DeckStats::~DeckStats(){
  cleanStats();
}

int DeckStats::percentVictories(string opponentsFile){
  map<string,DeckStat *>::iterator it = stats.find(opponentsFile);
  if (it == stats.end()){
    return 50;
  }else{
    return (it->second->percentVictories());
  }
}

DeckStat* DeckStats::getDeckStat(string opponentsFile){
  map<string,DeckStat *>::iterator it = stats.find(opponentsFile);
  if (it == stats.end()){
    return NULL;
  }else{
    return it->second;
  }
}

int DeckStats::nbGames(){
  int nbgames = 0;
  map<string,DeckStat *>::iterator it;
  for (it = stats.begin(); it != stats.end(); it++){
    DeckStat * d = it->second;
    nbgames+=d->nbgames;
  }
  return nbgames;
}


int DeckStats::percentVictories(){
  int victories = 0;
  int nbgames = 0;
  map<string,DeckStat *>::iterator it;
  for (it = stats.begin(); it != stats.end(); it++){
    DeckStat * d = it->second;
    nbgames+=d->nbgames;
    victories+=d->victories;
  }
  if (nbgames){
    return (victories * 100)/nbgames;
  }
  return 50;
}

void DeckStats::load(Player * player){
  char filename[512];
  sprintf(filename,"stats/%s.txt",player->deckFileSmall.c_str());
  load(options.profileFile(filename).c_str());
}

void DeckStats::load(const char * filename){
  cleanStats();
  std::ifstream file(filename);
  std::string s;

  if(file){
    while(std::getline(file,s)){
      string deckfile = s;
      std::getline(file,s);
      int games = atoi(s.c_str());
      std::getline(file,s);
      int victories = atoi(s.c_str());
      map<string,DeckStat *>::iterator it = stats.find(deckfile);
      if (it == stats.end()){
        stats[deckfile] = NEW DeckStat(games,victories);
      }
    }
    file.close();
  }
}

void DeckStats::save(Player * player){
  char filename[512];
  sprintf(filename,"stats/%s.txt",player->deckFileSmall.c_str());
  save(options.profileFile(filename).c_str());
}

void DeckStats::save(const char * filename){
  std::ofstream file(filename);
  char writer[512];
  if (file){
    map<string,DeckStat *>::iterator it;
    for (it = stats.begin(); it != stats.end(); it++){
      sprintf(writer,"%s\n", it->first.c_str());
      file<<writer;
      sprintf(writer,"%i\n", it->second->nbgames);
      file<<writer;
      sprintf(writer,"%i\n", it->second->victories);
      file<<writer;
    }
    file.close();
  }
}

void DeckStats::saveStats(Player *player, Player *opponent, GameObserver * game){
  int victory = 1;
  if (!game->gameOver){
    if (player->life == opponent->life) return;
    if (player->life < opponent->life) victory = 0;
  }else if (game->gameOver == player) {
    victory = 0;
  }
  load(player);
  map<string,DeckStat *>::iterator it = stats.find(opponent->deckFileSmall);
  if (it == stats.end()){
    stats[opponent->deckFileSmall] = NEW DeckStat(1,victory);
  }else{
    it->second->victories+=victory;
    it->second->nbgames+=1;
  }
  save(player);
}


StatsWrapper::StatsWrapper( int deckId )
{
    // Load deck statistics
  char buffer[512];
  DeckStats * stats = DeckStats::GetInstance();
  aiDeckNames.clear();
  aiDeckStats.clear();

  sprintf(buffer, "stats/player_deck%i.txt", deckId);
  string deckstats = options.profileFile(buffer);

  if(fileExists(deckstats.c_str())){
    stats->load(deckstats.c_str());
    percentVictories = stats->percentVictories();
    gamesPlayed = stats->nbGames();

    // Detailed deck statistics against AI
    int found = 1;
    int nbDecks = 0;
    while (found){
      found = 0;
      char buffer[512];
      char smallDeckName[512];
      sprintf(buffer, "%s/deck%i.txt",RESPATH"/ai/baka",nbDecks+1);
      if(fileExists(buffer)){
        MTGDeck * mtgd = NEW MTGDeck(buffer,NULL,1);
        found = 1;
        nbDecks++;

        sprintf(smallDeckName, "%s_deck%i","ai_baka",nbDecks);
        DeckStat* deckStat = stats->getDeckStat(string(smallDeckName));

        if ((deckStat != NULL) && (deckStat->nbgames>0)) {
          int percentVictories = stats->percentVictories(string(smallDeckName));
          aiDeckNames.push_back(string(mtgd->meta_name));
          aiDeckStats.push_back(deckStat);
        }

        delete mtgd;
      }
    }
  } 
  else {
    gamesPlayed = 0;
    percentVictories = 0;
  }
}

StatsWrapper::~StatsWrapper()
{
  aiDeckNames.clear();
  aiDeckStats.clear();
}