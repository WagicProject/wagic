#include "../include/config.h"
#include "../include/GuiLayers.h"
#include "../include/Player.h"

GuiLayer::GuiLayer(int id, GameObserver* _game):JGuiController(id, NULL){
  game = _game;
  modal = 0;
  hasFocus = false;
}

GuiLayer::~GuiLayer(){
  //TODO
}

int GuiLayer::getMaxId(){
  return mCount;
}

void GuiLayer::Update(float dt){
  for (int i=0;i<mCount;i++)
    if (mObjects[i]!=NULL)
      mObjects[i]->Update(dt);
}


void GuiLayer::resetObjects(){
  for (int i=0;i<mCount;i++)
    if (mObjects[i])
      delete mObjects[i];

  mCount = 0;
  mCurr = 0;
}

void GuiLayer::RenderMessageBackground(float x0, float y0, float width, int height){
  PIXEL_TYPE colors_up[] =
    {
      ARGB(0,255,255,255),
      ARGB(0,255,255,255),
      ARGB(128,255,255,255),
      ARGB(128,255,255,255)
    };

  PIXEL_TYPE colors_down[] =
    {
      ARGB(128,255,255,255),
      ARGB(128,255,255,255),
      ARGB(0,255,255,255),
      ARGB(0,255,255,255)
    };

  JRenderer * renderer = JRenderer::GetInstance();
  renderer->FillRect(x0,y0,width,height/2,colors_up);
  renderer->FillRect(x0,y0+height/2,width,height/2,colors_down);

  //  mEngine->DrawLine(0,y0,SCREEN_WIDTH,y0,ARGB(128,255,255,255));
  //  mEngine->DrawLine(0,y0+height,SCREEN_WIDTH,y0+height,ARGB(128,255,255,255));
}

void GuiLayer::RenderMessageBackground(float y0, int height){
  RenderMessageBackground(0,y0,SCREEN_WIDTH, height);

}

int GuiLayer::getIndexOf(JGuiObject * object){
  for (int i=0; i<mCount; i++){
    if (mObjects[i] == object)
      return i;
  }
  return -1;
}

JGuiObject * GuiLayer::getByIndex(int index){
  return mObjects[index];
}


GuiLayers::GuiLayers(){
  nbitems = 0;
}

GuiLayers::~GuiLayers(){
  LOG("==Destroying GuiLayers==");
  for (int i=0; i<nbitems; i++){
    delete objects[i];
  }
  LOG("==Destroying GuiLayers Successful==");
}
int GuiLayers::unstopableRenderInProgress(){
  for (int i=0; i<nbitems; i++){
    if (objects[i]->unstopableRenderInProgress())
      return 1;
  }
  return 0;
}



void GuiLayers::Add(GuiLayer * layer){
  if (nbitems >=MAX_GUI_LAYERS || nbitems < 0){
    LOG("OUT OF BOUND IN GuiLayers Add !!!");
    return;
  }
  objects[nbitems] = layer;
  nbitems++;
}

void GuiLayers::Remove(){
  nbitems --;
}

void GuiLayers::Update(float dt, Player * currentPlayer){

  for (int i=0; i<nbitems; i++){
    objects[i]->Update(dt);
  }
  int isAI = currentPlayer->isAI();
  u32 key;
  while ((key = JGE::GetInstance()->ReadButton())){
    GameObserver * game = GameObserver::GetInstance();
    if (game->waitForExtraPayment && key == PSP_CTRL_CROSS){
      game->waitForExtraPayment = NULL;
      continue;
    }
    for (int i=0; i<nbitems; i++){
	    if (!isAI){
	      if (0 != key)
	        if (objects[i]->CheckUserInput(key)) break;
	     }
    }
  }
  if (isAI) currentPlayer->Act(dt);

}

void GuiLayers::Render(){
  bool focusMakesItThrough = true;
  for (int i = 0; i < nbitems; ++i)
    {
      objects[i]->hasFocus = focusMakesItThrough;
      if (objects[i]->modal) focusMakesItThrough = false;
    }
  for (int i=nbitems-1; i>=0; i--){
    objects[i]->Render();
  }
}
