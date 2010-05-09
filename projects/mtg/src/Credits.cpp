#include "../include/Credits.h"
#include "../include/GameApp.h"
#include "../include/GameOptions.h"
#include "../include/config.h"
#include "../include/PlayerData.h"
#include "../include/DeckStats.h"
#include "../include/Translate.h"
#include "../include/MTGDeck.h"
#include "../include/GameObserver.h"
#include "../include/GameStateShop.h"

  CreditBonus::CreditBonus(int _value, string _text){
    value = _value;
    text = _text;

  }


  void CreditBonus::Render(float x, float y, JLBFont * font){
    char buffer[512];
    sprintf(buffer, "%s: %i", text.c_str(), value);
    font->DrawString(buffer,x,y);
  }

  Credits::Credits(){
    unlockedTex = NULL;
    unlockedQuad = NULL;
    unlocked = -1;
  }

  Credits::~Credits(){
    resources.Release(unlockedTex);
    for (unsigned int i = 0; i<bonus.size(); ++i)
    if (bonus[i])
      delete bonus[i];
    bonus.clear();
  }

void Credits::compute(Player * _p1, Player * _p2, GameApp * _app){
  p1 = _p1;
  p2 = _p2;
  app = _app;
  showMsg = (WRand() % 3);
  GameObserver * g = GameObserver::GetInstance();
  if (!g->turn) return;
  PlayerData * playerdata = NEW PlayerData(app->collection);
  if (!p1->isAI() && p2->isAI() && p1!= g->gameOver){
    gameLength = time(0) - g->startedAt;
    value = 400;
    if (app->gameType != GAME_TYPE_CLASSIC) value = 200;
    int difficulty = options[Options::DIFFICULTY].number;
    if (options[Options::DIFFICULTY_MODE_UNLOCKED].number && difficulty) {
      CreditBonus * b = NEW CreditBonus(100*difficulty, _("Difficulty Bonus"));
      bonus.push_back(b);
    }

    if (p1->life == 1) {
      CreditBonus * b = NEW CreditBonus(111, _("'Live dangerously and you live right' Bonus"));
      bonus.push_back(b);
    }

    int diff = p1->life - p2->life;
    if (diff < 0) diff = 0;
    if (diff > 500) diff = 500;
    if (diff){
      CreditBonus * b = NEW CreditBonus(diff, _("Life Delta Bonus"));
      bonus.push_back(b);
    }

    if (p1->game->library->nb_cards == 0) {
      CreditBonus * b = NEW CreditBonus(391, _("'Decree of Theophilus' Bonus"));
      bonus.push_back(b);
    }

    if ((p2->game->library->nb_cards == 0) && p1->game->library->nb_cards) {
      CreditBonus * b = NEW CreditBonus(p1->game->library->nb_cards * 3, _("Miller Bonus"));
      bonus.push_back(b);
    }

    if (g->turn < 15) {
      CreditBonus * b = NEW CreditBonus((20 - g->turn)*17, _("'Fast and Furious' Bonus"));
      bonus.push_back(b);
    }

    GameOptionAward * goa = NULL;
    // <Tasks handling>
    vector<Task*> finishedTasks;
    playerdata->taskList->getDoneTasks(_p1, _p2, _app, &finishedTasks);

    char buffer[512];

    for (vector<Task*>::iterator it = finishedTasks.begin(); it!=finishedTasks.end(); it++) {
      sprintf(buffer, _("Task: %s").c_str(), (*it)->getShortDesc().c_str());
      CreditBonus * b = NEW CreditBonus((*it)->getReward(), buffer);
      bonus.push_back(b);
      playerdata->taskList->removeTask(*it);
    }
    // </Tasks handling>

    if (unlocked == -1){
      unlocked = isDifficultyUnlocked();
      if (unlocked){
        unlockedTex = resources.RetrieveTexture("unlocked.png");
        unlockedQuad = resources.RetrieveQuad("unlocked.png", 2, 2, 396, 96);
        goa = (GameOptionAward*) &options[Options::DIFFICULTY_MODE_UNLOCKED];
        goa->giveAward();
        options.save();
      } else if ((unlocked = isMomirUnlocked())) {
          unlockedTex = resources.RetrieveTexture("momir_unlocked.png");
          unlockedQuad = resources.RetrieveQuad("momir_unlocked.png", 2, 2, 396, 96);
          goa = (GameOptionAward*) &options[Options::MOMIR_MODE_UNLOCKED];
          goa->giveAward();
          options.save();
      } else if ((unlocked = isEvilTwinUnlocked())) {
          unlockedTex = resources.RetrieveTexture("eviltwin_unlocked.png");
          unlockedQuad = resources.RetrieveQuad("eviltwin_unlocked.png", 2, 2, 396, 96);
          goa = (GameOptionAward*) &options[Options::EVILTWIN_MODE_UNLOCKED];
          goa->giveAward();
          options.save();
      }else if((unlocked = isRandomDeckUnlocked())) {
          unlockedTex = resources.RetrieveTexture("randomdeck_unlocked.png");
          unlockedQuad = resources.RetrieveQuad("randomdeck_unlocked.png", 2, 2, 396, 96);
          goa = (GameOptionAward*) &options[Options::RANDOMDECK_MODE_UNLOCKED];
          goa->giveAward();
          options.save();
      }else if((unlocked = unlockRandomSet())) {
          unlockedTex = resources.RetrieveTexture("set_unlocked.png");
          unlockedQuad = resources.RetrieveQuad("set_unlocked.png", 2, 2, 396, 96);
          MTGSetInfo * si = setlist.getInfo(unlocked - 1);
          if(si) unlockedString = si->getName(); //Show the set's pretty name for unlocks.
      }

      if (unlocked && options[Options::SFXVOLUME].number > 0){
        JSample * sample = resources.RetrieveSample("bonus.wav");
        if (sample){
          JSoundSystem::GetInstance()->PlaySample(sample);
        }
      }

    }

    vector<CreditBonus *>::iterator it;
    if (bonus.size()){
      CreditBonus * b = NEW CreditBonus(value, _("Victory"));
      bonus.insert(bonus.begin(),b);
      for (it = bonus.begin() + 1; it < bonus.end(); ++it)
        value += (*it)->value;
    }



    playerdata->credits += value;
    PriceList::updateKey();
    playerdata->taskList->passOneDay();
    if (playerdata->taskList->getTaskCount() < 6) {
      playerdata->taskList->addRandomTask();
      playerdata->taskList->addRandomTask();
    }

  }else{
    unlocked = 0;
    playerdata->taskList->passOneDay();
  }

  playerdata->save();
  SAFE_DELETE(playerdata);
}

void Credits::Render(){
  GameObserver * g = GameObserver::GetInstance();
  JRenderer * r = JRenderer::GetInstance();
  JLBFont * f = resources.GetJLBFont(Constants::MAIN_FONT);
  JLBFont * f2 = resources.GetJLBFont(Constants::MENU_FONT);
  JLBFont * f3 = resources.GetJLBFont(Constants::MAGIC_FONT);
  f->SetScale(1);
  f->SetColor(ARGB(255,255,255,255));
  f2->SetScale(1);
  f2->SetColor(ARGB(255,255,255,255));
  f3->SetScale(1);
  f3->SetColor(ARGB(255,255,255,255));
  char buffer[512];
  if (!g->turn){
    sprintf(buffer, "%s", _("Please check your deck (not enough cards?)").c_str() );
  }else{
    if (!p1->isAI() && p2->isAI() ){
      if (g->gameOver != p1){
        sprintf (buffer, _("Congratulations! You earn %i credits").c_str(), value);
        if (unlockedQuad){
          showMsg = 0;
          r->RenderQuad(unlockedQuad, 20, 20);
        }
        if(unlockedString.size()){
          f2->DrawString(unlockedString.c_str(),SCREEN_WIDTH/2, 80,JGETEXT_CENTER);
        }
      }else{
        sprintf (buffer, "%s", _("You have been defeated").c_str());
      }
    }else{
      int winner = 2;
      if (g->gameOver !=p1){
        winner = 1;
      }
      int p0life = p1->life;
      sprintf(buffer, _("Player %i wins (%i)").c_str(), winner, p0life );
    }
  }

  float y = 130;

  if (showMsg == 1) y = 50;
  vector<CreditBonus *>:: iterator it;
  for ( it=bonus.begin() ; it < bonus.end(); ++it){
    (*it)->Render(10,y,f3);
    y+=12;
  }
  f2->DrawString(buffer, 10, y);
  y+=15;

  //!!
  if (g->gameOver != p1) {
    sprintf(buffer, _("Game length: %i turns (%i seconds)").c_str(), g->turn, this->gameLength);
    f->DrawString(buffer, 10, y);
    y += 10;
    sprintf(buffer, _("Credits per minute: %i").c_str(), (int)(60*value/this->gameLength));
    f->DrawString(buffer, 10, y);
    y += 10;
    showMsg = 0;
  }

  if (showMsg == 1){
    f2->DrawString(_("Please support this project!").c_str() ,10,y+15);
    f->DrawString(_("Wagic is free, open source, and developed on the little free time I have").c_str() ,10,y+30);
    f->DrawString(_("If you enjoy this game, please consider donating a few bucks").c_str() ,10,y+42);
    f->DrawString(_("(Seriously, donate or I'll kill this cute little bunny)").c_str() ,10,y+54);
    f->DrawString(_("Thanks in advance for your support.").c_str() ,10,y+66);
    f2->DrawString("-> http://wololo.net/wagic" ,10,y+78);
  }

}


int Credits::isDifficultyUnlocked(){
  if (options[Options::DIFFICULTY_MODE_UNLOCKED].number) return 0;
  int nbAIDecks = 0;
  int found = 1;
  int wins = 0;
  DeckStats * stats = DeckStats::GetInstance();
  stats->load(p1);
  while (found){
    found = 0;
    char buffer[512];
    char aiSmallDeckName[512];
    sprintf(buffer, RESPATH"/ai/baka/deck%i.txt",nbAIDecks+1);
    if(fileExists(buffer)){
      found = 1;
      nbAIDecks++;
      sprintf(aiSmallDeckName, "ai_baka_deck%i",nbAIDecks);
      int percentVictories = stats->percentVictories(string(aiSmallDeckName));
      if (percentVictories >=67) wins++;
      if (wins >= 10) return 1;
    }
  }
  return 0;
}

int Credits::isMomirUnlocked(){
  if (options[Options::MOMIR_MODE_UNLOCKED].number) return 0;
  if (p1->game->inPlay->countByType("land") == 8) return 1;
  return 0;
}

int Credits::isEvilTwinUnlocked(){
  if (options[Options::EVILTWIN_MODE_UNLOCKED].number) return 0;
  if (p1->game->inPlay->nb_cards && (p1->game->inPlay->nb_cards == p2->game->inPlay->nb_cards)) return 1;
  return 0;
}

int Credits::isRandomDeckUnlocked(){
  if (0 == options[Options::DIFFICULTY].number) return 0;
  if (options[Options::RANDOMDECK_MODE_UNLOCKED].number) return 0;
  if (p1->life >= 20 ) return 1;
  return 0;
}

int Credits::addCreditBonus(int value){
  PlayerData * playerdata = NEW PlayerData();
  playerdata->credits += value;
  playerdata->save();
  SAFE_DELETE(playerdata);
  return value;
}

int Credits::unlockRandomSet(bool force){
  int setId = WRand() % setlist.size();

  if (force) {
    int init = setId;
    boolean found = false;
    do {
      if (1 != options[Options::optionSet(setId)].number)
        found = true;
      else {
        setId++;
        if (setId == setlist.size()) 
          setId = 0;
      }
    } while (setId != init && !found);
  }

  if (1 == options[Options::optionSet(setId)].number) 
    return 0;

  GameOptionAward* goa = (GameOptionAward*) &options[Options::optionSet(setId)];
  goa->giveAward();
  options.save();
  return setId + 1; //We add 1 here to show success/failure. Be sure to subtract later.
}
