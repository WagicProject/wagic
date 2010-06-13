#ifndef _TRASH_H_
#define _TRASH_H_

#include <vector>
#include "Pos.h"
#include "WEvent.h"

template <class T> void trash(T*);
class Trash
{
 public:
  static void cleanup();
};

template <class T>
class TrashBin
{
  std::vector<T*> bin;
  void put_out();
  int receiveEvent(WEvent* e);
  template <class Q> friend void trash(Q*);
  friend class Trash;
};

#endif // _TRASH_H_
