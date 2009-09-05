#include <iostream>
#include "../include/MTGDefinitions.h"
#include "../include/Pos.h"
#include "../include/CardGui.h"
#include "../include/Trash.h"

void Trash::put_out()
{
  for (std::vector<Pos*>::iterator it = bin.begin(); it != bin.end(); ++it)
    {
      std::cout << "DELETE " << *it << std::endl;
      if (CardView *c = dynamic_cast<CardView*>(*it))
        SAFE_DELETE(c);
      else
        SAFE_DELETE(*it);
    }
  bin.clear();
}

static Trash PosTrash;

void Trash::cleanup()
{
  PosTrash.put_out();
}

void trash(Pos* garbage)
{
  PosTrash.bin.push_back(garbage);
}
