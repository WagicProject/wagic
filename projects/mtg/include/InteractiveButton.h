//
//  InteractiveButton.h
//
//  Created by Michael Nguyen on 1/23/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef wagic_InteractiveButton_h
#define wagic_InteractiveButton_h

#include <string>
#include <JLBFont.h>
#include <JGui.h>
#include "WResource_Fwd.h"
#include "SimpleButton.h"

using std::string;

#define SCALE_SELECTED		1.2f
#define SCALE_NORMAL		1.0f

class InteractiveButton: public SimpleButton
{
private:
    JQuadPtr buttonImage;
    JButton mActionKey;
    
public:
    InteractiveButton(JGuiController* _parent, int id, int fontId, string text, float x, float y, JButton actionKey, bool hasFocus = false, bool autoTranslate = false);
    
    virtual void Entering();
    virtual bool ButtonPressed();
    virtual void setImage( const JQuadPtr imagePtr, float xOffset = 0, float yOffset = 0);
    virtual void checkUserClick();
//virtual void Update(float dt);
    virtual void Render();
    virtual ostream& toString(ostream& out) const;
    
};

#endif
