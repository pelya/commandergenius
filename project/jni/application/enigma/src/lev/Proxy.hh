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
 
#ifndef LEV_PROXY_HH_INCLUDED
#define LEV_PROXY_HH_INCLUDED

#include "enigma.hh"

#include <map>
#include <set>
#include <string>
#include <xercesc/dom/DOMDocument.hpp>

namespace enigma { namespace lev {
    enum controlType {force, balance, key, other};
    enum scoreUnitType {duration, number};
    enum scoreTargetType {time, pushes, moves, callback};
    enum levelStatusType {
        STATUS_RELEASED,
        STATUS_STABLE,
        STATUS_TEST,
        STATUS_EXPERIMENTAL,
        STATUS_UNKNOWN
    };
    
    /**
     * A standin for an addressable level file and its level metadata.
     * Every level index and the commandline register their levels with the
     * known level metadata. The unique proxy is henceforth used to access
     * level data and to load the level. The level metadata are updated on
     * level load and can be updated without a complete load of the level.
     * 
     * The supplied level address is analysed and normalized to a unique
     * schema. Urls, Enigma resource path addresses, absolute paths and
     * Oxyd level addresses are supported.
     *
     * Mismatches of levels to requested levels are detected on level load.
     * Old level revisions on the users Enigma home directory shadowing new
     * revisions of a new Enigma installation will be handled, too.
     */
    class Proxy {
    public:
        enum pathType { pt_url, pt_resource, pt_absolute, pt_oxyd};
        static const XMLCh levelNS[]; // the XML namespace
        static Proxy *loadedLevel(); // tmp ?
        
        /**
         * The registration of a level.
         * @arg levelPath  as stored in indices or entered on the commandline.
         *            valid formats are: 
         *            welcome, ./firefox, stable/welcome:, ftp://..., #oxyd#17
         * @arg indexPath  a path identifier of the index in strict standard form:
         *            stable, "", #commandline, #history, #oxyd, http://...
         *            resource path level packs use the subdirectory name below
         *            levels or "", zipped packs the filename without suffix
         * @arg levelId  the version independent level id as used for scoring
         * @arg levelTitle the english title of the level
         * @arg levelAuthor the name of the author
         * @arg levelScoreVersion the score version
         * @arg levelRelease the level compatibility release version
         * @arg levelHasEasymode support of an easy mode
         */
        static Proxy *registerLevel(std::string levelPath, std::string indexPath,
                std::string levelId, std::string levelTitle, std::string levelAuthor,
                int levelScoreVersion, int levelRelease, bool levelHasEasymode,
                GameType levelCompatibilty, levelStatusType status =STATUS_RELEASED,
                int levelRevision = 0);
        
        static Proxy *autoRegisterLevel(std::string indexPath, std::string filename);

        static std::string search(std::string text);
        static void countLevels();
        static std::set<std::string> getLevelIds(bool withEasy);
        static std::set<Proxy *> getProxies();

        void loadLevel();
        void loadMetadata(bool expectLevel);
        Proxy * copy(std::string newBasePath, std::string newPackPath, bool backup = true);
        
        /**
         * Retrieve and translate a level string. The key may be "title",
         * "subtitle" or any level specific string key. The priorities for
         * translation are as follows: protected translation - gettext 
         * translation - public translation - protected english - key
         * @arg key     the key for the search string
         * @return      the translation of the string
         */
        std::string getLocalizedString(const std::string &key);
        
        std::string getId();
        int getScoreVersion();
        int getReleaseVersion();
        int getRevisionNumber();
        levelStatusType getLevelStatus();
        std::string getAuthor();
        std::string getTitle(); // english title
        bool hasEasymode();
        std::string getContact();
        std::string getHomepage();
        controlType getControl();
        scoreUnitType getScoreUnit();
        std::string getScoreTarget();
        std::string getCredits(bool infoUsage);
        std::string getDedication(bool infoUsage);
        int getEasyScore();
        int getDifficultScore();
        GameType getEngineCompatibility();
        double getEnigmaCompatibility();
        
        /**
         * the level address that can be used independent of a level pack
         * as a crossreference. (stable/welcome, #oxyd#17, http://..., ~/test)
         */
        std::string getNormLevelPath();
        
        /**
         * The normalized level path with all critical characters substituted
         * by '~' to allow url's to be used and to make generated paths portable.
         * Url protocols like "http://" are substituted by "http/". The returned
         * path can be used as a local path element.
         */
        std::string getLocalSubstitutionLevelPath();
        
        /**
         * the type of the level address
         */
        Proxy::pathType getNormPathType();
        std::string getAbsLevelPath();
        void loadDependency(std::string depId);
        void release();
    private:
        static Proxy *currentLevel;
        static std::map<std::string, Proxy *> cache;
        static std::vector<Proxy *> loadedLibs;
        static std::vector<Proxy *> registeredLibs;
        static void releaseLibs();
        
        bool isLibraryFlag;
        pathType normPathType;
        std::string normLevelPath; // stable/welcome, #oxyd#17, http://..., ~/test
        std::string absLevelPath;
        std::string id; // level id - old filename or indexname
        std::string title; // old name
        std::string author;
        int scoreVersion;
        int releaseVersion;
        int revisionNumber;
        levelStatusType levelStatus;
        bool hasEasymodeFlag;
        scoreUnitType scoreUnit;
        /**
         * The compatibility that needs to be preset on level load.
         * Usually level set the compatibility themselves on load. But heritage
         * levels loaded from DAT-files do not. Thus we need to keep the type
         * info in the Proxy as an exception.
         */
        GameType engineCompatibility;
        XERCES_CPP_NAMESPACE_QUALIFIER DOMDocument *doc;
        XERCES_CPP_NAMESPACE_QUALIFIER DOMElement *infoElem;
        XERCES_CPP_NAMESPACE_QUALIFIER DOMNodeList *stringList;
        
        Proxy(bool proxyIsLibrary, pathType thePathType, std::string theNormLevelPath,
                std::string levelId, std::string levelTitle, std::string levelAuthor,
                int levelScoreVersion, int levelRelease, bool levelHasEasymode,
                GameType levelCompatibilty, levelStatusType status, int levelRevision = 0);
        ~Proxy();
        void load(bool onlyMetadata, bool expectLevel);
        void loadDoc();
        void loadLuaCode();
        void processDependencies();   
        void registerPreloadDependency(std::string depPath, std::string depId,
            int depRelease, bool depPreload, std::string depUrl);
        std::string getType();
        bool updateId();
        bool updateReleaseVersion();
        int scoreText2Int(std::string text);
    };
}} // namespace enigma::lev
#endif

