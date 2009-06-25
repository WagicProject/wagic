#include "../include/config.h"
#include "../include/ConstraintResolver.h"


int ConstraintResolver::untap(GameObserver * game, MTGCardInstance * card){
  if (!card->isUntapping()){
    return 0;
  }
  int ok = 1;
  ManaCost * untapManaCost = NEW ManaCost();
  Blockers * blockers = card->getBlockers();
  Blocker * blocker;
  blockers->rewind();
  Player * player = game->currentPlayer;
  while ((blocker = blockers->next())){
#if defined (WIN32) || defined (LINUX)
    char buf[4096];
    sprintf(buf, "next\n");
    OutputDebugString(buf);
#endif
    untapManaCost->add(blocker->untapManaCost());
  }
  if (player->getManaPool()->canAfford(untapManaCost)){
    blockers->rewind();
    while ((blocker = blockers->next())){
      if (!blocker->unblock()){
	ok = 0;
	break;
      }
    }
  }else{
    ok = 0;
  }

  if (ok) {
    player->getManaPool()->pay(untapManaCost);
    card->attemptUntap();
  }
  delete untapManaCost;
  return ok;
}
