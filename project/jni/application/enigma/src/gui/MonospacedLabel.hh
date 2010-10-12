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
 
#ifndef GUI_MONOSPACEDLABEL_HH_INCLUDED
#define GUI_MONOSPACEDLABEL_HH_INCLUDED

#include "gui/widgets.hh"


namespace enigma { namespace gui {
    /**
     * A text label that uses the same width for each character even for
     * proportional fonts. Character positions of the label text is predictable
     * and alignment of several labels is thus possible. Usefull for output of
     * formatted numbers like score values.<p>
     * Not all characters have to be monospaced. The set of characters that
     * should have a constant width can be limited f.e. to the numbers 0-9.
     */
    class MonospacedLabel : public Label {
    public:
        /**
         * The standard constructor.
         * @arg text  The constant output text.
         * @arg widthSample  The character that defines the width. Default is 'm'.
         * @arg monospaceCharacters  The set of monospace characters. An empty
         *        string means all characters. Example "0123456789".
         * @arg halign
         * @arg valign
         */
        MonospacedLabel (const std::string &text="", char widthSample = 'm',
               std::string monospaceCharacters = "", 
               HAlignment halign=HALIGN_CENTER, VAlignment valign=VALIGN_CENTER);

        // Widget interface
        virtual void draw (ecl::GC &gc, const ecl::Rect &r);
        virtual void naturalsize (int &w, int &h) const;
    protected:
        char sampleChar;
        std::string monoChars;
    };
}} // namespace enigma::gui
#endif
