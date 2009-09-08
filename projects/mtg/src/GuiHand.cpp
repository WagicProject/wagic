#include "../include/config.h"
#include "../include/GameApp.h"
#include "../include/Trash.h"
#include "../include/GuiHand.h"

const float GuiHand::ClosedRowX = 459;
const float GuiHand::LeftRowX = 420;
const float GuiHand::RightRowX = 460;

const float GuiHand::OpenX = 394;
const float GuiHand::ClosedX = 494;


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
}

GuiHandSelf::~GuiHandSelf(){
  SAFE_DELETE(limitor);
}

void GuiHandSelf::Repos()
{
  float y = 48.0;
  if (Closed == state)
    for (vector<CardView*>::iterator it = cards.begin(); it != cards.end(); ++it)
      {
        (*it)->x = ClosedRowX; (*it)->y = y;
        y += 20;
      }
  else
    {
      bool flip = false;
      for (vector<CardView*>::iterator it = cards.begin(); it != cards.end(); ++it)
        {
          (*it)->x = flip ? RightRowX : LeftRowX;
          (*it)->y = y;
          if (flip) y += 65;
          flip = !flip;
        }
    }
}

bool GuiHandSelf::CheckUserInput(u32 key)
{
  //u32 trigger = options[REVERSE_TRIGGERS];
  u32 trigger = PSP_CTRL_LTRIGGER;
  if (trigger == key)
    {
      state = (Open == state ? Closed : Open);
      cs->Limit(Open == state ? limitor : NULL);
      backpos.x = Open == state ? OpenX : ClosedX;
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
  backpos.Render(back);
  for (vector<CardView*>::iterator it = cards.begin(); it != cards.end(); ++it)
    (*it)->Render();
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
              Repos();
	      cards.erase(it);
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
	  card = NEW CardView(event->card, *(event->card->view));
	else
	  card = NEW CardView(event->card, ClosedRowX, 0);
	card->t = -4*M_PI; card->alpha = 255;
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
