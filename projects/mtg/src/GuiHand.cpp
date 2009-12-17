#include "../include/config.h"
#include "../include/GameApp.h"
#include "../include/Trash.h"
#include "../include/GuiHand.h"
#include "../include/OptionItem.h"

const float GuiHand::ClosedRowX = 459;
const float GuiHand::LeftRowX = 420;
const float GuiHand::RightRowX = 460;

const float GuiHand::OpenX = 394;
const float GuiHand::ClosedX = 494;
const float GuiHand::OpenY = SCREEN_HEIGHT - 50;
const float GuiHand::ClosedY = SCREEN_HEIGHT;

bool HandLimitor::select(Target* t)
{
  if (CardView* c = dynamic_cast<CardView*>(t))
    return hand->isInHand(c);
  else return false;
}
bool HandLimitor::greyout(Target* t)
{
  return true;
}
HandLimitor::HandLimitor(GuiHand* hand) : hand(hand) {}

GuiHand::GuiHand(CardSelector* cs, MTGHand* hand) : GuiLayer(), hand(hand), cs(cs)
{
  JTexture* texture = resources.GetTexture("handback.png");
  if (texture)
    {
      back = NEW JQuad(texture, 0, 0, 101, 250);
      back->SetTextureRect(1, 0, 100, 250);
    }
  else
    {
      back = NULL;
      GameApp::systemError = "Error loading hand texture : " __FILE__;
    }
}

GuiHand::~GuiHand()
{
  delete(back);
  for (vector<CardView*>::iterator it = cards.begin(); it != cards.end(); ++it)
    delete(*it);
}

void GuiHand::Update(float dt)
{
  for (vector<CardView*>::iterator it = cards.begin(); it != cards.end(); ++it)
    (*it)->Update(dt);
}

bool GuiHand::isInHand(CardView* card)
{
  vector<CardView*>::iterator it;
  it = find(cards.begin(), cards.end(), card);
  return (it != cards.end());
}

GuiHandOpponent::GuiHandOpponent(CardSelector* cs, MTGHand* hand) : GuiHand(cs, hand) {}

void GuiHandOpponent::Render()
{
  JQuad * quad = resources.GetQuad("back_thumb");

  float x = 45;
  for (vector<CardView*>::iterator it = cards.begin(); it != cards.end(); ++it)
    {
      (*it)->x = x;
      (*it)->y = 2;
      (*it)->zoom = 0.3;
      (*it)->Render(quad);
      x += 18;
    }
}

GuiHandSelf::GuiHandSelf(CardSelector* cs, MTGHand* hand) : GuiHand(cs, hand), state(Closed), backpos(ClosedX, SCREEN_HEIGHT - 250, 1.0, 0, 255)
{
  limitor = NEW HandLimitor(this);
  if (OptionHandDirection::HORIZONTAL == options[Options::HANDDIRECTION].number)
    {
      backpos.t = M_PI/2;
      backpos.y = ClosedY;
      backpos.x = SCREEN_WIDTH - 30 * 7 - 14;
      backpos.UpdateNow();
    }
}

GuiHandSelf::~GuiHandSelf(){
  SAFE_DELETE(limitor);
}

void GuiHandSelf::Repos()
{
  float y = 48.0;
  if (Closed == state && OptionClosedHand::VISIBLE == options[Options::CLOSEDHAND].number)
    {
      float dist = 180.0 / cards.size(); if (dist > 20) dist = 20.0; else y = 40.0;
      cout << dist << endl;
      for (vector<CardView*>::iterator it = cards.begin(); it != cards.end(); ++it)
        {
          (*it)->x = ClosedRowX; (*it)->y = y;
          y += dist;
        }
    }
  else
    {
      bool q = (Closed == state);
      if (OptionHandDirection::HORIZONTAL == options[Options::HANDDIRECTION].number)
        {
          y = SCREEN_WIDTH - 30;
          float dist = 240.0 / cards.size(); if (dist > 30) dist = 30; else y = SCREEN_WIDTH - 15;
          for (vector<CardView*>::reverse_iterator it = cards.rbegin(); it != cards.rend(); ++it)
            {
              (*it)->x = y;
              (*it)->y = SCREEN_HEIGHT - 30;
              y -= dist;
              (*it)->alpha = (q ? 0 : 255);
            }
          backpos.x = y + SCREEN_HEIGHT - 14;
        }
      else
        {
          float dist = 224.0 / ((cards.size() + 1) / 2); if (dist > 65) dist = 65;
          bool flip = false;
          for (vector<CardView*>::iterator it = cards.begin(); it != cards.end(); ++it)
            {
              (*it)->x = flip ? RightRowX : LeftRowX;
              (*it)->y = y;
              if (flip) y += dist;
              flip = !flip;
              (*it)->alpha = (q ? 0 : 255);
            }
        }
    }
}

bool GuiHandSelf::CheckUserInput(u32 key)
{
  u32 trigger = (options[Options::REVERSETRIGGERS].number ? PSP_CTRL_LTRIGGER : PSP_CTRL_RTRIGGER);
  if (trigger == key)
    {
      state = (Open == state ? Closed : Open);
      if (Open == state) cs->Push();
      cs->Limit(Open == state ? limitor : NULL, CardSelector::handZone);
      if (Closed == state) cs->Pop();
      if (OptionHandDirection::HORIZONTAL == options[Options::HANDDIRECTION].number)
        backpos.y = Open == state ? OpenY : ClosedY;
      else
        backpos.x = Open == state ? OpenX : ClosedX;
      if (Open == state && OptionClosedHand::INVISIBLE == options[Options::CLOSEDHAND].number)
        {
          if (OptionHandDirection::HORIZONTAL == options[Options::HANDDIRECTION].number)
            for (vector<CardView*>::iterator it = cards.begin(); it != cards.end(); ++it)
              {
                (*it)->y = SCREEN_HEIGHT + 30; (*it)->UpdateNow();
              }
          else
            for (vector<CardView*>::iterator it = cards.begin(); it != cards.end(); ++it)
              {
                (*it)->x = SCREEN_WIDTH + 30; (*it)->UpdateNow();
              }
        }
      Repos();
      return true;
    }
  return false;
}


void GuiHandSelf::Update(float dt)
{
  backpos.Update(dt);
  GuiHand::Update(dt);
}

void GuiHandSelf::Render()
{
  //Empty hand
  if (state == Open && cards.size() == 0){
    JLBFont * mFont  = resources.GetJLBFont(Constants::MAIN_FONT);
    mFont->SetColor(ARGB(255,255,0,0));
    if (OptionHandDirection::HORIZONTAL == options[Options::HANDDIRECTION].number){
      back->SetColor(ARGB(255,255,0,0));
      JRenderer::GetInstance()->RenderQuad(back,backpos.actX, backpos.actY, backpos.actT, backpos.actZ, backpos.actZ);
      back->SetColor(ARGB(255,255,255,255));
      mFont->DrawString("0",SCREEN_WIDTH - 10,backpos.actY);
    }
    else
      backpos.Render(back);
    return;
  }

  backpos.Render(back);
  if (OptionClosedHand::VISIBLE == options[Options::CLOSEDHAND].number || state == Open)
    for (vector<CardView*>::iterator it = cards.begin(); it != cards.end(); ++it)
      (*it)->Render();
}

float GuiHandSelf::LeftBoundary()
{
  float min = SCREEN_WIDTH + 10;
  if (OptionClosedHand::VISIBLE == options[Options::CLOSEDHAND].number || state == Open)
    for (vector<CardView*>::iterator it = cards.begin(); it != cards.end(); ++it)
      if ((*it)->x - CardGui::Width / 2 < min) min = (*it)->x - CardGui::Width / 2;
  return min;
}

int GuiHandSelf::receiveEventPlus(WEvent* e)
{
  if (WEventZoneChange* event = dynamic_cast<WEventZoneChange*>(e))
    if (hand == event->to)
      {
	CardView* card;
	if (event->card->view)
	  card = NEW CardView(CardSelector::handZone, event->card, *(event->card->view));
	else
	  card = NEW CardView(CardSelector::handZone, event->card, ClosedRowX, 0);
	card->t = 6*M_PI;
	cards.push_back(card);
	cs->Add(card);
        Repos();
	return 1;
      }
  return 0;
}
int GuiHandSelf::receiveEventMinus(WEvent* e)
{
  if (WEventZoneChange* event = dynamic_cast<WEventZoneChange*>(e))
    {
      if (hand == event->from)
	for (vector<CardView*>::iterator it = cards.begin(); it != cards.end(); ++it)
	  if (event->card->previous == (*it)->card)
	    {
	      CardView* cv = *it;
	      cs->Remove(cv);
	      cards.erase(it);
              Repos();
              trash(cv);
	      return 1;
	    }
      return 1;
    }
  return 0;
}

int GuiHandOpponent::receiveEventPlus(WEvent* e)
{
  if (WEventZoneChange* event = dynamic_cast<WEventZoneChange*>(e))
    if (hand == event->to)
      {
	CardView* card;
	if (event->card->view)
	  card = NEW CardView(CardSelector::handZone, event->card, *(event->card->view));
	else
	  card = NEW CardView(CardSelector::handZone, event->card, ClosedRowX, 0);
        card->alpha = 255; card->t = -4*M_PI;
	cards.push_back(card);
	return 1;
      }
  return 0;
}
int GuiHandOpponent::receiveEventMinus(WEvent* e)
{
  if (WEventZoneChange* event = dynamic_cast<WEventZoneChange*>(e))
    {
      if (hand == event->from)
	for (vector<CardView*>::iterator it = cards.begin(); it != cards.end(); ++it)
	  if (event->card->previous == (*it)->card)
	    {
	      CardView* cv = *it;
	      cards.erase(it);
              trash(cv);
	      return 1;
	    }
      return 0;
    }
  return 0;
}

// I wanna write it like that. GCC doesn't want me to without -O.
// I'm submitting a bug report.
//	  it->x = (it->x + (flip ? RightRowX : LeftRowX)) / 2;
