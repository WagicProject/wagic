#include "PrecompiledHeader.h"

#include "ManaCostHybrid.h"

ManaCostHybrid::ManaCostHybrid(){
  init(0,0,0,0);
}

ManaCostHybrid::ManaCostHybrid(int c1,int v1,int c2,int v2){
  init(c1,v1,c2,v2);
}

void ManaCostHybrid::init(int c1,int v1,int c2,int v2){
  color1 = c1;
  color2 = c2;
  value1 = v1;
  value2 = v2;
}

int ManaCostHybrid::getConvertedCost(){
  if (value2 > value1) return value2;
  return value1;
}

int ManaCostHybrid::hasColor(int color){
  if (((color1 == color) && value1) || ((color2 == color) && value2)) return 1;
  return 0;
}
