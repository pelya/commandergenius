/*
 * Copyright (C) 2000,2003 Daniel Heck
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
#ifndef ECL_ARGP_HH
#define ECL_ARGP_HH

#include "ecl_util.hh"
#include <list>
#include <algorithm>

namespace ecl 
{ 
    class ArgParser : public ecl::Nocopy {
    public:
        // Constructors.
        ArgParser ();

        template <class Iter>
        ArgParser(Iter beg, Iter end) {
            feed (beg, end);
        }

        // Destructor.
        virtual ~ArgParser();

        // Types.
        enum ErrorType {
            UnknownOpt,         // Unkown option found
            AmbiguousOpt,       // Abbreviation matches two options
            MissingParam,       // Expected parameter is missing
            InvalidParam        // Option does not take parameter
        };

        //
        // Public interface.
        //
        
        virtual void on_error (ErrorType t, const std::string &arg);
        virtual void on_argument (const std::string &arg);
        virtual void on_option (int id, const std::string &param);

        //
        // Functions.
        //

        /* Feed new arguments into the parser. */
        template <class ForwardIterator>
        void feed (ForwardIterator begin, ForwardIterator end) {
            std::copy(begin, end, std::back_inserter(m_arglist));
        }
    
        /* Define a new option. */
        void def (int id, const std::string &name, char abbr = 0, bool param = false);
	void def (bool *boolvar, const std::string &name, char abbr = 0);

        /* Parse all command line arguments, calling the appropriate
           handlers at the right time. */
        void parse();

        std::string errormsg(ErrorType t, const std::string &arg) const;

    private:
        struct Option {
            Option (int id_, char s='\0', const std::string& l="", bool param=false)
                : id(id_), shortopt(s), longopt(l), takesparam(param), boolvar(0)
            {}
		
            int         id;
            char        shortopt;
            std::string longopt;
            bool        takesparam;
	    bool	*boolvar;
        };


        // Private Variables.
        typedef std::list<Option>::iterator option_iterator;
    
        std::list<Option>      m_opts;
        std::list<std::string> m_arglist;
        std::string            m_lastopt; // Last option seen

        // Private functions.
	void process_option(Option &opt, const std::string &param);

        void getlongopt (const std::string& arg);
        void getshortopt (const std::string& arg);

        template <class Iter>
        static Iter
        find_prefix(Iter beg, Iter end, const std::string& val) {
            typename std::string::size_type s = val.size();
            for (; beg != end; ++beg) {
                // This is the way the comparison should be done,
                // but gcc doesn't like it yet...
                //   if ((beg->longopt).compare(0, s, val) == 0)
                if ((beg->longopt).substr(0, s) == val)
                    break;
            }
            return beg;
        }
    };
}

#endif
