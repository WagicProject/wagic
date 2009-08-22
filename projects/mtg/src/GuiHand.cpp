#include "../include/config.h"
#include "../include/GameApp.h"
#include "../include/GuiHand.h"

bool HandLimitor::select(Target* t)
{
  vector<CardView*>::iterator it;
  it = find(hand->cards.begin(), hand->cards.end(), t);
  return (it != hand->cards.end());
}
bool HandLimitor::greyout(Target* t)
{
  return true;
}
HandLimitor::HandLimitor(GuiHand* hand) : hand(hand) {}

GuiHand::GuiHand(CardSelector* cs, MTGHand* hand) : GuiLayer(), hand(hand), cs(cs)
{
  JTexture* texture = GameApp::CommonRes->GetTexture("graphics/handback.png");
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
}

void GuiHand::Update(float dt)
{
  for (vector<CardView*>::iterator it = cards.begin(); it != cards.end(); ++it)
    (*it)->Update(dt);
}

GuiHandOpponent::GuiHandOpponent(CardSelector* cs, MTGHand* hand) : GuiHand(cs, hand) {}

void GuiHandOpponent::Render()
{
  JQuad * quad = GameApp::CommonRes->GetQuad("back_thumb");

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

GuiHandSelf::GuiHandSelf(CardSelector* cs, MTGHand* hand) : GuiHand(cs, hand), state(Closed), backpos(ClosedX)
{
  limitor = new HandLimitor(this);
}
bool GuiHandSelf::CheckUserInput(u32 key)
{
  //u32 trigger = options[REVERSE_TRIGGERS];
  u32 trigger = PSP_CTRL_LTRIGGER;
  if (trigger == key)
    {
      state = (Open == state ? Closed : Open);
      cs->Limit(Open == state ? limitor : NULL);
      return true;
    }
  return false;
}

void GuiHandSelf::Render()
{
  if (Closed == state)
    {
      backpos += (ClosedX - backpos) / 100;
      JRenderer::GetInstance()->RenderQuad(back, backpos, SCREEN_HEIGHT - 250);
      float y = 48.0;
      for (vector<CardView*>::iterator it = cards.begin(); it != cards.end(); ++it)
	{
	  (*it)->x += (ClosedRowX - (*it)->x) / 100;
	  (*it)->y += (y - (*it)->y) / 70;
	  (*it)->Render();
	  y += 20;
	}
    }
  else
    {
      backpos += (OpenX - backpos) / 100;
      JRenderer::GetInstance()->RenderQuad(back, backpos, SCREEN_HEIGHT - 250);
      bool flip = false;
      float y = 48.0;
      for (vector<CardView*>::iterator it = cards.begin(); it != cards.end(); ++it)
	{
	  if (flip)
	    (*it)->x += (RightRowX - (*it)->x) / 100;
	  else
	    (*it)->x += (LeftRowX - (*it)->x) / 100;
	  // I wanna write it like that. GCC doesn't want me to without -O.
	  // I'm submitting a bug report.
	  //	  it->x = (it->x + (flip ? RightRowX : LeftRowX)) / 2;
	  (*it)->y += (y - (*it)->y) / 70;
	  (*it)->Render();
	  if (flip) y += 65;
	  flip = !flip;
	}
    }
}

float GuiHandSelf::LeftBoundary()
{
  float min = SCREEN_WIDTH + 10;
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
	  card = NEW CardView(event->card, *(event->card->view));
	else
	  card = NEW CardView(event->card, ClosedRowX, 0);
	card->t = 6*M_PI;
	cards.push_back(card);
	cs->Add(card);
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
	      delete cv;
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
	  card = NEW CardView(event->card, *(event->card->view));
	else
	  card = NEW CardView(event->card, ClosedRowX, 0);
	card->t = -4*M_PI;
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
	      delete cv;
	      return 1;
	    }
      return 1;
    }
  return 0;
}
