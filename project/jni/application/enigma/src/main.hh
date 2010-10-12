/*
 * Copyright (C) 2003,2004 Daniel Heck
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
#ifndef ENIGMA_MAIN_HH
#define ENIGMA_MAIN_HH

#define ENIGMACOMPATIBITLITY 1.01
#define PREFFILENAME "enigmarc.xml"
#define RATINGSFILENAME "ratings.xml"

#include "file.hh"
#include "PreferenceManager.hh"
#include "StateManager.hh"
#include "DOMErrorReporter.hh"
#include "DOMSchemaResolver.hh"
#include <string>
#include <vector>
#include <memory>
#include <xercesc/dom/DOMImplementation.hpp>
#include <xercesc/dom/DOMImplementationLS.hpp>
#if _XERCES_VERSION >= 30000
#include <xercesc/dom/DOMLSParser.hpp>
#else
#include <xercesc/dom/DOMBuilder.hpp>
#endif
#include <xercesc/util/TransService.hpp>

namespace enigma
{
/* -------------------- Application services -------------------- */

    class ImageManager;         // in resources.hh
    class FontManager;          // in resources.hh

    /**
     * Main object for initialization and resource access.  Even though this
     * class is a Highlander, it is not implemented as a singleton. Instead a 
     * global variable "Application app" is provided for fast access to the 
     * unique instance. All major resources are directly accessible via public
     * ivars. They should be read only. We do without set/get-methods to gain
     * simple and fast access.
     * <p>The Application provides a set of path and pathlist (<code>GameFs</code>)
     * ivars. They are initialized OS and installation dependent. For consistency
     * every file access to a relative path should be resolved via the appropriate
     * path ivars. All paths are provided in local code pages.</p>
     */
    class Application {
    public:
        static Application *get_instance();

        Application();

        void init(int argc, char **argv);
        void shutdown();
        std::string getVersionInfo();
        void setLanguage(std::string newLanguage);
        /**
         * Define a new user path. Files stored to user path use the new path
         * at once and the path is added to the resource path for file lookups.
         * But existing files on the previous user path are not copied or moved
         * to the new location! This remains a manual task for the user. The new
         * user path is added to the preferences.
         * @param newPath local encoded new user path or empty string for default
         */
        void setUserPath(std::string newPath);
        
        /**
         * Define a new user image path. Images are stored to new path
         * at once and the path is added to the resource path for images lookups.
         * But existing images on the previous user images path are not copied or moved
         * to the new location! This remains a manual task for the user. The new
         * user images path is added to the preferences.
         * @param newPath local encoded new path or empty string for default
         */
        void setUserImagePath(std::string newPath);

        //---------- Variables ----------//

        std::vector<std::string>  args; // List of command line arguments
        std::string               progCallPath; // callpath with name of this program

        bool wizard_mode;
        bool nograb;            // Do not grab mouse cursor during game
        std::string language;   // Language to use
        std::string defaultLanguage;
        std::string argumentLanguage;
        
        /**
         * pathlist for enigma release dependent system files. Files like 
         * XML schema definitions, Lua initialization code should be resolved
         * via this pathlist. The pathlist is set to the enigma release dependent
         * data directory preceeded by an optional commandline data path (used
         * by developers to run enigma without installation).
         */
        GameFS *systemFS;
        
        /**
         * pathlist for resources that are either version independent or include
         * versioning in their filename schema. Used for read access to files like
         * sounds, fonts, images and versioned levelindices and levels. The pathlist
         * is set to the enigma release dependent data directory preceeded by
         * OS library resource paths, preceeded by the optional commandline data path,
         * preceeded by the user data path.
         */
        GameFS *resourceFS; // other r data sounds,..., inidices, levels
        
        /**
         * the path to the user preferences file. The directory is OS dependent
         * with priority of the "HOME" environment. The filename itself is
         * standard, but can be overriden by command line option (main usage for
         * developers to start enigma in test configuration).
         */
        std::string prefPath;
        
        /**
         * the path to the gettext l10n data. 
         */
        std::string l10nPath;
        
        /**
         * the path to user data. The base directory for files like score, user
         * levelindices, levels, etc.. The directory is OS dependent
         * with priority of the "HOME" environment, but can be overriden by a
         * user preference. This allows a user to store his data on USB sticks
         * and shared partitions.
         */
        std::string userPath;
        
        /**
         * the path for large image data. Files like thumbs and screenshots
         * that the user may not want to be stored on the userPath. The main
         * reason for this separation of user data should be limited space on
         * userPath. The userImagePath defaults to userPath, but can be overriden
         * by a user preference.
         */
        std::string userImagePath;
        
        /**
         * the singleton instance of our preference manager.
         */
        PreferenceManager * prefs;
        
        /**
         * the singleton instance of our state manager.
         */
        StateManager * state;
        
        /**
         * A xerces transcoder for utf-8.
         */
        XERCES_CPP_NAMESPACE_QUALIFIER XMLTranscoder   *xmlUtf8Transcoder;
        
        /**
         * The implementation of DOM Core.
         */
        XERCES_CPP_NAMESPACE_QUALIFIER DOMImplementation   *domImplementationCore;

        /**
         * The implementation of DOM Load and Save.
         */
        XERCES_CPP_NAMESPACE_QUALIFIER DOMImplementationLS *domImplementationLS;

        /**
         * A configured DOM parser for reusage.
         */
#if _XERCES_VERSION >= 30000
        XERCES_CPP_NAMESPACE_QUALIFIER DOMLSParser *domParser;
#else    
        XERCES_CPP_NAMESPACE_QUALIFIER DOMBuilder *domParser;
#endif
        /**
         * The error handler attached to the domParser
         */
        DOMErrorReporter *domParserErrorHandler;

        /**
         * The schema resolver attached to the domParser
         */
        DOMSchemaResolver *domParserSchemaResolver;
        
        /**
         * A configured DOM serializer for reusage.
         */
#if _XERCES_VERSION >= 30000
        XERCES_CPP_NAMESPACE_QUALIFIER DOMLSSerializer *domSer;
#else    
        XERCES_CPP_NAMESPACE_QUALIFIER DOMWriter *domSer;
#endif
        /**
         * The error handler attached to the domSer
         */
        DOMErrorReporter *domSerErrorHandler;
        bool errorInit;
        bool isMakePreviews;

    private:
        void initSysDatapaths(const std::string &prefFilename);
        void initXerces();
        void initUserDatapaths();
        void updateMac1_00();
        std::string systemAppDataPath;    // dir path to the apps data
        std::string systemCmdDataPath;    // commandline override of systemAppDataPath
        std::string userStdPath;          // standard user data path
        std::string userStdPathMac1_00;   // standard user data path as of Mac 1.00
        
        void init_i18n();
    };

#define APP enigma::Application::get_instance()

    extern Application app;

    //---------- Logging ----------//
    extern std::ostream Log;
    
    /**
     * Flag to use in expensive asserts as first operand in a logical or
     * statement to allow the assertion to be blocked in favour of speed.
     * F.e. <code>ASSERT(noAssert || long_lasting_check(), XLevelRuntime, "");<code>
     */
    extern bool noAssert;

    //---------- Command line options ----------//
    extern bool   WizardMode;
    extern bool   Nograb;
}

#endif
