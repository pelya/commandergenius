/*
 * Copyright (C) 2002,2003,2004,2005,2006 Daniel Heck
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
#ifndef GUI_LEVELWIDGET_HH_INCLUDED
#define GUI_LEVELWIDGET_HH_INCLUDED

#include "gui/widgets.hh"
#include "gui/LevelPreviewCache.hh"
#include "lev/Index.hh"
#include "lev/ScoreManager.hh"


namespace enigma { namespace gui {
/* -------------------- LevelWidget -------------------- */

    class LevelMenu;

    /**
     * 
     */
    class LevelWidget : public Widget {
    public:
        LevelWidget(bool withScoreIcons = true, bool withEditBorder = false);

        //---------- Widget interface ----------//
        void draw(ecl::GC &gc, const ecl::Rect &r);
        void tick (double time);

        void set_listener(ActionListener *al) {
            listener = al;
        }
        void trigger_action();

        virtual void realize (const ecl::Rect &r);

        //---------- Cursor motion ----------//

        void page_up();
        void page_down();
        void start();
        void end();

        bool on_event(const SDL_Event &e);
        void syncFromIndexMgr();  // external change of currentIndex, currentLevel

    private:
        //---------- Private functions ----------//
        void syncToIndexMgr();
        void set_current (int newsel);
        void scroll_up(int lines);
        void scroll_down(int lines);
        void set_selected (int newfirst, int newsel);
        bool draw_level_preview (ecl::GC &gc, int x, int y, 
               lev::Proxy *proxy, bool selected, bool isCross, bool locked,
               bool allowGeneration, bool &didGenerate);

        bool handle_keydown (const SDL_Event *e);
        bool handle_mousedown (const SDL_Event *e);

        //---------- Variables ----------//
        bool    displayScoreIcons;
        bool    displayEditBorder;
        LevelPreviewCache *preview_cache;
        lev::ScoreManager *scoreMgr;
        lev::Index        *curIndex;
        ActionListener    *listener;

        int  ifirst;    // Index of "upper left" level
        int  iselected; // Index of selected level
        int  width;     // number of buttons in a row
        int  height;    // number of buttons in a column
        int  buttonw;   // pixelwidth of a button
        int  buttonh;   // pixelheight of a button
        std::vector<ecl::Rect>  m_areas; // Screen areas occupied by level previews
        std::vector<bool> pending_redraws;
        bool isInvalidateUptodate;
        double lastUpdate;

        // some image pointers for efficiency
        ecl::Surface *img_link;
        ecl::Surface *img_copy;
        ecl::Surface *img_feather;
        ecl::Surface *img_easy;
        ecl::Surface *img_hard;
        ecl::Surface *img_changed;
        ecl::Surface *img_unavailable;
    //    Surface *img_unknown;
        ecl::Surface *img_par;
        ecl::Surface *img_wrEasy;
        ecl::Surface *img_wrDifficult;
        ecl::Surface *img_border;
        ecl::Surface *img_editborder;
    };

}} // namespace enigma::gui
#endif
