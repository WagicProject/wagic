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

void CardGui::Update(float dt)
{
  PlayGuiObject::Update(dt);
}


void CardGui::Render()
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
    //MTGCard * mtgcard = card->model;
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

    JQuad* q = alternateThumbQuad(card);
    if (q){
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

JQuad * CardGui::alternateThumbQuad(MTGCard * card){
  JRenderer * renderer = JRenderer::GetInstance();
  JQuad * q;
  switch(card->getColor())
    {
  case Constants::MTG_COLOR_GREEN: q = cache.getQuad("sets/green_thumb.jpg");break;
    case Constants::MTG_COLOR_BLUE : q = cache.getQuad("sets/blue_thumb.jpg");break;
    case Constants::MTG_COLOR_RED  : q = cache.getQuad("sets/red_thumb.jpg");break;
    case Constants::MTG_COLOR_BLACK: q = cache.getQuad("sets/black_thumb.jpg");break;
    case Constants::MTG_COLOR_WHITE: q = cache.getQuad("sets/white_thumb.jpg");break;
    default: q = cache.getQuad("sets/black_thumb.jpg");break;
    }
  return q;
}

void CardGui::alternateRender(MTGCard * card, const Pos& pos){
  // Draw the "unknown" card model
  JRenderer * renderer = JRenderer::GetInstance();
  JQuad * q;
  switch(card->getColor())
    {
  case Constants::MTG_COLOR_GREEN: q = cache.getQuad("sets/green.jpg");break;
    case Constants::MTG_COLOR_BLUE : q = cache.getQuad("sets/blue.jpg");break;
    case Constants::MTG_COLOR_RED  : q = cache.getQuad("sets/red.jpg");break;
    case Constants::MTG_COLOR_BLACK: q = cache.getQuad("sets/black.jpg");break;
    case Constants::MTG_COLOR_WHITE: q = cache.getQuad("sets/white.jpg");break;
    default: q = cache.getQuad("sets/black.jpg");break;
    }
  float scale = pos.actZ * 250 / q->mHeight;
  q->SetColor(ARGB((int)pos.actA,255,255,255));
  renderer->RenderQuad(q, pos.actX, pos.actY, pos.actT, scale, scale);

  // Write the title
  JLBFont * font = GameApp::CommonRes->GetJLBFont("magic");
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
  //MTGCard * mtgcard = card->model;
  alternateRender(card,pos);
}



MTGCardInstance* CardView::getCard() { return card; }

TransientCardView::TransientCardView(MTGCardInstance* card, float x, float y) : CardGui(card, x, y){}
TransientCardView::TransientCardView(MTGCardInstance* card, const Pos& ref) : CardGui(card, ref) {};

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
