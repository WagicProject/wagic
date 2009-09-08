#ifndef _CARDSELECTOR_H_
#define _CARDSELECTOR_H_

#include <vector>
#include "GuiLayers.h"
#include "DuelLayers.h"
#include "Pos.h"

using std::vector;

class PlayGuiObject;
class DuelLayers;

template <typename T>
struct LimitorFunctor
{
  virtual bool select(T*) = 0;
  virtual bool greyout(T*) = 0;
  typedef T Target;
};

template <typename T=PlayGuiObject>
class ObjectSelector : public GuiLayer
{
 protected:
 vector<T*> cards;
 T* active;
 bool showBig;
 DuelLayers* duel;
 LimitorFunctor<T>* limitor;
 Pos bigpos;

 T* handLast; T* playLast;

 public:
 ObjectSelector(DuelLayers*);
 void Add(T*);
 void Remove(T*);
 bool CheckUserInput(u32 key);
 void Update(float dt);
 void Render();
 void Limit(LimitorFunctor<T>* limitor);

 typedef T Target;
};

typedef ObjectSelector<> CardSelector;
typedef LimitorFunctor<CardSelector::Target> Limitor;

struct Exp
{
  static inline bool test(CardSelector::Target*, CardSelector::Target*);
};

#endif
