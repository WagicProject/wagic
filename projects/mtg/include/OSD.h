#ifndef _OSD_H_
#define _OSD_H_

class OSDLayer : public PlayGuiObjectController
{
  virtual void Update(float dt);
  virtual bool CheckUserInput(u32 key);
  virtual void Render();
};

#endif
