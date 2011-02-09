#include "PrecompiledHeader.h"

#include "ManaCostHybrid.h"
#include "MTGDefinitions.h"

ManaCostHybrid::ManaCostHybrid()
{
    init(0, 0, 0, 0);
}

ManaCostHybrid::ManaCostHybrid(const ManaCostHybrid& hybridManaCost)
{
    color1 = hybridManaCost.color1;
    color2 = hybridManaCost.color2;
    value1 = hybridManaCost.value1;
    value2 = hybridManaCost.value2;
}

ManaCostHybrid::ManaCostHybrid(const ManaCostHybrid* hybridManaCost)
{
    if (hybridManaCost)
    {
        color1 = hybridManaCost->color1;
        color2 = hybridManaCost->color2;
        value1 = hybridManaCost->value1;
        value2 = hybridManaCost->value2;
    }
    else
        color1 = color2 = value1 = value2 = 0;

}

ManaCostHybrid::ManaCostHybrid(int c1, int v1, int c2, int v2)
{
    init(c1, v1, c2, v2);
}

void ManaCostHybrid::init(int c1, int v1, int c2, int v2)
{
    color1 = c1;
    color2 = c2;
    value1 = v1;
    value2 = v2;
}

int ManaCostHybrid::getConvertedCost()
{
    if (value2 > value1)
        return value2;
    return value1;
}

int ManaCostHybrid::hasColor(int color)
{
    if (((color1 == color) && value1) || ((color2 == color) && value2))
        return 1;
    return 0;
}

string ManaCostHybrid::toString()
{
    ostringstream oss;
    if ( color1 != 0 && color2 != 0)
        oss << "{" << Constants::MTGColorChars[color1] << "/" << Constants::MTGColorChars[color2] << "}";
    return oss.str();
}

ostream& operator<<(ostream& out, ManaCostHybrid& r)
{
  return out<< r.toString();
}

ostream& operator<<(ostream& out, ManaCostHybrid* r)
{
  return out<< r->toString();
}
