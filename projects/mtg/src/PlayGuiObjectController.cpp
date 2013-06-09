#include "PrecompiledHeader.h"

#include "PlayGuiObjectController.h"

#include "PlayGuiObject.h"
#include "GameObserver.h"

int PlayGuiObjectController::showBigCards = 1;

int PlayGuiObjectController::getClosestItem(int direction)
{
    return getClosestItem(direction, 35);
}

int PlayGuiObjectController::getClosestItem(int direction, float tolerance)
{
    if (mObjects.size() == 0)
    {
        return -1;
    }
    if (mObjects.size() == 1)
    {
        return mCurr;
    }

    float maxDist = SCREEN_WIDTH * SCREEN_WIDTH;
    PlayGuiObject * current = (PlayGuiObject *) mObjects[mCurr];
    int closest_match = -1;
    int available = 0;
    float x0, y0, x1, y1;
    x0 = current->x;
    y0 = current->y;
    for (size_t i = 0; i < mObjects.size(); i++)
    {
        if ((int) i == mCurr) continue;
        PlayGuiObject * other = (PlayGuiObject *) mObjects[i];
        x1 = other->x;
        y1 = other->y;
        float dist = (x0 - x1) * (x0 - x1) + (y0 - y1) * (y0 - y1);
        if (dist >= maxDist) continue;
        //Potential match !
        int ok = 0;
        switch (direction)
        {
        case DIR_DOWN:
            if (y1 > y0)
            {
                available = 1;
                if (fabs(x0 - x1) < tolerance) ok = 1;
            }
            break;
        case DIR_UP:
            if (y1 < y0)
            {
                available = 1;
                if (fabs(x0 - x1) < tolerance) ok = 1;
            }
            break;
        case DIR_LEFT:
            if (x1 < x0)
            {
                available = 1;
                if (fabs(y0 - y1) < tolerance) ok = 1;
            }
            break;
        case DIR_RIGHT:
            if (x1 > x0)
            {
                available = 1;
                if (fabs(y0 - y1) < tolerance) ok = 1;
            }
            break;
        }
        if (ok)
        {
            closest_match = i;
            maxDist = dist;
        }
    }
    if (closest_match == -1)
    {
        if (available) return getClosestItem(direction, tolerance + 5);
        return mCurr;
    }
    return closest_match;
}

void PlayGuiObjectController::Update(float dt)
{
    last_user_move += dt;
    for (size_t i = 0; i < mObjects.size(); i++)
    {
        if (mObjects[i] != NULL)
        {
            mObjects[i]->Update(dt);
        }
    }
}

bool PlayGuiObjectController::CheckUserInput(JButton)
{
    /*
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
     */
    return false;
}
