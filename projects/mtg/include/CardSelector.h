#ifndef _CARDSELECTOR_H_
#define _CARDSELECTOR_H_

#include <vector>
#include <stack>
#include "GuiLayers.h"
#include "Pos.h"

using std::vector;

class PlayGuiObject;
class DuelLayers;

enum {
  BIG_MODE_SHOW = 0,
  BIG_MODE_TEXT = 1,
  BIG_MODE_HIDE = 2,
  NB_BIG_MODES = 3
};

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
public:
  typedef enum {
    nullZone, handZone, playZone
  } SelectorZone;
  struct SelectorMemory
  {
    T* object;
    float x, y;
    SelectorMemory(T* object) : object(object) { if (object) { x = object->x; y = object->y; } };
    SelectorMemory() { object = NULL; x = y = 0; };
  };

protected:
  vector<T*> cards;
  T* active;
  DuelLayers* duel;
  LimitorFunctor<T>* limitor;
  Pos bigpos;
  map<const SelectorZone, SelectorMemory> lasts;
  stack< pair<LimitorFunctor<T>*, SelectorZone> > limitorStack;
  stack<SelectorMemory> memoryStack;

  T* fetchMemory(SelectorMemory&);

public:
  ObjectSelector(DuelLayers*);
  int bigMode;
  void Add(T*);
  void Remove(T*);
  bool CheckUserInput(JButton key);
  bool CheckUserInput(int x, int y);
  void Update(float dt);
  void Render();
  void Push();
  void Pop();

  void Limit(LimitorFunctor<T>* limitor, SelectorZone);
  void PushLimitor();
  void PopLimitor();

  typedef T Target;
};

typedef ObjectSelector<> CardSelector;
typedef LimitorFunctor<CardSelector::Target> Limitor;


namespace CardSelectorSingleton
{
  /*
  ** CardSelector is essentially a singleton in its usage
  ** It's not enforced, but it needs to eventually migrate to the real thing
  ** For now, this function will fake it out - it's up to the client caller to make sure
  ** that this gets destroyed via a Terminate call (this is currently handled in DualLayers's destructor)
  */
  CardSelector* Instance();

  /*
  ** Create the singleton pointer. Instance() isn't valid until this is called.
  */
  CardSelector* Create(DuelLayers* inDuelLayers);

  /*
  ** Teardown the singleton pointer instance.
  */
  void Terminate();
}


struct Exp
{
  static inline bool test(CardSelector::Target*, CardSelector::Target*);
};

#endif
