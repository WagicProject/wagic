#ifndef _TRASH_H_
#define _TRASH_H_

#include <vector>
#include "Pos.h"
#include "WEvent.h"

void trash(Pos*);

class Trash
{
  std::vector<Pos*> bin;
  void put_out();
  int receiveEvent(WEvent* e);
  friend void trash(Pos*);

 public:
  static void cleanup();
};

#endif // _TRASH_H_
