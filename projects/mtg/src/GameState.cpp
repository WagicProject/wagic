#include "../include/config.h"
#include "../include/Translate.h"
#include "../include/GameState.h"
#include "../include/Player.h"
#include "../include/SimpleMenu.h"
#include "../include/DeckStats.h"


int GameState::fillDeckMenu(SimpleMenu * _menu, string path, string smallDeckPrefix, Player * statsPlayer){
  int found = 1;
  int nbDecks = 0;
  _menu->autoTranslate = false;
  while (found){
    found = 0;
    char buffer[512];
    char smallDeckName[512];
    char deckDesc[512];
    sprintf(buffer, "%s/deck%i.txt",path.c_str(),nbDecks+1);
    if(fileExists(buffer)){
      MTGDeck * mtgd = NEW MTGDeck(buffer,NULL,1);
      found = 1;
      nbDecks++;
      sprintf(smallDeckName, "%s_deck%i",smallDeckPrefix.c_str(),nbDecks);
      
      if (statsPlayer){
        DeckStats * stats = DeckStats::GetInstance();
        stats->load(statsPlayer);
        int percentVictories = stats->percentVictories(string(smallDeckName));
        string difficulty;
        if (percentVictories < 34){
          difficulty = "(hard)";
        }else if (percentVictories < 67){
          difficulty = "";
        }else{
          difficulty = "(easy)";
        }
        sprintf(deckDesc, "%s %s",mtgd->meta_name.c_str(), _(difficulty).c_str());
      }else{
        sprintf(deckDesc, "%s",mtgd->meta_name.c_str());
      }
      deckDesc[16] = 0;
      _menu->Add(nbDecks,deckDesc,mtgd->meta_desc);
      delete mtgd;
    }
  }
  return nbDecks;
}