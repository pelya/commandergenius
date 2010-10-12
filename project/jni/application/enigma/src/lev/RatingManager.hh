/*
 * Copyright (C) 2006, 2007 Ronald Lamprecht
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
 
#ifndef RATINGMGR_HH_INCLUDED
#define RATINGMGR_HH_INCLUDED

#include "lev/Proxy.hh"
#include "enigma.hh"

#include <map>
#include <string>
#include <ctime>

namespace enigma { namespace lev {
    /**
     * The public rating info is currently just a data structure as the data
     * have no behaviour. No public direct access to the data should occur.
     */
    struct Rating
    {
        // Constructor
        Rating();

        // Constants
        enum { DEFAULT_TIME = 99*60+59 };

        // Variables.
        short      intelligence;
        short      dexterity;
        short      patience;
        short      knowledge;
        short      speed;
        short           bestScoreEasy;         // Best time in seconds or Minimum moves to solve level(for easy mode) 
        short           bestScoreDifficult;    // Best time in seconds or Minimum moves to solve level(for normal mode)
        std::string   bestScoreEasyHolder;       // player name(s) for 'best_time_easy'
        std::string   bestScoreDifficultHolder;  // same for 'best_time_normal'
        short   parScoreEasy;
        short   parScoreDifficult;
        int     numSolvedEasy;
        int     numSolvedDifficult;
        short   pdmSolvedEasy;  // per deca mille - 1/10000
        short   pdmSolvedDifficult;
        short   averageRating;  // rating * 10
    };

    /**
     * A singelton manager for public ratings and score statistics
     * The singelton loads the ratings on initialization and caches them. An
     * update methods allows to online update the ratings and a save method that
     * should be called on game exit saves the cached ratings if they have been
     * modified.
     *
     * The key to rating access is the Proxy. A rating is bound to a level id
     * together with its scoring version.
     * An up to date rating file is distributed with the application. These
     * ratings are filled up by existing ratings and stored on the users enigma
     * path. On an update which can be incremental or full the downloaded ratings
     * replace their predecessors. All other ratings are kept.
     *
     * Only the rating administrator can and should edit ratings by copying
     * the current offical rating file to his user enigma path, editing
     * the ratings and leaving Enigma what saves the ratings with the current
     * time stamp. The rating administrator has to ensure that he plays
     * a clean Enigma version without any additional level packs on his 
     * resource path!
     *
     * Note: the time handling is a little bit weird due to limitations in the
     * C++ standard libs. As no convertion for GMT to time_t exists and we
     * would have to know the daylight saving offsets for a correct localtime
     * handling we cheat with our time handling: We save correct GMT time values.
     * But we read these values and convert them to local times without daylight
     * saving correctures. This does not harm as we use these values only for
     * comparisons and nothing more!
     */
    class RatingManager {
    public:
        static RatingManager *instance();
        ~RatingManager();
        
        /**
         * Online updates the ratings from the update URLs. Online updates
         * are only processed if the delay period after the last successful
         * update is over. This feature reduced network access and server
         * load.
         * Unsuccessful online updates do not harm.
         */
        void update();
        
        /**
         * Save the ratings if changes are pending.
         */
        void save();
        
        /**
         * Registers an 0.92 index.txt rating in the cache. This rating will be
         * used and stored in the new users ratings.xml only if no newer rating
         * exists.
         */
        void registerRating(std::string levelId, short levelScoreVersion,
                short intelligence, short dexterity, short patience,
                short knowledge, short speed, short bestScoreEasy, 
                std::string bestScoreEasyHolder, short bestScoreDifficult,
                std::string bestScoreDifficultHolder);
        short getIntelligence(Proxy *levelProxy);
        void setIntelligence(Proxy *levelProxy, short intelligence);
        short getDexterity(Proxy *levelProxy);
        void setDexterity(Proxy *levelProxy, short dexterity);
        short getPatience(Proxy *levelProxy);
        void setPatience(Proxy *levelProxy, short patience);
        short getKnowledge(Proxy *levelProxy);
        void setKnowledge(Proxy *levelProxy, short knowledge);
        short getSpeed(Proxy *levelProxy);
        void setSpeed(Proxy *levelProxy, short speed);
        short getDifficulty(Proxy *levelProxy);
        short getBestScore(Proxy *levelProxy, int difficulty);
        std::string getBestScoreHolder(Proxy *levelProxy, int difficulty, int cut = 0);
        short getBestScoreEasy(Proxy *levelProxy);
        std::string getBestScoreEasyHolder(Proxy *levelProxy, int cut = 0);
        short getBestScoreDifficult(Proxy *levelProxy);
        std::string getBestScoreDifficultHolder(Proxy *levelProxy, int cut = 0);
        short getParScore(Proxy *levelProxy, int difficulty);
        short getParScoreEasy(Proxy *levelProxy);
        short getParScoreDifficult(Proxy *levelProxy);
        short getNumSolvedEasy(Proxy *levelProxy);
        short getNumSolvedDifficult(Proxy *levelProxy);
        short getPdmSolvedEasy(Proxy *levelProxy);
        std::string getPcSolvedEasy(Proxy *levelProxy);
        short getPdmSolvedDifficult(Proxy *levelProxy);
        std::string getPcSolvedDifficult(Proxy *levelProxy);
        short getDAverageRating(Proxy *levelProxy);
        std::string getAverageRating(Proxy *levelProxy);
    protected:
        RatingManager();
    private:
        enum loadResult { updated, checked, error};
        static RatingManager *theSingleton;
        std::map<std::string, Rating *> cache;
        std::time_t ratingVersion;
        std::string ratingVersionString;
        std::string urlFullUpdate;
        std::string urlIncrementalUpdate;
        short updateMinDelay;
        Rating * findRating(Proxy * levelProxy);
        Rating * registerNewRating(Proxy * levelProxy);
        std::string cutHolders(std::string org, int factor);
        
        /**
         * Loads the ratings from a given URI and updates the cached ratings.
         * Not existing ratings are always added, existing ratings are updated
         * only if the rating version is newer than the cached version.
         * @arg path  a local filepath or a URL
         * @return    ratings cache updated, no updates but file checked, or error
         */
        loadResult loadURI(std::string path);
        bool didAddRatings;
        bool didEditRatings;
    };
}} // namespace enigma::lev

#endif
