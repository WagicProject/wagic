#include "../include/config.h"
#include "../include/GameApp.h"
#include "../include/Player.h"
#include "../include/Tasks.h"
#include "../include/AIPlayer.h"
#include "../include/Translate.h"
#include "../include/MTGDefinitions.h"
#include <JRenderer.h>

vector<string> Task::AIDeckNames;

/*---------------- Utils -----------------*/
// TODO: Move to dedicated file

//!! Copypaste from GameStateDeckViewer.cpp StringExplode. Move and #include here and there
void ExplodeStr(string str, string separator, vector<string>* results){
    int found;
    results->clear();
    found = str.find_first_of(separator);
    while(found != (int)string::npos){
        if(found > 0){
          results->push_back(str.substr(0,found));
        } else {
          results->push_back(" ");
        }
        str = str.substr(found+1);
        found = str.find_first_of(separator);
    }
    if(str.length() > 0){
        results->push_back(str);
    }
}

string ImplodeStr(string separator, vector<string> strs){
  string result = "";
  for (vector<string>::iterator it = strs.begin(); it!=strs.end(); it++){
    result += (it == strs.begin()?"":separator) + (*it);
  }
  return result;
}

/*---------------- Task -----------------*/

Task::Task(char _type) {
  reward = 0;
  expiresIn = 1;
  accepted = FALSE;
  if (_type == ' ') {
    type = TASK_BASIC;
  } else {
    type = _type;
  }
}

int Task::getReward() {
  if (reward == 0) {
    reward = computeReward();
  }

  return reward;
}

string Task::toString() {
  storeCommonAttribs();
  storeCustomAttribs();
  return ImplodeStr(ITEM_SEPARATOR, persistentAttribs);
}

// Store basic attributes to vector, for saving
void Task::storeCommonAttribs() {
  char buff[256];

  persistentAttribs.clear();
  persistentAttribs.push_back(string(1, type));

  sprintf(buff, "%i", expiresIn);
  persistentAttribs.push_back(string(buff));

  sprintf(buff, "%i", accepted?1:0);
  persistentAttribs.push_back(string(buff));

  sprintf(buff, "%i", opponent);
  persistentAttribs.push_back(string(buff));

  sprintf(buff, "%i", reward);
  persistentAttribs.push_back(string(buff));

  persistentAttribs.push_back(getDesc());

  persistentAttribs.push_back(getOpponentName());
}

int Task::restoreCommonAttribs() {
  if (persistentAttribs.size() < COMMON_ATTRIBS_COUNT) {
    #if defined (WIN32) || defined (LINUX)
      OutputDebugString("\nTasks.cpp::restoreCommonAttribs: Not enough attributes loaded\n");
    #endif
    
    return -1;
  }
  expiresIn = atoi(persistentAttribs[1].c_str());
  accepted = (persistentAttribs[2].compare("1") == 0);
  opponent = atoi(persistentAttribs[3].c_str());
  reward = atoi(persistentAttribs[4].c_str());
  description = persistentAttribs[5];
  opponentName = persistentAttribs[6];
  return 1;
}

void Task::storeCustomAttribs() {
  // To be extended in child class
}

void Task::restoreCustomAttribs() {
  // To be extended in child class
}

string Task::getOpponentName() {
  if (opponentName == "") {
    opponentName = Task::getAIDeckName(opponent);
  }
  
  return opponentName;
}

string Task::getDesc() {
  return (description == "") ? (description = createDesc()) : description;
}

void Task::randomize() {
  opponent = rand() % getAIDeckCount() + 1;
  opponentName = "";
  setExpiration((rand()%3)+1);
}

bool Task::isExpired() {  
  return (expiresIn <= 0);
}

int Task::getExpiration() {
  return expiresIn;
}

void Task::setExpiration(int _expiresIn) {
  expiresIn = _expiresIn;
}

void Task::passOneDay() {
  expiresIn--;
  reward = (int) reward * 0.9; // Todo: degradation and minreward constants
  if (reward < 33) {
    reward = 33;
  }
}


// AI deck buffering code

void Task::loadAIDeckNames() {
  int found = 1;
  int nbDecks = 0;
  while (found){
    found = 0;
    char buffer[512];
    char smallDeckName[512];
    char deckDesc[512];
    sprintf(buffer, "%s/deck%i.txt",RESPATH"/ai/baka",nbDecks + 1);

    if(fileExists(buffer)){
      found = 1;
      nbDecks++;
      // TODO: Creating MTGDeck only for getting decks name. Find an easier way.
      MTGDeck * mtgd = NEW MTGDeck(buffer,NULL,1);
      AIDeckNames.push_back(mtgd->meta_name);
      delete mtgd;    
    }
  }
}

int Task::getAIDeckCount() {
  if (AIDeckNames.size() == 0) {
    loadAIDeckNames();
  }
  return AIDeckNames.size();
}

string Task::getAIDeckName(int id) {
  if (AIDeckNames.size() == 0) {
    loadAIDeckNames();
  }
  return ((unsigned int)id <= AIDeckNames.size()) ? AIDeckNames.at(id-1) : "<Undefined>";
}

// End of AI deck buffering code

// Each child class has to be added to the switch in this function (clumsy..)
Task* Task::createFromStr(string params, bool rand) {
  vector<string> exploded;
  Task *result;

  ExplodeStr(params, ITEM_SEPARATOR, &exploded);

  switch (exploded[0][0]) {
    case TASK_WIN_AGAINST:
      result = new TaskWinAgainst();
      break;
    case TASK_SLAUGHTER:
      result = new TaskSlaughter();
      break;
    case TASK_DELAY:
      result = new TaskDelay();
      break;
    case TASK_IMMORTAL:
      result = new TaskImmortal();
      break;
    case TASK_MASSIVE_BURIAL:
      result = new TaskMassiveBurial();
      break;      
    default:
      #if defined (WIN32) || defined (LINUX)
        OutputDebugString("\nTasks.cpp::createFromStr: Undefined class type\n");
      #endif
      result = new TaskWinAgainst();
  }

  if (!result) {
    return NULL;
  }

  result->persistentAttribs = exploded;
  
  if (exploded.size() >= COMMON_ATTRIBS_COUNT) { 
    result->restoreCommonAttribs();
    if (exploded.size() > COMMON_ATTRIBS_COUNT) { 
      result->restoreCustomAttribs();
    }
  } else if (rand) {
    result->randomize();
  }

  return result;
}

/*---------------- TaskList -----------------*/

TaskList::TaskList(string _fileName) {
  fileName = _fileName;
  if (fileName == "") {
    fileName = options.profileFile(PLAYER_TASKS).c_str();
  }
  load(fileName);
}

int TaskList::save(string _fileName) {
  if (_fileName != "") {
    fileName = _fileName;
  }
  if (fileName == "") {
    return -1;
  }

  std::ofstream file(fileName.c_str());
  if (file){
    #if defined (WIN32) || defined (LINUX)
        OutputDebugString("\nsaving\n");
    #endif
    
    file << "# Format: <Type>|<Expiration>|<Accepted>|<Opponent>|<Reward>|<Description>[|Additional attributes]\n";

    for (vector<Task*>::iterator it = tasks.begin(); it!=tasks.end(); it++){
      file << (*it)->toString() << "\n";
    }
    file.close();
  }

  return 1;
}

int TaskList::load(string _fileName) {
  Task *task;
  if (_fileName != "") {
    fileName = _fileName;
  }
  if (fileName == "") {
    return -1;
  }

  std::ifstream file(fileName.c_str());
  std::string s;

  if (file) {
    while(std::getline(file,s)) {
      if (!s.size()) continue;
      if (s[s.size()-1] == '\r') s.erase(s.size()-1); //Handle DOS files
      if (s[0] == '#') { 
        continue;
      }

      task = Task::createFromStr(s);
      if (task) {
        this->addTask(task);
      } else {
        #if defined (WIN32) || defined (LINUX)
          OutputDebugString("\nTaskList::load: error creating task\n");
        #endif
      }
    }
    file.close();
  } else {
    return -1;
  }

  return 1;
}

void TaskList::addTask(Task *task) {
  tasks.push_back(task);
}

void TaskList::addTask(string params, bool rand) {
  addTask(Task::createFromStr(params, rand));
}

void TaskList::removeTask(Task *task) {
  vector<Task*>::iterator it;

  it = find(tasks.begin(), tasks.end(), task);
  
  if (it != tasks.end()) {
    SAFE_DELETE(*it);
    tasks.erase(it);
  } else {
    // TODO: task not found handling.
  }
}

void TaskList::passOneDay() {
  // TODO: "You have failed the task" message to the user when accepted task expires
  for (vector<Task*>::iterator it = tasks.begin(); it!=tasks.end(); ){
    (*it)->passOneDay();
    if ((*it)->isExpired()) {
      SAFE_DELETE(*it);
      it = tasks.erase(it);
    } else {
      it++;
    }
  }
}

void TaskList::getDoneTasks(Player * _p1, Player * _p2, GameApp * _app, vector<Task*>* result) {
  result->clear();
  // TODO: Return only accepted tasks
  for (vector<Task*>::iterator it = tasks.begin(); it!=tasks.end(); it++) {
    if ((*it)->isDone(_p1, _p2, _app)) {
      result->push_back(*it);
    }
  }  
}

int TaskList::getTaskCount() {
  return tasks.size();
}

void TaskList::Render() {
  JRenderer * r = JRenderer::GetInstance();  
  JLBFont * f = resources.GetJLBFont(Constants::MAIN_FONT);
  JLBFont * f2 = resources.GetJLBFont(Constants::MAGIC_FONT);
  JLBFont * f3 = resources.GetJLBFont(Constants::MENU_FONT); //OPTION_FONT
  f2->SetColor(ARGB(255, 205, 237, 240));
  f3->SetColor(ARGB(255, 219, 206, 151));

  r->FillRect(0,0,SCREEN_WIDTH,SCREEN_HEIGHT,ARGB(128,0,0,0));
  r->FillRect(10,10,SCREEN_WIDTH-10,SCREEN_HEIGHT-10,ARGB(128,0,0,0));
  
  float posX = 20, posY = 20;
  char buffer[300];
  string title = _("Task Board");

  f3->DrawString(title.c_str(), (SCREEN_WIDTH-20)/2 - title.length()*4, posY);
  posY += 20;

  if (0 == tasks.size()) {
    f->DrawString(_("These are no tasks that need to be done. Come again tomorrow.").c_str(), posX, posY);
    posY += 20;
    return;
  } 

  for (vector<Task*>::iterator it = tasks.begin(); it!=tasks.end(); it++) {
    sprintf(buffer, "%s", (*it)->getShortDesc().c_str());
    f2->DrawString(buffer, posX, posY);
    sprintf(buffer, "Days left: %i", (*it)->getExpiration());
    f->DrawString(buffer, SCREEN_WIDTH - 180, posY);    
    sprintf(buffer, "Reward: %i", (*it)->getReward());
    f->DrawString(buffer, SCREEN_WIDTH - 90, posY);    
    posY += 15;

    sprintf(buffer, "%s", (*it)->getDesc().c_str());
    f->DrawString(buffer, posX+10, posY);    
    posY += 15;
    //r->DrawLine((SCREEN_WIDTH)/2 - 200, posY, (SCREEN_WIDTH)/2 + 200, posY, ARGB(128, 255, 255, 255));
  }
}

void TaskList::addRandomTask(int diff) {
  // TODO: Weighted random (rarity of tasks) 
  //       - based on counts of finished tasks? 
  //         Winning a task several times may slightly lessen the probability of it being generated
  string s (TASKS_ALL);
  char taskType[2];
  sprintf(taskType, "%c", s[rand()%s.length()]);
  addTask(string(taskType), TRUE);
}


TaskList::~TaskList() {
  for (unsigned int i=0; i<tasks.size(); i++) {
    SAFE_DELETE(tasks[i]);
  }
}

/*----------------------------------------*/
/*---------------- Tasks -----------------*/
/*----------------------------------------*/

/*----------- TaskWinAgainst -------------*/

TaskWinAgainst::TaskWinAgainst(int _opponent) : Task(TASK_WIN_AGAINST) {
  opponent = _opponent;
}

int TaskWinAgainst::computeReward() {
  return 75 + rand() % 75;
}

string TaskWinAgainst::createDesc() {  
  char buffer[4096];
  switch (rand()%2) {
    case 0:
      sprintf(buffer, _("You have to defeat %s before it causes too much harm.").c_str(), getOpponentName().c_str());
      break;
    case 1:
      sprintf(buffer, _("Please defeat %s as soon as possible.").c_str(), getOpponentName().c_str());
      break;
  }
  return buffer;
}

string TaskWinAgainst::getShortDesc() {
  char buffer[4096];
  string result;
  
  sprintf(buffer, _("Defeat %s").c_str(), getOpponentName().c_str());
  result = buffer;
  
  return result;
}

bool TaskWinAgainst::isDone(Player * _p1, Player * _p2, GameApp * _app) {
  GameObserver * g = GameObserver::GetInstance();
  AIPlayerBaka * baka = (AIPlayerBaka*)_p2;
  return ( 
          (baka) 
          && (!_p1->isAI()) && (_p2->isAI()) && (g->gameOver != _p1) // Human player wins
          && (baka->deckId == opponent)
         );
}

/*----------- TaskSlaughter -------------*/

TaskSlaughter::TaskSlaughter(int _opponent, int _targetLife) : TaskWinAgainst(_opponent) {
  type = TASK_SLAUGHTER;
  targetLife = _targetLife;
}

int TaskSlaughter::computeReward() {
  return 2*TaskWinAgainst::computeReward() - targetLife*9;
}

void TaskSlaughter::randomize() {
  Task::randomize();
  targetLife = -15 - rand()%10;
}

string TaskSlaughter::createDesc() {
  char buffer[4096];
  switch (rand()%2) {
    case 0:
      sprintf(buffer, _("Defeat %s in a way it won't forget. Bring it to %i life.").c_str(), getOpponentName().c_str(), targetLife);
      break;
    case 1:
      sprintf(buffer, _("Slaughter %s! Beat it to %i life or less.").c_str(), getOpponentName().c_str(), targetLife);
      break;
  }
  return buffer;
}

string TaskSlaughter::getShortDesc(){
  char buffer[4096];
  sprintf(buffer, _("Slaughter %s (%i lives)").c_str(), getOpponentName().c_str(), targetLife);
  return buffer;
}

bool TaskSlaughter::isDone(Player * _p1, Player * _p2, GameApp * _app) {
  return TaskWinAgainst::isDone(_p1, _p2, _app) 
         && (_p2->life <= targetLife);
}

void TaskSlaughter::storeCustomAttribs() {
  char buff[256];
  
  sprintf(buff, "%i", targetLife);
  persistentAttribs.push_back(buff);
}

void TaskSlaughter::restoreCustomAttribs() {
  targetLife = atoi(persistentAttribs[COMMON_ATTRIBS_COUNT].c_str());
}

/*----------- TaskDelay -------------*/
// Now serves as both 'Delay' and 'Fast defeat' task.

TaskDelay::TaskDelay(int _opponent, int _turn) : TaskWinAgainst(_opponent) {
  type = TASK_DELAY;
  turn = _turn;
  afterTurn = TRUE;
}

int TaskDelay::computeReward() {
  return TaskWinAgainst::computeReward() + (afterTurn ? turn*30 : (17-turn)*(17-turn)*24);
}

void TaskDelay::randomize() {
  Task::randomize();
  afterTurn = rand()%2;
  turn = afterTurn ? rand()%15 + 20 : 15 - rand()%7;
}

string TaskDelay::createDesc() {
  char buffer[4096];
  if (afterTurn) {
    switch (rand()%2) {
      case 0:
        sprintf(buffer, _("Defeat %s after keeping it occupied for %i turns.").c_str(), getOpponentName().c_str(), turn);
        break;
      case 1:
        sprintf(buffer, _("Defeat %s, but play with it for %i turns first.").c_str(), getOpponentName().c_str(), turn);
        break;
    }
  } else {
    switch (rand()%2) {
      case 0:
        sprintf(buffer, _("Defeat %s and make sure it doesn't take more than %i turns.").c_str(), getOpponentName().c_str(), turn);
        break;
      case 1:
        sprintf(buffer, _("Defeat %s, in a duel no longer than %i turns.").c_str(), getOpponentName().c_str(), turn);
        break;
    }
  }
  return buffer;
}

string TaskDelay::getShortDesc(){
  char buffer[4096];
  if (afterTurn) {
    sprintf(buffer, _("Delay %s for %i turns").c_str(), getOpponentName().c_str(), turn);
  } else {
    sprintf(buffer, _("Defeat %s before turn %i").c_str(), getOpponentName().c_str(), turn + 1);
  }
  return buffer;
}

bool TaskDelay::isDone(Player * _p1, Player * _p2, GameApp * _app) {
  GameObserver * g = GameObserver::GetInstance();
  return TaskWinAgainst::isDone(_p1, _p2, _app) 
        && (afterTurn ? (g->turn >= turn) : (g->turn <= turn));
}

void TaskDelay::storeCustomAttribs() {
  char buff[256];
  
  sprintf(buff, "%i", turn);
  persistentAttribs.push_back(buff);
  sprintf(buff, "%i", afterTurn);
  persistentAttribs.push_back(buff);
}

void TaskDelay::restoreCustomAttribs() {
  turn = atoi(persistentAttribs[COMMON_ATTRIBS_COUNT].c_str());
  if (persistentAttribs.size() > COMMON_ATTRIBS_COUNT + 1) {
    afterTurn = atoi(persistentAttribs[COMMON_ATTRIBS_COUNT+1].c_str());    
  }
}

/* ------------ TaskImmortal ------------ */

TaskImmortal::TaskImmortal(int _targetLife) : Task(TASK_IMMORTAL) {
  targetLife = _targetLife;
  level = (targetLife < 100) ? 0 : ((targetLife < 1000) ? 1 : 2);
}

int TaskImmortal::computeReward() {
  return targetLife*2 + 150 + rand()%50;
}

string TaskImmortal::createDesc() {
  char buffer[4096];
  
  sprintf(buffer, _("Defeat any opponent, having at least %i lives in the end.").c_str(), targetLife);

  return buffer;
}

string TaskImmortal::getShortDesc(){
  char buffer[4096];
  
  switch (level) {
    case 0:
      sprintf(buffer, _("Win flawlessly (%i lives)").c_str(), targetLife);
      break;
    case 1:
      sprintf(buffer, _("Reach Invulnerability (%i lives)").c_str(), targetLife);
      break;
    case 2:
      sprintf(buffer, _("Reach Immortality! (%i lives)").c_str(), targetLife);
      break;
  }

  return buffer;
}

bool TaskImmortal::isDone(Player * _p1, Player * _p2, GameApp * _app) {
  GameObserver * g = GameObserver::GetInstance();
  return (!_p1->isAI()) && (_p2->isAI()) && (g->gameOver != _p1) // Human player wins
          && (_p1->life >= targetLife);
}

void TaskImmortal::storeCustomAttribs() {
  char buff[256];
  
  sprintf(buff, "%i", targetLife);
  persistentAttribs.push_back(buff);
  
  sprintf(buff, "%i", level);
  persistentAttribs.push_back(buff);
}

void TaskImmortal::restoreCustomAttribs() {
  targetLife = atoi(persistentAttribs[COMMON_ATTRIBS_COUNT].c_str());
  level = atoi(persistentAttribs[COMMON_ATTRIBS_COUNT+1].c_str());
}

void TaskImmortal::randomize() {
  Task::randomize();
  level = rand() % 3;
  switch (level) {
    case 0:
      targetLife = 20 + rand()%10;
      break;
    case 1:
      targetLife = 100 + 5*(rand()%5);
      break;
    case 2:
      targetLife = 1000 + 50*(rand()%10);
      break;
  }
}
/* ------------ TaskMassiveBurial ------------ */

TaskMassiveBurial::TaskMassiveBurial(int _color, int _bodyCount) : Task(TASK_MASSIVE_BURIAL) {
  color = _color;
  bodyCount = _bodyCount;
  if ( (0 == color) || (0 == bodyCount) ) {
    randomize();
  }
}

int TaskMassiveBurial::computeReward() {  
  return rand()%150 + bodyCount * ((Constants::MTG_COLOR_LAND == color) ? 70 : 50);
}

string TaskMassiveBurial::createDesc() {
  char buffer[4096];

  sprintf(buffer, _("Bury %i %s cards to your opponent's graveyard and defeat him.").c_str(), bodyCount, Constants::MTGColorStrings[color]);

  return buffer;
}

string TaskMassiveBurial::getShortDesc(){
  char buffer[4096];
  switch (color) {
    case Constants::MTG_COLOR_GREEN:
      sprintf(buffer, _("Tame the nature (%i)").c_str(), bodyCount);
      break;
    case Constants::MTG_COLOR_BLUE:
      sprintf(buffer, _("Evaporation (%i)").c_str(), bodyCount);
      break;
    case Constants::MTG_COLOR_RED:
      sprintf(buffer, _("Bring the order (%i)").c_str(), bodyCount);
      break;
    case Constants::MTG_COLOR_BLACK:
      sprintf(buffer, _("Exorcism (%i)").c_str(), bodyCount);
      break;
    case Constants::MTG_COLOR_WHITE:
      sprintf(buffer, _("Dusk (%i)").c_str(), bodyCount);
      break;
    case Constants::MTG_COLOR_LAND:
      sprintf(buffer, _("Selective disaster (%i)").c_str(), bodyCount);
      break;
  }
  return buffer;
}

bool TaskMassiveBurial::isDone(Player * _p1, Player * _p2, GameApp * _app) {
  int countColor = 0;
  vector<MTGCardInstance *> cards = _p2->game->graveyard->cards;
  
  for (vector<MTGCardInstance *>::iterator it = cards.begin(); it!=cards.end(); it++){
    if ((*it)->hasColor(color)) {
      countColor++;
    }
  } 

  return (countColor >= bodyCount);
}

void TaskMassiveBurial::storeCustomAttribs() {
  char buff[256];
  sprintf(buff, "%i", color);
  persistentAttribs.push_back(buff);
 
  sprintf(buff, "%i", bodyCount);
  persistentAttribs.push_back(buff);
}

void TaskMassiveBurial::restoreCustomAttribs() {
  color = atoi(persistentAttribs[COMMON_ATTRIBS_COUNT].c_str());
  bodyCount = atoi(persistentAttribs[COMMON_ATTRIBS_COUNT+1].c_str());
}

void TaskMassiveBurial::randomize() {
  Task::randomize();
  color = rand()%(Constants::MTG_NB_COLORS - 1) + 1;
  bodyCount = 5 + ((Constants::MTG_COLOR_LAND == color) ? rand()%10 : rand()%20);
}

/* ------------ Task template ------------ 

TaskXX::TaskXX() : Task(TASK_XX) {
  // TODO: Implement
}

int TaskXX::computeReward() {
  // TODO: Implement
  return 100;
}

string TaskXX::createDesc() {
  // TODO: Implement
  char buffer[4096];
  
  switch (rand()%2) {
    case 0:
      sprintf(buffer, _("%s").c_str(), getOpponentName().c_str());
      break;
    case 1:
      sprintf(buffer, _("%s").c_str(), getOpponentName().c_str());
      break;
  }
  return buffer;
}

string TaskXX::getShortDesc(){
  // TODO: Implement
  char buffer[4096];
  sprintf(buffer, _("%s").c_str(), getOpponentName().c_str());
  return buffer;
}

bool TaskXX::isDone(Player * _p1, Player * _p2, GameApp * _app) {
  // TODO: Implement
  return TRUE;
}

void TaskXX::storeCustomAttribs() {
  // TODO: Implement
  char buff[256];
  persistentAttribs.push_back(VarXX);
  
  sprintf(buff, "%i", VarXY);
  persistentAttribs.push_back(buff);
}

void TaskXX::restoreCustomAttribs() {
  // TODO: Implement
  VarXX = persistentAttribs[COMMON_ATTRIBS_COUNT];
  VarXY = atoi(persistentAttribs[COMMON_ATTRIBS_COUNT+1].c_str());
}

void TaskXX::randomize() {
  // TODO: Implement
  Task::randomize();
  VarXX = rand()%10 + 1;
}

*/
