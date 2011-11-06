#ifndef _TRASH_H_
#define _TRASH_H_

#include <vector>
#include "Pos.h"
#include "WEvent.h"
#include "DamagerDamaged.h"

class CardView;
struct AttackerDamaged;
struct DamagerDamaged;
typedef DamagerDamaged DefenserDamaged;

template<class T> void trash(T*);

template<class T>
class TrashBin
{
    std::vector<T> bin;
    void put_out();
    int receiveEvent(WEvent* e);
    template<class Q> friend void trash(Q*);
    friend class Trash;
};


class Trash
{
private:
    TrashBin<CardView*> CardViewTrash;
    TrashBin<DefenserDamaged*> DefenserDamagedTrash;
    TrashBin<AttackerDamaged*> AttackerDamagedTrash;

public:
    Trash(){};
    void cleanup();
    void trash(CardView* garbage);
    void trash(DefenserDamaged* garbage);
    void trash(AttackerDamaged* garbage);
};

#endif // _TRASH_H_
