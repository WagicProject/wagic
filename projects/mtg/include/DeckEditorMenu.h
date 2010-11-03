#pragma once
#include "DeckMenu.h"

class DeckEditorMenu :
  public DeckMenu
{
public:
  DeckEditorMenu(int id, JGuiListener* listener = NULL, int fontId = 1, const char * _title = "");
  void Render();
  ~DeckEditorMenu();
};
