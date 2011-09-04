#ifndef _SIMPLEPAD_H_
#define _SIMPLEPAD_H_

#include <string>
#include <JGui.h>
#include <JLBFont.h>
#include "hge/hgeparticle.h"

enum SIMPLE_KEYS{
  KPD_A,  KPD_B,  KPD_C,  KPD_D,  KPD_E,  KPD_F,
  KPD_G,  KPD_H,  KPD_I,  KPD_J,  KPD_K,  KPD_L,
  KPD_M,  KPD_N,  KPD_O,  KPD_P,  KPD_Q,  KPD_R,
  KPD_S,  KPD_T,  KPD_U,  KPD_V,  KPD_W,  KPD_X,
  KPD_Y,  KPD_Z,  KPD_SPACE, KPD_OK,  KPD_CANCEL,
  KPD_DEL,  KPD_CAPS,  KPD_0,  KPD_1,  KPD_2,  KPD_3,
  KPD_4,  KPD_5,  KPD_6,  KPD_7,  KPD_8,  KPD_9,
  KPD_MAX, 
  KPD_NOWHERE = 254,
  KPD_INPUT = 255,
};

struct SimpleKey
{
    SimpleKey(string _ds, int _id);
    string displayValue;
    unsigned char id;
    unsigned char adjacency[4];
    float mX, mY;
};

class SimplePad
{
public:
    friend class GameSettings;

    string buffer;
    string title;
    unsigned int cursorPos();
    bool isActive()
    {
        return bActive;
    }
    ;
    void Render();
    void Update(float dt);
    void pressKey(unsigned char id);

    SimplePad();
    ~SimplePad();

    float mX, mY;

private:
    void linkKeys(int from, int to, int dir);
    SimpleKey * Add(string display, unsigned char id);
    void MoveSelection(unsigned char dir);
    void Start(string value, string * _dest = NULL);
    string Finish();

    bool bActive;
    bool bCapslock;
    bool bShowCancel, bShowNumpad;
    bool bCanceled;
    int nbitems;
    unsigned int cursor;
    int selected;
    int priorKey; //The prior key from those places.
    SimpleKey * keys[KPD_MAX];
    string * dest;
    string original; //For cancelling.
};

#endif
