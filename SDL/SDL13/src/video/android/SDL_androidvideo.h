/*
    SDL - Simple DirectMedia Layer
    Copyright (C) 1997-2009 Sam Lantinga

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

    Sam Lantinga
    slouken@libsdl.org
*/
#ifndef _SDL_androidvideo_h
#define _SDL_androidvideo_h

#include "SDL_version.h"
#include "SDL_config.h"
#include "SDL_video.h"
#include "SDL_joystick.h"

extern int SDL_ANDROID_sWindowWidth;
extern int SDL_ANDROID_sWindowHeight;
extern int SDL_ANDROID_sFakeWindowWidth; // SDL 1.2 only
extern int SDL_ANDROID_sFakeWindowHeight; // SDL 1.2 only
extern int SDL_ANDROID_ShowScreenUnderFinger;
extern SDL_Rect SDL_ANDROID_ShowScreenUnderFingerRect, SDL_ANDROID_ShowScreenUnderFingerRectSrc;
extern int SDL_ANDROID_CallJavaSwapBuffers();
extern void SDL_ANDROID_CallJavaShowScreenKeyboard();
extern int SDL_ANDROID_drawTouchscreenKeyboard();
extern void SDL_ANDROID_VideoContextLost();
extern void SDL_ANDROID_VideoContextRecreated();
extern void SDL_ANDROID_processAndroidTrackballDampening();
extern SDL_VideoDevice *ANDROID_CreateDevice_1_3(int devindex);

#if SDL_VERSION_ATLEAST(1,3,0)
extern SDL_Window * ANDROID_CurrentWindow;
#endif

// Exports from SDL_androidinput.c - SDL_androidinput.h is too encumbered
enum { MAX_MULTITOUCH_POINTERS = 16 };
extern void ANDROID_InitOSKeymap();
extern int SDL_ANDROID_isJoystickUsed;
extern SDL_Joystick *SDL_ANDROID_CurrentJoysticks[MAX_MULTITOUCH_POINTERS+1];

#endif /* _SDL_androidvideo_h */
