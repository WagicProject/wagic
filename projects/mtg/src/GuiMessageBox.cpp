#include "GuiMessageBox.h"


void GuiMessageBox::CheckUserInput(){
  if (mEngine->GetButtonClick(mActionButton))
    {
      if (mObjects[mCurr] != NULL && mObjects[mCurr]->ButtonPressed())
	{
	  if (mListener != NULL)
	    {
	      mListener->ButtonPressed(mId, mObjects[mCurr]->GetId());
	      return;
	    }
	}
    }

  if (mEngine->GetButtonState(PSP_CTRL_LEFT) || mEngine->GetButtonState(PSP_CTRL_UP) || mEngine->GetAnalogY()<64)
    {
      if (KeyRepeated(PSP_CTRL_UP, dt))
	{
	  int n = mCurr;
	  n--;
	  if (n<0)
	    {
	      if ((mStyle&JGUI_STYLE_WRAPPING))
		n = mCount-1;
	      else
		n = 0;
	    }

	  if (n != mCurr && mObjects[mCurr] != NULL && mObjects[mCurr]->Leaving(PSP_CTRL_UP))
	    {
	      mCurr = n;
	      mObjects[mCurr]->Entering();
	    }
	}
    }
  else if (mEngine->GetButtonState(PSP_CTRL_RIGHT) || mEngine->GetButtonState(PSP_CTRL_DOWN) || mEngine->GetAnalogY()>192)
    {
      if (KeyRepeated(PSP_CTRL_DOWN, dt))
	{
	  int n = mCurr;
	  n++;
	  if (n>mCount-1)
	    {
	      if ((mStyle&JGUI_STYLE_WRAPPING))
		n = 0;
	      else
		n = mCount-1;
	    }

	  if (n != mCurr && mObjects[mCurr] != NULL && mObjects[mCurr]->Leaving(PSP_CTRL_DOWN))
	    {
	      mCurr = n;
	      mObjects[mCurr]->Entering();
	    }
	}
    }
  else
    mLastKey = 0;
}
