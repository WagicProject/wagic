/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2011 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/
#include "SDL_config.h"

#include <android/log.h>

#include "SDL_events.h"
#include "SDL_touch.h"
#include "../../events/SDL_mouse_c.h"
#include "../../events/SDL_touch_c.h"

#include "SDL_androidtouch.h"


#define ACTION_DOWN 0
#define ACTION_UP 1
#define ACTION_MOVE 2
#define ACTION_CANCEL 3
#define ACTION_OUTSIDE 4
#define ACTION_POINTER_DOWN 5
#define ACTION_POINTER_UP 6


static SDL_TouchID gTouchID = 0;

SDL_Touch* CreateTouchInstance()
{
	SDL_Touch touch;
	touch.id = gTouchID;

	touch.x_min = 0;
	touch.x_max = 1;
	touch.native_xres = 1;
    touch.xres = 1;

    touch.y_min = 0;
	touch.y_max = 1;
	touch.native_yres = 1;
    touch.yres = 1;

    touch.pressure_min = 0;
	touch.pressure_max = 1;
	touch.native_pressureres = 1;
    touch.pressureres = 1;

	SDL_AddTouch(&touch, "");

	return SDL_GetTouch(touch.id);
}

void Android_OnTouch(int index, int action, float x, float y, float p)
{
    if (!Android_Window)
	{
        return;
    }

    if ((action != ACTION_CANCEL) && (action != ACTION_OUTSIDE))
	{
        //SDL_SetMouseFocus(Android_Window);
        //SDL_SendMouseMotion(Android_Window, 0, (int)x, (int)y);
		SDL_Touch* touch = SDL_GetTouch(gTouchID);
		if (touch == NULL)
		{
			touch = CreateTouchInstance();
		}

        //char buffer[64];
        //sprintf(buffer, "Touch action: %d x: %f y: %f, index: %d", action, x, y, index);
        //__android_log_write(ANDROID_LOG_DEBUG, "Wagic", buffer);

        switch(action)
		{
        case ACTION_DOWN:
        case ACTION_POINTER_DOWN:
			SDL_SendFingerDown(touch->id, index, SDL_TRUE, x, y, p);
            //SDL_SendMouseButton(Android_Window, SDL_PRESSED, SDL_BUTTON_LEFT);
            break;

		case ACTION_MOVE:
			SDL_SendTouchMotion(touch->id, index, SDL_FALSE, x, y, 1);
			break;

        case ACTION_UP:
            // this means a completed gesture
            SDL_SendFingerDown(touch->id, index, SDL_FALSE, x, y, p);
            break;

        case ACTION_POINTER_UP:
            // this is an individual finger up - due to some false hits on finger 0 lifting, only honor this event
            // if the finger up is anything BUT finger 0, this prevents false action triggers
            if (index > 0)
			    SDL_SendFingerDown(touch->id, index, SDL_FALSE, x, y, p);
            //SDL_SendMouseButton(Android_Window, SDL_RELEASED, SDL_BUTTON_LEFT);
            break;
        }
    }
	else
	{
        //SDL_SetMouseFocus(NULL);
    }
}

/* vi: set ts=4 sw=4 expandtab: */
