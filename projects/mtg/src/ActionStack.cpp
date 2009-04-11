/*
  The Action Stack contains all information for Game Events that can be interrupted (Interruptible)
*/
#include "../include/config.h"
#include "../include/ActionStack.h"
#include "../include/MTGAbility.h"
#include "../include/GameObserver.h"
#include "../include/Damage.h"
#include "../include/ManaCost.h"
#include "../include/GameOptions.h"
// WALDORF - added to support drawing big cards during interrupts
#include "../include/CardGui.h"


/*
  NextGamePhase requested by user
*/

int NextGamePhase::resolve(){
  GameObserver::GetInstance()->nextGamePhase();
  return 1;
}

void NextGamePhase::Render(){
  int nextPhase = (GameObserver::GetInstance()->getCurrentGamePhase() + 1) % Constants::MTG_PHASE_CLEANUP;
  JLBFont * mFont = GameApp::CommonRes->GetJLBFont(Constants::MAIN_FONT);
  mFont->SetBase(0);
  mFont->SetScale(DEFAULT_MAIN_FONT_SCALE);
  char buffer[200];
  int playerId = 1;
  if (GameObserver::GetInstance()->currentActionPlayer == GameObserver::GetInstance()->players[1]) playerId = 2;
  sprintf(buffer, "Player %i : -> %s", playerId, Constants::MTGPhaseNames[nextPhase]);
  mFont->DrawString(buffer, x + 30 , y, JGETEXT_LEFT);
}

NextGamePhase::NextGamePhase(int id): Interruptible(id){
  mHeight = 40;
  type = ACTION_NEXTGAMEPHASE;
}


/* Ability */
int StackAbility::resolve(){
  return (ability->resolve());
}
void StackAbility::Render(){
  JLBFont * mFont = GameApp::CommonRes->GetJLBFont(Constants::MAIN_FONT);
  mFont->SetBase(0);
  mFont->SetScale(DEFAULT_MAIN_FONT_SCALE);
  char buffer[200];
  sprintf(buffer, "%s", ability->getMenuText());
  mFont->DrawString(buffer, x + 30 , y, JGETEXT_LEFT);
  JRenderer * renderer = JRenderer::GetInstance();
  JQuad * quad = ability->source->getThumb();
  if (quad){
    quad->SetColor(ARGB(255,255,255,255));
    float scale = 30 / quad->mHeight;
    renderer->RenderQuad(quad, x  , y , 0,scale,scale);
  }else{
    mFont->DrawString(ability->source->getName(),x,y-15);
  }
}
StackAbility::StackAbility(int id,MTGAbility * _ability): Interruptible(id),ability(_ability){
  type=ACTION_ABILITY;
}


/* Spell Cast */

Spell::Spell(MTGCardInstance * _source): Interruptible(0), TargetsList(){
  source = _source;
  mHeight= 40;
  type = ACTION_SPELL;
  cost = NEW ManaCost();
}


Spell::Spell(int id, MTGCardInstance * _source, Targetable * _targets[], int nb_targets, ManaCost * _cost): Interruptible(id), TargetsList(_targets, nb_targets),cost(_cost){
  source = _source;
  mHeight = 40;
  type = ACTION_SPELL;
}


Spell::~Spell(){
  SAFE_DELETE(cost);
}

int Spell::resolve(){
  GameObserver * game = GameObserver::GetInstance();
  //TODO Remove target if it's not targettable anymore
   while (source->next){
    source = source->next;
  }
  source = source->controller()->game->putInPlay(source);

  //Play SFX
  if (GameOptions::GetInstance()->values[OPTIONS_SFXVOLUME].getIntValue() > 0){
    JSample * sample = source->getSample();
    if (sample){
      JSoundSystem::GetInstance()->PlaySample(sample);
    }
  }

  AbilityFactory af;
  af.addAbilities(game->mLayers->actionLayer()->getMaxId(), this);
  return 1;
}

void Spell::Render(){
  JLBFont * mFont = GameApp::CommonRes->GetJLBFont(Constants::MAIN_FONT);
  mFont->SetBase(0);
  mFont->SetScale(DEFAULT_MAIN_FONT_SCALE);
  mFont->DrawString(source->getName(), x + 30 , y, JGETEXT_LEFT);
  JRenderer * renderer = JRenderer::GetInstance();
  JQuad * quad = source->getThumb();
  if (quad){
    quad->SetColor(ARGB(255,255,255,255));
    float scale = mHeight  / quad->mHeight;
    renderer->RenderQuad(quad, x  , y , 0,scale,scale);
  }else{
    //
  }
  // WALDORF - added these lines to render a big card as well as the small one
  // in the interrupt window. A big card will be rendered no matter whether
  // the user has been using big cards or not. However, I do take into which
  // kind of big card they like.
  // The card will be rendered in the same place as the GuiHand
  // card. It doesn't attempt to hide the GUIHand card, it
  // just overwrites it.
  // I stole the render code from RenderBig() in CardGUI.cpp

  quad = source->getQuad();
  if (quad){
      quad->SetColor(ARGB(220,255,255,255));
      float scale = 257.f / quad->mHeight;
      renderer->RenderQuad(quad, 10 , 20 , 0.0f,scale,scale);
  }
  else
  {
      MTGCard * mtgcard = source->model;
      JLBFont * font = GameApp::CommonRes->GetJLBFont("graphics/magic");
      CardGui::alternateRender(mtgcard, NULL, 10 + 90 , 20 + 130, 0.0f,0.9f);

      quad = source->getThumb();
      if (quad){
          float scale = 250 / quad->mHeight;
          quad->SetColor(ARGB(40,255,255,255));
          renderer->RenderQuad(quad, 20, 20, 0.0f, scale, scale);
      }
  }

  // WALDORF - end


  Damageable * target = getNextDamageableTarget();
  if (target){
    quad = target->getIcon();
    if (quad){
      quad->SetColor(ARGB(255,255,255,255));
      float scale = 30 / quad->mHeight;
      renderer->RenderQuad(quad, x + 150  , y , 0,scale,scale);
    }else{
      if (target->type_as_damageable == DAMAGEABLE_MTGCARDINSTANCE)
        mFont->DrawString(((MTGCardInstance *)target)->getName(),x+120,y);
    }
  }
}


/* Put a card in graveyard */

PutInGraveyard::PutInGraveyard(int id, MTGCardInstance * _card):Interruptible(id){
  card = _card;
  removeFromGame = 0;
  type = ACTION_PUTINGRAVEYARD;
}

int PutInGraveyard::resolve(){
  GameObserver * g = GameObserver::GetInstance();
  MTGGameZone * zone = card->getCurrentZone();
  if (zone == g->players[0]->game->inPlay || zone == g->players[1]->game->inPlay){
    card->owner->game->putInZone(card,zone,card->owner->game->graveyard);
    return 1;
  }
  return 0;
}

void PutInGraveyard::Render(){
  JLBFont * mFont = GameApp::CommonRes->GetJLBFont(Constants::MAIN_FONT);
  mFont->SetBase(0);
  mFont->SetScale(DEFAULT_MAIN_FONT_SCALE);
  if (!removeFromGame){
    mFont->DrawString("goes to graveyard", x + 30 , y, JGETEXT_LEFT);
  }else{
    mFont->DrawString("is removed from game", x + 30 , y, JGETEXT_LEFT);
  }
  JRenderer * renderer = JRenderer::GetInstance();
  JQuad * quad = card->getThumb();
  if (quad){
    quad->SetColor(ARGB(255,255,255,255));
    float scale = 30 / quad->mHeight;
    renderer->RenderQuad(quad, x  , y , 0,scale,scale);
  }else{
    mFont->DrawString(card->getName(),x,y-15);
  }
}


/* Draw a Card */
DrawAction::DrawAction(int id, Player * _player, int _nbcards):Interruptible(id), nbcards(_nbcards), player(_player){
}

int DrawAction::resolve(){
  for (int i = 0 ; i < nbcards ; i++){
    player->game->drawFromLibrary();
  }
  return 1;
}

void DrawAction::Render(){
  JLBFont * mFont = GameApp::CommonRes->GetJLBFont(Constants::MAIN_FONT);
  mFont->SetBase(0);
  mFont->SetScale(DEFAULT_MAIN_FONT_SCALE);
  char buffer[200];
  int playerId = 1;
  if (player ==  GameObserver::GetInstance()->players[1]) playerId = 2;
  sprintf(buffer, "Player %i draws %i card", playerId, nbcards);
  mFont->DrawString(buffer, x + 20 , y, JGETEXT_LEFT);
}

/* The Action Stack itself */
int ActionStack::addPutInGraveyard(MTGCardInstance * card){
  PutInGraveyard * death = NEW PutInGraveyard(mCount,card);
  addAction(death);
  return 1;
}

int ActionStack::addAbility(MTGAbility * ability){
  StackAbility * stackAbility = NEW StackAbility(mCount,ability);
  addAction(stackAbility);
  return 1;
}

int ActionStack::addDraw(Player * player, int nb_cards){
  DrawAction * draw = NEW DrawAction(mCount,player, nb_cards);
  addAction(draw);
  return 1;
}

int ActionStack::addDamage(MTGCardInstance * _source, Damageable * _target, int _damage){
  Damage * damage = NEW Damage(mCount, _source, _target, _damage);
  addAction(damage);
  return 1;
}

int ActionStack::AddNextGamePhase(){
  if (getNext(NULL,NOT_RESOLVED)) return 0;
  NextGamePhase * next = NEW NextGamePhase(mCount);
  addAction(next);
  int playerId = 0;
  if (game->currentActionPlayer == game->players[1]) playerId = 1;
  interruptDecision[playerId] = 1;
  return 1;
}

int ActionStack::setIsInterrupting(Player * player){
  if (player == game->players[0]){
    interruptDecision[0] = -1;
  }else{
    interruptDecision[1] = -1;
  }
  game->isInterrupting = player;
  askIfWishesToInterrupt = NULL;
  return 1;
}


int ActionStack::addAction(Interruptible * action){
  for (int i=0; i<2; i++){
    interruptDecision[i] = 0;
  }
  Add(action);
  return 1;
}

int ActionStack::addSpell(MTGCardInstance * _source, Targetable * _targets[], int _nbtargets, ManaCost * mana){
#if defined (WIN32) || defined (LINUX)
  char    buf[4096], *p = buf;
  sprintf(buf, "Add spell\n");
  OutputDebugString(buf);
#endif
  Spell * spell = NEW Spell(mCount,_source,_targets,_nbtargets, mana);
  return addAction(spell);
}


Interruptible * ActionStack::_(int id){
  if (id < 0) id = mCount + id;
  if (id > mCount -1) return NULL;
  return (Interruptible *)mObjects[id];
}

ActionStack::ActionStack(int id, GameObserver* _game):GuiLayer(id, _game){
  for (int i=0; i<2; i++){
    interruptDecision[i] = 0;
  }
  askIfWishesToInterrupt = NULL;
  timer = -1;
  currentState = -1;
  mode = ACTIONSTACK_STANDARD;
  checked = 0;

}

int ActionStack::has(MTGAbility * ability){
  for (int i = 0; i < mCount ; i++){
    if (((Interruptible *)mObjects[i])->type==ACTION_ABILITY){
      StackAbility * action = ((StackAbility *)mObjects[i]);
      if (action->state == NOT_RESOLVED && action->ability == ability) return 1;
    }
  }
  return 0;
}

int ActionStack::has(Interruptible * action){
  for (int i = 0; i < mCount ; i++){
    if (mObjects[i] == action) return 1;
  }
  return 0;
}

int ActionStack::resolve(){
  Interruptible * action = getLatest(NOT_RESOLVED);

  if (!action)
    return 0;


  if (action->resolve()){
    action->state = RESOLVED_OK;
  }else{
    action->state = RESOLVED_NOK;
  }
  if (action->type == ACTION_DAMAGE) ((Damage * )action)->target->afterDamage();
  if (action->type == ACTION_DAMAGES){
    DamageStack * ds = (DamageStack *) action;
    for (int i = 0; i < ds->mCount; i++){
      Damage * damage = ((Damage *) ds->mObjects[i]);
      damage->state = ds->state;
    }
    unpackDamageStack(ds);
    ds->mCount = 0;
  }

  if (!getNext(NULL,NOT_RESOLVED)){
    for (int i = 0; i< 2 ; i++){
      interruptDecision[i] = 0;
    }
  }else{
    for (int i = 0; i< 2 ; i++){
      if (interruptDecision[i] != 2) interruptDecision[i] = 0;
    }
  }

  return 1;

}

Interruptible * ActionStack::getPrevious(Interruptible * next, int type, int state, int display){
  int n = getPreviousIndex( next,  type, state, display);
  if (n==-1) return NULL;
  return ((Interruptible *) mObjects[n]);
}

int ActionStack::getPreviousIndex(Interruptible * next, int type, int state, int display){
  int found = 0;
  if (!next) found = 1;
  for (int i = mCount -1; i >= 0 ; i--){
    Interruptible * current = (Interruptible *)mObjects[i];
    if (found && (type == 0 || current->type == type) && (state == 0 || current->state == state) && (display == -1 || current->display == display)){
      return i;
    }
    if (current == next) found = 1;
  }
  if (!found) return getPreviousIndex(NULL,type, state, display);
  return -1;
}

int ActionStack::count( int type, int state, int display){
  int result = 0;
  for (int i = 0; i < mCount ; i++){
    Interruptible * current = (Interruptible *)mObjects[i];
    if((type == 0 || current->type == type) && (state == 0 || current->state == state) && (display == -1 || current->display == display)){
      result++;
    }
  }
  return result;
}

Interruptible * ActionStack::getNext(Interruptible * previous, int type, int state, int display){
  int n = getNextIndex( previous,  type, state, display);
  if (n==-1) return NULL;
  return ((Interruptible *) mObjects[n]);
}

int ActionStack::getNextIndex(Interruptible * previous, int type, int state, int display){
  int found = 0;
  if (!previous) found = 1;
  for (int i = 0; i < mCount ; i++){
    Interruptible * current = (Interruptible *)mObjects[i];
    if (found && (type == 0 || current->type == type) && (state == 0 || current->state == state) && (display == -1 || current->display == display)){
      return i;
    }
    if (current == previous) found = 1;
  }
  if (!found) return getNextIndex(NULL,type, state, display);
  return -1;
}


Interruptible * ActionStack::getLatest(int state){
  for (int i = mCount-1; i >=0; i--){
    Interruptible * action = ((Interruptible *)mObjects[i]);
    if (action->state == state) return action;
  }
  return NULL;
}

void ActionStack::unpackDamageStack(DamageStack * ds){
  for (int j = 0; j < ds->mCount; j++){
    Damage * damage = ((Damage *)ds->mObjects[j]);
    Add(damage);
  }
}

void ActionStack::unpackDamageStacks(){
  for (int i = mCount-1; i >=0; i--){
    Interruptible * action = ((Interruptible *)mObjects[i]);
    if (action->type == ACTION_DAMAGES){
      DamageStack * ds = (DamageStack *) action;
      unpackDamageStack(ds);
    }
  }
}

void ActionStack::repackDamageStacks(){
  std::vector<JGuiObject *>::iterator iter = mObjects.begin() ;

  while( iter != mObjects.end() ){
    Interruptible * action = ((Interruptible *) *iter);
    int found = 0;
    if (action->type == ACTION_DAMAGE){
      Damage * damage = (Damage *) action;
      for (int j = 0; j < mCount; j++){
        Interruptible * action2 = ((Interruptible *)mObjects[j]);
        if (action2->type == ACTION_DAMAGES){
          DamageStack * ds = (DamageStack *) action2;
          for (int k = 0; k< ds->mCount; k++){
            Damage * dsdamage = ((Damage *)ds->mObjects[k]);
            if (dsdamage==damage){
              //Remove(damage);
              iter = mObjects.erase( iter ) ;
              found = 1;
              mCount--;
            }
          }
        }
      }
    }
    if (!found) ++iter;
  }

/*
  for (int i = mCount-1; i >=0; i--){
    Interruptible * action = ((Interruptible *)mObjects[i]);
    if (action->type == ACTION_DAMAGE){
      Damage * damage = (Damage *) action;
      for (int j = 0; j < mCount; j++){
	      Interruptible * action2 = ((Interruptible *)mObjects[j]);
	      if (action2->type == ACTION_DAMAGES){
	        DamageStack * ds = (DamageStack *) action2;
	        for (int k = 0; k< ds->mCount; k++){
	          Damage * dsdamage = ((Damage *)ds->mObjects[k]);
            if (dsdamage==damage){
              //Remove(damage);
              mObjects[i] = mObjects[mCount-1];
              mCount--;
            }
	        }
	      }
      }
    }
  }
*/
}

void ActionStack::Update(float dt){
  askIfWishesToInterrupt = NULL;
  //modal = 0;
  GameObserver * game = GameObserver::GetInstance();
  TargetChooser * tc = game->getCurrentTargetChooser();
  int newState = game->getCurrentGamePhase();
  currentState = newState;

  //Select Stack's display mode
  if (mode==ACTIONSTACK_STANDARD && tc && !checked){
    checked = 1;
    unpackDamageStacks();
    for (int i = 0; i < mCount ; i++){
      Interruptible * current = (Interruptible *)mObjects[i];
      if (tc->canTarget(current)){
	if (mObjects[mCurr]) mObjects[mCurr]->Leaving(PSP_CTRL_UP);
	current->display = 1;
	mCurr = i;
	mObjects[mCurr]->Entering();
	mode=ACTIONSTACK_TARGET;
	modal = 1;
      }else{
	current->display = 0;
      }
    }
    if (mode != ACTIONSTACK_TARGET){
      repackDamageStacks();
    }
  }else if (mode==ACTIONSTACK_TARGET && !tc){
    mode = ACTIONSTACK_STANDARD;
    checked = 0;
    repackDamageStacks();
  }

  if (mode == ACTIONSTACK_STANDARD){
    modal = 0;
    if (getLatest(NOT_RESOLVED)){
      int currentPlayerId = 0;
      int otherPlayerId = 1;
      if (game->currentPlayer != game->players[0]){
	currentPlayerId = 1;
	otherPlayerId = 0;
      }
      if (interruptDecision[currentPlayerId] == 0){
	askIfWishesToInterrupt = game->players[currentPlayerId];
	game->isInterrupting = game->players[currentPlayerId];
	modal = 1;
      }else if (interruptDecision[currentPlayerId] == -1){
	game->isInterrupting = game->players[currentPlayerId];

      }else{
	if (interruptDecision[otherPlayerId] == 0){
	  askIfWishesToInterrupt = game->players[otherPlayerId];
	  game->isInterrupting = game->players[otherPlayerId];
	  modal = 1;
	}else if (interruptDecision[otherPlayerId] == -1){
	  game->isInterrupting = game->players[otherPlayerId];
	}else{
	  resolve();
	}
      }
    }
  }else if (mode == ACTIONSTACK_TARGET){
    GuiLayer::Update(dt);
  }
  if (askIfWishesToInterrupt){
    // WALDORF - added code to use a game option setting to determine how
    // long the Interrupt timer should be. If it is set to zero (0), the
    // game will wait for ever for the user to make a selection.
    if (GameOptions::GetInstance()->values[OPTIONS_INTERRUPT_SECONDS].getIntValue() > 0)
    {
      if (timer < 0) timer = GameOptions::GetInstance()->values[OPTIONS_INTERRUPT_SECONDS].getIntValue();
      timer -= dt;
      if (timer < 0) cancelInterruptOffer();
    }
  }
}

void ActionStack::cancelInterruptOffer(int cancelMode){
  if (game->isInterrupting == game->players[0]){
    interruptDecision[0] = cancelMode;
  }else{
    interruptDecision[1] = cancelMode;
  }
  askIfWishesToInterrupt = NULL;
  game->isInterrupting = NULL;
  timer = -1;
}

void ActionStack::endOfInterruption(){
  if (game->isInterrupting == game->players[0]){
    interruptDecision[0] = 0;
  }else{
    interruptDecision[1] = 0;
  }
  game->isInterrupting = NULL;
}


bool ActionStack::CheckUserInput(u32 key){
  if (mode == ACTIONSTACK_STANDARD){
    if (askIfWishesToInterrupt){
      if (PSP_CTRL_CROSS == key){
	      setIsInterrupting(askIfWishesToInterrupt);
	      return true;
      }else if ((PSP_CTRL_CIRCLE == key) || (PSP_CTRL_RTRIGGER == key) ){
	      cancelInterruptOffer();
	      return true;
      }else if ((PSP_CTRL_SQUARE == key)){
	      cancelInterruptOffer(2);
	      return true;
      }
      return true;
    }else if (game->isInterrupting){
      if (PSP_CTRL_CROSS == key){
	      endOfInterruption();
	      return true;
      }
    }
  }else if (mode == ACTIONSTACK_TARGET){
    if (modal){
      if (PSP_CTRL_UP == key){
	      if( mObjects[mCurr]){
	        int n = getPreviousIndex(((Interruptible *) mObjects[mCurr]), 0, 0, 1);
	        if (n != -1 && n != mCurr && mObjects[mCurr]->Leaving(PSP_CTRL_UP)){
	          mCurr = n;
	          mObjects[mCurr]->Entering();
#if defined (WIN32) || defined (LINUX)
	          char buf[4096];
	          sprintf(buf, "Stack UP TO mCurr = %i\n", mCurr);
	          OutputDebugString(buf);
#endif
	        }
	      }
	      return true;
      }else if (PSP_CTRL_DOWN == key){
	      if( mObjects[mCurr]){
	        int n = getNextIndex(((Interruptible *) mObjects[mCurr]), 0, 0, 1);
	        if (n!= -1 && n != mCurr && mObjects[mCurr]->Leaving(PSP_CTRL_DOWN)){
	          mCurr = n;
	          mObjects[mCurr]->Entering();
#if defined (WIN32) || defined (LINUX)
	          char buf[4096];
	          sprintf(buf, "Stack DOWN TO mCurr = %i\n", mCurr);
	          OutputDebugString(buf);
#endif
	        }
	      }
	      return true;
      }else if (PSP_CTRL_CIRCLE == key){
#if defined (WIN32) || defined (LINUX)
	      char buf[4096];
	      sprintf(buf, "Stack CLIKED mCurr = %i\n", mCurr);
	      OutputDebugString(buf);
#endif
	      game->stackObjectClicked(((Interruptible *) mObjects[mCurr]));
	      return true;
      }
      return true; //Steal the input to other layers if we're visible
    }
    if (PSP_CTRL_TRIANGLE == key){
      if (modal) {modal = 0;} else {modal = 1;}
      return true;
    }
  }
  return false;
}




int ActionStack::CombatDamages(){
  CombatDamages(1);
  CombatDamages(0);
  return 1;
}

int ActionStack::CombatDamages(int strike){
  DamageStack * damages = NEW DamageStack(mCount,game);
  int damageitems = damages->CombatDamages(strike);
  if (damageitems){
    addAction(damages);
  }else{
    SAFE_DELETE(damages);
  }
  return damageitems;
}

//Cleans history of last turn
int ActionStack::garbageCollect(){
std::vector<JGuiObject *>::iterator iter = mObjects.begin() ;

while( iter != mObjects.end() ){
  Interruptible * current = ((Interruptible *) *iter);
  if (current->state != NOT_RESOLVED){
    iter = mObjects.erase( iter ) ;
    mCount--;
    SAFE_DELETE(current);
  }else {
    ++iter ;
  }
}
  return 1;
}

void ActionStack::Fizzle(Interruptible * action){
  if (action->type == ACTION_SPELL){
    Spell * spell = (Spell *) action;
    spell->source->controller()->game->putInGraveyard(spell->source);
  }
  action->state = RESOLVED_NOK;
}

void ActionStack::Render(){
  int x0 = 250;
  int y0 = 30;
  int width = 200;
  int height = 90;
  int currenty = y0 + 5 ;

  if (mode == ACTIONSTACK_STANDARD){
    if (!askIfWishesToInterrupt || !askIfWishesToInterrupt->displayStack()) return;


    for (int i=0;i<mCount ;i++){
      Interruptible * current = (Interruptible *)mObjects[i];
      if (current->state==NOT_RESOLVED) height += current->mHeight;
    }

    JLBFont * mFont = GameApp::CommonRes->GetJLBFont(Constants::MAIN_FONT);
    mFont->SetBase(0);
    mFont->SetScale(DEFAULT_MAIN_FONT_SCALE);
    mFont->SetColor(ARGB(255,255,255,255));
    JRenderer * renderer = JRenderer::GetInstance();

    //JQuad * back = GameApp::CommonRes->GetQuad("interrupt");
    //float xScale = width / back->mWidth;
    //float yScale = height / back->mHeight;
    renderer->FillRoundRect(x0 + 16 ,y0 + 16 ,width +2 ,height +2  , 10, ARGB(128,0,0,0));
    renderer->FillRoundRect(x0 - 5 ,y0 - 5 ,width + 2,height +2  , 10, ARGB(200,0,0,0));
    //renderer->RenderQuad(back,x0,y0,0,xScale, yScale);
    renderer->DrawRoundRect(x0 - 5 ,y0 - 5 ,width + 2,height +2  , 10, ARGB(255,255,255,255));


    for (int i=0;i<mCount ;i++){
      Interruptible * current = (Interruptible *)mObjects[i];
      if (current && current->state==NOT_RESOLVED){
	      current->x = x0 + 5;
	      if (i != mCount -1){
	        current->y = currenty;
	        currenty += current->mHeight;
	      }else{
	        current->y = currenty + 40 ;
	        currenty += current->mHeight + 40;
	      }
	      current->Render();
      }
    }

    char buffer[200];
    // WALDORF - changed "interrupt ?" to "Interrupt?". Don't display count down
    // seconds if the user disables auto progressing interrupts by setting the seconds
    // value to zero in Options.
    if (GameOptions::GetInstance()->values[OPTIONS_INTERRUPT_SECONDS].getIntValue() == 0)
        sprintf(buffer, "Interrupt?");
    else
        sprintf(buffer, "Interrupt?  %i", static_cast<int>(timer));

    //WALDORF - removed all the unnecessary math. just display the prompt at the
    // top of the box.
    //mFont->DrawString(buffer, x0 + 5 , currenty - 40 - ((Interruptible *)mObjects[mCount-1])->mHeight);
    mFont->DrawString(buffer, x0 + 5, y0);


    if (mCount > 1) sprintf(buffer, "X Interrupt - 0 No - [] No to All");
    else            sprintf(buffer, "X Interrupt - 0 No");

    // WALDORF - puts the button legend right under the prompt. the stack
    // will be displayed below it now. no more need to do wierd currY math.
    //mFont->DrawString(buffer, x0 + 5 , currenty);
     mFont->DrawString(buffer, x0 + 5, y0 + 14);
  }else if (mode == ACTIONSTACK_TARGET && modal){
    for (int i=0;i<mCount ;i++){
      Interruptible * current = (Interruptible *)mObjects[i];
      if (current->display) height += current->mHeight;
    }

    JLBFont * mFont = GameApp::CommonRes->GetJLBFont(Constants::MAIN_FONT);
    mFont->SetBase(0);
    mFont->SetScale(DEFAULT_MAIN_FONT_SCALE);

    JRenderer * renderer = JRenderer::GetInstance();
    renderer->FillRect(x0 ,y0 , width ,height , ARGB(200,0,0,0));
    renderer->DrawRect(x0 - 1 ,y0 - 1 ,width + 2 ,height +2  , ARGB(255,255,255,255));


    for (int i=0;i<mCount ;i++){
      Interruptible * current = (Interruptible *)mObjects[i];
      if (mObjects[i]!=NULL && current->display){
	      ((Interruptible *)mObjects[i])->x = x0 + 5;
	      if (i != mCount -1){
	        ((Interruptible *)mObjects[i])->y = currenty;
	        currenty += ((Interruptible *)mObjects[i])->mHeight;
	      }else{
	        ((Interruptible *)mObjects[i])->y = currenty + 40 ;
	        currenty += ((Interruptible *)mObjects[i])->mHeight + 40;
	      }
	      mObjects[i]->Render();
      }
    }
  }
}

#if defined (WIN32) || defined (LINUX)

void Interruptible::Dump(){
  string stype, sstate, sdisplay = "";
  switch (type){
    case ACTION_SPELL:
      stype = "spell";
      break;
    case ACTION_DAMAGE:
      stype = "damage";
      break;
    case ACTION_DAMAGES:
      stype = "damages";
      break;
    case ACTION_NEXTGAMEPHASE:
      stype = "next phase";
      break;
    case ACTION_DRAW:
      stype = "draw";
      break;
    case ACTION_PUTINGRAVEYARD:
      stype = "put in graveyard";
      break;
    case ACTION_ABILITY:
      stype = "ability";
      break;
    default:
      stype = "unknown";
      break;
  }

  switch(state){
    case NOT_RESOLVED:
      sstate = "not resolved";
      break;
    case RESOLVED_OK:
      sstate = "resolved";
      break;
    case RESOLVED_NOK:
      sstate = "fizzled";
      break;
    default:
      sstate = "unknown";
      break;
  }
  
  char buf[4096];
  sprintf(buf, "    type %s(%i) - state %s(%i) - display %i\n", stype.c_str(), type,  sstate.c_str(),state, display);
    OutputDebugString(buf);
}

void ActionStack::Dump(){
   OutputDebugString("=====\nDumping Action Stack=====\n");
  for (int i=0;i<mCount ;i++){
    Interruptible * current = (Interruptible *)mObjects[i];
    current->Dump();
  }
}


#endif
