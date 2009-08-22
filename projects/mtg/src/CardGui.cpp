#include "JGE.h"
#include "../include/config.h"
#include "../include/CardGui.h"
#include "../include/ManaCostHybrid.h"
#include "../include/Subtypes.h"
#include "../include/Translate.h"
#include "../include/MTGDefinitions.h"
#include <Vector2D.h>


const float CardGui::Width = 28.0;
const float CardGui::Height = 40.0;
const float CardGui::BigWidth = 200.0;
const float CardGui::BigHeight = 285.0;

CardGui::CardGui(MTGCardInstance* card, float x, float y) : PlayGuiObject(Height, x, y, false), quad(cache.getQuad(card)), card(card) {}
CardGui::CardGui(MTGCardInstance* card, const Pos& ref) : PlayGuiObject(Height, ref, false), quad(cache.getQuad(card)), card(card) {}

CardView::CardView(MTGCardInstance* card, float x, float y) : CardGui(card, x, y) {
  card->view = this;
}

CardView::CardView(MTGCardInstance* card, const Pos& ref) : CardGui(card, ref) {
  card->view = this;
}

void CardView::Update(float dt)
{
  PlayGuiObject::Update(dt);
}


void CardView::Render()
{
  JLBFont * mFont = GameApp::CommonRes->GetJLBFont(Constants::MAIN_FONT);

  JRenderer * renderer = JRenderer::GetInstance();
  GameObserver * game = GameObserver::GetInstance();

  TargetChooser * tc = NULL;
  if (game) tc = game->getCurrentTargetChooser();

  if (quad) {
    const float scale = actZ * 40 / quad->mHeight;
    renderer->RenderQuad(GameApp::CommonRes->GetQuad("shadow"), actX + (scale-1)*15, actY + (scale-1)*15, actT, 28*scale, 40*scale);
    quad->SetColor(ARGB(static_cast<unsigned char>(actA),255,255,255));
    renderer->RenderQuad(quad, actX, actY, actT, scale, scale);
  }
  else {
    int color = card->getColor();
    MTGCard * mtgcard = card->model;
    const float scale = actZ;

    renderer->RenderQuad(GameApp::CommonRes->GetQuad("shadow"), actX + (scale-1)*15, actY + (scale-1)*15, actT, 28*scale, 40*scale);

    mFont->SetColor(ARGB(static_cast<unsigned char>(actA), 0, 0, 0));

    JQuad * icon = NULL;
    if (card->hasSubtype("plains"))
      icon = GameApp::CommonRes->GetQuad("c_white");
    else if (card->hasSubtype("swamp"))
      icon = GameApp::CommonRes->GetQuad("c_black");
    else if (card->hasSubtype("forest"))
      icon = GameApp::CommonRes->GetQuad("c_green");
    else if (card->hasSubtype("mountain"))
      icon = GameApp::CommonRes->GetQuad("c_red");
    else if (card->hasSubtype("island"))
      icon = GameApp::CommonRes->GetQuad("c_blue");
    if (icon) icon->SetHotSpot(16,16);

    {
      JQuad* q;
      // Draw the "unknown" card model
      switch(card->getColor())
	{
	case Constants::MTG_COLOR_GREEN: q = GameApp::CommonRes->GetQuad("green_thumb"); break;
	case Constants::MTG_COLOR_BLUE : q = GameApp::CommonRes->GetQuad("blue_thumb"); break;
	case Constants::MTG_COLOR_RED  : q = GameApp::CommonRes->GetQuad("red_thumb"); break;
	case Constants::MTG_COLOR_BLACK: q = GameApp::CommonRes->GetQuad("black_thumb"); break;
	case Constants::MTG_COLOR_WHITE: q = GameApp::CommonRes->GetQuad("white_thumb"); break;
	default: q = GameApp::CommonRes->GetQuad("black_thumb"); break;
	}
      q->SetColor(ARGB(static_cast<unsigned char>(actA),255,255,255));
      renderer->RenderQuad(q, actX, actY, actT, scale, scale);
    }

    mFont->SetScale(DEFAULT_MAIN_FONT_SCALE * 0.5 * actZ);
    mFont->DrawString(card->getName().c_str(), actX - actZ * Width / 2 + 1, actY - actZ * Height / 2 + 1);
    if (icon) { icon->SetColor(ARGB(static_cast<unsigned char>(actA),255,255,255)); renderer->RenderQuad(icon, actX, actY, 0); }
    if (tc && !tc->canTarget(card)) renderer->FillRect(actX - actZ*Width/2, actY - actZ*Height/2, actZ*Width, actZ*Height, ARGB(200,0,0,0));
    mFont->SetScale(DEFAULT_MAIN_FONT_SCALE);
  }

  if (card->isCreature()){
    mFont->SetScale(DEFAULT_MAIN_FONT_SCALE);
    char buffer[200];
    sprintf(buffer, "%i/%i",card->power,card->life);
    renderer->FillRect(actX + 2, actY + 30 - 12, 25, 12, ARGB(((static_cast<unsigned char>(actA))/2),0,0,0));
    mFont->SetColor(ARGB(static_cast<unsigned char>(actA),255,255,255));
    mFont->DrawString(buffer, actX + 4, actY + 30 - 10);
  }

  PlayGuiObject::Render();
}


void CardGui::alternateRender(MTGCard * card, JQuad ** manaIcons, const Pos& pos){
  // Draw the "unknown" card model
  JRenderer * renderer = JRenderer::GetInstance();
  JQuad * q;
  switch(card->getColor())
    {
    case Constants::MTG_COLOR_GREEN: q = GameApp::CommonRes->GetQuad("green"); break;
    case Constants::MTG_COLOR_BLUE : q = GameApp::CommonRes->GetQuad("blue"); break;
    case Constants::MTG_COLOR_RED  : q = GameApp::CommonRes->GetQuad("red"); break;
    case Constants::MTG_COLOR_BLACK: q = GameApp::CommonRes->GetQuad("black"); break;
    case Constants::MTG_COLOR_WHITE: q = GameApp::CommonRes->GetQuad("white"); break;
    default: q = GameApp::CommonRes->GetQuad("black"); break;
    }
  float scale = pos.actZ * 250 / q->mHeight;
  q->SetColor(ARGB((int)pos.actA,255,255,255));
  renderer->RenderQuad(q, pos.actX, pos.actY, pos.actT, scale, scale);

  // Write the title
  JLBFont * font = GameApp::CommonRes->GetJLBFont("graphics/magic");
  float backup_scale = font->GetScale();
  font->SetColor(ARGB((int)pos.actA, 0, 0, 0));
  font->SetScale(0.8 * pos.actZ);

  {
    const char* name = _(card->getName()).c_str();
    float w = font->GetStringWidth(name) * 0.8 * pos.actZ;
    if (w > BigWidth - 30)
      font->SetScale((BigWidth - 30) / w);
    font->DrawString(name, pos.actX + 22 - BigWidth / 2, pos.actY + 22 - BigHeight / 2);
  }

  // Write the description
  {
    font->SetScale(0.8 * pos.actZ);
    const std::vector<string> txt = card->formattedText();
    unsigned i = 0;
    for (std::vector<string>::const_iterator it = txt.begin(); it != txt.end(); ++it, ++i)
      font->DrawString(it->c_str(), pos.actX + 22 - BigWidth / 2, pos.actY + 35 + 11 * i);
  }

  // Write the strength
  if (card->isCreature())
    {
      char buffer[32];
      sprintf(buffer, "%i/%i", card->power, card->toughness);
      float w = font->GetStringWidth(buffer) * 0.8;
      font->DrawString(buffer, pos.actX + 65 - w / 2, pos.actY + 106);
  }

  // Mana
  {
    ManaCost* manacost = card->getManaCost();
    ManaCostHybrid* h;
    unsigned int j = 0;
    unsigned char t = (JGE::GetInstance()->GetTime() / 3) & 0xFF;
    unsigned char v = t + 127;
    while ((h = manacost->getHybridCost(j)))
      {
	float scale = pos.actZ * 0.05 * cosf(2*M_PI*((float)t)/256.0);
	if (scale < 0)
	  {
	    renderer->RenderQuad(manaIcons[h->color1], pos.actX - 12 * j + 75 + 3 * sinf(2*M_PI*((float)t)/256.0), pos.actY - 115 + 3 * cosf(2*M_PI*((float)(t-35))/256.0), 0, 0.4 + scale, 0.4 + scale);
	    renderer->RenderQuad(manaIcons[h->color2], pos.actX - 12 * j + 75 + 3 * sinf(2*M_PI*((float)v)/256.0), pos.actY - 115 + 3 * cosf(2*M_PI*((float)(v-35))/256.0), 0, 0.4 - scale, 0.4 - scale);
	  }
	else
	  {
	    renderer->RenderQuad(manaIcons[h->color2], pos.actX - 12 * j + 75 + 3 * sinf(2*M_PI*((float)v)/256.0), pos.actY - 115 + 3 * cosf(2*M_PI*((float)(v-35))/256.0), 0, 0.4 - scale, 0.4 - scale);
	    renderer->RenderQuad(manaIcons[h->color1], pos.actX - 12 * j + 75 + 3 * sinf(2*M_PI*((float)t)/256.0), pos.actY - 115 + 3 * cosf(2*M_PI*((float)(t-35))/256.0), 0, 0.4 + scale, 0.4 + scale);
	  }
	++j;
      }
    for (int i = Constants::MTG_NB_COLORS - 2; i >= 1; --i)
      {
	for (int cost = manacost->getCost(i); cost > 0; --cost)
	  {
	    renderer->RenderQuad(manaIcons[i], pos.actX - 12*j + 75, pos.actY - 115, 0, 0.4 * pos.actZ, 0.4 * pos.actZ);
	    ++j;
	  }
      }
    // Colorless mana
    if (int cost = manacost->getCost(0))
      {
	char buffer[10];
	sprintf(buffer, "%d", cost);
	renderer->RenderQuad(manaIcons[0], pos.actX - 12*j + 75, pos.actY - 115, 0, 0.4 * pos.actZ, 0.4 * pos.actZ);
	float w = font->GetStringWidth(buffer);
	font->DrawString(buffer, pos.actX - 12*j + 76 - w/2, pos.actY - 120);
      }
  }

  {
    string s = "";
    for (int i = card->nb_types - 1; i > 0; --i)
      {
	s += _(Subtypes::subtypesList->find(card->types[i]));
	s += " - ";
      }
    s += _(Subtypes::subtypesList->find(card->types[0]));
    font->DrawString(s.c_str(), pos.actX + 22 - BigWidth / 2, pos.actY + 17);
  }

  font->SetScale(backup_scale);
}

void CardGui::RenderBig(const Pos& pos){
  JRenderer * renderer = JRenderer::GetInstance();

  if (quad){
    quad->SetColor(ARGB((int)pos.actA,255,255,255));
    float scale = pos.actZ * 257.f / quad->mHeight;
    renderer->RenderQuad(quad, pos.actX, pos.actY, pos.actT, scale, scale);
    return;
  }

  JQuad * q;
  if ((q = cache.getThumb(card)))
    {
      float scale = pos.actZ * 250 / q->mHeight;
      q->SetColor(ARGB((int)pos.actA,255,255,255));
      renderer->RenderQuad(q, pos.actX, pos.actY, pos.actT, scale, scale);
      return;
    }

  // If we come here, we do not have the picture.
  MTGCard * mtgcard = card->model;
  alternateRender(card,manaIcons,pos);
}



MTGCardInstance* CardView::getCard() { return card; }

TransientCardView::TransientCardView(MTGCardInstance* card, float x, float y) : CardView(card, x, y){}
TransientCardView::TransientCardView(MTGCardInstance* card, const Pos& ref) : CardView(card, ref.actX, ref.actY) {};
void TransientCardView::Render()
{
  CardView::Render();
}


/*
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


  for (int i = card->nb_types-1; i>=0; i--){
    v.x = ((-width/2)+10) * scale;
    v.y = (height/2-20 - 12 * i) * scale;
    v.Rotate(rotation);
    string s = Subtypes::subtypesList->find(card->types[i]);
    mFont->DrawString(_(s).c_str(),x+v.x,y+v.y);
  }

  mFont->SetScale(backup);
}


CardGui::CardGui(int id, MTGCardInstance * _card, float desiredHeight, float x, float y, bool hasFocus): PlayGuiObject(id, desiredHeight, x, y, hasFocus){
  LOG("==Creating NEW CardGui Object. CardName:");
  LOG(_card->getName());

  actX = x;
  actY = y;

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
    mParticleSys->MoveTo(actX + 15, actY + 2 * mHeight / 3);
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

  actX += 10 * dt * (x - actX);
  actY += 10 * dt * (y - actY);

  PlayGuiObject::Update(dt);
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
  float myH = 60 * mScale;
  GameObserver * game = GameObserver::GetInstance();
  TargetChooser * tc = NULL;
  if (game) tc = game->getCurrentTargetChooser();
  float myX = actX + (32 * tap * mScale);
  float myY = actY + (20 * tap * mScale);
  if (quad){
    mScale = mHeight / quad->mHeight;
    myH = mHeight;
    myW = quad->mWidth * mScale;
    myX = actX + (quad->mHeight/2 * tap * mScale);
    myY = actY + (quad->mWidth/2 * tap * mScale);
  }

  if (mHeight-defaultHeight){
    if (card->isTapped())
      renderer->FillRect(myX + 1 * (mHeight - defaultHeight) - myH, myY + 1 * (mHeight - defaultHeight), myH, myW, ARGB(128,0,0,0));
    else
      renderer->FillRect(myX + 1 * (mHeight - defaultHeight), myY + 1 * (mHeight-defaultHeight), myW, myH, ARGB(128,0,0,0));
  }

   if(quad){
    quad->SetColor(ARGB(alpha, 255, 255, 255));

    if (tc && !tc->canTarget(card)) quad->SetColor(ARGB(alpha, 50, 50, 50));
    renderer->RenderQuad(quad, myX, myY ,rotation, mScale, mScale);
    quad->SetColor(ARGB(alpha, 255, 255, 255));
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
      renderer->FillRect(myX - myH, myY, myH, myW, ARGB(255,Constants::_r[color]/2+50,Constants::_g[color]/2+50,Constants::_b[color]/2+50));
      renderer->DrawRect(myX - myH, myY, myH, myW, ARGB(255,Constants::_r[color],Constants::_g[color],Constants::_b[color]));
      mFont->SetScale(DEFAULT_MAIN_FONT_SCALE * 0.8 * mScale);
      mFont->DrawString(buffer, myX - (myH) + 4, myY + 1);
      if (mIcon) renderer->RenderQuad(mIcon, myX - myH / 2,myY + myW / 2, M_PI_2, mScale, mScale);
      if (tc && !tc->canTarget(card))
	  renderer->FillRect(myX - myH, myY, myH, myW, ARGB(200,0,0,0));
    }else{
      renderer->FillRect(myX, myY , myW,  myH, ARGB(255,Constants::_r[color]/2+50,Constants::_g[color]/2+50,Constants::_b[color]/2+50));
      renderer->DrawRect(myX, myY , myW,  myH, ARGB(255,Constants::_r[color],Constants::_g[color],Constants::_b[color]));
      mFont->SetScale(DEFAULT_MAIN_FONT_SCALE * 0.5 * mScale);
      mFont->DrawString(buffer, myX + 4, myY + 1);
      if (mIcon) renderer->RenderQuad(mIcon,myX + myW/2, myY + myH / 2, 0, mScale, mScale);
      if (tc && !tc->canTarget(card))
	renderer->FillRect(myX, myY, myW, myH, ARGB(200,0,0,0));
    }
    mFont->SetScale(DEFAULT_MAIN_FONT_SCALE);
   }

  if (tc && tc->alreadyHasTarget(card)){
    if (card->isTapped())
      renderer->FillRect(myX - myH, myY, myH, myW, ARGB(128,255,0,0));
    else
      renderer->FillRect(myX, myY, myW, myH, ARGB(128,255,0,0));
  }

  if (card->isCreature()){
    mFont->SetScale(DEFAULT_MAIN_FONT_SCALE);
    char buffer[200];
    sprintf(buffer, "%i/%i",card->power,card->life);
    renderer->FillRect(actX + 2, actY + mHeight - 12, 25, 12, ARGB(128,0,0,0));
    mFont->SetColor(ARGB(255,255,255,255));
    mFont->DrawString(buffer, actX + 4, actY + mHeight - 10);
  }

  if (mParticleSys && card->changedZoneRecently > 0){
    renderer->SetTexBlend(BLEND_SRC_ALPHA, BLEND_ONE);
    mParticleSys->Render();
    // set normal blending
    renderer->SetTexBlend(BLEND_SRC_ALPHA, BLEND_ONE_MINUS_SRC_ALPHA);
  }

  PlayGuiObject::Render();
}

float CardGui::Height()
{
  return card->getQuad()->mHeight;
}
float CardGui::Width()
{
  return card->getQuad()->mWidth;
}

CardGui::~CardGui(){
  LOG("==Destroying CardGui object");
  LOG(this->card->getName());
  LOG("==CardGui object destruction Successful");
}
*/

ostream& CardView::toString(ostream& out) const
{
  return (CardGui::toString(out) << " : CardView ::: card : " << card
	  << ";  actX,actY : " << actX << "," << actY << "; t : " << t
	  << " ; actT : " << actT << " ; quad : " << quad);
}
ostream& CardGui::toString(ostream& out) const
{
  return (out << "CardGui ::: x,y " << x << "," << y);
}
