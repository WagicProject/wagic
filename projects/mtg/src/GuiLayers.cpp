#include "../include/config.h"
#include "../include/GuiLayers.h"
#include "../include/Player.h"

GuiLayer::GuiLayer(){
  modal = 0;
  hasFocus = false;
  mCount = 0;
  mCurr = 0;
  mActionButton = JGE_BTN_OK;
}

GuiLayer::~GuiLayer(){
  resetObjects();
}

void GuiLayer::Add(JGuiObject *object){
  mObjects.push_back(object);
  mCount++;
}

int GuiLayer::Remove(JGuiObject *object){
  for (int i=0;i<mCount;i++)
    if (mObjects[i]==object){
      delete mObjects[i];
      mObjects.erase(mObjects.begin()+i);
      mCount--;
      if (mCurr == mCount)
        mCurr = 0;
      return 1;
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

