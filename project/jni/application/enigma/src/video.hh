/*
 * Copyright (C) 2002,2003,2004,2005 Daniel Heck
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef VIDEO_HH_INCLUDED
#define VIDEO_HH_INCLUDED

#include <memory>
#include "SDL.h"
#include "ecl_fwd.hh"
#include "ecl_geom.hh"

namespace video
{

    enum VideoModes {
        VM_None     = -1,
        VM_640x480  = 0,
        VM_640x512  = 1,
        VM_800x600  = 2,
        VM_1024x768 = 3,
        VM_COUNT
    };

    struct VMInfo {
        VideoModes  videomode;
        int width, height;      // Screen width and height in pixels
        int tile_size;          // Tile size in pixels
        const char *name;       // Menu text 
        const char *initscript; // Lua initialization script
        const char *gfxdir;     // Directory that contains the graphics
        int thumbw, thumbh;     // Width and height of thumbnails
        const char *thumbsdir;  // Directory that contains the thumbnails
        ecl::Rect gamearea;
        ecl::Rect statusbararea;
        ecl::Rect sb_timearea;
        ecl::Rect sb_movesarea;
        ecl::Rect sb_itemarea;
        ecl::Rect sb_textarea;
        VideoModes  fallback_videomode;
        bool     available;     // Is this video mode available?
    };

    void Init();
    void Shutdown();

    void ChangeVideoMode();
    bool SetFullscreen(bool onoff);

    /*! Switch between windowed and fullscreen mode. Return true if
       fullscreen mode is active afterwards. */
    bool ToggleFullscreen();

    bool IsFullScreen();

    /*! Return information about arbitrary video mode. */
    const VMInfo *GetInfo (VideoModes vm);

    /*! Return information about current video mode. */
    const VMInfo *GetInfo ();

    bool ModeAvailable (VideoModes vm);

    //! Return the current video mode
    VideoModes GetVideoMode();

    /*! Return the number of bits per pixel in the current video
      mode. [currently always 16] */
    int GetColorDepth();

    ecl::Screen *GetScreen();

    /*! The backbuffer is surface that has the same size and pixel
      format as the screen.  This surface is used by ShowScreen() and
      FX_* functions below. */
    ecl::Surface *BackBuffer();
    
    /*! Update gamma correction using current options. */
    void UpdateGamma();

    void               SetCaption (const char *str);
    const std::string& GetCaption();

    /*! Take a screenshot and save it as a PNG to file FNAME. */
    void Screenshot (const std::string &fname);

/* -------------------- Input grabbing -------------------- */

    class TempInputGrab {
    public:
        TempInputGrab (bool onoff);
        ~TempInputGrab();
    private:
        bool old_onoff;
    };

    bool GetInputGrab();
    bool SetInputGrab (bool onoff);

/* -------------------- Mouse cursor -------------------- */

    void SetMouseCursor(ecl::Surface *s, int hotx, int hoty);
    void HideMouse();
    void ShowMouse();
    int Mousex();
    int Mousey();

/* -------------------- Visual effects -------------------- */

    class TransitionEffect {
    public:
        virtual ~TransitionEffect() {}
        virtual void tick (double dtime) = 0;
        virtual bool finished() const = 0;
    };

    enum FadeMode { FADEIN, FADEOUT };
    void FX_Fade (FadeMode mode);
    void FX_Fly (ecl::Surface *newscr, int originx, int originy);

    enum TransitionModes
    {
	TM_RANDOM,
	TM_FADEOUTIN,
	TM_SQUARES,
	TM_FLY_N, TM_FLY_S, TM_FLY_W, TM_FLY_E,
	TM_FLY_NW, TM_FLY_NE, TM_FLY_SE, TM_FLY_SW,
        TM_PUSH_RANDOM, TM_PUSH_N, TM_PUSH_S, TM_PUSH_W, TM_PUSH_E
    };
    void ShowScreen (TransitionModes tm, ecl::Surface *newscr);

    TransitionEffect *MakeEffect (TransitionModes tm, ecl::Surface *newscr);
}

#endif
