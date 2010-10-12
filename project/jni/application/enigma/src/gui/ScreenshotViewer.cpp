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
 *
 */

#include "gui/ScreenshotViewer.hh"
#include "gui/HelpMenu.hh"
#include "ecl.hh"
#include "enigma.hh"
#include "main.hh"
#include "nls.hh"
#include "video.hh"

using namespace ecl;
using namespace std;

namespace enigma { namespace gui {

    static const char *helptext[] = {
        N_("Escape:"),              N_("Back"),
        "F1:",                      N_("Show this help"),
        N_("Page Up:"),             N_("Show previous screenshot"),
        N_("Page Down:"),           N_("Show next screenshot"),
        0
    };
    
    ScreenshotViewer::ScreenshotViewer(lev::Proxy *aLevel) :
            levelProxy (aLevel), shotNumber (0) {
    }
    
    ScreenshotViewer::~ScreenshotViewer() {
    }
    
    bool ScreenshotViewer::on_event (const SDL_Event &e) {
        switch (e.type) {
            case SDL_MOUSEBUTTONDOWN:
                if (e.button.button == SDL_BUTTON_RIGHT) {
                    Menu::quit();
                    return true;
                }
                break;
            case SDL_KEYDOWN:
                SDLKey keysym = e.key.keysym.sym;
                switch (keysym) {
                case SDLK_PAGEUP: 
                    if (shotNumber > 0 )
                        --shotNumber;
                    invalidate_all();
                    return true;
                case SDLK_PAGEDOWN:
                    shotNumber++;
                    invalidate_all();
                    return true;
                // eat up widget activation keys - we have no widgets 
                case SDLK_DOWN:  
                case SDLK_UP:    
                case SDLK_RIGHT: 
                case SDLK_LEFT:  
                    return true;
                case SDLK_F1:     
                    displayHelp(helptext, 200);
                    invalidate_all();
                    return true;
                default:
                    break;
                }
                break;
        }
        return false;
    }
    
    void ScreenshotViewer::draw_background (ecl::GC &gc) {
        std::string filename = "screenshots/" + 
                levelProxy->getLocalSubstitutionLevelPath() + 
                (shotNumber > 0 ? ecl::strf("#%d", shotNumber) : "") + ".png";
        std::string fullPath;
        if (app.resourceFS->findFile(filename, fullPath)) {
            const video::VMInfo *vminfo = video::GetInfo();
            ecl::Surface * image = ecl::LoadImage(fullPath.c_str());
            if (image->width() == vminfo->width && image->height() == vminfo->height) {
                blit(gc, 0,0, image);
            } else {
                ecl::Surface * imageZoomed = image->zoom(vminfo->width, vminfo->height);
                blit(gc, 0,0, imageZoomed);
                delete imageZoomed;
            }
            delete image;
        } else {
            blit(gc, 0,0, enigma::GetImage("menu_bg", ".jpg"));
            Font *f = enigma::GetFont("menufont");
            f->render (gc, 30, 60, _("No screenshot available:"));
            f->render (gc, 30, 100, filename.c_str());
        }
    }
}} // namespace enigma::gui
