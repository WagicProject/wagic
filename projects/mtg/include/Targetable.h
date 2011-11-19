#ifndef _TARGETABLE_H_
#define _TARGETABLE_H_

class Targetable
{
protected:
    GameObserver* observer;
public:
    Targetable(GameObserver* observer) : observer(observer) {};
    virtual const string getDisplayName() const = 0;
    inline GameObserver* getObserver() { return observer; };
    virtual void setObserver(GameObserver*g) { observer = g; };
};

#endif
