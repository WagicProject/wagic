/* This class handles the display on the main game screen :
   cards in play, graveyard, library, games phases, Players avatars
*/

#include <math.h>
#include "../include/config.h"
#include "../include/MTGGuiPlay.h"
#include "../include/MTGCardInstance.h"
#include "../include/CardGui.h"
#include "../include/CardDisplay.h"
#include "../include/Translate.h"

#define ZX_MAIN 100
#define ZY_MAIN 22
#define ZH_CREATURES 50
#define Z_CARDWIDTH 30
#define Z_CARDHEIGHT 40
#define Z_MAIN_NBCARDS 7
#define Z_SPELLS_NBCARDS 3
#define ZX_SPELL 450
#define ZY_SPELL 22




MTGGuiPlay::MTGGuiPlay(int id, GameObserver * _game):PlayGuiObjectController(id, _game){
  currentPlayer = NULL;
  offset = 0;


  mPhaseBarTexture = GameApp::CommonRes->GetTexture("graphics/phasebar.png");
  for (int i=0; i < 12; i++){
    phaseIcons[2*i] = NEW JQuad(mPhaseBarTexture, i*28, 0, 28, 28);
    phaseIcons[2*i + 1] = NEW JQuad(mPhaseBarTexture, i*28, 28, 28, 28);
  }
  mGlitter = NEW JQuad(mPhaseBarTexture, 392, 0, 5, 5);
  mGlitter->SetHotSpot(2.5,2.5);
  mGlitterAlpha = -1;
  mFont= GameApp::CommonRes->GetJLBFont("graphics/simon");

  //load all the icon images
  mIcons[Constants::MTG_COLOR_ARTIFACT] = GameApp::CommonRes->GetQuad("c_artifact");
  mIcons[Constants::MTG_COLOR_LAND] = GameApp::CommonRes->GetQuad("c_land");
  mIcons[Constants::MTG_COLOR_WHITE] = GameApp::CommonRes->GetQuad("c_white");
  mIcons[Constants::MTG_COLOR_RED] = GameApp::CommonRes->GetQuad("c_red");
  mIcons[Constants::MTG_COLOR_BLACK] = GameApp::CommonRes->GetQuad("c_black");
  mIcons[Constants::MTG_COLOR_BLUE] = GameApp::CommonRes->GetQuad("c_blue");
  mIcons[Constants::MTG_COLOR_GREEN] = GameApp::CommonRes->GetQuad("c_green");
  for (int i=0; i < 7; i++){
    mIcons[i]->SetHotSpot(16,16);
  }

  mBgTex = GameApp::CommonRes->GetTexture("graphics/background.png");
  if (mBgTex) mBg = NEW JQuad(mBgTex, 0, 0, 480, 272);
  else {
    mBg = NULL;
    GameApp::systemError = "error Loading Texture mBgTex in MTGGuiPlay intialization";
  }

  mBgTex2 = GameApp::CommonRes->GetTexture("graphics/back.jpg");
  if (mBgTex2){
    mBg2 = NEW JQuad(mBgTex2, 0, 0, 480, 255);
    for (int i= 0; i < 4; i++){
      alphaBg[i] = 255;
    }
  }else{
    mBg2 = NULL;
    GameApp::systemError = "error Loading Texture mBgTex2 in MTGGuiPlay intialization";
  }

  alphaBg[0] = 0;
  AddPlayersGuiInfo();
}


CardGui * MTGGuiPlay::getByCard(MTGCardInstance * card){
  for (int i = offset; i < mCount; i++){
    CardGui * cardg = (CardGui *)mObjects[i];
    if(cardg && cardg->card == card){
      return cardg;
    }
  }
  return NULL;

}

void MTGGuiPlay::initCardsDisplay(){
  for (int i = 0; i < SCREEN_WIDTH/5; i++){
    for(int j=0; j < SCREEN_HEIGHT/5; j++){
      cardsGrid[i][j] = NULL;
    }
  }
  cards_x_limit = 12;
  for (int i = 0; i < 2; i++){
    nb_creatures[i] = 0;
    nb_lands[i] = 0;
    nb_spells[i] = 0;
  }

  for (int i = 6; i < mCount; i++){
    CardGui * cardg = (CardGui *)mObjects[i];
    cardg->x = 0;
    cardg->y = 0;
  }
}


void MTGGuiPlay::adjustCardPosition(CardGui * cardg){
  int x5 = cardg->x / 5;
  int y5 = cardg->y / 5;

  while (cardsGrid[x5][y5] && x5 <SCREEN_WIDTH/5 && y5 < SCREEN_HEIGHT/5 ){
    x5++;
    y5++;
  }
  cardg->x = x5 * 5;
  cardg->y = y5 * 5;
  cardsGrid[x5][y5] = cardg->card;
}

void MTGGuiPlay::setCardPosition(CardGui * cardg, int player, int playerTurn, int spellMode){
  GameObserver * g = GameObserver::GetInstance();
  MTGCardInstance * card = cardg->card;
  if (!(cardg->x ==0 && cardg->y ==0)) return ;
  if (card->target)
    return;
  if (spellMode && (card->isACreature() || card->hasType("land"))) return;
  if (!spellMode && !card->isACreature() && !card->hasType("land")) return;
  if (card->isACreature()){
    int x_offset = nb_creatures[player] % cards_x_limit;
    int y_offset = nb_creatures[player] / cards_x_limit;
    cardg->x= ZX_MAIN + (Z_CARDWIDTH * x_offset);
    cardg->y=ZY_MAIN +  ZH_CREATURES + (Z_CARDHEIGHT * y_offset) + 100 * (1-player);
    nb_creatures[player]++;

    if (playerTurn){
      if (card->isAttacker()){
	      cardg->y=122 + 30 * (1-player);
        //Sets position of opponents as well
        if (player == 1){
          for (list<MTGCardInstance *>::iterator it= card->blockers.begin(); it !=card->blockers.end() ; ++it){
            CardGui * c = getByCard(*it);
            if (c) {
              setCardPosition(c,1-player,1-playerTurn,spellMode);
              adjustCardPosition(c);
            }
          }
        }else{
          for (list<MTGCardInstance *>::reverse_iterator it= card->blockers.rbegin(); it !=card->blockers.rend() ; ++it){
            CardGui * c = getByCard(*it);
            if (c) {
              setCardPosition(c,1-player,1-playerTurn,spellMode);
              adjustCardPosition(c);
            }
          }
        }
        
      }
    }else{
      if (card->isDefenser()){
	      CardGui * targetg = getByCard(card->isDefenser());
	      if (targetg) cardg->x = targetg->x;
	      cardg->y=122 + 30 * (1-player);
      }
    }

  }else if(card->hasType("land")){
    int x_offset = nb_lands[player] % cards_x_limit;
    int y_offset = nb_lands[player] / cards_x_limit;
    cardg->x=ZX_MAIN + (Z_CARDWIDTH * x_offset);
    cardg->y=ZY_MAIN + (Z_CARDHEIGHT * y_offset) + 200 * (1-player);
    nb_lands[player]++;
  }else{
    int y_offset = nb_spells[player] % Z_SPELLS_NBCARDS;
    int x_offset = nb_spells[player] / Z_SPELLS_NBCARDS;
    cardg->x=ZX_SPELL - (Z_CARDWIDTH * x_offset);
    cardg->y=ZY_SPELL + (Z_CARDHEIGHT * y_offset) + 125 * (1-player);
    nb_spells[player]++;
    cards_x_limit = 12 - (nb_spells[player] + 2)/ Z_SPELLS_NBCARDS;
  }
  adjustCardPosition(cardg);
}


void MTGGuiPlay::setTargettingCardPosition(CardGui * cardg, int player, int playerTurn){
  MTGCardInstance * card = cardg->card;
  MTGCardInstance * target = card->target;
  if (!target)
    return;
  CardGui * targetg = getByCard(target);
  if (targetg){
    cardg->y=targetg->y + 5;
    cardg->x=targetg->x + 5;
  }
  adjustCardPosition(cardg);
  return;
}

void MTGGuiPlay::forceUpdateCards(){
  GameObserver * game = GameObserver::GetInstance();
  Player * player = game->players[0];
  int player0Mode =(game->currentPlayer == player);
  int nb_cards = player->game->inPlay->nb_cards;
  resetObjects();
  AddPlayersGuiInfo();
  offset = mCount;
  bool hasFocus = player0Mode;
  offset = 6;

  Player * opponent = game->players[1];
  int opponent_cards = opponent ->game->inPlay->nb_cards;

    for (int i = 0;i<nb_cards; i++){
      if (hasFocus) mCurr = mCount ;
      CardGui * object = NEW CardGui(mCount, player->game->inPlay->cards[i],40, i*35 + 10, 200, hasFocus);
      Add(object);
      hasFocus = false;
    }
    hasFocus = !player0Mode;
    for (int i = 0;i<opponent_cards; i++){
      if (hasFocus) mCurr = mCount ;
      CardGui * object = NEW CardGui(mCount, opponent->game->inPlay->cards[i],40, i*35 + 10, 10, hasFocus);
      Add(object);
      hasFocus = false;
    }

    currentPlayer = game->currentPlayer;
}

int MTGGuiPlay::receiveEvent(WEvent *event){
  WEventZoneChange * e = dynamic_cast<WEventZoneChange*>(event);
  if (!e) return 0;
  int ok = 0;
  for (int i = 0; i < 2 ; i++){
    Player * p = game->players[i];
    if (e->from == p->game->inPlay || e->to == p->game->inPlay ) ok = 1;
  }
  if (!ok) return 0;
  forceUpdateCards();
  updateCards();
  return 1;
}

void MTGGuiPlay::updateCards(){
  GameObserver * game = GameObserver::GetInstance();
  Player * player = game->players[0];
  int player0Mode =(game->currentPlayer == player);
  int nb_cards = player->game->inPlay->nb_cards;
  MTGCardInstance * attackers[MAX_ATTACKERS];
  for (int i = 0; i <MAX_ATTACKERS; i++){
    attackers[i] = NULL;
  }

  offset = 6;

  Player * opponent = game->players[1];
  int opponent_cards = opponent ->game->inPlay->nb_cards;



  //This is just so that we display the cards of the current player first, so that blockers are correctly positionned
  initCardsDisplay();
  for (int j= 0; j < 2; j++){
    if (j != player0Mode){
      for (int i =0; i<nb_cards; i++){
	      CardGui * cardGui = (CardGui *)mObjects[i + offset];
	      setCardPosition(cardGui, 0, player0Mode, 1);
      }
      for (int i =0; i<nb_cards; i++){
	      CardGui * cardGui = (CardGui *)mObjects[i + offset];
        MTGCardInstance * card = cardGui->card;
        setCardPosition(cardGui, 0, player0Mode, 0);
      }
    }else{
      for (int i =0; i<opponent_cards; i++){
	      CardGui * cardGui = (CardGui *)mObjects[nb_cards + i + offset];
	      setCardPosition(cardGui, 1, !player0Mode,1);
      }
      for (int i =0; i<opponent_cards; i++){
	      CardGui * cardGui = (CardGui *)mObjects[nb_cards + i + offset];
	      MTGCardInstance * card = cardGui->card;
        setCardPosition(cardGui, 1, !player0Mode,0);
      }
    }
  }

  for (int i =0; i<nb_cards; i++){
    CardGui * cardGui = (CardGui *)mObjects[i + offset ];
    setTargettingCardPosition(cardGui, 0, player0Mode);
  }

  for (int i =0; i<opponent_cards; i++){
    CardGui * cardGui = (CardGui *)mObjects[nb_cards + i + offset];
    setTargettingCardPosition(cardGui, 1, !player0Mode);
  }

}

void MTGGuiPlay::AddPlayersGuiInfo(){
  //init with the players objects
  if (mCount == 0){
    Add(NEW GuiAvatar(-1,50,2,155,false, GameObserver::GetInstance()->players[0]));
    Add(NEW GuiAvatar(-2,50,2,30,false,GameObserver::GetInstance()->players[1]));

    Add(NEW GuiGraveyard(-3,30,40,150,false, GameObserver::GetInstance()->players[0]));
    Add(NEW GuiLibrary(-4,30,40,180,false, GameObserver::GetInstance()->players[0]));


    Add(NEW GuiGraveyard(-5,30,40,30,false, GameObserver::GetInstance()->players[1]));
    Add(NEW GuiLibrary(-6,30,40,60,false, GameObserver::GetInstance()->players[1]));
  }
}

void MTGGuiPlay::Update(float dt){
  updateCards();
  PlayGuiObjectController::Update(dt);
}



bool MTGGuiPlay::CheckUserInput(u32 key){
  for (int i = 2; i<6;i++){
    GuiGameZone * zone = (GuiGameZone *)mObjects[i];
    if (zone->showCards){
      return zone->cd->CheckUserInput(key);
    }
  }
  return PlayGuiObjectController::CheckUserInput(key);
}


void MTGGuiPlay::RenderPlayerInfo(int playerid){
  JRenderer * r = JRenderer::GetInstance();
  Player * player = GameObserver::GetInstance()->players[playerid];

  //Avatar - already done in main Render phase
  //GuiAvatar * avatar = (GuiAvatar *)mObjects[3*playerid];
  //avatar->Render();



  //Mana
  ManaCost * cost = player->getManaPool();
  int nbicons = 0;
  for (int j=0; j<6;j++){
    int value = cost->getCost(j);
    for (int i=0; i<value; i++){
      float x = 10 + (nbicons %4) * 15;
      float y = 90 + 125 * (1-playerid) + (15 * (nbicons / 4));
      r->RenderQuad(mIcons[j],x,y,0,0.5, 0.5);
      nbicons++;
    }
  }
}


void MTGGuiPlay::RenderPhaseBar(){
  GameObserver * game = GameObserver::GetInstance();
  JRenderer * renderer = JRenderer::GetInstance();
  int currentPhase = game->getCurrentGamePhase();
  for (int i=0; i < 12; i++){
    int index = 2*i + 1 ;
    if (i==currentPhase-1){
      index-=1;
    }
    renderer->RenderQuad(phaseIcons[index], 200 + 14*i,0,0,0.5,0.5);
  }
  if (game->currentlyActing()->isAI()){
    mFont->SetColor(ARGB(255,128,128,128));
  }else{
    mFont->SetColor(ARGB(255,255,255,255));
  }
  mFont->SetScale(DEFAULT_MAIN_FONT_SCALE);
  mFont->DrawString(_(Constants::MTGPhaseNames[currentPhase]).c_str(), 375, 2);
}

void MTGGuiPlay::Render(){
  LOG("Start MTGGuiPlay Render\n");
  JRenderer * renderer = JRenderer::GetInstance();

  //alphaBg[1] = 255;
  //alphaBg[2]= 255;
  //alphaBg[3] = 255;
  //mBg2->SetColor(ARGB(alphaBg[0], alphaBg[1],alphaBg[2],alphaBg[3]));
  renderer->RenderQuad(mBg2,0,17);

  if (game->currentGamePhase >= Constants::MTG_PHASE_COMBATBEGIN && game->currentGamePhase < Constants::MTG_PHASE_COMBATEND){
    if (alphaBg[0] < 50){
      alphaBg[3]-=12;
      alphaBg[2]-=12;
      alphaBg[0]+=3;
    }
    alphaBg[1] = 255;

  }else{
    if (alphaBg[0]){
      alphaBg[0]-=3;
      alphaBg[3]+=12;
      alphaBg[2]+=12;
    }
    alphaBg[1] = 255;
  }
  renderer->FillRect(0,0,480,272,ARGB(alphaBg[0], alphaBg[1],alphaBg[2],alphaBg[3]));

  renderer->RenderQuad(mBg,0,0);

  for (int i=mCount-1;i>=0;i--){
    if (mObjects[i]!=NULL && i!=mCurr){
      mObjects[i]->Render();
    }
  }

  RenderPhaseBar();
  RenderPlayerInfo(0);
  RenderPlayerInfo(1);

  int opponentHand = game->players[1]->game->hand->nb_cards;
  char buffer[10];
  sprintf(buffer,"%i",opponentHand);
  mFont->SetColor(ARGB(128,0,0,0));
  mFont->DrawString(buffer, 56, 20);
  mFont->SetColor(ARGB(255,255,255,255));
  mFont->DrawString(buffer, 54, 18);

  if (mGlitterAlpha < 0){
    mGlitterAlpha = 510;
    int position = rand() % 2;
    if (position){
      mGlitterX = 65 + rand() % (420);
      mGlitterY = 17 + rand() % (5);
    }else{
      mGlitterX = 65 + rand() % (5);
      mGlitterY = 15 + rand() % (250);
    }
  }
  mGlitter->SetColor(ARGB((255-abs(255-mGlitterAlpha)),240,240,255));
  renderer->RenderQuad(mGlitter,mGlitterX,mGlitterY, (float)(mGlitterAlpha)/(float)255, 1.2*float(mGlitterAlpha)/float(255),1.2*float(mGlitterAlpha)/float(255));
  mGlitterAlpha-=10;

  if (mCount && mObjects[mCurr] != NULL){
    mObjects[mCurr]->Render();
    if (hasFocus && mCurr >= offset && showBigCards && !game->currentlyActing()->isAI() ){
        //For some reason RenderBig crashes when the testsuite is playing, so we add a "isAI()" test...which was supposed to be there at some point anyways...
        CardGui * cardg = ((CardGui *)mObjects[mCurr]);
        cardg->RenderBig(-1,-1,showBigCards-1);
    }
  }
  LOG("End MTGGuiPlay Render\n");
}

MTGGuiPlay::~MTGGuiPlay(){
  LOG("==Destroying MTGGuiPlay==");
  delete mBg;
  //delete mBgTex;

  delete 	mGlitter;
  for (int i=0; i < 12; i++){
    delete phaseIcons[2*i] ;
    delete phaseIcons[2*i + 1];
  }
  //delete	mPhaseBarTexture;

  SAFE_DELETE(mBg2);
  //SAFE_DELETE(mBgTex2);

  LOG("==Destroying MTGGuiPlay Successful==");

}
