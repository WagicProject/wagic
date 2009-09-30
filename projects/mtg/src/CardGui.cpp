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

CardGui::CardGui(MTGCardInstance* card, float x, float y) : PlayGuiObject(Height, x, y, false), card(card) {}
CardGui::CardGui(MTGCardInstance* card, const Pos& ref) : PlayGuiObject(Height, ref, false), card(card) {}

CardView::CardView(const CardSelector::SelectorZone owner, MTGCardInstance* card, float x, float y) : CardGui(card, x, y), owner(owner) {
  const Pos* ref = card->view;
  while (card)
    {
      if (ref == card->view) card->view = this;
      card = card->next;
    }
}

CardView::CardView(const CardSelector::SelectorZone owner, MTGCardInstance* card, const Pos& ref) : CardGui(card, ref), owner(owner) {
  const Pos* r = card->view;
  while (card)
    {
      if (r == card->view) card->view = this;
      card = card->next;
    }
}

void CardGui::Update(float dt)
{
  PlayGuiObject::Update(dt);
}


void CardGui::Render()
{
  JLBFont * mFont = resources.GetJLBFont(Constants::MAIN_FONT);

  JRenderer * renderer = JRenderer::GetInstance();
  GameObserver * game = GameObserver::GetInstance();

  TargetChooser * tc = NULL;
  if (game) tc = game->getCurrentTargetChooser();

  bool alternate = true;
  JQuad * quad = resources.RetrieveCard(card,CACHE_THUMB);
  if (quad) alternate = false;
  else quad = alternateThumbQuad(card);

  float cardScale = quad ? 40 / quad->mHeight : 1;
  float scale = actZ * cardScale;

  JQuad* shadow = resources.GetQuad("shadow");
  shadow->SetColor(ARGB(static_cast<unsigned char>(actA)/2,255,255,255));
  renderer->RenderQuad(shadow, actX + (actZ-1)*15, actY + (actZ-1)*15, actT, 28*actZ/16, 40*actZ/16); 

  if (quad) {
    quad->SetColor(ARGB(static_cast<unsigned char>(actA),255,255,255));
    renderer->RenderQuad(quad, actX, actY, actT, scale, scale);
  }

  if (alternate) {
    mFont->SetColor(ARGB(static_cast<unsigned char>(actA), 0, 0, 0));
    mFont->SetScale(DEFAULT_MAIN_FONT_SCALE * 0.5 * actZ);
    mFont->DrawString(card->getName().c_str(), actX - actZ * Width / 2 + 1, actY - actZ * Height / 2 + 1);
    mFont->SetScale(DEFAULT_MAIN_FONT_SCALE);

    JQuad * icon = NULL;
    if (card->hasSubtype("plains"))
      icon = resources.GetQuad("c_white");
    else if (card->hasSubtype("swamp"))
      icon = resources.GetQuad("c_black");
    else if (card->hasSubtype("forest"))
      icon = resources.GetQuad("c_green");
    else if (card->hasSubtype("mountain"))
      icon = resources.GetQuad("c_red");
    else if (card->hasSubtype("island"))
      icon = resources.GetQuad("c_blue");

    if (icon){
      icon->SetHotSpot(16,16);
      icon->SetColor(ARGB(static_cast<unsigned char>(actA),255,255,255)); 
      renderer->RenderQuad(icon, actX, actY, 0); 
    }
    
  }

  if (card->isCreature()){
    mFont->SetScale(DEFAULT_MAIN_FONT_SCALE);
    char buffer[200];
    sprintf(buffer, "%i/%i",card->power,card->life);
    renderer->FillRect(actX - (12*actZ) , actY + 6* actZ, 25*actZ, 12*actZ, ARGB(((static_cast<unsigned char>(actA))/2),0,0,0));
    mFont->SetColor(ARGB(static_cast<unsigned char>(actA),255,255,255));
    mFont->SetScale(actZ);
    mFont->DrawString(buffer, actX - 10*actZ , actY + 8*actZ);
    mFont->SetScale(1);
  }

  if (tc && !tc->canTarget(card)) {
    shadow->SetColor(ARGB(200,255,255,255));
    renderer->RenderQuad(shadow, actX, actY, actT, 28*actZ/16 + 1, 40*actZ/16);
  }

  PlayGuiObject::Render();
}

JQuad * CardGui::alternateThumbQuad(MTGCard * card){
  JQuad * q;
  int nb_colors = 0;
  for(int i=0;i<Constants::MTG_NB_COLORS;i++){
    if(card->colors[i])
      nb_colors++;
  }

  if(nb_colors > 1){
    q = resources.RetrieveTempQuad("gold_thumb.jpg");
  }
  else{
    switch(card->getColor())
      {
      case Constants::MTG_COLOR_ARTIFACT  : q = resources.RetrieveTempQuad("artifact_thumb.jpg");break;
      case Constants::MTG_COLOR_GREEN: q = resources.RetrieveTempQuad("green_thumb.jpg");break;
      case Constants::MTG_COLOR_BLUE : q = resources.RetrieveTempQuad("blue_thumb.jpg");break;
      case Constants::MTG_COLOR_RED  : q = resources.RetrieveTempQuad("red_thumb.jpg");break;
      case Constants::MTG_COLOR_BLACK: q = resources.RetrieveTempQuad("black_thumb.jpg");break;
      case Constants::MTG_COLOR_WHITE: q = resources.RetrieveTempQuad("white_thumb.jpg");break;
      case Constants::MTG_COLOR_LAND  : q = resources.RetrieveTempQuad("land_thumb.jpg");break;
      default: q = resources.RetrieveTempQuad("gold_thumb.jpg");break;
      }
  }
  if(q && q->mTex)
    q->SetHotSpot(q->mTex->mWidth/2,q->mTex->mHeight/2);
  return q;
}

void CardGui::alternateRender(MTGCard * card, const Pos& pos){
  // Draw the "unknown" card model
  JRenderer * renderer = JRenderer::GetInstance();
  JQuad * q;
  int nb_colors = 0;
  for(int i=0;i<Constants::MTG_NB_COLORS;i++){
    if(card->colors[i])
      nb_colors++;
  }

  if(nb_colors > 1){
    q = resources.RetrieveTempQuad("gold.jpg");
  }
  else{
    switch(card->getColor())
      {
      case Constants::MTG_COLOR_ARTIFACT: q = resources.RetrieveTempQuad("artifact.jpg");break;
      case Constants::MTG_COLOR_GREEN: q = resources.RetrieveTempQuad("green.jpg");break;
      case Constants::MTG_COLOR_BLUE : q = resources.RetrieveTempQuad("blue.jpg");break;
      case Constants::MTG_COLOR_RED  : q = resources.RetrieveTempQuad("red.jpg");break;
      case Constants::MTG_COLOR_BLACK: q = resources.RetrieveTempQuad("black.jpg");break;
      case Constants::MTG_COLOR_WHITE: q = resources.RetrieveTempQuad("white.jpg");break;
      case Constants::MTG_COLOR_LAND: q = resources.RetrieveTempQuad("land.jpg");break;
      default: q = resources.RetrieveTempQuad("gold.jpg");break;
      }
  }
  if(q && q->mTex){
   q->SetHotSpot(q->mTex->mWidth/2,q->mTex->mHeight/2);

    float scale = pos.actZ * 250 / q->mHeight;
    q->SetColor(ARGB((int)pos.actA,255,255,255));
    renderer->RenderQuad(q, pos.actX, pos.actY, pos.actT, scale, scale);
  }
  // Write the title
  JLBFont * font = resources.GetJLBFont("magic");
  float backup_scale = font->GetScale();
  font->SetColor(ARGB((int)pos.actA, 0, 0, 0));
  font->SetScale(0.8 * pos.actZ);

  {
    char name[4096];
    sprintf(name, _(card->getName()).c_str());
    float w = font->GetStringWidth(name) * 0.8 * pos.actZ;
    if (w > BigWidth - 30)
      font->SetScale((BigWidth - 30) / w);
    font->DrawString(name, pos.actX + (22 - BigWidth / 2)*pos.actZ, pos.actY + (25 - BigHeight / 2)*pos.actZ);
  }

  // Write the description
  {
    font->SetScale(0.8 * pos.actZ);
    const std::vector<string> txt = card->formattedText();
    unsigned i = 0;
    for (std::vector<string>::const_iterator it = txt.begin(); it != txt.end(); ++it, ++i)
      font->DrawString(it->c_str(), pos.actX + (22 - BigWidth / 2)*pos.actZ, pos.actY + (-BigHeight/2 + 80 + 11 * i)*pos.actZ);
  }

  // Write the strength
  if (card->isCreature())
    {
      char buffer[32];
      sprintf(buffer, "%i/%i", card->power, card->toughness);
      float w = font->GetStringWidth(buffer) * 0.8;
      font->DrawString(buffer, pos.actX + (65 - w / 2)*pos.actZ, pos.actY + (106)*pos.actZ);
  }

  // Mana
  {
    ManaCost* manacost = card->getManaCost();
    ManaCostHybrid* h;
    unsigned int j = 0;
    unsigned char t = (JGE::GetInstance()->GetTime() / 3) & 0xFF;
    unsigned char v = t + 127;
    float yOffset = -112;
    while ((h = manacost->getHybridCost(j)))
      {
        float scale = pos.actZ * 0.05 * cosf(2*M_PI*((float)t)/256.0);

        if (scale < 0)
          {
            renderer->RenderQuad(manaIcons[h->color1], pos.actX + (-12 * j + 75 + 3 * sinf(2*M_PI*((float)t)/256.0))*pos.actZ, pos.actY + (yOffset + 3 * cosf(2*M_PI*((float)(t-35))/256.0))*pos.actZ, 0, 0.4 + scale, 0.4 + scale);
            renderer->RenderQuad(manaIcons[h->color2], pos.actX + (-12 * j + 75 + 3 * sinf(2*M_PI*((float)v)/256.0))*pos.actZ, pos.actY + (yOffset + 3 * cosf(2*M_PI*((float)(v-35))/256.0))*pos.actZ, 0, 0.4 - scale, 0.4 - scale);
          }
        else
          {
            renderer->RenderQuad(manaIcons[h->color2], pos.actX + (- 12 * j + 75 + 3 * sinf(2*M_PI*((float)v)/256.0))*pos.actZ, pos.actY + (yOffset + 3 * cosf(2*M_PI*((float)(v-35))/256.0))*pos.actZ, 0, 0.4 - scale, 0.4 - scale);
            renderer->RenderQuad(manaIcons[h->color1], pos.actX + (- 12 * j + 75 + 3 * sinf(2*M_PI*((float)t)/256.0))*pos.actZ, pos.actY + (yOffset + 3 * cosf(2*M_PI*((float)(t-35))/256.0))*pos.actZ, 0, 0.4 + scale, 0.4 + scale);
          }
        ++j;
      }
    for (int i = Constants::MTG_NB_COLORS - 2; i >= 1; --i)
      {
        for (int cost = manacost->getCost(i); cost > 0; --cost)
          {
            renderer->RenderQuad(manaIcons[i], pos.actX + (-12*j + 75)*pos.actZ, pos.actY + (yOffset)*pos.actZ, 0, 0.4 * pos.actZ, 0.4 * pos.actZ);
            ++j;
          }
      }
    // Colorless mana
    if (int cost = manacost->getCost(0))
      {
        char buffer[10];
        sprintf(buffer, "%d", cost);
        renderer->RenderQuad(manaIcons[0], pos.actX + (- 12*j + 75)*pos.actZ, pos.actY +(yOffset)*pos.actZ, 0, 0.4 * pos.actZ, 0.4 * pos.actZ);
        float w = font->GetStringWidth(buffer);
        font->DrawString(buffer, pos.actX +(- 12*j + 76 - w/2)*pos.actZ, pos.actY + (yOffset - 5)*pos.actZ);
      }
  }

  //types
  {
    string s = "";
    for (int i = card->nb_types - 1; i > 0; --i)
      {
        s += _(Subtypes::subtypesList->find(card->types[i]));
        s += " - ";
      }
    s += _(Subtypes::subtypesList->find(card->types[0]));
    font->DrawString(s.c_str(), pos.actX + (22 - BigWidth / 2)*pos.actZ, pos.actY + (49 - BigHeight / 2)*pos.actZ);
  }

  //expansion and rarity
  font->SetColor(ARGB((int)pos.actA, 0, 0, 0));
  {
    char buf[512];
    switch(card->getRarity()){
      case Constants::RARITY_M:
      case Constants::RARITY_R:
        sprintf(buf,"%s Rare",MtgSets::SetsList->values[card->setId].c_str());
        break;
      case Constants::RARITY_U:
        sprintf(buf,"%s Uncommon",MtgSets::SetsList->values[card->setId].c_str());
        break;
      case Constants::RARITY_C:
        sprintf(buf,"%s Common",MtgSets::SetsList->values[card->setId].c_str());
        break;
      default:
        sprintf(buf,"%s",MtgSets::SetsList->values[card->setId].c_str());
        break;
    }
    
    switch(card->getColor())
    {
    case Constants::MTG_COLOR_BLACK: 
    case Constants::MTG_COLOR_LAND: 
      font->SetColor(ARGB((int)pos.actA,255,255,255));
      font->DrawString(buf, pos.actX + (22 - BigWidth / 2)*pos.actZ, pos.actY + (BigHeight / 2 - 30)*pos.actZ);
      break;
    default:
      font->DrawString(buf, pos.actX + (22 - BigWidth / 2)*pos.actZ, pos.actY + (BigHeight / 2 - 30)*pos.actZ);
      break; //Leave black
    }
    
  }

  font->SetScale(backup_scale);
}

void CardGui::alternateRenderBig(const Pos& pos){
  alternateRender(card,pos);
}

void CardGui::RenderBig(MTGCard* card, const Pos& pos){
  JRenderer * renderer = JRenderer::GetInstance();

  JQuad * quad = resources.RetrieveCard(card);
  if (quad){
    quad->SetColor(ARGB((int)pos.actA,255,255,255));
    float scale = pos.actZ * 257.f / quad->mHeight;
    renderer->RenderQuad(quad, pos.actX, pos.actY, pos.actT, scale, scale);
    return;
  }

  JQuad * q;
  if ((q = resources.RetrieveCard(card,CACHE_THUMB)))
    {
      float scale = pos.actZ * 250 / q->mHeight;
      q->SetColor(ARGB((int)pos.actA,255,255,255));
      renderer->RenderQuad(q, pos.actX, pos.actY, pos.actT, scale, scale);
      return;
    }

  // If we come here, we do not have the picture.
  alternateRender(card,pos);
}

void CardGui::RenderBig(const Pos& pos){
  RenderBig(card,pos);
}



MTGCardInstance* CardView::getCard() { return card; }

TransientCardView::TransientCardView(MTGCardInstance* card, float x, float y) : CardGui(card, x, y){}
TransientCardView::TransientCardView(MTGCardInstance* card, const Pos& ref) : CardGui(card, ref) {};

ostream& CardView::toString(ostream& out) const
{
  return (CardGui::toString(out) << " : CardView ::: card : " << card
          << ";  actX,actY : " << actX << "," << actY << "; t : " << t
          << " ; actT : " << actT);
}
ostream& CardGui::toString(ostream& out) const
{
  return (out << "CardGui ::: x,y " << x << "," << y);
}
