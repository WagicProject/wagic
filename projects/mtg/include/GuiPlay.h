#ifndef _GUIPLAY_H_
#define _GUIPLAY_H_

#include "GuiLayers.h"
#include "CardGui.h"

class GuiPlay : public GuiLayer
{
 public:
  static const float HORZWIDTH = 300.0f;
  static const float VERTHEIGHT = 80.0f;
  typedef vector<CardView*>::iterator iterator;

 protected:
  class CardStack {
  protected:
    float baseX, baseY;
    float x, y;

  public:
    void reset(float x, float y);
    void Enstack(CardView*);
    void RenderSpell(MTGCardInstance*, iterator begin, iterator end, float x, float y);
  };

  class HorzStack : public CardStack {
  protected:
    const float maxWidth;
    float maxHeight;
  public:
    HorzStack(float width = HORZWIDTH);
    void reset(float x, float y);
    void Render(CardView*, iterator begin, iterator end);
    void Enstack(CardView*);
  };
  class VertStack : public CardStack {
  protected:
    float maxHeight;
  public:
    VertStack(float height = VERTHEIGHT);
    void Enstack(CardView*);
    inline float nextX();
  };
  class BattleField : public HorzStack {
    static const float HEIGHT;
    unsigned attackers;
    unsigned blockers;
    unsigned currentAttacker;
    float height;

  public:
    void addAttacker(MTGCardInstance*);
    void removeAttacker(MTGCardInstance*);
    void reset(float x, float y);
    BattleField(float width = HORZWIDTH);
    void EnstackAttacker(CardView*);
    void EnstackBlocker(CardView*);
    void Update(float dt);
    void Render();
  };

  class Lands : public HorzStack {};
  class Creatures : public HorzStack {};
  class Spells : public VertStack {};

 protected:
  GameObserver* game;
  Creatures selfCreatures, opponentCreatures;
  BattleField battleField;
  Lands selfLands, opponentLands;
  Spells selfSpells, opponentSpells;
  CardSelector* cs;
  iterator end_spells;

  vector<CardView*> cards;

 public:
  GuiPlay(GameObserver*, CardSelector*);
  ~GuiPlay();
  virtual void Render();
  void Replace();
  void Update(float dt);
  virtual int receiveEventPlus(WEvent * e);
  virtual int receiveEventMinus(WEvent * e);
};

#endif // _GUIPLAY_H_
