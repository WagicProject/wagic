#include "PrecompiledHeader.h"

#include "MTGDefinitions.h"
#include "Pos.h"
#include "CardGui.h"
#include "DamagerDamaged.h"
#include "Trash.h"

template<class T>
void TrashBin<T>::put_out()
{
    for (typename std::vector<T>::iterator it = bin.begin(); it != bin.end(); ++it)
        SAFE_DELETE(*it);
    bin.clear();
}

void Trash::cleanup()
{
    CardViewTrash.put_out();
    DefenserDamagedTrash.put_out();
    AttackerDamagedTrash.put_out();
}

void Trash::trash(CardView* garbage)
{
    CardViewTrash.bin.push_back(garbage);
}
void Trash::trash(DefenserDamaged* garbage)
{
    DefenserDamagedTrash.bin.push_back(garbage);
}
void Trash::trash(AttackerDamaged* garbage)
{
    AttackerDamagedTrash.bin.push_back(garbage);
}
