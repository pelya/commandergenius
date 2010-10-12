/*
 * Copyright (C) 2005 Ronald Lamprecht
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

#ifndef GUI_TF_HH_INCLUDED
#define GUI_TF_HH_INCLUDED


#include "gui/widgets.hh"

namespace enigma { namespace gui {

    /**
     * Gui widget for simple text insertion. Cursor keys, backspace and delete,
     * home and end are supported. As a widget it handels utf-8 strings. But
     * in contrast to other widget there is of course no gettext l10n translation.
     * This widget is designed to be part of a menu.
     * <par>
     * ToDo: supply full Linux umlaut support;
     *       add hook for external or subclass char insertion validation 
     *         (NumberField, ...);
     *       move mouse cursor out of Textfield without deselection
     */
    class TextField : public Button {
    public:
        /**
         * Creates a boarderd text input field.
         * @param t   preset utf-8 input string
         * @param al  currently unused
         */
        TextField(const std::string &t = "", ActionListener *al=0);
        
        /**
         * resets the input string.
         * @param t   new utf-8 input string
         */
        void set_text(const std::string &t);
        
        /**
         * returns the inserted text
         * @return    the utf-8 coded string
         */
        std::string getText();
        
        void setInvalidChars(std::string forbiddenChars); // currently limited to 1st coding page
        void setMaxChars(int max);
        bool wasLastActionReturn();
        
        // Widget interface.
        virtual void tick (double dtime);
        virtual bool on_event(const SDL_Event &/*e*/);
        void draw(ecl::GC &gc, const ecl::Rect &r);

    protected:
        double cursorTime;
        bool showCursor;

    private:
        std::string textPreCursor;
        std::string textPostCursor;
        /**
         * a byte vector describing for each utf-8 character the number of 
         * occupied bytes in the string.
         */
        std::vector<unsigned char> charSizesPreCursor;
        
         /**
         * a byte vector describing for each utf-8 character the number of 
         * occupied bytes in the string. This vector is in reverse order to
         * the string itself!!
         */
        std::vector<unsigned char> charSizesPostCursor;
        bool isLimitedToValidChars;
        std::string validChars;
        std::string invalidChars;
        int maxChars;
        bool isLastActionReturn;
        static ecl::Font *menufont;
    };
}} // namespace enigma::gui
#endif
