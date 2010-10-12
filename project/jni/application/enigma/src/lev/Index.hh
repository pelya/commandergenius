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
 
#ifndef LEV_INDEX_HH_INCLUDED
#define LEV_INDEX_HH_INCLUDED

#include "lev/Proxy.hh"

#include <map>
#include <string>
#include <vector>

#define INDEX_DEFAULT_GROUP "User"
#define INDEX_EVERY_GROUP "Every Group"
#define INDEX_ALL_PACKS "All Packs"

#define INDEX_STARTUP_PACK_NAME   "Startup Levels"
#define INDEX_AUTO_PACK_NAME      "Auto Folder"
#define INDEX_HISTORY_PACK_NAME   "History"
#define INDEX_SEARCH_PACK_NAME    "Search Result"
#define INDEX_CLIPBOARD_PACK_NAME "Clipboard"

#define INDEX_STARTUP_PACK_LOCATION   5100
#define INDEX_AUTO_PACK_LOCATION      5200
#define INDEX_HISTORY_PACK_LOCATION   5300 
#define INDEX_SEARCH_PACK_LOCATION    5400
#define INDEX_CLIPBOARD_PACK_LOCATION 5500

#define INDEX_USER_PACK_LOCATION     50000
#define INDEX_DEFAULT_PACK_LOCATION  69000

#define INDEX_GROUP_COLUMN_UNKNOWN -1000



namespace enigma { namespace lev {    
    enum LevelAdvanceMode {
        ADVANCE_NEXT_MODE,       // honor NextLevelMode
        ADVANCE_STRICTLY,        // move to the next level in index
        ADVANCE_UNSOLVED         // move to next not yet solved level
    };

    enum NextLevelMode {
        NEXT_LEVEL_STRICTLY,    // move to the next level in index
        NEXT_LEVEL_UNSOLVED,    // move to next not yet solved level
        NEXT_LEVEL_NOT_BEST,    // move to next level where player is not best score holder
        NEXT_LEVEL_OVER_PAR     // move to next level with a score over PAR 
    };
    
    /**
     * 
     */
    class Index  {
    public:
        static void initGroups();
        static void registerIndex(Index *anIndex);
        static Index * findIndex(std::string anIndexName);
        static Index * getCurrentIndex();
        static bool setCurrentIndex(std::string anIndexName);
        static Index * nextGroupIndex();
        static Index * previousGroupIndex();
        static Proxy * getCurrentProxy();
        static std::string getCurrentGroup();
        static void setCurrentGroup(std::string groupName);
        static std::vector<std::string> getGroupNames();
        static std::vector<Index *> * getGroup(std::string groupName);
        static std::string getGroupSelectedIndex(std::string groupName);
        static int getGroupSelectedColumn(std::string groupName);
        static void setGroupSelectedIndex(std::string groupName, std::string indexName);
        static void setGroupSelectedColumn(std::string groupName, int column);
        static void deleteGroup(std::string groupName);
        static void moveGroup(std::string groupName, int newPos);
        static void renameGroup(std::string oldName, std::string newName);
        static void insertGroup(std::string groupName, int newPos);
        static double getNextUserLocation();
        
        /**
         * Convention: method names *Level() can take int pos or Proxy as arg.
         */
        Index(std::string anIndexName = "Unnamed Pack", 
                std::string aGroupName = INDEX_DEFAULT_GROUP, 
                double defaultLocation = INDEX_DEFAULT_PACK_LOCATION);
        ~Index();
        
        std::string getName();
        std::string getGroupName();
        std::string getDefaultGroupName();
        double getLocation();
        double getDefaultLocation();
        void setDefaultLocation(double defLocation);
        void moveToGroup(std::string groupName);
        void locateBehind(std::string indexName);
        void renameIndex(std::string newName);
        virtual bool isSource(Proxy * aProxy);

        int getCurrentPosition(); // 0 .. size-1
        int getCurrentLevel(); // 1 .. size
        Proxy * getCurrent();
        void setCurrentPosition(int newPos);
        int getScreenFirstPosition();
        void setScreenFirstPosition(int iFirstPos);
        virtual bool mayPlayLevel(int levelNumber);
        Proxy * getProxy(int pos);
        bool containsProxy(Proxy * aProxy);
        bool hasNormLevelPath(std::string path);
        virtual bool advanceLevel(LevelAdvanceMode advMode);
                
        /*! Return number of levels */
        virtual int size() const;
        virtual void appendProxy(Proxy * newLevel, controlType varCtrl = force,
                scoreUnitType varUnit = duration, std::string varTarget = "time",
                std::map<std::string, std::string> varExtensions = nullExtensions);
        virtual void clear();
        void updateFromProxies();

        // ---------- LevelPack legacy methods ---to be renamed ------- */
        /*! Return the default SoundSet (see options::SoundSet for meaning) */
        virtual const char* get_default_SoundSet() const;

        /*! Returns true if it's a twoplayer levelpack, but has no
          it-yinyang (needed to add it-yinyang to inventory if
          oxyd-linkgame is played as single-player) */
        virtual bool needs_twoplayers() const;


    protected:
        std::string indexName;
        std::string indexGroup;
        double indexLocation;
        std::string defaultGroup;
        double indexDefaultLocation;
        int currentPosition; // 0,...
        int screenFirstPosition; // LevelWidget ifirst
        std::vector<Proxy *> proxies;
        static std::map<std::string, std::string> nullExtensions;
    
    private:
        /**
         * A map of index names to the indices themselves.
         */
        static std::map<std::string, Index *> indices;
        
        /**
         * A map of index group names to vectors of indices. The vectors
         * are sorted by the user sequence preference in the index group menu.
         * Every index is listed in the group the user asigned it to.
         */
        static std::map<std::string, std::vector<Index *> *> indexGroups;
        
        /**
         * Current active index. This index is selected in the Levelpack menu,
         * shown and used in the submenus and stored in the user preferences.
         * It's default is the "Tutorial" index.
         */
        static Index * currentIndex;
        static std::string currentGroup;
        
        static void addIndexToGroup(Index *anIndex, std::vector<Index *> * aGroup);
        static void removeIndexFromGroup(Index *anIndex, std::string groupName);

    };

}} // namespace enigma::lev
#endif
