#include "GuiMessageBox.h"

bool GuiMessageBox::CheckUserInput(JButton key)
{
    if (mActionButton == key)
    {
        if (mObjects[mCurr] != NULL && mObjects[mCurr]->ButtonPressed())
        {
            if (mListener != NULL)
            {
                mListener->ButtonPressed(mId, mObjects[mCurr]->GetId());
                return true;
            }
        }
    }

    if ((PSP_CTRL_LEFT == key) || (PSP_CTRL_UP == key)) // || mEngine->GetAnalogY()<64)
    {
        int n = mCurr;
        n--;
        if (n < 0)
        {
            if ((mStyle & JGUI_STYLE_WRAPPING))
                n = mCount - 1;
            else
                n = 0;
        }

        if (n != mCurr && mObjects[mCurr] != NULL && mObjects[mCurr]->Leaving(PSP_CTRL_UP))
        {
            mCurr = n;
            mObjects[mCurr]->Entering();
        }
        return true;
    }
    else if ((PSP_CTRL_RIGHT == key) || (PSP_CTRL_DOWN == key)) // || mEngine->GetAnalogY()>192)
    {
        int n = mCurr;
        n++;
        if (n > mCount - 1)
        {
            if ((mStyle & JGUI_STYLE_WRAPPING))
                n = 0;
            else
                n = mCount - 1;
        }

        if (n != mCurr && mObjects[mCurr] != NULL && mObjects[mCurr]->Leaving(PSP_CTRL_DOWN))
        {
            mCurr = n;
            mObjects[mCurr]->Entering();
        }
        return true;
    }
    return false;
}
