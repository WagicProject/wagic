#ifndef _MANACOST_HYBRID_H_
#define _MANACOST_HYBRID_H_

class ManaCostHybrid
{

public:
    uint8_t color1;
    uint8_t color2;
    uint8_t value1;
    uint8_t value2;
    ManaCostHybrid();
    ManaCostHybrid(const ManaCostHybrid& hybridManaCost);
    ManaCostHybrid(const ManaCostHybrid* hybridManaCost);
    ManaCostHybrid(int c1, int v1, int c2, int v2);

    void init(int c1, int v1, int c2, int v2);
    int hasColor(int color);
    string toString();
    int getConvertedCost();
    
    friend std::ostream& operator<<(std::ostream& out, ManaCostHybrid& m);
    friend std::ostream& operator<<(std::ostream& out, ManaCostHybrid* m);

};

#endif
