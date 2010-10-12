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
 
#include "gui/SearchMenu.hh"
#include "ecl.hh"
#include "enigma.hh"
#include "errors.hh"
#include "lev/Index.hh"
#include "lev/Proxy.hh"
#include "nls.hh"
#include "video.hh"

#include "main.hh"

using namespace ecl;
using namespace std;

namespace enigma { namespace gui {

     SearchMenu::SearchMenu() : didSearch (false) {
        const video::VMInfo &vminfo = *video::GetInfo();
        ecl::Font *menufont = enigma::GetFont("menufont");
        
        Label * shallowTitle = new Label(N_("Shallow Search:"), HALIGN_LEFT);
// TRANSLATORS: the translation can have double size of the english text 
        std::string helpText = _("case independent search in title, author, filename");
        std::string workString = helpText;
        std::string::size_type breakPos = breakString(menufont, workString, 
                    " ", 380);
        Label * shallowHelp1 = new UntranslatedLabel(workString.substr(0,breakPos), HALIGN_LEFT);
        Label * shallowHelp2 = new UntranslatedLabel(workString.substr(breakPos), HALIGN_LEFT);
        shallowSearch = new TextField("", this);
        
        this->add(shallowTitle, Rect(vminfo.width/2 - 190, vminfo.height/2 - 100, 380, 35));
        this->add(shallowHelp1, Rect(vminfo.width/2 - 190, vminfo.height/2 - 40, 380, 25));
        this->add(shallowHelp2, Rect(vminfo.width/2 - 190, vminfo.height/2 - 10, 380, 25));
        this->add(shallowSearch, Rect(vminfo.width/2 - 190, vminfo.height/2 + 55, 380, 35));
       
        // Create buttons - positioning identical to Levelmenu
        Label * dummy1 = new Label();
        Label * dummy2 = new Label();
        but_ignore = new StaticTextButton(N_("Undo"), this);
        but_search = new StaticTextButton(N_("Search"), this);
        
        HList * commandHList = new HList;
        commandHList->set_spacing(10);
        commandHList->set_alignment(HALIGN_CENTER, VALIGN_TOP);
        commandHList->set_default_size(140, 35);
        commandHList->add_back(dummy1);
        commandHList->add_back(dummy2);
        commandHList->add_back(but_ignore);
        commandHList->add_back(but_search);
        this->add(commandHList, Rect(10, vminfo.height-50, vminfo.width-20, 35));
    }
    
    bool SearchMenu::isSearchQuit() {
        return didSearch;
    }
    
    void SearchMenu::on_action(Widget *w) {
        if (w == but_search || 
                (w == shallowSearch && shallowSearch->wasLastActionReturn())) {
            lev::Index::setCurrentIndex(lev::Proxy::search(shallowSearch->getText()));
            didSearch = true;
            Menu::quit();
        } else if (w == but_ignore) {
            Menu::quit();
        }
    }
    
    void SearchMenu::draw_background(ecl::GC &gc) {
        video::SetCaption(("Enigma - Search Menu"));
        blit(gc, 0,0, enigma::GetImage("menu_bg", ".jpg"));
    }
}} // namespace enigma::gui
