#include "../include/config.h"
#include "../include/CardDisplay.h"
#include "../include/CardGui.h"
#include "../include/TargetChooser.h"
#include "../include/MTGGameZones.h"
#include "../include/GameObserver.h"

CardDisplay::CardDisplay() : mId(0), game(GameObserver::GetInstance()) {
  tc = NULL;
  listener = NULL;
  nb_displayed_items = 7;
  start_item = 0;
  x = 0;
  y = 0;
  zone = NULL;
}

CardDisplay::CardDisplay(int id, GameObserver* game, int _x, int _y, JGuiListener * _listener, TargetChooser * _tc, int _nb_displayed_items ) : mId(id), game(game), x(_x), y(_y) {
  tc = _tc;
  listener = _listener;
  nb_displayed_items = _nb_displayed_items;
  start_item = 0;
  if (x + nb_displayed_items * 30 + 25 > SCREEN_WIDTH) x = SCREEN_WIDTH - (nb_displayed_items * 30 + 25);
  if (y + 55 > SCREEN_HEIGHT) y = SCREEN_HEIGHT - 55;
  zone = NULL;
}


void CardDisplay::AddCard(MTGCardInstance * _card){
  CardGui * card = NEW CardView(CardSelector::nullZone, _card, x + 20 + (mCount - start_item) * 30, y + 25);
  Add(card);
}


void CardDisplay::init(MTGGameZone * zone){
  resetObjects();
  if (!zone) return; 
  start_item = 0;
  for (int i= 0; i< zone->nb_cards; i++){
    AddCard(zone->cards[i]);
  }
  if (mCount) mObjects[0]->Entering();
}

void CardDisplay::rotateLeft(){
  if (start_item==0) return;
  for (int i = 0; i<mCount; i++){
    CardGui * cardg = (CardGui *)mObjects[i];
    cardg->x+=30;
  }
  start_item --;
}

void CardDisplay::rotateRight(){
  if (start_item==mCount-1) return;
  for (int i= 0; i<mCount; i++){
    CardGui * cardg = (CardGui *)mObjects[i];
    cardg->x-=30;
  }
  start_item ++;
}



void CardDisplay::Update(float dt){
  bool update = false;

  if (zone){
    int size = zone->cards.size();
    for (int i = start_item; i< start_item + nb_displayed_items && i < mCount; i++){
      if (i > size - 1) {update = true; break;}
      CardGui * cardg = (CardGui *)mObjects[i];
      if (cardg->card != zone->cards[i]) update = true;
    }
  }
  PlayGuiObjectController::Update(dt);
  if (update) init(zone);
}

bool CardDisplay::CheckUserInput(JButton key){
  if (JGE_BTN_SEC == key)
    {
      if (listener){
	      listener->ButtonPressed(mId, 0);
	      return true;
      }
    }

  if (!mCount)
    return false;

  if (mActionButton == key)
    {
      if (mObjects[mCurr] && mObjects[mCurr]->ButtonPressed()){
	CardGui * cardg = (CardGui *)mObjects[mCurr];
	if (tc)
	  {
	    tc->toggleTarget(cardg->card);
	    return true;
	  }else{
	  if (game) game->ButtonPressed(cardg);
	  return true;
	}
      }
      return true;
    }


  switch(key)
    {
    case JGE_BTN_LEFT :
      {
	int n = mCurr;
	n--;
	if (n<start_item){
	  if (n< 0){n = 0;}
	  else{ rotateLeft();}
	}
	if (n != mCurr && mObjects[mCurr] != NULL && mObjects[mCurr]->Leaving(JGE_BTN_LEFT)){
	  mCurr = n;
	  mObjects[mCurr]->Entering();
	}
	return true;
      }
    case JGE_BTN_RIGHT :
      {
	int n = mCurr;
	n++;
	if (n>= mCount){n = mCount-1;}
	if (n>= start_item + nb_displayed_items){
	  rotateRight();
	}
	if (n != mCurr && mObjects[mCurr] != NULL && mObjects[mCurr]->Leaving(JGE_BTN_RIGHT)){
	  mCurr = n;
	  mObjects[mCurr]->Entering();
	}
      }
      return true;
    default:
      ;
    }
  return false;
}


void CardDisplay::Render(){

  JRenderer * r = JRenderer::GetInstance();
  r->DrawRect(x,y,nb_displayed_items * 30 + 20, 50, ARGB(255,255,255,255));
  if (!mCount) return;
  for (int i = start_item; i< start_item + nb_displayed_items && i < mCount; i++){
    if (mObjects[i]){
      mObjects[i]->Render();
      if (tc){
	CardGui * cardg = (CardGui *)mObjects[i];
	if( tc->alreadyHasTarget(cardg->card)){
	  r->DrawCircle(cardg->x + 5, cardg->y+5,5, ARGB(255,255,0,0));
	}else if (!tc->canTarget(cardg->card)){
	  r->FillRect(cardg->x,cardg->y,30,40,ARGB(200,0,0,0));
	}
      }
    }
  }

  //TODO: CardSelector should handle the graveyard and the library in the future...
  if (mCount && mObjects[mCurr] != NULL){
    mObjects[mCurr]->Render();
    CardGui * cardg = ((CardGui *)mObjects[mCurr]);
    Pos pos = Pos(CardGui::BigWidth / 2, CardGui::BigHeight / 2 - 10, 1.0, 0.0, 220);
    int showMode = BIG_MODE_SHOW;
    if (game){
      showMode = game->mLayers->cs->bigMode;
      pos.actY = 150;
      if (x < (CardGui::BigWidth / 2)) pos.actX = SCREEN_WIDTH - 10 - CardGui::BigWidth / 2;
    }

    switch(showMode){
        case BIG_MODE_SHOW:
          cardg->RenderBig(pos);
          break;
        case BIG_MODE_TEXT:
          cardg->alternateRenderBig(pos);
          break;
        default:
          break;
    }

  }
}

ostream& CardDisplay::toString(ostream& out) const
{
  return (out << "CardDisplay ::: x,y : " << x << "," << y << " ; start_item : " << start_item << " ; nb_displayed_items " << nb_displayed_items << " ; tc : " << tc << " ; listener : " << listener);
}


std::ostream& operator<<(std::ostream& out, const CardDisplay& m)
{
  return m.toString(out);
}
