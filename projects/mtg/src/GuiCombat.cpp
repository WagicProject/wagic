#include "../include/config.h"
#include "../include/GameApp.h"
#include "../include/GuiCombat.h"

GuiCombat::GuiCombat(CardSelector* cs) : GuiLayer(), cs(cs)
{
}

GuiCombat::~GuiCombat()
{
}

void GuiCombat::Update(float dt)
{
}

bool GuiCombat::CheckUserInput(u32 key)
{
  return true;
}

void GuiCombat::Render()
{
}

int GuiCombat::receiveEventPlus(WEvent* e)
{
  if (WEventCreatureAttacker* event = dynamic_cast<WEventCreatureAttacker*>(e))
    {
      if (NULL == event->after) return 0;
      cout << "Attacker : " << event->card->name << event->before << " -> " << event->after << endl;
      attackers.push_back(event->card);
      return 1;
    }
  else if (WEventCreatureBlocker* event = dynamic_cast<WEventCreatureBlocker*>(e))
    {
      if (NULL == event->after) return 0;
      cout << "Blocker : " << event->card->name << event->before << " -> " << event->after << endl;
      return 1;
    }
  else if (WEventCreatureBlockerRank* event = dynamic_cast<WEventCreatureBlockerRank*>(e))
    {
      cout << "Order : " << event->card->name << event->exchangeWith << " -> " << event->attacker << endl;
      return 1;
    }
  return 0;
}
int GuiCombat::receiveEventMinus(WEvent* e)
{
  // (WEventZoneChange* event = dynamic_cast<WEventZoneChange*>(e)
  if (WEventCreatureAttacker* event = dynamic_cast<WEventCreatureAttacker*>(e))
    {
      if (NULL == event->before) return 0;
      vector<MTGCardInstance*>::iterator it = find(attackers.begin(), attackers.end(), event->card);
      if (it != attackers.end())
	attackers.erase(it);
      return 1;
    }
  return 0;
}
