#include <stdio.h>
#include "../include/JGE.h"
#include "../include/JRenderer.h"
#include "../include/JLBFont.h"
#include "../include/JGBKFont.h"
#include "../include/JInputSystem.h"

char input_table[3][9][4]={
	{{',','a','b','c'},{'.','d','e','f'},{'!','g','h','i'},
	{'-','j','k','l'},{' ','m',' ','n'},{'?','o','p','q'},
	{'(','r','s','t'},{':','u','v','w'},{')','x','y','z'}},

	{{'^','A','B','C'},{'@','D','E','F'},{'*','G','H','I'},
	{'_','J','K','L'},{' ','M',' ','N'},{'"','O','P','Q'},
	{'=','R','S','T'},{';','U','V','W'},{'/','X','Y','Z'}},

	{{'=','+','0','-'},{'^','*','1','/'},{'|','(','2',')'},
	{'%','[','3',']'},{' ','4',' ','5'},{'&','{','6','}'},
	{'!','<','7','>'},{'$','#','8','~'},{':','\'','9','"'}}};

JInputSystem* JInputSystem::m_pJInputSystem=NULL;
JInputSystem * JInputSystem::GetInstance()
{
	if(m_pJInputSystem==NULL)
		m_pJInputSystem = new JInputSystem();
	return m_pJInputSystem;
}
void JInputSystem::Destory()
{
	if (m_pJInputSystem!=NULL)
	{
		delete m_pJInputSystem;
	}
	m_pJInputSystem = NULL;
}
JInputSystem::JInputSystem(void)
{
	//SetInputActive(false);
	mIsInputActive=false;
	mBitmapFont12 = NULL;
	mBitmapFont12 = new JGBKFont();
	mBitmapFont12->Init("ASC12", "GBK12", 12,true);

	mTimer=0.0f;
	mInPut[0]=0;
	mpInput=NULL;
	mPY[0]=0;
	mStatus = eInputEng;

	mEnablePYSel=false;
	mPYShowFirstIndex=0;
	mPYSelIndex=0;
	mPYSelTableSize=1;

	mHZShowFirstIndex=0;
	mHZSelIndex=0;
	mHZSelTableSize=6;
	mIsHZ_H=true;
	mHZ=NULL;
}

JInputSystem::~JInputSystem(void)
{
	if(mBitmapFont12)
		delete mBitmapFont12;
}



//////////////////////////////////////////////////////////////////////////
/// Update:
//////////////////////////////////////////////////////////////////////////
void JInputSystem::Update()
{
	if (JGE::GetInstance()->GetButtonState(PSP_CTRL_DOWN) && JGE::GetInstance()->GetButtonClick(PSP_CTRL_CROSS))		// exit when the CROSS is pressed
	{
		JInputSystem::GetInstance()->DisableInputMode();
		return;
	}

	switch(mStatus)
	{
	case eInputEng:
		UpdateInputEng();
		break;
	case eInputChi:
		UpdateInputChi();
		break;
	case eSelPY:
		UpdateSelPY();
		break;
	case eSelHZ:
		mIsHZ_H?UpdateSelHZ_H():UpdateSelHZ();
		//UpdateSelHZ_H();
		break;
	case eInputNum:
		UpdateInputNum();
		break;
	default:
		break;
	}

	// switch the input type.
	if (JGE::GetInstance()->GetButtonClick(PSP_CTRL_RTRIGGER))
	{	
		switch(mStatus)
		{
		case eInputEng:
			mStatus = eInputChi;
			break;
		case eInputChi:
		case eSelPY:
		case eSelHZ:
			mStatus = eInputNum;
			mPY[0]=0;
			break;
		case eInputNum:
			mStatus = eInputEng;
			break;
		default:
			break;
		}
	}
	// Update Timer
	mTimer += JGE::GetInstance()->GetDelta();
	if (mTimer > 400 || mTimer < 0)
	{
		mTimer = 0;
	}
}

void JInputSystem::UpdateInputEng()
{
	char* pBuf = mpInput;
	int a,b,c;
	if (GetInputKey(a,b,c))
	{
		if(b==4 && c==0)
		{
			int len = strlen(pBuf);
			if(len>0)
			{
				if(pBuf[len-1]>0)
				{
					pBuf[len-1]=0;
				}
				else
				{
					if(len>=2)
					{
						if(pBuf[len-2]<0)
						{
							pBuf[len-2]=0;
						}
					}
				}
			}
		}
		else
		{
			char cc[2];
			cc[0]=input_table[a][b][c];
			cc[1]=0;
			strcat(pBuf,(cc));
		}
	}
}

void JInputSystem::UpdateInputChi()
{
	if(mStatus != eInputChi)
		return;

	int a,b,c;
	if (GetInputKey(a,b,c))
	{
		if(b==4 && c==0)
		{// press Backspace.
			int len = strlen(mPY);
			if(len>0)
			{
				mPY[len-1]=0;
			}
			else
			{
				len = strlen(mpInput);
				if(len>0)
				{
					if(mpInput[len-1]>0)
					{
						mpInput[len-1]=0;
					}
					else
					{
						if(len>=2)
						{
							if(mpInput[len-2]<0)
							{
								mpInput[len-2]=0;
							}
						}
					}
				}
			}
		}
		else if(b==4 && c==2 && mPY[0]!=0){//press Space
			mStatus = mEnablePYSel?eSelPY:eSelHZ;
			//mStatus = eSelPY;
		}
		else if(a==0 && c!=0 && !(b==4 && c==2))
		{// Input PY
			char cc[2];
			cc[0]=input_table[a][b][c];
			cc[1]=0;
			strcat(mPY,(cc));
		}
		else
		{// Input String
			char cc[2];
			cc[0]=input_table[a][b][c];
			cc[1]=0;
			strcat(mpInput,(cc));
		}
	}
	else
	{
		if((JGE::GetInstance()->GetButtonClick(PSP_CTRL_RIGHT) || JGE::GetInstance()->GetButtonClick(PSP_CTRL_DOWN)) && mPY[0]!=0)
			mStatus = mEnablePYSel?eSelPY:eSelHZ;
			//mStatus = eSelPY;
	}
}

void JInputSystem::UpdateSelPY()
{
	if(JGE::GetInstance()->GetButtonClick(PSP_CTRL_LEFT) || JGE::GetInstance()->GetButtonClick(PSP_CTRL_CROSS))
	{
		mStatus = eInputChi;
		mPYSelIndex = 0;
		mPYShowFirstIndex=0;
	}
	else if(JGE::GetInstance()->GetButtonClick(PSP_CTRL_UP))
	{
		mPYSelIndex--;
		if (mPYSelIndex<0)
			mPYSelIndex = 0;
	}
	else if(JGE::GetInstance()->GetButtonClick(PSP_CTRL_DOWN))
	{
		PY_index* pyindex=NULL;
		int len = GetNexPYIndex(mPY,pyindex);
		if(len>0 && mPYSelIndex < len-1 )
			mPYSelIndex++;
	}
	else if(JGE::GetInstance()->GetButtonClick(PSP_CTRL_RIGHT) || JGE::GetInstance()->GetButtonClick(PSP_CTRL_CIRCLE))
		mStatus = eSelHZ;
}

void JInputSystem::UpdateSelHZ()
{
	PY_index* pyindex=NULL;
	int totalLen = 0;
	char* str=NULL;
	int len = GetNexPYIndex(mPY,pyindex);
	if(len>0 && mPYSelIndex < len)
	{
		str = pyindex[mPYSelIndex].PY_mb;
		totalLen=strlen(str)/2;
	}
	else
		return;

	if(JGE::GetInstance()->GetButtonClick(PSP_CTRL_CROSS))
	{
		mStatus = eInputChi;
		mPYSelIndex = 0;
		mPYShowFirstIndex=0;
		mHZSelIndex = 0;
		mHZShowFirstIndex=0;
		mPY[0]=0;
	}
	else if(JGE::GetInstance()->GetButtonClick(PSP_CTRL_TRIANGLE))
	{
		mStatus = mEnablePYSel?eSelPY:eInputChi;
		//mStatus = eSelPY;
		mHZSelIndex = 0;
		mHZShowFirstIndex=0;
	}
	else if(JGE::GetInstance()->GetButtonClick(PSP_CTRL_CIRCLE))
	{
		int inlen = strlen(mpInput);
		mpInput[inlen]=str[mHZSelIndex*2];
		mpInput[inlen+1]=str[mHZSelIndex*2+1];
		mpInput[inlen+2]=0;

		mStatus = eInputChi;
		mPYSelIndex = 0;
		mPYShowFirstIndex=0;
		mHZSelIndex = 0;
		mHZShowFirstIndex=0;
		mPY[0]=0;
	}
	else if(JGE::GetInstance()->GetButtonClick(PSP_CTRL_UP))
	{
		mHZSelIndex--;
		if (mHZSelIndex<0)
			mHZSelIndex = 0;
	}
	else if(JGE::GetInstance()->GetButtonClick(PSP_CTRL_DOWN))
	{
		if(len>0 && mHZSelIndex < totalLen -1)
			mHZSelIndex++;
	}
	else if(JGE::GetInstance()->GetButtonClick(PSP_CTRL_RIGHT))
	{
		if(totalLen - mHZSelIndex > mHZSelTableSize)
		{
			mHZSelIndex+=mHZSelTableSize;
			mHZShowFirstIndex+=mHZSelTableSize;
		}
		else if(totalLen - mHZShowFirstIndex > mHZSelTableSize && totalLen > mHZSelIndex)
		{
			mHZSelIndex = totalLen-1;
			mHZShowFirstIndex+=mHZSelTableSize;
		}

	}
	else if(JGE::GetInstance()->GetButtonClick(PSP_CTRL_LEFT))
	{
		if(mHZSelIndex>=mHZSelTableSize)
		{
			mHZSelIndex-=mHZSelTableSize;
			mHZShowFirstIndex-=mHZSelTableSize;
			if(mHZShowFirstIndex<0)
				mHZShowFirstIndex=0;
		}
		else if(mHZSelIndex > 0 && mHZShowFirstIndex > 0)
		{
			mHZSelIndex=0;
			mHZShowFirstIndex = 0;
		}
	}
}
void JInputSystem::UpdateInputNum()
{
	if(mStatus != eInputNum)
		return;
	char* pBuf = mpInput;
	int a,b,c;
	if (GetInputKey(a,b,c))
	{
		a=2;
		if(b==4 && c==0)
		{
			int len = strlen(pBuf);
			if(len>0)
			{
				if(pBuf[len-1]>0)
				{
					pBuf[len-1]=0;
				}
				else
				{
					if(len>=2)
					{
						if(pBuf[len-2]<0)
						{
							pBuf[len-2]=0;
						}
					}
				}
			}
		}
		else
		{
			char cc[2];
			cc[0]=input_table[a][b][c];
			cc[1]=0;
			strcat(pBuf,(cc));
		}
	}
}




//////////////////////////////////////////////////////////////////////////
/// Draw
//////////////////////////////////////////////////////////////////////////
void JInputSystem::Draw()
{
	DrawInputHelp(2,181);
	//DrawStatus(450,3/*17*//*5*/);

	float x,y;
	x=SCREEN_WIDTH_F/2;
	y=SCREEN_HEIGHT_F/2;

	DrawStatus(x-25,y-8);
	DrawInputString(x,y);
	if(mStatus!=eInputEng)
	{
		DrawPYInput(x,y+14);
 		//if(mEnablePYSel) 
 			//DrawPYSel(220,15);
 		
		mIsHZ_H?DrawHZSel_H(x,y+28):DrawHZSel(x,y+28);
	}
}

void JInputSystem::DrawInputString( float x,float y )
{
	// render text
	int len = strlen12(mpInput);
	
	int dlen=5;
	JRenderer::GetInstance()->FillRect(x-7,y-8,len+dlen+7,15,ARGB(255,0,0,0));
	JRenderer::GetInstance()->DrawLine(x-7,y-8,x+len+dlen,y-8,ARGB(255,255,255,255));
	JRenderer::GetInstance()->DrawLine(x-7,y+7,x+len+dlen+1,y+7,ARGB(255,255,255,255));
	JRenderer::GetInstance()->DrawLine(x-7,y-8,x-7,y+7,ARGB(255,255,255,255));
	JRenderer::GetInstance()->DrawLine(x+len+dlen,y-8,x+len+dlen,y+7,ARGB(255,255,255,255));

	if(mTimer > 200)
		printf12("|",x+len+1,y);

	if(len<=0)
		return;

	printf12(mpInput, x+1, y); 
	//DrawStr1(mInPut,x,y);

	return;
}

void JInputSystem::DrawStr1( char* str, float x, float y, u32 color)
{
	y+=7;
	int len = strlen12(str);	
	JRenderer::GetInstance()->FillRect(x-7,y-7,len+7,14,color);
	printf12(str,x,y);
	JRenderer::GetInstance()->DrawLine(x-7,y-7,x+len,y-7,ARGB(255,255,255,255));
	JRenderer::GetInstance()->DrawLine(x-7,y+7,x+len,y+7,ARGB(255,255,255,255));
	JRenderer::GetInstance()->DrawLine(x-7,y-7,x-7,y+7,ARGB(255,255,255,255));
	JRenderer::GetInstance()->DrawLine(x+len,y-7,x+len,y+7,ARGB(255,255,255,255));
}

void JInputSystem::DrawStatus( float x,float y )
{
	switch(mStatus)
	{
	case eInputEng:
		DrawStr1("Eng",x,y);
		break;
	case eInputChi:
		DrawStr1("Chi",x,y);
		break;
	case eSelPY:
		DrawStr1("PYSel",x,y);
		break;
	case eSelHZ:
		DrawStr1("HZSel",x,y);
		break;
	case eInputNum:
		DrawStr1("Num",x,y);
		break;
	default:
		break;
	}
}

void JInputSystem::DrawPYInput( float x,float y )
{
	if(mStatus == eInputNum || mStatus == eInputEng)
		return;
	PY_index* pyindex=NULL;
	int indexlen = GetNexPYIndex(mPY,pyindex);
	if(!pyindex || indexlen == 0)
	{
		int pylen = strlen(mPY);
		if (pylen > 0)
		{		
			mPY[strlen(mPY)-1]=0;
		}
		else
			mPY[0]=0;
	}

	int len = 6+strlen12(mPY);
	if(mStatus==eInputChi)
		JRenderer::GetInstance()->FillRect(x-7,y-7,len+7,14,ARGB(255,100,100,100));
	else
		JRenderer::GetInstance()->FillRect(x-7,y-7,len+7,14,ARGB(255,0,0,0));
	JRenderer::GetInstance()->DrawLine(x-7,y-7,x+len,y-7,ARGB(255,255,255,255));
	JRenderer::GetInstance()->DrawLine(x-7,y+7,x+len,y+7,ARGB(255,255,255,255));
	JRenderer::GetInstance()->DrawLine(x-7,y-7,x-7,y+7,ARGB(255,255,255,255));
	JRenderer::GetInstance()->DrawLine(x+len,y-7,x+len,y+7,ARGB(255,255,255,255));

	if(mTimer > 200 && mStatus == eInputChi)
	{
		char bb[8];
		sprintf(bb,"%s_",mPY);
		printf12(bb, x+1, y); 
	}
	else
	{
		char bb[8];
		sprintf(bb,"%s",mPY);
		printf12(bb, x+1, y);
	}
}

void JInputSystem::DrawPYSel( float x,float y )
{
	PY_index* pyindex=NULL;
	int len = GetNexPYIndex(mPY,pyindex);
	//if(len==1)
		//mStatus=eSelHZ;
	if(pyindex && len>0)
	{
		int totalLen=len;
		int startIndex=mPYShowFirstIndex;
		int endIndex=mPYShowFirstIndex+(mPYSelTableSize-1);
		int curIndex=mPYSelIndex-startIndex;

		if(endIndex+1 >= totalLen)
			endIndex=totalLen-1;
		if(mPYSelIndex > endIndex) 
		{
			startIndex += mPYSelIndex-endIndex;
			mPYShowFirstIndex = startIndex;
			endIndex = mPYSelIndex;
			curIndex = endIndex-startIndex;
		}
		else if(mPYSelIndex < startIndex)
		{
			endIndex -= startIndex - mPYSelIndex;
			mPYShowFirstIndex = endIndex -(mPYSelTableSize-1);
			startIndex = mPYSelIndex;
			curIndex = 0;
		}

		int i;
		unsigned int slen=0;

		// count the max string len.
		for(i = startIndex; i<=endIndex; i++)
		{
			char buf[8];
			sprintf(buf," %s",((PY_index*)(pyindex+i))->PY);
			buf[0]=mPY[0];
			if(slen<strlen(buf))
				slen = strlen(buf);
		}

		// Draw back ground
		slen*=6;
		for(i=0; i <= endIndex - startIndex; i++)
		{
			if(mStatus==eSelPY && curIndex==i)
				JRenderer::GetInstance()->FillRect(x-7,y-7+14*i,slen-2+7,14,ARGB(255,100,100,100));
			else
				JRenderer::GetInstance()->FillRect(x-7,y-7+14*i,slen-2+7,14,ARGB(255,0,0,0));
			JRenderer::GetInstance()->DrawLine(x-8,y-7+14*i,x+slen-2,y-7+14*i,ARGB(255,255,255,255));
			JRenderer::GetInstance()->DrawLine(x-8,y+7+14*i,x+slen-2,y+7+14*i,ARGB(255,255,255,255));
			JRenderer::GetInstance()->DrawLine(x-8,y-7+14*i,x-8,y+7+14*i,ARGB(255,255,255,255));
			JRenderer::GetInstance()->DrawLine(x+slen-2,y-7+14*i,x+slen-2,y+7+14*i,ARGB(255,255,255,255));
		}


		//return;
		for(i=0; i <= endIndex - startIndex; i++)
		{
			char buf[8];
			sprintf(buf," %s",((PY_index*)(pyindex+i+startIndex))->PY);
			buf[0]=mPY[0];
			printf12(buf,x,y+14*i);
		}
	}
}


void JInputSystem::DrawHZSel( float x,float y )
{
	PY_index* pyindex=NULL;
	int len = GetNexPYIndex(mPY,pyindex);
	if(len>0 && mPYSelIndex < len)
	{
		char* str = pyindex[mPYSelIndex].PY_mb;
		//PSPCommonDrawUtility::GetInstance()->printf12(str,x,y);

		int totalLen=strlen(str)/2;
		int startIndex=mHZShowFirstIndex;
		int endIndex=mHZShowFirstIndex+(mHZSelTableSize-1);
		int curIndex=mHZSelIndex-startIndex;

		if(endIndex+1 >= totalLen)
			endIndex=totalLen-1;
		if(mHZSelIndex > endIndex) 
		{
			startIndex += mHZSelIndex-endIndex;
			mHZShowFirstIndex = startIndex;
			endIndex = mHZSelIndex;
			curIndex = endIndex-startIndex;
		}
		else if(mHZSelIndex < startIndex)
		{
			endIndex -= startIndex - mHZSelIndex;
			mHZShowFirstIndex = endIndex -(mHZSelTableSize-1);
			startIndex = mHZSelIndex;
			curIndex = 0;
		}

		int i;
		int slen=12;

		// Draw back ground
		for(i=0; i <= endIndex - startIndex; i++)
		{
			if(mStatus==eSelHZ && curIndex==i)
				JRenderer::GetInstance()->FillRect(x-7,y-7+14*i,slen-2+7,14,ARGB(255,100,100,100));
			else
				JRenderer::GetInstance()->FillRect(x-7,y-7+14*i,slen-2+7,14,ARGB(255,0,0,0));
			JRenderer::GetInstance()->DrawLine(x-8,y-7+14*i,x+slen-2,y-7+14*i,ARGB(255,255,255,255));
			JRenderer::GetInstance()->DrawLine(x-8,y+7+14*i,x+slen-2,y+7+14*i,ARGB(255,255,255,255));
			JRenderer::GetInstance()->DrawLine(x-8,y-7+14*i,x-8,y+7+14*i,ARGB(255,255,255,255));
			JRenderer::GetInstance()->DrawLine(x+slen-2,y-7+14*i,x+slen-2,y+7+14*i,ARGB(255,255,255,255));
		}

		for(i=0; i <= endIndex - startIndex; i++)
		{
			char buf[8];
			buf[0]=str[(startIndex+i)*2];
			buf[1]=str[(startIndex+i)*2+1];
			buf[2]=0;
			buf[3]=0;
			printf12(buf,x+1,1+y+14*i);
		}
	}
}

void JInputSystem::DrawInputHelp( float x, float y )
{
	int a,b,c,i,j;
	GetInputKey(a,b,c);

	float L=90;
	u32 color = ARGB(255,255,255,255);
	JRenderer* renderer = JRenderer::GetInstance();		

	for(j=0; j<3; j++)
	{
		for(i=0; i<3; i++)
		{
			if(j+i*3==b)
				renderer->FillRect(x+j*L/3,y+i*L/3,L/3,L/3,ARGB(255,100,100,100));
			else
				renderer->FillRect(x+j*L/3,y+i*L/3,L/3,L/3,ARGB(255,0,0,0));
		}
	}

	renderer->DrawLine(x,      y,      x+L,    y,      color);
	renderer->DrawLine(x,      y+L/3,  x+L,    y+L/3,  color);
	renderer->DrawLine(x,      y+L*2/3,x+L,    y+L*2/3,color);
	renderer->DrawLine(x-1,      y+L,    x+L+1,    y+L,    color);
	renderer->DrawLine(x,      y,      x,      y+L,color);
	renderer->DrawLine(x+L/3,  y,      x+L/3,  y+L,color);
	renderer->DrawLine(x+L*2/3,y,      x+L*2/3,y+L,color);
	renderer->DrawLine(x+L,    y,      x+L,    y+L,color);

	for(j=0; j<3; j++)
	{
		for(i=0; i<3; i++)
		{
			char cc[2];
			cc[1]=0;

			cc[0]=input_table[a][j*3+i][0];
			printf12(cc, (int)(x+L/3/2+i*L/3+2), (int)(j*L/3+y+8), JGETEXT_CENTER); 
			cc[0]=input_table[a][j*3+i][1];
			printf12(cc, (int)(x+8+i*L/3+2),     (int)(j*L/3+y+L/3/2), JGETEXT_CENTER);
			cc[0]=input_table[a][j*3+i][3];
			printf12(cc, (int)(x+L/3-8+i*L/3+2), (int)(j*L/3+y+L/3/2), JGETEXT_CENTER);
			cc[0]=input_table[a][j*3+i][2];
			printf12(cc, (int)(x+L/3/2+i*L/3+2), (int)(j*L/3+y+L/3-7), JGETEXT_CENTER);
		}
	}

	x+=L/2/**1.5f*/;
	y+=L/2-8;
	printf12("%f%f",x,y);

	x--;
	y+=19;
	JRenderer::GetInstance()->DrawLine(x-4,y,x+4,y,ARGB(255,255,255,255));
	JRenderer::GetInstance()->DrawLine(x-4,y-3,x-4,y,ARGB(255,255,255,255));
	JRenderer::GetInstance()->DrawLine(x+4,y-3,x+4,y,ARGB(255,255,255,255));


	return;
}
//////////////////////////////////////////////////////////////////////////
///
//////////////////////////////////////////////////////////////////////////
bool JInputSystem::GetInputKey( int& a, int& b, int& c )
{
	if(JGE::GetInstance()->GetButtonState(PSP_CTRL_LTRIGGER))
		a=1;
	else
		a=0;

	if (mStatus == eInputNum)
	{
		a=2;
	}

	if(JGE::GetInstance()->GetAnalogX() < 50 && JGE::GetInstance()->GetAnalogY() < 50)
		b=0;
	else if(JGE::GetInstance()->GetAnalogX() > 50 && JGE::GetInstance()->GetAnalogX() < 200 && JGE::GetInstance()->GetAnalogY() < 50)
		b=1;
	else if(JGE::GetInstance()->GetAnalogX() > 200 && JGE::GetInstance()->GetAnalogY() < 50)
		b=2;
	else if(JGE::GetInstance()->GetAnalogY() > 50 && JGE::GetInstance()->GetAnalogY() < 200 && JGE::GetInstance()->GetAnalogX() < 50)
		b=3;
	if(JGE::GetInstance()->GetAnalogY() > 50 && JGE::GetInstance()->GetAnalogY() < 200 && JGE::GetInstance()->GetAnalogX() > 50 && JGE::GetInstance()->GetAnalogX() < 200)
		b=4;
	if(JGE::GetInstance()->GetAnalogY() > 50 && JGE::GetInstance()->GetAnalogY() < 200 && JGE::GetInstance()->GetAnalogX() > 200)
		b=5;
	if(JGE::GetInstance()->GetAnalogY() > 200 && JGE::GetInstance()->GetAnalogX() < 50)
		b=6;
	if(JGE::GetInstance()->GetAnalogY() > 200 && JGE::GetInstance()->GetAnalogX() > 50 && JGE::GetInstance()->GetAnalogX() < 200)
		b=7;
	if(JGE::GetInstance()->GetAnalogY() > 200 && JGE::GetInstance()->GetAnalogX() > 200)
		b=8;

	if(JGE::GetInstance()->GetButtonClick(PSP_CTRL_TRIANGLE))
		c=0;
	else if (JGE::GetInstance()->GetButtonClick(PSP_CTRL_SQUARE))
		c=1;
	else if (JGE::GetInstance()->GetButtonClick(PSP_CTRL_CIRCLE))
		c=3;
	else if (JGE::GetInstance()->GetButtonClick(PSP_CTRL_CROSS))
		c=2;
	else 
		return false;

	return true;
}

char* JInputSystem::GetChinese( char* str)
{
	char first = str[0];
	char* after = str+1;
	if(first=='i')return NULL;        /*¥˙ÍÛ∆¥“Ù ‰»ÅE */
	if(first=='u')return NULL;
	if(first=='v')return NULL;

	PY_index* py_index = PY_index_headletter[first-'a'];
	const int len = PY_index_headsize[first-'a'];
	for (int i=0; i<len; i++)
	{
		//char *PY;
		//char *PY_mb;
		if(stricmp(after,py_index[i].PY)==0)
			return py_index[i].PY_mb;
	}
	return NULL;
}

int JInputSystem::GetNexPYIndex( char* str, PY_index* &py_index )
{
	const unsigned int pylen = strlen(str);
	if(pylen == 0 || pylen > 7)// > 6
		return 0;

	int i,j;
	PY_index* rt_py_index=NULL;
	char first = str[0];
	rt_py_index = PY_index_headletter[first-'a'];
	int index_len = PY_index_headsize[first-'a'];	
	py_index = rt_py_index;

	for(i=1; i < (int)pylen; i++)
	{
		for(j=0; j<index_len; j++)
		{
			if(str[i] == rt_py_index[j].PY[i-1])
			{
				rt_py_index = rt_py_index+j;
				index_len -= j;
				j=-1;
				break;
			}
		}
		if(j!=-1)
		{
			py_index = NULL;
			return 0;
		}
	}
	py_index = rt_py_index;

	for(i=0; i<index_len; i++)
	{
		for(j=1;j<(int)pylen;j++)
		{
			if(str[j]!=py_index[i].PY[j-1])
			{
				index_len = i;
				return index_len;
			}
		}
	}

	return index_len;
}

void JInputSystem::DrawHZSel_H( float x,float y )
{
	x++;
	PY_index* pyindex=NULL;
	int len = GetNexPYIndex(mPY,pyindex);
	if(len>0 && mPYSelIndex < len)
	{
		char* str = pyindex[mPYSelIndex].PY_mb;
		//PSPCommonDrawUtility::GetInstance()->printf12(str,x,y);

		int totalLen=strlen(str)/2;
		int startIndex=mHZShowFirstIndex;
		int endIndex=mHZShowFirstIndex+(mHZSelTableSize-1);
		int curIndex=mHZSelIndex-startIndex;

		if(endIndex+1 >= totalLen)
			endIndex=totalLen-1;
		if(mHZSelIndex > endIndex) 
		{
			startIndex += mHZSelIndex-endIndex;
			mHZShowFirstIndex = startIndex;
			endIndex = mHZSelIndex;
			curIndex = endIndex-startIndex;
		}
		else if(mHZSelIndex < startIndex)
		{
			endIndex -= startIndex - mHZSelIndex;
			mHZShowFirstIndex = endIndex -(mHZSelTableSize-1);
			startIndex = mHZSelIndex;
			curIndex = 0;
		}

		int i;
		int slen=12;

		// Draw back ground
		for(i=0; i <= endIndex - startIndex; i++)
		{
			if(mStatus==eSelHZ && curIndex==i)
				JRenderer::GetInstance()->FillRect(x-7+15*i,y-7,15,slen+3,ARGB(255,100,100,100));
			else
				JRenderer::GetInstance()->FillRect(x-7+15*i,y-7,15,slen+3,ARGB(255,0,0,0));
			JRenderer::GetInstance()->DrawLine(x-8+15*i,y-7,x+slen-5+15*i,y-7,ARGB(255,255,255,255));
			JRenderer::GetInstance()->DrawLine(x-8+15*i,y+7,x+slen-4+15*i,y+7,ARGB(255,255,255,255));
			JRenderer::GetInstance()->DrawLine(x-8+15*i,y-7,x-8+15*i,y+7,ARGB(255,255,255,255));
			JRenderer::GetInstance()->DrawLine(x+slen-5+15*i,y-7,x+slen-5+15*i,y+7,ARGB(255,255,255,255));
		}

		for(i=0; i <= endIndex - startIndex; i++)
		{
			char buf[8];
			buf[0]=str[(startIndex+i)*2];
			buf[1]=str[(startIndex+i)*2+1];
			buf[2]=0;
			buf[3]=0;
			printf12(buf,x+15*i,1+y);
		}
	}
}

void JInputSystem::UpdateSelHZ_H()
{
	PY_index* pyindex=NULL;
	int totalLen = 0;
	char* str=NULL;
	int len = GetNexPYIndex(mPY,pyindex);
	if(len>0 && mPYSelIndex < len)
	{
		str = pyindex[mPYSelIndex].PY_mb;
		totalLen=strlen(str)/2;
	}
	else
		return;

	if(JGE::GetInstance()->GetButtonClick(PSP_CTRL_CROSS))
	{
		mStatus = eInputChi;
		mPYSelIndex = 0;
		mPYShowFirstIndex=0;
		mHZSelIndex = 0;
		mHZShowFirstIndex=0;
		mPY[0]=0;
	}
	else if(JGE::GetInstance()->GetButtonClick(PSP_CTRL_TRIANGLE))
	{
		mStatus = mEnablePYSel?eSelPY:eInputChi;
		//mStatus = eSelPY;
		mHZSelIndex = 0;
		mHZShowFirstIndex=0;
	}
	else if(JGE::GetInstance()->GetButtonClick(PSP_CTRL_CIRCLE))
	{
		int inlen = strlen(mpInput);
		mpInput[inlen]=str[mHZSelIndex*2];
		mpInput[inlen+1]=str[mHZSelIndex*2+1];
		mpInput[inlen+2]=0;

		mStatus = eInputChi;
		mPYSelIndex = 0;
		mPYShowFirstIndex=0;
		mHZSelIndex = 0;
		mHZShowFirstIndex=0;
		mPY[0]=0;
	}
	else if(JGE::GetInstance()->GetButtonClick(PSP_CTRL_LEFT))
	{
		mHZSelIndex--;
		if (mHZSelIndex<0)
			mHZSelIndex = 0;
	}
	else if(JGE::GetInstance()->GetButtonClick(PSP_CTRL_RIGHT))
	{
		if(len>0 && mHZSelIndex < totalLen -1)
			mHZSelIndex++;
	}
	else if(JGE::GetInstance()->GetButtonClick(PSP_CTRL_DOWN))
	{
		if(totalLen - mHZSelIndex > mHZSelTableSize)
		{
			mHZSelIndex+=mHZSelTableSize;
			mHZShowFirstIndex+=mHZSelTableSize;
		}
		else if(totalLen - mHZShowFirstIndex > mHZSelTableSize && totalLen > mHZSelIndex)
		{
			mHZSelIndex = totalLen-1;
			mHZShowFirstIndex+=mHZSelTableSize;
		}

	}
	else if(JGE::GetInstance()->GetButtonClick(PSP_CTRL_UP))
	{
		if(mHZSelIndex>=mHZSelTableSize)
		{
			mHZSelIndex-=mHZSelTableSize;
			mHZShowFirstIndex-=mHZSelTableSize;
			if(mHZShowFirstIndex<0)
				mHZShowFirstIndex=0;
		}
		else if(mHZSelIndex > 0 && mHZShowFirstIndex > 0)
		{
			mHZSelIndex=0;
			mHZShowFirstIndex = 0;
		}
	}
}

void JInputSystem::printf12( char* str,float x, float y, float scale/*=1.0f*/, PIXEL_TYPE color/*=ARGB(255,255,255,255)*/,int type/*=JGETEXT_LEFT*/ )
{
	mBitmapFont12->SetScale(scale);
	mBitmapFont12->SetColor(color);
	mBitmapFont12->RenderString((u8*)str,x,y,type);
}

int JInputSystem::strlen12( char* buff, float scale/*=1.0f*/ )
{
	mBitmapFont12->SetScale(scale);
	return mBitmapFont12->GetStringWidth((u8*)buff);
}











