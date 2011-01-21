#ifndef _GUIPLAY_H_
#define _GUIPLAY_H_

#include "GuiLayers.h"
#include "CardGui.h"

class GuiPlay: public GuiLayer
{
public:
    static const float HORZWIDTH;
    static const float VERTHEIGHT;
    typedef vector<CardView*>::iterator iterator;

protected:
    class CardStack
    {
    protected:
        unsigned total;
        float baseX, baseY;
        float x, y;

    public:
        void reset(unsigned total, float x, float y);
        void Enstack(CardView*);
        void RenderSpell(MTGCardInstance*, iterator begin, iterator end, float x, float y);
    };

    class HorzStack: public CardStack
    {
    public:
        HorzStack();
        void Render(CardView*, iterator begin, iterator end);
        void Enstack(CardView*);
    };
    class VertStack: public CardStack
    {
    protected:
        unsigned count;
    public:
        VertStack();
        void reset(unsigned total, float x, float y);
        void Render(CardView*, iterator begin, iterator end);
        void Enstack(CardView*);
        inline float nextX();
    };
    class BattleField: public HorzStack
    {
        static const float HEIGHT;
        unsigned attackers;
        unsigned blockers;
        unsigned currentAttacker;
        float height;

    public:
        int red;
        int colorFlow;

        void addAttacker(MTGCardInstance*);
        void removeAttacker(MTGCardInstance*);
        void reset(float x, float y);
        BattleField();
        void EnstackAttacker(CardView*);
        void EnstackBlocker(CardView*);
        void Update(float dt);
        void Render();
    };

    class Lands: public HorzStack {};
    class Creatures: public HorzStack {};
    class Spells: public VertStack {};

protected:
    GameObserver* game;
    Creatures selfCreatures, opponentCreatures;
    BattleField battleField;
    Lands selfLands, opponentLands;
    Spells selfSpells, opponentSpells;
    iterator end_spells;

    vector<CardView*> cards;

public:
    GuiPlay(GameObserver*);
    ~GuiPlay();
    virtual void Render();
    void Replace();
    void Update(float dt);
    virtual int receiveEventPlus(WEvent * e);
    virtual int receiveEventMinus(WEvent * e);
};

#endif // _GUIPLAY_H_
