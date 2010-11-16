#include "MTGSpellStack.h"

MTGSpellStack::MTGSpellStack()
{
    cursor = -1;
}

void MTGSpellStack::addSpell(Ability * ability)
{
    cursor++;
    spellStack[cursor] ability;
}

int MTGSpellStack::resolve()
{
    if (cursor < 0)
        return 0;
    int result = cursor;
    cursor--;
    (spellStack[cursor + 1])->resolve();
    return (result + 1);
}
