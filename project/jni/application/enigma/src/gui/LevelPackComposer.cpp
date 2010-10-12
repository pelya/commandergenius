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
 
#include "gui/LevelPackComposer.hh"
#include "gui/HelpMenu.hh"
#include "ecl.hh"
#include "enigma.hh"
#include "errors.hh"
#include "nls.hh"
#include "sound.hh"
#include "video.hh"
#include "lev/Index.hh"

#include "main.hh"

#include <cstdio>

using namespace ecl;
using namespace std;

namespace enigma { namespace gui {
    
    lev::PersistentIndex * LevelPackComposer::clipboard = NULL;

    static const char *helptext[] = {
        N_("Shift click:"),         N_("Add to clipboard"),
        N_("Shift delete:"),        N_("Clear clipboard"),
        N_("F8:"),                  N_("Insert clipboard as reference"),
        N_("F9:"),                  N_("Insert clipboard as copy"),
//        N_("F10:"),                 N_("Move clipboard levels"),
        N_("Alt left arrow:"),      N_("Exchange level with predecessor"),
        N_("Alt right arrow:"),     N_("Exchange level with successor"),
        N_("Delete:"),              N_("Delete level"),
        N_("F5:"),                  N_("Update index from levels"),
        0
    };
#if 0
        // fake gettext to register the following strings for I18N
        _("F10")
        _("Move clipboard levels")
#endif

    
    LevelPackComposer::LevelPackComposer(bool enableEdit) : 
            isEditable (enableEdit), isModified (false) {
        if (clipboard == NULL) {
            std::vector<std::string> dummy;
            clipboard = new lev::PersistentIndex(" ", false); // mark as incomplete
        }
        
        curIndex = dynamic_cast<lev::PersistentIndex *>(lev::Index::getCurrentIndex());
        
        const video::VMInfo &vminfo = *video::GetInfo();

        // Add navigation buttons
        pgup     = new ImageButton("ic-up", "ic-up1", this);
        pgdown   = new ImageButton("ic-down", "ic-down1", this);
        start    = new ImageButton("ic-top", "ic-top1", this);
        end      = new ImageButton("ic-bottom", "ic-bottom1", this);
    
        Rect r(vminfo.width-30, 60, 20, 50);
        r.y = 60;
        add (pgup, r);
        r.y += 60;
        add (pgdown, r);
        r.y = 60 + 240;
        add (start, r);
        r.y += 60;
        add (end, r);

        // Prepare level selection widget
        levelwidget = new LevelWidget(false, isEditable);
        levelwidget->set_listener(this);
        ecl::Rect previewarea(10, 60, vminfo.width-50, vminfo.height-130);
        levelwidget->realize (previewarea);
        levelwidget->set_area (previewarea);
    
        this->add(levelwidget);

        // Information area
        lbl_lpinfo = new Label();
        lbl_clipinfo = new Label();
        lbl_levelname = new Label();
        lbl_clipcontent = new Label();
        
        HList *hl = new HList;
        hl->set_spacing(10);
        hl->set_alignment(HALIGN_CENTER, VALIGN_TOP);
        hl->set_default_size(vminfo.width/2 - 10, 28);
        hl->add_back (lbl_lpinfo);
        hl->add_back (lbl_clipinfo);
        this->add (hl, Rect (5, 10, vminfo.width - 10, 28));
    
        hl = new HList;
        hl->set_spacing(10);
        hl->set_alignment(HALIGN_CENTER, VALIGN_TOP);
        clipContentWidth = vminfo.width/2 - 10;
        hl->set_default_size(clipContentWidth, 28);
        hl->add_back (lbl_levelname);
        hl->add_back (lbl_clipcontent);
        this->add (hl, Rect (5, 10+20, vminfo.width - 10, 28));
    
        // Create buttons - positioning identical to Levelmenu
        but_ignore = new StaticTextButton(N_("Undo"), this);
        but_back = new StaticTextButton(N_("Ok"), this);
        
        HList * commandHList = new HList;
        commandHList->set_spacing(10);
        commandHList->set_alignment(HALIGN_CENTER, VALIGN_TOP);
        commandHList->set_default_size(140, 35);
        commandHList->add_back(new Label());
        commandHList->add_back(new Label());
        commandHList->add_back(but_ignore);
        commandHList->add_back(but_back);
        this->add(commandHList, Rect(10, vminfo.height-50, vminfo.width-20, 35));
        
    }

    void LevelPackComposer::tick(double dtime) 
    {
        levelwidget->tick(dtime);
        static double timeaccu = 0.0;
    
//        // info texts disappear after some time
//        if (shown_text_ttl>0.0) {
//            shown_text_ttl -= dtime;
//            if (shown_text_ttl <= 0.0)
//                shown_text = "";
//        }
        timeaccu += dtime;
        if (timeaccu > 0.1) {
            update_info();
            timeaccu = 0.0;
        }
    }

    bool LevelPackComposer::on_event (const SDL_Event &e) {
        // Pass all events to the level widget first
        bool handled=levelwidget->on_event(e);
    
        if (!handled) {
            if (e.type == SDL_KEYDOWN) {
                switch (e.key.keysym.sym) {
                case SDLK_DELETE:
                    if (SDL_GetModState() & KMOD_SHIFT) {
                        // delete clipboard
                        clipboard->clear();
                        handled=true;
                    } else {
                        // delete level
                        if (isEditable) {
                            int pos = curIndex->getCurrentPosition();
                            lev::Proxy * curProxy = curIndex->getCurrent();
                            if (curProxy == NULL) {
                                // levelpack is empty
                                handled=true;
                                break;
                            }
                            if (curIndex->isSource(curProxy) && 
                                    backups.find(curProxy->getNormLevelPath()) == backups.end()) {
                                // mark as deletion candidate - the final check
                                // if we delete it really occurs on save
                                deletions.insert(curProxy->getNormLevelPath());
                            }
                            curIndex->erase(pos);
                            if (pos >= curIndex->size() && pos > 0)
                                curIndex->setCurrentPosition(pos-1);
                            levelwidget->syncFromIndexMgr();
                            isModified = true;
                            invalidate_all();
                            handled=true;
                        }
                    }
                    break;
                case SDLK_F8:
                    if (isEditable) {
                        int pos = (curIndex->size() == 0) ? 0 : curIndex->getCurrentPosition() + 1;
                        for (int i = 0; i < clipboard->size(); i++) {
                            lev::Variation var = clipboard->getVariation(i);
                            curIndex->insertProxy(pos++, clipboard->getProxy(i), true,
                                    var.ctrl, var.unit, var.target, var.extensions);
                            isModified = true;
                        }
                        invalidate_all();
                        handled=true;
                    }
                    break;
                case SDLK_F9:
                    if (isEditable && !curIndex->isCross()) {
                        int pos = (curIndex->size() == 0) ? 0 : curIndex->getCurrentPosition() + 1;
                        for (int i = 0; i < clipboard->size(); i++) {
                            lev::Variation var = clipboard->getVariation(i);
                            lev::Proxy *levelCopy = clipboard->getProxy(i)->copy(app.userPath + "/levels",
                                    curIndex->getPackPath(), true);
                            if (levelCopy == NULL) {
                                // insert a crossreference
                                curIndex->insertProxy(pos++, clipboard->getProxy(i), true,
                                        var.ctrl, var.unit, var.target, var.extensions);
                            } else {
                                // insert reference to our copy
                                curIndex->insertProxy(pos++, levelCopy, true,
                                        var.ctrl, var.unit, var.target, var.extensions);
                                backups.insert(levelCopy->getNormLevelPath());
                                deletions.erase(levelCopy->getNormLevelPath());
                            }
                            isModified = true;
                        }
                        invalidate_all();
                        handled=true;
                    }
                    break;
                case SDLK_LEFT:     
                    if (isEditable && (SDL_GetModState() & KMOD_ALT)) {
                        int pos = curIndex->getCurrentPosition();
                        if (pos > 0) {
                            curIndex->exchange(pos, pos-1);
                            levelwidget->syncFromIndexMgr();
                            isModified = true;
                        }
                        invalidate_all();
                        handled=true;
                    }
                    break;
                case SDLK_RIGHT:     
                    if (isEditable && (SDL_GetModState() & KMOD_ALT)) {
                        int pos = curIndex->getCurrentPosition();
                        if (pos < curIndex->size() - 1) {
                            curIndex->exchange(pos, pos+1);
                            levelwidget->syncFromIndexMgr();
                            isModified = true;
                        }
                        invalidate_all();
                        handled=true;
                    }
                    break;
                case SDLK_F5:
                    if (isEditable) {
                        curIndex->updateFromProxies();
                        isModified = true;
                        invalidate_all();
                        handled=true;
                    }
                    break;
                case SDLK_F1:     
                    displayHelp(helptext, 200);
                    invalidate_all();
                    handled=true;
                    break;
                default:
                    break;
                }
            }
            else
                handled = Menu::on_event (e);
        }
        return handled;
     }
     
     void LevelPackComposer::on_action(Widget *w) {
        if (w==levelwidget) {
            lev::Index *ind = lev::Index::getCurrentIndex();
            int ilevel = ind->getCurrentPosition();
            if (w->lastModifierKeys() & KMOD_SHIFT) {
                lev::Variation var;
                lev::Proxy * curProxy = lev::Index::getCurrentProxy();
                if (curProxy->getNormPathType() != lev::Proxy::pt_absolute) {
                    // all but absolute commandline proxies may be put on the clipboard
                    if (curIndex != NULL)
                        var = curIndex->getVariation(curIndex->getCurrentPosition());
                    clipboard->appendProxy(curProxy, var.ctrl, 
                            var.unit, var.target, var.extensions);
                    sound::EmitSoundEvent ("menuok");
                } else {
                    sound::EmitSoundEvent ("menustop");
                }
            }
        } else if (w == but_back) {
            if (isModified) {
                // save index
                curIndex->save(true);
                // delete levelfiles
                std::set<std::string>::iterator it;
                for (it = deletions.begin(); it != deletions.end(); it++) {
                    if (!curIndex->hasNormLevelPath(*it)) {
                        // delete plain files on user path - ignore system and zip levels
                        std::string path = app.userPath + "/levels/" + (*it);
                        std::remove((path + ".xml").c_str());
                        std::remove((path + ".lua").c_str());
                    }
                }
                // delete backups
                std::string base = app.userPath + "/levels/";
                for (it = backups.begin(); it != backups.end(); it++) {
                    std::remove((base + *it + ".xml~").c_str());
                    std::remove((base + *it + ".lua~").c_str());
                }
            }
            Menu::quit();
        } else if (w == but_ignore) {
            if (isModified) {
                // we need to reload the index
                curIndex->loadDoc();
                // restore backups
                std::string base = app.userPath + "/levels/";
                std::set<std::string>::iterator it;
                for (it = backups.begin(); it != backups.end(); it++) {
                    std::remove((base + *it + ".xml").c_str());
                    std::rename((base + *it + ".xml~").c_str(), (base + *it + ".xml").c_str());
                    std::remove((base + *it + ".lua").c_str());
                    std::rename((base + *it + ".lua~").c_str(), (base + *it + ".lua").c_str());
                }
            }
            Menu::quit();
        } else if (w == pgup) {
            levelwidget->page_up();
        } else if (w == pgdown) {
            levelwidget->page_down();
        } else if (w == start) {
            levelwidget->start();
        } else if (w == end) {
            levelwidget->end();
        }
    }
    void LevelPackComposer::update_info() {
        // Note: all format strings have to be translated directly
        // as the formatted strings can no longer be translated.
        // The instant language change is guaranteed by the frequent
        // call of is method!
        
        lev::Index *ind = lev::Index::getCurrentIndex();
        int size = ind->size();
        lev::Proxy *curProxy = ind->getCurrent();
        
        lbl_lpinfo->set_text(ecl::strf(_("%s: %d levels"),
                ind->getName().c_str(), size));
                
        if (size == 0) {
            // empty level pack
            lbl_levelname->set_text ("-");
        }
        else {
            lbl_levelname->set_text(ecl::strf("#%d:(%s)",
                      ind->getCurrentLevel(), 
                      curProxy->getNormLevelPath().c_str()));
        }
        
        int csize = clipboard->size();
        lbl_clipinfo->set_text(ecl::strf(_("Clipboard: %d levels"), csize));
        if (csize == 0) {
            // empty level pack
            lbl_clipcontent->set_text ("-");
        }
        else {
            std::string clipstring = clipboard->getProxy(0)->getTitle();
            for (int i = 1; i < csize; i++)
                clipstring += ", " + clipboard->getProxy(i)->getTitle(); 
            lbl_clipcontent->set_text(clipstring);
            if (enigma::GetFont("menufont")->get_width(clipstring.c_str()) > clipContentWidth)
                lbl_clipcontent->set_alignment(HALIGN_RIGHT);
            else
                lbl_clipcontent->set_alignment(HALIGN_CENTER);
        }
        
    }
    
    void LevelPackComposer::draw_background(ecl::GC &gc) {
        video::SetCaption(("Enigma - Level Pack Composer"));
        blit(gc, 0,0, enigma::GetImage("menu_bg", ".jpg"));
        if (isModified)
    
            blit(gc, 0,0, enigma::GetImage("changed"));
    }
    

}} // namespace enigma::gui
