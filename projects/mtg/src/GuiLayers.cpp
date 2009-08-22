#include "../include/config.h"
#include "../include/GuiLayers.h"
#include "../include/Player.h"

GuiLayer::GuiLayer(){
  modal = 0;
  hasFocus = false;
  mCount = 0;
  mCurr = 0;
  mActionButton = PSP_CTRL_CIRCLE;
}

GuiLayer::~GuiLayer(){
  resetObjects();
}

void GuiLayer::Add(JGuiObject *object){
  mObjects.push_back(object);
  mCount++;
}

int GuiLayer::Remove(JGuiObject *object){
  for (int i=0;i<mCount;i++){
    if (mObjects[i]==object){
      delete mObjects[i];
      mObjects.erase(mObjects.begin()+i);
      mCount--;
      if (mCurr == mCount)
	      mCurr = 0;
      return 1;
    }
  }
  return 0;
}

int GuiLayer::getMaxId(){
  return mCount;
}

void GuiLayer::Render(){
 for (int i=0;i<mCount;i++)
    if (mObjects[i]!=NULL)
      mObjects[i]->Render();
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
  mObjects.clear();
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

