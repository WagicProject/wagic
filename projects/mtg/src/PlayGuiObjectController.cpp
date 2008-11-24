#include "../include/debug.h"
#include "../include/PlayGuiObjectController.h"

#include "../include/PlayGuiObject.h"
#include "../include/GameObserver.h"

int PlayGuiObjectController::showBigCards = 1;

int PlayGuiObjectController::getClosestItem(int direction){
  return getClosestItem(direction, 35);
}


int PlayGuiObjectController::getClosestItem(int direction, float tolerance){
  if (mCount == 0){
    return -1;
  }
  if (mCount == 1){
    return mCurr;
  }

  float maxDist = SCREEN_WIDTH * SCREEN_WIDTH;
  PlayGuiObject * current = (PlayGuiObject *)mObjects[mCurr];
  int closest_match = -1;
  int available = 0;
  float x0, y0, x1, y1;
  x0 = current->x;
  y0 = current->y;
  for (int i=0;i<mCount;i++){
    if (i == mCurr) continue;
    PlayGuiObject * other = (PlayGuiObject *) mObjects[i];
    x1 = other->x;
    y1 = other->y;
    float dist = (x0-x1)*(x0-x1) + (y0-y1)*(y0-y1);
    if (dist>=maxDist) continue;
    //Potential match !
    int ok = 0;
    switch(direction){
    case DIR_DOWN:
      if (y1 > y0){
	available = 1;
	if (fabs(x0-x1) < tolerance ) ok = 1;
      }
      break;
    case  DIR_UP:
      if (y1 < y0){
	available = 1;
	if (fabs(x0-x1) < tolerance ) ok = 1;
      }
      break;
    case DIR_LEFT:
      if (x1 < x0){
	available = 1;
	if (fabs(y0-y1) < tolerance ) ok = 1;
      }
      break;
    case DIR_RIGHT:
      if (x1 > x0){
	available = 1;
	if (fabs(y0-y1) < tolerance ) ok = 1;
      }
      break;
    }
    if (ok){
      closest_match = i;
      maxDist = dist;
    }
  }
  if (closest_match == -1){
    if (available) return getClosestItem(direction,tolerance+5);
    return mCurr;
  }
  return closest_match;
}


/*
  int PlayGuiObjectController::getClosestItem(int direction, float tolerance){
  if (mCount == 0){
  return -1;
  }
  if (mCount == 1){
  return mCurr;
  }

  float MaxTolerance = SCREEN_HEIGHT;
  PlayGuiObject * current = (PlayGuiObject *)mObjects[mCurr];
  int found = 0;
  int closest_match_id = -1;
  for (int i=0;i<mCount;i++){
  fprintf(stderr, "distance STEP 3-%i\n", i);
  if (i != mCurr){ //Don't wanna return the same object as currently selected
  if (closest_match_id == -1){
  closest_match_id = i;
  }
  if (mObjects[i]!=NULL){
  float x0, y0, x1, y1,closest;
  PlayGuiObject * closest_match = (PlayGuiObject *)mObjects[closest_match_id];
  PlayGuiObject * other = (PlayGuiObject *) mObjects[i];
  fprintf(stderr, "distance STEP 4-%i\n", i);
  switch(direction){
  case DIR_DOWN:
  x0 = current->x;
  y0 = current->y;
  x1 = other->x;
  y1 = other->y;
  closest = closest_match->y - y0;
  break;
  case DIR_UP:
  x0 = current->x;
  y0 = other->y;
  x1 = other->x;
  y1 = current->y;
  closest = y1 - closest_match->y;
  break;
  case DIR_LEFT:
  MaxTolerance = SCREEN_WIDTH;
  x0 = current->y;
  y1 = current->x;
  x1 = other->y;
  y0 = other->x;
  closest =  y1 - closest_match->x;
  break;
  case DIR_RIGHT:
  MaxTolerance = SCREEN_WIDTH;
  x0 = current->y;
  fprintf(stderr, "distance STEP 401\n");

  y0 = current->x;
  fprintf(stderr, "distance STEP 402\n");
  x1 = other->y;
  fprintf(stderr, "distance STEP 403\n");
  y1 = other->x;
  fprintf(stderr, "distance STEP 404\n");
  closest = closest_match->x - y0;
  fprintf(stderr, "distance STEP 405\n");
  break;
  }
  fprintf(stderr, "distance STEP 5\n");
  float distance = y1-y0;
  float lateral_distance = fabs(x1-x0);
  fprintf(stderr, "distance STEP 6 \n");
  if (lateral_distance < tolerance){
  fprintf(stderr, "distance STEP 7\n");
  if (distance > 0 && (!found || (distance < closest && closest > 0 ))){

  found = 1;
  closest_match_id = i;
  fprintf(stderr, "distance STEP 8\n");
  }
  }
  }//      if (mObjects[i]!=NULL)
  }
  }
  if (!found){
  fprintf(stderr, "NOT FOUND !\n");
  if (tolerance < MaxTolerance){
  fprintf(stderr, "distance STEP 9\n");
  return getClosestItem(direction, tolerance + 5);
  }else{
  fprintf(stderr, "Closest Match ID:  %i\n", mCurr);
  return mCurr;
  }
  }
  fprintf(stderr, "Closest Match ID:  %i\n", closest_match_id);
  return closest_match_id;

  }
*/
void PlayGuiObjectController::Update(float dt){
  last_user_move +=dt;
  for (int i=0;i<mCount;i++){
    if (mObjects[i]!=NULL){
      mObjects[i]->Update(dt);
    }
  }
}


bool PlayGuiObjectController::CheckUserInput(u32 key){
  if (!mCount)
    return false;
  if (game != NULL){
    if (mActionButton == key){
      if (mObjects[mCurr] != NULL && mObjects[mCurr]->ButtonPressed()){
	game->ButtonPressed(mId, (PlayGuiObject *)mObjects[mCurr]);
	return true;
      }
    }
    if (PSP_CTRL_CROSS == key){
      game->cancelCurrentAction();
      return true;
    }
  }

  last_user_move = 0;
  switch (key)
    {
    case PSP_CTRL_LEFT:
      {
	int n = getClosestItem(DIR_LEFT);
	if (n != mCurr && mObjects[mCurr] != NULL && mObjects[mCurr]->Leaving(PSP_CTRL_LEFT))
	  {
	    mCurr = n;
	    mObjects[mCurr]->Entering();
	  }
	return true;
      }
    case PSP_CTRL_RIGHT:
      {
	int n = getClosestItem(DIR_RIGHT);
	if (n != mCurr && mObjects[mCurr] != NULL && mObjects[mCurr]->Leaving(PSP_CTRL_RIGHT))
	  {
	    mCurr = n;
	    mObjects[mCurr]->Entering();
	  }
	return true;
      }
    case PSP_CTRL_UP:
      {
	int n = getClosestItem(DIR_UP);
	if (n != mCurr && mObjects[mCurr] != NULL && mObjects[mCurr]->Leaving(PSP_CTRL_UP))
	  {
	    mCurr = n;
	    mObjects[mCurr]->Entering();
	  }
	return true;
      }
    case PSP_CTRL_DOWN:
      {
	int n = getClosestItem(DIR_DOWN);
	if (n != mCurr && mObjects[mCurr] != NULL && mObjects[mCurr]->Leaving(PSP_CTRL_DOWN))
	  {
	    mCurr = n;
	    mObjects[mCurr]->Entering();
	  }
	return true;
      }
    case PSP_CTRL_TRIANGLE:
      showBigCards = (showBigCards + 1) % 3;
      return true;
    }
  return false;
}
