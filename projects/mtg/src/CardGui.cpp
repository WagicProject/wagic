#include "../include/config.h"
#include "../include/CardGui.h"
#include "../include/ManaCostHybrid.h"
#include "../include/Subtypes.h"
#include "../include/Translate.h"
#include "../include/MTGDefinitions.h"
#include <Vector2D.h>

void CardGui::alternateRender(MTGCard * card, JQuad ** manaIcons, float x, float y, float rotation, float scale){
  JLBFont * mFont = GameApp::CommonRes->GetJLBFont(Constants::MAGIC_FONT);
  float backup = mFont->GetScale();
  JQuad * mIcons[7];
  if (!manaIcons){
    mIcons[Constants::MTG_COLOR_ARTIFACT] = GameApp::CommonRes->GetQuad("c_artifact");
    mIcons[Constants::MTG_COLOR_LAND] = GameApp::CommonRes->GetQuad("c_land");
    mIcons[Constants::MTG_COLOR_WHITE] = GameApp::CommonRes->GetQuad("c_white");
    mIcons[Constants::MTG_COLOR_RED] = GameApp::CommonRes->GetQuad("c_red");
    mIcons[Constants::MTG_COLOR_BLACK] = GameApp::CommonRes->GetQuad("c_black");
    mIcons[Constants::MTG_COLOR_BLUE] = GameApp::CommonRes->GetQuad("c_blue");
    mIcons[Constants::MTG_COLOR_GREEN] = GameApp::CommonRes->GetQuad("c_green");
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
    renderer->FillRoundRect(x+points[0].x + 2  ,y+points[0].y +2 ,width*scale-8,height*scale-8,2,ARGB(255,Constants::_r[color],Constants::_g[color],Constants::_b[color]));
    renderer->FillRect(x+points[0].x + 6 ,y+points[0].y + 6 ,width*scale-12,height*scale-12,bgcolor2);
  }else{
    for (int i=0; i < 4; i++){
      renderer->DrawLine(x + points[i].x,y + points[i].y,x + points[(i+1)%4].x,y + points[(i+1)%4].y,bgcolor);
    }
  }


  ManaCost * manacost = card->getManaCost();
  int nbicons = 0;
  ManaCostHybrid * h;

  unsigned int j = 0;
  while ((h = manacost->getHybridCost(j))){
    for (int i = 0; i < 2; i++){
      int color = h->color1;
      int value = h->value1;
      if (i) {
        color = h->color2;
        value = h->value2;
      }
      v.x = (width/2 - 24 - 16*nbicons + 8*i)*scale;
      v.y = ((-height/2) + 16 + 8*i) * scale;
      v.Rotate(rotation);
      if (color == 0){
        sprintf(buf,"%i",value);
        mFont->SetColor(ARGB(255,255,255,255));
        mFont->DrawString(buf,x+v.x,y+v.y);
      }else{
        renderer->RenderQuad(manaIcons[color],x+v.x,y+v.y,rotation,0.4*scale, 0.4*scale);
      }
    }
    nbicons++;
    j++;
  }

  for (int i = 1; i < Constants::MTG_NB_COLORS - 1; i++){

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
    s = _(s);
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
  int over = strlen(_(card->getName()).c_str()) - 23;
  float multiply = 1.4;
  if (over > 0){
    multiply = 1.1;
  }
  mFont->SetScale(scale * multiply);

  mFont->SetColor(ARGB(255,Constants::_r[color],Constants::_g[color],Constants::_b[color]));
  mFont->DrawString(_(card->getName()).c_str(),x+v.x,y+v.y);
  mFont->SetScale(scale);
  mFont->SetColor(ARGB(255,255,255,255));


  if (card->isACreature()){
    v.x = (width/2-40) * scale;
    v.y = (height/2-30) * scale;
    v.Rotate(rotation);
    sprintf(buf,"%i/%i",card->power,card->toughness);
    mFont->DrawString(buf,x+v.x,y+v.y);
  }

  for (int i = card->nb_types-1; i>=0; i--){
    v.x = ((-width/2)+10) * scale;
    v.y = (height/2-20 - 12 * i) * scale;
    v.Rotate(rotation);
    string s = Subtypes::subtypesList->find(card->types[i]);
    mFont->DrawString(_(s).c_str(),x+v.x,y+v.y);
  }

  mFont->SetScale(backup);
}


CardGui::CardGui(int id, MTGCardInstance * _card, float desiredHeight,float _x, float _y, bool hasFocus): PlayGuiObject(id, desiredHeight, _x,  _y,  hasFocus){
  LOG("==Creating NEW CardGui Object. CardName:");
  LOG(_card->getName());

  card = _card;
  type = GUI_CARD;

  alpha = 255;
  mParticleSys = NULL;

  if (card->hasColor(Constants::MTG_COLOR_RED)){
    mParticleSys = GameApp::Particles[3];
  }else if (card->hasColor(Constants::MTG_COLOR_BLUE)){
    mParticleSys = GameApp::Particles[1];
  }else if (card->hasColor(Constants::MTG_COLOR_GREEN)){
    mParticleSys = GameApp::Particles[2];
  }else if (card->hasColor(Constants::MTG_COLOR_BLACK)){
    mParticleSys = GameApp::Particles[4];
  }else if (card->hasColor(Constants::MTG_COLOR_WHITE)){
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
    if (card->changedZoneRecently == 0) card->changedZoneRecently-= dt;//must not become zero atm
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
  JQuad * quad = NULL;
  JRenderer * renderer = JRenderer::GetInstance();
  if (xpos == -1){
    xpos = 300;
    if (x > SCREEN_WIDTH / 2)
      xpos = 10;
  }
  if(ypos == -1)
    ypos = 20;
  if (!alternate){
    quad = card->getQuad();
    if (quad){
      quad->SetColor(ARGB(220,255,255,255));
      float scale = 257.f / quad->mHeight;
      renderer->RenderQuad(quad, xpos   , ypos , 0.0f,scale,scale);
    }else{
      quad = card->getThumb();
      alternate = 1;
    }
  }


  if (alternate){
    MTGCard * mtgcard = card->model;
    CardGui::alternateRender(mtgcard, NULL, xpos + 90  , ypos + 130, 0.0f,0.9f);
    if (quad){
      float scale = 250 / quad->mHeight;
      quad->SetColor(ARGB(40,255,255,255));
      renderer->RenderQuad(quad,xpos,ypos,0,scale,scale);
    }
  }
}

void CardGui::Render(){

  JLBFont * mFont = GameApp::CommonRes->GetJLBFont(Constants::MAIN_FONT);

  JRenderer * renderer = JRenderer::GetInstance();
  JQuad * quad = card->getThumb();
#if defined (WIN32) || defined (LINUX)
  //This shouldn't be done for the PSP. Basically it forces the system to load
  // The big image if it cannot find the thumbnail. That's great for image quality on a PC,
  // But it kills the performance for those who don't have thumbnails on the PSP
  if (!quad || quad->mHeight * 2 < mHeight){
    JQuad * quad2 = card->getQuad();
    if (quad2)
      quad = quad2;
  }
#endif
  float tap = (float)(card->isTapped());
  float rotation = M_PI_2 * tap;
  float mScale = mHeight / 64;
  float myW = 45 * mScale;
  float myH = 60*mScale;
  GameObserver * game = GameObserver::GetInstance();
  TargetChooser * tc = NULL;
  if (game) tc = game->getCurrentTargetChooser();
  float myX = x + (32 * tap * mScale);
  float myY = y+(20 * tap * mScale);
  if (quad){
    mScale = mHeight / quad->mHeight;
    myH = mHeight;
    myW = quad->mWidth * mScale;
    myX = x + (quad->mHeight/2 * tap * mScale);
    myY = y+(quad->mWidth/2 * tap * mScale);
  }

  if (mHeight-defaultHeight){
    if (card->isTapped()){
      renderer->FillRect(myX + 1*(mHeight-defaultHeight) - myH , myY + 1*(mHeight-defaultHeight) , myH, myW,   ARGB(128,0,0,0));
    }else{
      renderer->FillRect(myX + 1*(mHeight-defaultHeight)  , myY + 1*(mHeight-defaultHeight) , myW,  myH, ARGB(128,0,0,0));
    }
  }

   if(quad){
    quad->SetColor(ARGB( alpha,255,255,255));

    if (tc){
      if (!tc->canTarget(card)){
	      quad->SetColor(ARGB( alpha,50,50,50));
      }
    }
    renderer->RenderQuad(quad,  myX , myY , rotation,mScale,mScale);
    quad->SetColor(ARGB( alpha,255,255,255));
  }else{
    int color = card->getColor();




    char buffer[200];
    sprintf(buffer, "%s",card->getName());
    mFont->SetColor(ARGB(255,Constants::_r[color],Constants::_g[color],Constants::_b[color]));

    JQuad * mIcon = NULL;
    if (card->hasSubtype("plains")){
      mIcon = GameApp::CommonRes->GetQuad("c_white");
    }else if(card->hasSubtype("swamp")){
      mIcon = GameApp::CommonRes->GetQuad("c_black");
    }else if(card->hasSubtype("forest")){
      mIcon = GameApp::CommonRes->GetQuad("c_green");
    }else if(card->hasSubtype("mountain")){
      mIcon = GameApp::CommonRes->GetQuad("c_red");
    }else if(card->hasSubtype("island")){
      mIcon = GameApp::CommonRes->GetQuad("c_blue");
    }
    if (mIcon) mIcon->SetHotSpot(16,16);
    if (card->isTapped()){
      renderer->FillRect(myX  - myH , myY  , myH, myW,   ARGB(255,(Constants::_r[color]) /2 + 50,(Constants::_g[color]) /2 + 50,(Constants::_b[color])/ 2 + 50));
      renderer->DrawRect(myX  - myH , myY  , myH, myW,   ARGB(255,Constants::_r[color],Constants::_g[color],Constants::_b[color]));
      mFont->SetScale(DEFAULT_MAIN_FONT_SCALE * 0.8 * mScale);
      mFont->DrawString(buffer,myX - myH + 4, myY + 1);
      if (mIcon) renderer->RenderQuad(mIcon,myX - myH/2, myY + myW/2,M_PI_2,mScale,mScale);
      if (tc){
        if (!tc->canTarget(card)){
	        renderer->FillRect(myX  - myH , myY  , myH, myW,   ARGB(200,0,0,0));
        }
      }
    }else{
      renderer->FillRect(myX   , myY , myW,  myH, ARGB(255,(Constants::_r[color]) /2 + 50,(Constants::_g[color]) /2 + 50,(Constants::_b[color]) /2 + 50));
      renderer->DrawRect(myX   , myY , myW,  myH, ARGB(255,Constants::_r[color],Constants::_g[color],Constants::_b[color]));
      mFont->SetScale(DEFAULT_MAIN_FONT_SCALE * 0.5 * mScale);
      mFont->DrawString(buffer,myX+4,myY + 1);
      if (mIcon) renderer->RenderQuad(mIcon,myX + myW/2, myY + myH/2,0,mScale, mScale);
      if (tc){
        if (!tc->canTarget(card)){
	        renderer->FillRect(myX   , myY , myW,  myH,   ARGB(200,0,0,0));
        }
      }
    }
    mFont->SetScale(DEFAULT_MAIN_FONT_SCALE);
  }

  if (tc && tc->alreadyHasTarget(card)){
    if (card->isTapped()){
      renderer->FillRect(myX- myH , myY  , myH, myW,   ARGB(128,255,0,0));
    }else{
      renderer->FillRect(myX   , myY  , myW,  myH, ARGB(128,255,0,0));
    }
  }

  if (card->isACreature()){
    mFont->SetScale(DEFAULT_MAIN_FONT_SCALE);
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

ostream& CardGui::toString(ostream& out) const
{
  return (out << "CardGui ::: mParticleSys : " << mParticleSys << " ; alpha : " << alpha << " ; card : " << card);
}
