#include "PrecompiledHeader.h"

#include "CardSelectorSingleton.h"

#include "DuelLayers.h"
#include "Navigator.h"

/*
** 
*/
namespace CardSelectorSingleton
{
  static CardSelectorBase* sCardSelectorInstance = NULL;

  CardSelectorBase* Create(DuelLayers* inDuelLayers)
  {
    if (sCardSelectorInstance == NULL)
      sCardSelectorInstance = NEW CardSelector(inDuelLayers);
      //sCardSelectorInstance = NEW Navigator(inDuelLayers);

    return sCardSelectorInstance;
  }

  CardSelectorBase* Instance()
  {
    assert(sCardSelectorInstance);
    return sCardSelectorInstance;
  }

  void Terminate()
  {
    SAFE_DELETE(sCardSelectorInstance);
    sCardSelectorInstance = NULL;
  }
}

