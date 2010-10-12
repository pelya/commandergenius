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
#include "ecl_argp.hh"
#include <iostream>

using namespace std;
using ecl::ArgParser;


ArgParser::ArgParser() {
}

ArgParser::~ArgParser(){
}

void ArgParser::on_error (ErrorType t, const std::string &arg) {
    cerr << "Error: " << errormsg(t, arg) << endl;
}

void ArgParser::on_argument (const std::string &/*arg*/) {
}

void ArgParser::on_option (int /*id*/, const std::string &/*param*/)
{
}

void ArgParser::def (int id, const std::string &name, 
		     char abbr, bool takesparam)
{
    m_opts.push_back (Option(id, abbr, string("--") + name, takesparam));
}

void ArgParser::def (bool *boolvar, const std::string &name, 
		     char abbr) 
{
    Option opt(-1, abbr, string("--") + name, false);
    opt.boolvar = boolvar;
    m_opts.push_back(opt);
}

string ArgParser::errormsg(ErrorType t, const std::string &arg) const {
    string errmsg;
    switch (t) {
    case InvalidParam: errmsg = "Invalid parameter for option `"; break;
    case AmbiguousOpt: errmsg = "Ambiguous command line option `"; break;
    case UnknownOpt:   errmsg = "Unknown command line option `"; break;
    case MissingParam: errmsg = "Missing parameter for option `"; break;
    default:
        return string();
    }
    errmsg += arg;
    errmsg += "'.";
    return errmsg;
}

void ArgParser::process_option(Option &opt, const std::string &param) {
    if (opt.boolvar)
	*opt.boolvar = true;
    else
	on_option(opt.id, param);
}

void ArgParser::getlongopt (const std::string &arg) {
    string::const_iterator eqpos = find(arg.begin(), arg.end(), '=');
    string optname(arg.begin(), eqpos);

    option_iterator i, j;
    j = i = find_prefix (m_opts.begin(), m_opts.end(), optname);

    if (i == m_opts.end()) {
        on_error (UnknownOpt, optname);
    }
    else if (find_prefix(++j, m_opts.end(), optname) != m_opts.end()) {
        on_error (AmbiguousOpt, optname);
    }
    else if (i->takesparam) {
	if (eqpos != arg.end()) {
	    process_option (*i, string(eqpos+1, arg.end()));
	}
	else if (!m_arglist.empty()) {
	    process_option (*i, m_arglist.front());
	    m_arglist.pop_front();
	}
	else
            on_error (MissingParam, optname);
    }
    else if (eqpos != arg.end()) {
        on_error (InvalidParam, optname);
    }
    else
        process_option(*i, "");
}

void ArgParser::getshortopt (const std::string &arg) {
    option_iterator i = m_opts.begin();
    for (; i != m_opts.end(); ++i)
	if (i->shortopt == arg[1])
	    break;

    string option = arg.substr(0, 2);

    if (i == m_opts.end()) {
        on_error (UnknownOpt, option);
    }
    else if (i->takesparam) {
        string param = arg.substr(2);
	if (param.empty()) {
	    if (!m_arglist.empty()) {
		param = m_arglist.front();
		m_arglist.pop_front();
                process_option (*i, param);
	    }
	    else
                on_error (MissingParam, option);
	} else
            process_option (*i, param);
    }
    else {
        process_option (*i, "");

        // The following characters can be options in their own right
        // (single-character options can be grouped, as in -hvx), so
        // put them back into the argument list.
	std::string newarg("-");
	newarg.append(arg.begin()+2, arg.end());
	if (newarg != "-")
	    m_arglist.push_front(newarg);
    }
}

void ArgParser::parse () {
    bool no_more_opts = false;
    while (!m_arglist.empty()) {
	std::string arg = m_arglist.front();
	m_arglist.pop_front();

	if (!no_more_opts && arg.size() >= 2 && arg[0] == '-') {
            // Note: "-" is treated as an ordinary argument
	    if (arg[1] == '-') {
                if (arg.size() == 2) // "--" terminates the option list
                    no_more_opts = true;
                else
                    getlongopt (arg);
            }
	    else
                getshortopt (arg);
	}
	else
            on_argument (arg);
    }
}
