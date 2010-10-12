/*
 * Copyright (C) 2006 Ronald Lamprecht
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
 */
#include "gui/InfoMenu.hh"
#include "ecl.hh"
#include "enigma.hh"
#include "video.hh"
#include "nls.hh"

using namespace ecl;
using namespace std;

namespace enigma { namespace gui {
    InfoMenu::InfoMenu(const char **infotext, int pages) : info (infotext),
            curPage (0), numPages (pages) {
        const video::VMInfo &vminfo = *video::GetInfo();

        but_ok = new StaticTextButton(N_("Ok"), this);
        pgup     = new ImageButton("ic-up", "ic-up1", this);
        pgdown   = new ImageButton("ic-down", "ic-down1", this);

        add(but_ok, Rect(vminfo.width-120, vminfo.height-60, 100, 40));        
        add(pgup, Rect(vminfo.width-30, vminfo.height/2, 20, 50));        
        add(pgdown, Rect(vminfo.width-30, vminfo.height/2 +70, 20, 50));        
    }
    
    void InfoMenu::draw_background(ecl::GC &gc) {
        const video::VMInfo &vminfo = *video::GetInfo();
        blit(gc, 0,0, enigma::GetImage("menu_bg", ".jpg"));
        
        Font *f = enigma::GetFont("menufont");
        int row = 0;
        for (int p=0; p<curPage; p++) {
            while (info[row])
                row++;
            // select first row of next page
            row++;
        }
        for (int i = 0; info[row]; row++, i++) {
            const char *t = _(info[row]);
            f->render (gc, 40 + (vminfo.width-640)/2, 
                    20 + (vminfo.height-480)/2 + i*f->get_height(), t);
        }
    }
    
    void InfoMenu::on_action (gui::Widget *w) {
        if (w == but_ok) {
            Menu::quit();
        } else if (w == pgup) {
            if (curPage > 0) {
                curPage--;
                invalidate_all();
            }
        } else if (w == pgdown) {
            if (curPage < numPages - 1) {
                curPage++;
                invalidate_all();
            }
        }
    }
    
    void displayInfo(const char **infotext, int pages) {
        FX_Fade (video::FADEOUT);
        InfoMenu menu(infotext, pages);
        menu.draw_all();
        FX_Fade (video::FADEIN);
        menu.manage();
    }

}} // namespace enigma::gui
