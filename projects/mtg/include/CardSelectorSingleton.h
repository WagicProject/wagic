#ifndef CARDSELECTORSINGLETON_H
#define CARDSELECTORSINGLETON_H

#include "CardSelector.h"

class DuelLayers;

namespace CardSelectorSingleton
{
  /*
  ** CardSelector is essentially a singleton in its usage
  ** It's not enforced, but it needs to eventually migrate to the real thing
  ** For now, this function will fake it out - it's up to the client caller to make sure
  ** that this gets destroyed via a Terminate call (this is currently handled in DualLayers's destructor)
  */
  CardSelectorBase* Instance();

  /*
  ** Create the singleton pointer. Instance() isn't valid until this is called.
  */
  CardSelectorBase* Create(DuelLayers* inDuelLayers);

  /*
  ** Teardown the singleton pointer instance.
  */
  void Terminate();
}


#endif //CARDSELECTORSINGLETON_H
