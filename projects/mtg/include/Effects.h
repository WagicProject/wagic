#ifndef _EFFECTS_H_
#define _EFFECTS_H_

#include <JGui.h>

class Effect: public JGuiObject
{
    static int id_counter;
public:
    Effect() : JGuiObject(++id_counter) {};
};

#endif // _EFFECTS_H_
