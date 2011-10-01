#ifndef _TARGETABLE_H_
#define _TARGETABLE_H_

#define TARGET_CARD 1
#define TARGET_PLAYER 2
#define TARGET_STACKACTION 3

class Targetable
{
protected:
    GameObserver* observer;
public:
    Targetable(GameObserver* observer) : observer(observer) {};
    virtual int typeAsTarget() = 0;
    virtual const string getDisplayName() const = 0;
    inline GameObserver* getObserver() { return observer; };
    virtual void setObserver(GameObserver*g) { observer = g; };
};

#endif
