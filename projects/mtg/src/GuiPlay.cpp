#include "../include/config.h"
#include "../include/GameApp.h"
#include "../include/GuiPlay.h"

#define CARD_WIDTH (31)

const float GuiPlay::HORZWIDTH = 300.0f;
const float GuiPlay::VERTHEIGHT = 80.0f;

void GuiPlay::CardStack::reset(float x, float y)
{
  this->x = 0; baseX = x;
  this->y = 0; baseY = y;
}

void GuiPlay::CardStack::RenderSpell(MTGCardInstance* card, iterator begin, iterator end, float x, float y)
{
  while (begin != end)
    {
      if ((*begin)->card->target == card)
	{
	  RenderSpell(card, begin+1, end, x, y - 10);
	  (*begin)->x = x; (*begin)->y = y;
	  (*begin)->Render();
	  return;
	}
      ++begin;
    }
}

GuiPlay::HorzStack::HorzStack(float width) : maxWidth(width) {}
GuiPlay::VertStack::VertStack(float height) : maxHeight(height) {}

void GuiPlay::HorzStack::reset(float x, float y)
{
  GuiPlay::CardStack::reset(x, y);
  maxHeight = 0;
}

void GuiPlay::HorzStack::Render(CardView* card, iterator begin, iterator end)
{
  RenderSpell(card->card, begin, end, card->x, card->y - 10);
  card->Render();
}

void GuiPlay::HorzStack::Enstack(CardView* card)
{
  card->x = x + baseX; card->y = y + baseY;
  x += CARD_WIDTH;
  if (maxHeight < card->mHeight) maxHeight = card->mHeight;
  if (x > maxWidth) { x = 0; y += maxHeight + 2; maxHeight = 0; }
}

void GuiPlay::VertStack::Enstack(CardView* card)
{
  if (y > maxHeight) x += CARD_WIDTH;
  card->x = x + baseX; card->y = y + baseY;
  y += 8;
}

inline float GuiPlay::VertStack::nextX() { return x + CARD_WIDTH; }

GuiPlay::BattleField::BattleField(float width) : HorzStack(width), attackers(0), blockers(0), height(0.0) {}
const float GuiPlay::BattleField::HEIGHT = 80.0f;
void GuiPlay::BattleField::addAttacker(MTGCardInstance*) { ++attackers; }
void GuiPlay::BattleField::removeAttacker(MTGCardInstance*) { --attackers; }
void GuiPlay::BattleField::reset(float x, float y) { HorzStack::reset(x, y); currentAttacker = 1; }
void GuiPlay::BattleField::EnstackAttacker(CardView* card)
{
  GameObserver* game = GameObserver::GetInstance();
  card->x = currentAttacker * (HORZWIDTH-20) / (attackers + 1); card->y = baseY + (game->players[0] == card->card->controller() ? 20 + y : -20 - y);
  ++currentAttacker;
  //  JRenderer::GetInstance()->RenderQuad(resources.GetQuad("BattleIcon"), card->actX, card->actY, 0, 0.5 + 0.1 * sinf(JGE::GetInstance()->GetTime()), 0.5 + 0.1 * sinf(JGE::GetInstance()->GetTime()));
}
void GuiPlay::BattleField::EnstackBlocker(CardView* card)
{
  GameObserver* game = GameObserver::GetInstance();
  if (card->card && card->card->defenser && card->card->defenser->view) card->x = card->card->defenser->view->x; 
  card->y = baseY + (game->players[0] == card->card->controller() ? 20 + y : -20 - y);
}
void GuiPlay::BattleField::Update(float dt)
{
  if (0 == attackers)
    height -= 10 * dt * height;
  else
    height += 10 * dt * (HEIGHT - height);
}
void GuiPlay::BattleField::Render()
{
  if (height > 3)
    JRenderer::GetInstance()->FillRect(22, SCREEN_HEIGHT / 2 + 10 - height / 2, 250, height, ARGB(127, 0, 0, 0));
}

GuiPlay::GuiPlay(GameObserver* game, CardSelector* cs) : game(game), cs(cs)
{
  end_spells = cards.end();
}

GuiPlay::~GuiPlay()
{
  for (iterator it = cards.begin(); it != cards.end(); ++it){
    delete(*it);
  }
}

bool isSpell(CardView* c) { return c->card->isSpell(); }
void GuiPlay::Replace()
{
  opponentSpells.reset(18, 80);
  selfSpells.reset(18, 200);
  end_spells = stable_partition(cards.begin(), cards.end(), &isSpell);

  for (iterator it = cards.begin(); it != end_spells; ++it)
    if (!(*it)->card->target)
      {
	if (game->players[0] == (*it)->card->controller()) selfSpells.Enstack(*it);
	else opponentSpells.Enstack(*it);
      }

  float x = 24 + MAX(opponentSpells.nextX(), selfSpells.nextX());
  opponentLands.reset(x, 50);
  opponentCreatures.reset(x, 95);
  battleField.reset(x, 145);
  selfCreatures.reset(x, 195);
  selfLands.reset(x, 240);

  for (iterator it = end_spells; it != cards.end(); ++it)
    {
      if ((*it)->card->isCreature())
	{
	  if ((*it)->card->isAttacker()) battleField.EnstackAttacker(*it);
	  else if ((*it)->card->isDefenser()) battleField.EnstackBlocker(*it);
	  else if (game->players[0] == (*it)->card->controller()) selfCreatures.Enstack(*it);
	  else opponentCreatures.Enstack(*it);
	}
      else if ((*it)->card->isLand())
	{
	  if (game->players[0] == (*it)->card->controller()) selfLands.Enstack(*it);
	  else opponentLands.Enstack(*it);
	}
    }
}

void GuiPlay::Render()
{
  battleField.Render();
  for (iterator it = cards.begin(); it != end_spells; ++it)
    if (!(*it)->card->target)
      (*it)->Render();
  for (iterator it = end_spells; it != cards.end(); ++it)
    if ((*it)->card->isCreature())
      {
	if (game->players[0] == (*it)->card->controller()) selfCreatures.Render(*it, cards.begin(), end_spells);
	else opponentCreatures.Render(*it, cards.begin(), end_spells);
      }
    else if ((*it)->card->isLand())
      {
	if (game->players[0] == (*it)->card->controller()) selfLands.Render(*it, cards.begin(), end_spells);
	else opponentLands.Render(*it, cards.begin(), end_spells);
      }
}
void GuiPlay::Update(float dt)
{
  battleField.Update(dt);
  for (iterator it = cards.begin(); it != cards.end(); ++it)
    (*it)->Update(dt);
}

int GuiPlay::receiveEventPlus(WEvent * e)
{
  if (WEventZoneChange *event = dynamic_cast<WEventZoneChange*>(e))
    {
      if ((game->players[0]->inPlay() == event->to) ||
	  (game->players[1]->inPlay() == event->to))
	{
	  CardView * card;
	  if (event->card->view)
	    card = NEW CardView(event->card, *(event->card->view));
	  else
	    card = NEW CardView(event->card, 0, 0);
	  cards.push_back(card);
	  card->t = 0; card->alpha = 255;
	  cs->Add(card);
	  Replace();
	  return 1;
	}
    }
  else if (WEventCreatureAttacker* event = dynamic_cast<WEventCreatureAttacker*>(e))
    {
      if (NULL != event->after)
	battleField.addAttacker(event->card);
      else if (NULL != event->before)
	battleField.removeAttacker(event->card);
      Replace();
    }
  else if (dynamic_cast<WEventCreatureBlocker*>(e))
    {
      Replace();
    }
  else if (WEventCardTap* event = dynamic_cast<WEventCardTap*>(e))
    {
      if (CardView* cv = dynamic_cast<CardView*>(event->card->view))
	cv->t = event->after ? M_PI / 2 : 0;
      else
	event->card->view->actT = event->after ? M_PI / 2 : 0;
      return 1;
    }
  return 0;
}
int GuiPlay::receiveEventMinus(WEvent * e)
{
  if (WEventZoneChange *event = dynamic_cast<WEventZoneChange*>(e))
    {
      if ((game->players[0]->inPlay() == event->from) ||
	  (game->players[1]->inPlay() == event->from))
	for (iterator it = cards.begin(); it != cards.end(); ++it)
	  if (event->card->previous == (*it)->card)
	    {
	      CardView* cv = *it;
	      cs->Remove(cv);
	      cards.erase(it);
              cv->card->view = NULL;
	      delete cv;
	      Replace();
	      return 1;
	    }
    }
  return 0;
}
