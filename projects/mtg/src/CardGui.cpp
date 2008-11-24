#include "../include/debug.h"
#include "../include/CardGui.h"
#include <Vector2D.h>

void CardGui::alternateRender(MTGCard * card, JLBFont * mFont, JQuad ** manaIcons, float x, float y, float rotation, float scale){
  JQuad * mIcons[7];
  if (!manaIcons){
    mIcons[MTG_COLOR_ARTIFACT] = GameApp::CommonRes->GetQuad("c_artifact");
    mIcons[MTG_COLOR_LAND] = GameApp::CommonRes->GetQuad("c_land");
    mIcons[MTG_COLOR_WHITE] = GameApp::CommonRes->GetQuad("c_white");
    mIcons[MTG_COLOR_RED] = GameApp::CommonRes->GetQuad("c_red");
    mIcons[MTG_COLOR_BLACK] = GameApp::CommonRes->GetQuad("c_black");
    mIcons[MTG_COLOR_BLUE] = GameApp::CommonRes->GetQuad("c_blue");
    mIcons[MTG_COLOR_GREEN] = GameApp::CommonRes->GetQuad("c_green");
    for (int i=0; i < 7; i++){
      mIcons[i]->SetHotSpot(16,16);
    }
    manaIcons = mIcons;
  }
  Vector2D v;
  Vector2D points[4];
  PIXEL_TYPE bgcolor = ARGB(255,128,128,128);
  PIXEL_TYPE bgcolor2 = ARGB(255,80,80,80);
  char buf[25];
  int width = 200;
  int height = 285;

  JRenderer * renderer = JRenderer::GetInstance();
  mFont->SetRotation(rotation);
  mFont->SetScale(scale);

  int color = card->getColor();

  points[0].x = -width/2;
  points[0].y = -height/2 ;
  points[1].x = width/2;
  points[1].y = -height/2;
  points[2].x = width/2;
  points[2].y = height/2;
  points[3].x = -width/2;
  points[3].y = height/2;

  for (int i=0; i < 4; i++){
    points[i].x *= scale;
    points[i].y *= scale;
    points[i].Rotate(rotation);
  }

  if (rotation == 0){
    renderer->FillRoundRect(x+points[0].x + 2  ,y+points[0].y +2 ,width*scale-8,height*scale-8,2,ARGB(255,_r[color],_g[color],_b[color]));
    renderer->FillRect(x+points[0].x + 6 ,y+points[0].y + 6 ,width*scale-12,height*scale-12,bgcolor2);
  }else{
    for (int i=0; i < 4; i++){
      renderer->DrawLine(x + points[i].x,y + points[i].y,x + points[(i+1)%4].x,y + points[(i+1)%4].y,bgcolor);
    }
  }


  ManaCost * manacost = card->getManaCost();
  int nbicons = 0;
  for (int i = 1; i < MTG_NB_COLORS - 1; i++){

    int cost = manacost->getCost(i);
    for (int j=0; j < cost; j++){
      v.x = (width/2 - 20 - 16*nbicons)*scale;
      v.y = ((-height/2) + 20) * scale;
      v.Rotate(rotation);
      renderer->RenderQuad(manaIcons[i],x+v.x,y+v.y,rotation,0.5*scale, 0.5*scale);
      nbicons++;
    }
  }
  int cost = manacost->getCost(0);
  if (cost !=0){
    v.x = (width/2 - 20 - 16*nbicons)*scale;
    v.y = ((-height/2) + 14) * scale;
    v.Rotate(rotation);
    sprintf(buf,"%i",cost);
    mFont->SetColor(ARGB(255,255,255,255));
    mFont->DrawString(buf,x+v.x,y+v.y);
  }

  if (!card->formattedTextInit){
    std::string s(card->getText());
    std::string::size_type found=s.find_first_of("{}");
    while (found!=string::npos)
      {
	s[found]='/';
	found=s.find_first_of("{}",found+1);
      }
    std::string::size_type len = 24;
    while (s.length() > 0){
      std::string::size_type cut = s.find_first_of("., \t)", 0);
      if (cut >= len || cut == string::npos){
	card->formattedText.push_back(s.substr(0,len));
	if (s.length() > len){
	  s = s.substr(len,s.length()-len);
	}else{
	  s = "";
	}
      }else{
	std::string::size_type newcut = cut;
	while (newcut < len && newcut != string::npos){
	  cut = newcut;
	  newcut = s.find_first_of("., \t)", newcut + 1);
	}
	card->formattedText.push_back(s.substr(0,cut+1));
	if (s.length() > cut+1){
	  s = s.substr(cut+1,s.length() - cut - 1);
	}else{
	  s = "";
	}
      }
    }
    card->formattedTextInit = 1;
  }



  for (std::vector<string>::size_type i=0; i < card->formattedText.size(); i++){
    sprintf(buf, "%s", card->formattedText[i].c_str());
    v.x = (-width/2 + 12 )*scale;
    v.y = (50 + static_cast<signed int>(16*i - height/2)) * scale;
    v.Rotate(rotation);
    mFont->DrawString(buf,x+v.x,y+v.y);
  }



  v.x = ((-width/2)+10) * scale;
  v.y = ((-height/2) + 25) * scale;
  v.Rotate(rotation);
  int over = strlen(card->getName()) - 23;
  float multiply = 1.4;
  if (over > 0){
    multiply = 1.1;
  }
  mFont->SetScale(scale * multiply);
  mFont->SetColor(ARGB(255,_r[color],_g[color],_b[color]));
  mFont->DrawString(card->getName(),x+v.x,y+v.y);
  mFont->SetScale(scale);
  mFont->SetColor(ARGB(255,255,255,255));


  if (card->isACreature()){
    v.x = (width/2-40) * scale;
    v.y = (height/2-30) * scale;
    v.Rotate(rotation);
    sprintf(buf,"%i/%i",card->power,card->toughness);
    mFont->DrawString(buf,x+v.x,y+v.y);
  }

}


CardGui::CardGui(int id, MTGCardInstance * _card, float desiredHeight,float _x, float _y, bool hasFocus): PlayGuiObject(id, desiredHeight, _x,  _y,  hasFocus){
  LOG("==Creating NEW CardGui Object. CardName:");
  LOG(_card->getName());

  card = _card;
  type = GUI_CARD;

  alpha = 255;
  mParticleSys = NULL;

  if (card->hasColor(MTG_COLOR_RED)){
    mParticleSys = GameApp::Particles[3];
  }else if (card->hasColor(MTG_COLOR_BLUE)){
    mParticleSys = GameApp::Particles[1];
  }else if (card->hasColor(MTG_COLOR_GREEN)){
    mParticleSys = GameApp::Particles[2];
  }else if (card->hasColor(MTG_COLOR_BLACK)){
    mParticleSys = GameApp::Particles[4];
  }else if (card->hasColor(MTG_COLOR_WHITE)){
    mParticleSys = GameApp::Particles[0];
  }else{
    mParticleSys = GameApp::Particles[5];
  }

  LOG("==CardGui Object Creation Succesfull");
}


void CardGui::Update(float dt){
  alpha = 255;

  if  (card->changedZoneRecently > 0) alpha = 255.f - 255.f * card->changedZoneRecently;
  if (mParticleSys && card->changedZoneRecently == 1.f){
    mParticleSys->MoveTo(x+15, y+2*mHeight/3);
    mParticleSys->Fire();
  }
  if (card->changedZoneRecently){
    if (mParticleSys) mParticleSys->Update(dt);
    card->changedZoneRecently-= (5 *dt);
    if (card->changedZoneRecently < 0){
      if (mParticleSys)  mParticleSys->Stop();
    }
    if (card->changedZoneRecently < -3){
      card->changedZoneRecently = 0;
      mParticleSys = NULL;
    }
  }
  PlayGuiObject::Update(dt);
}

void CardGui::RenderBig(float xpos, float ypos, int alternate){
  if (xpos == -1){
    xpos = 300;
    if (x > SCREEN_WIDTH / 2)
      xpos = 10;
  }
  if(ypos == -1)
    ypos = 20;
  if (!alternate){
    JRenderer * renderer = JRenderer::GetInstance();
    JQuad * quad = card->getQuad();
    if (quad){
      quad->SetColor(ARGB(220,255,255,255));
      renderer->RenderQuad(quad, xpos   , ypos , 0.0f,0.9f,0.9f);
    }else{
      alternate = 1;
    }
  }


  if (alternate){
    MTGCard * mtgcard = card->model;
    JLBFont * font = GameApp::CommonRes->GetJLBFont("graphics/magic");
    CardGui::alternateRender(mtgcard, font, NULL, xpos + 90  , ypos + 130, 0.0f,0.9f);
  }
}

void CardGui::Render(){

  JLBFont * mFont = GameApp::CommonRes->GetJLBFont(MAIN_FONT);

  JRenderer * renderer = JRenderer::GetInstance();
  JQuad * quad = card->getThumb();
  if (!quad || quad->mHeight * 2 < mHeight){
    JQuad * quad2 = card->getQuad();
    if (quad2)
      quad = quad2;
  }

  float tap = (float)(card->isTapped());
  float rotation = M_PI_2 * tap;
  if (quad){
    float mScale = mHeight / quad->mHeight;
    float myX = x + (quad->mHeight/2 * tap * mScale);
    float myY = y+(quad->mWidth/2 * tap * mScale);
    if (mHeight-defaultHeight){
      if (card->isTapped()){
	renderer->FillRect(myX + 1*(mHeight-defaultHeight) - quad->mHeight * mScale , myY + 1*(mHeight-defaultHeight) , quad->mHeight * mScale, quad->mWidth * mScale,   ARGB(128,0,0,0));
      }else{
	renderer->FillRect(myX + 1*(mHeight-defaultHeight)  , myY + 1*(mHeight-defaultHeight) , quad->mWidth * mScale,  quad->mHeight * mScale, ARGB(128,0,0,0));
      }
    }

    quad->SetColor(ARGB( alpha,255,255,255));
    GameObserver * game = GameObserver::GetInstance();
    TargetChooser * tc = NULL;
    if (game) tc = game->getCurrentTargetChooser();
    if (tc){
      if (!tc->canTarget(card)){
	quad->SetColor(ARGB( alpha,50,50,50));
      }
    }
    renderer->RenderQuad(quad,  myX , myY , rotation,mScale,mScale);
    if (tc && tc->alreadyHasTarget(card)){
      if (card->isTapped()){
	renderer->FillRect(myX- quad->mHeight * mScale , myY  , quad->mHeight * mScale, quad->mWidth * mScale,   ARGB(128,255,0,0));
      }else{
	renderer->FillRect(myX   , myY  , quad->mWidth * mScale,  quad->mHeight * mScale, ARGB(128,255,0,0));
      }
    }
    quad->SetColor(ARGB( alpha,255,255,255));
  }else{
    int color = card->getColor();
    float mScale = mHeight / 64;
    float myX = x + (32 * tap * mScale);
    float myY = y+(20 * tap * mScale);

    char buffer[200];
    sprintf(buffer, "%s",card->getName());
    mFont->SetColor(ARGB(255,_r[color],_g[color],_b[color]));
    if (card->isTapped()){
      renderer->FillRect(myX  - 64 * mScale , myY  , 64 * mScale, 40 * mScale,   ARGB(255,0,0,0));
      renderer->DrawRect(myX  - 64 * mScale , myY  , 64 * mScale, 40 * mScale,   ARGB(255,_r[color],_g[color],_b[color]));
      mFont->SetScale(0.20);
      mFont->DrawString(buffer,myX - (64 * mScale)+4,myY + 1);
    }else{
      renderer->FillRect(myX   , myY , 40 * mScale,  64 * mScale, ARGB(255,0,0,0));
      renderer->DrawRect(myX   , myY , 40 * mScale,  64 * mScale, ARGB(255,_r[color],_g[color],_b[color]));
      mFont->SetScale(0.40);
      mFont->DrawString(buffer,myX+4,myY + 1);
    }


    mFont->SetScale(1.0);
  }
  if (card->isACreature()){
    mFont->SetScale(0.75);
    char buffer[200];
    sprintf(buffer, "%i/%i",card->power,card->life);
    renderer->FillRect(x+2,y + mHeight - 12, 25 , 12 ,ARGB(128,0,0,0));
    mFont->SetColor(ARGB(255,255,255,255));
    mFont->DrawString(buffer,x+4,y + mHeight - 10);
  }

  if (mParticleSys && card->changedZoneRecently > 0){
    renderer->SetTexBlend(BLEND_SRC_ALPHA, BLEND_ONE);
    mParticleSys->Render();
    // set normal blending
    renderer->SetTexBlend(BLEND_SRC_ALPHA, BLEND_ONE_MINUS_SRC_ALPHA);
  }
}



CardGui::~CardGui(){
  LOG("==Destroying CardGui object");
  LOG(this->card->getName());
  LOG("==CardGui object destruction Successful");
}



