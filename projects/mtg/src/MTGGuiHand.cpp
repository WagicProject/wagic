#include "../include/config.h"
#include "../include/MTGGuiHand.h"
#include "../include/CardGui.h"

MTGGuiHand::MTGGuiHand(GameObserver* game) : game(game) {
  mShowHand = HAND_HIDE;
  mAnimState = 0;
  currentPlayer = NULL;
  mFont = GameApp::CommonRes->GetJLBFont("graphics/simon");
  for (int i = 0; i < 2; i++){
    currentId[i] = 0;
  }

}


void MTGGuiHand::updateCards(){
  Player * player = GameObserver::GetInstance()->currentlyActing();
  if (player->isAI()) player = GameObserver::GetInstance()->players[0];
  int nb_cards = player->game->hand->nb_cards;
  if (mCount != nb_cards || player != currentPlayer ){ //if the number of cards has changed, then an update occured (is this test engouh ?)
    resetObjects();
    if (currentId[player->getId()] >= nb_cards) currentId[player->getId()] = nb_cards - 1;
    for (int i = 0;i<nb_cards; i++){
      CardView* object = NEW CardView(player->game->hand->cards[i], (float)450 - (nb_cards-i) *35, SCREEN_HEIGHT_F - mAnimState*60);
      Add(object);
      if ( i == currentId[player->getId()]) mCurr = i;
    }
    currentPlayer = player;
  }


}


void MTGGuiHand::Update(float dt){
  updateCards();
  for (int i=0;i<mCount;i++){
    if (mObjects[i]!=NULL){
      ((CardGui *)mObjects[i])->y = SCREEN_HEIGHT - mAnimState*60;
    }
  }

  if (mShowHand == HAND_SHOW_ANIMATION){
    mAnimState +=7 *dt;
    if (mAnimState > 1){
      mAnimState = 1;
      mShowHand = HAND_SHOW;
    }
  }else if(mShowHand == HAND_HIDE_ANIMATION){
    mAnimState -=7 *dt;
    if (mAnimState < 0){
      mAnimState = 0;
      mShowHand = HAND_HIDE;
      modal = false;
    }
  }

  GuiCardsController::Update(dt);
  currentId[game->currentlyActing()->getId()] = mCurr;
}



bool MTGGuiHand::CheckUserInput(u32 key){
  if (PSP_CTRL_LTRIGGER == key) {
    if (mShowHand == HAND_HIDE){
      mShowHand = HAND_SHOW_ANIMATION;
      modal = true;
    }
    if (mShowHand == HAND_SHOW){
      mShowHand = HAND_HIDE_ANIMATION;
    }
    return true;
  }else if (PSP_CTRL_LEFT == key){
    //mGamePhase = NO_USER_INPUT;
  }

  if (mShowHand == HAND_HIDE || currentPlayer->isAI()){
    return false;
  }else{
    //    GuiCardsController::CheckUserInput(key);
    return true;
  }
}




void MTGGuiHand::Render(){
  if (mShowHand != HAND_HIDE){
    //	if (currentPlayer && !currentPlayer->isAI()){
    RenderMessageBackground(440-mCount * 35 , SCREEN_HEIGHT - mAnimState*60 - 10, mCount * 35 + 20, 70);
    for (int i=0;i<mCount;i++){
      if (mObjects[i]!=NULL && i!=mCurr){
	mObjects[i]->Render();
      }
    }
    if (mCount && mObjects[mCurr] != NULL){
      mObjects[mCurr]->Render();
      //      if (showBigCards) ((CardGui *)mObjects[mCurr])->RenderBig(10,-1,showBigCards-1);
    }
  }
}

// WALDORF - added method
void MTGGuiHand::showHand (bool show)
{
    if (show) mShowHand = HAND_SHOW;
    else mShowHand = HAND_HIDE;
}
