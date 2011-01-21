#ifndef _MANACOST_HYBRID_H_
#define _MANACOST_HYBRID_H_

class ManaCostHybrid
{
public:
    int color1;
    int color2;
    int value1;
    int value2;
    ManaCostHybrid();
    int hasColor(int color);
    ManaCostHybrid(int c1, int v1, int c2, int v2);
    void init(int c1, int v1, int c2, int v2);
    int getConvertedCost();
};

#endif
