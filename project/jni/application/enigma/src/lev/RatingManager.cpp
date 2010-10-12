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
 *
 */

#include "lev/RatingManager.hh"

#include "main.hh"
#include "ecl_util.hh"
#include "errors.hh"
#include "LocalToXML.hh"
#include "Utf8ToXML.hh"
#include "XMLtoUtf8.hh"

#include <iostream>
#include <cstdlib>
#include <sstream>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/util/XercesVersion.hpp>
#if _XERCES_VERSION < 30000
#include <xercesc/framework/LocalFileFormatTarget.hpp>
#endif

using namespace std;
using namespace enigma;
XERCES_CPP_NAMESPACE_USE 


namespace enigma { namespace lev {
    std::time_t timeValue(const std::string &timeString) {
        std::tm time;
        time.tm_year = atoi(timeString.substr(0,4).c_str()) - 1900;
        time.tm_mon  = atoi(timeString.substr(5,2).c_str()) - 1;
        time.tm_mday = atoi(timeString.substr(8,2).c_str());
        time.tm_hour = atoi(timeString.substr(11,2).c_str());
        time.tm_min  = atoi(timeString.substr(14,2).c_str());
        time.tm_sec  = atoi(timeString.substr(17,2).c_str());
        time.tm_isdst = false;
        return std::mktime(&time);
    }

    std::time_t timeValue(const XMLCh * const string) {
        std::string timeString = XMLtoUtf8(string).c_str();
        return timeValue(timeString);
    }

    Rating::Rating() 
    {
        intelligence    = 0;
        dexterity       = 0;
        patience        = 0;
        knowledge       = 0;
        speed           = 0;
        bestScoreEasy   = -1;          //DEFAULT_TIME;
        bestScoreDifficult = -1;       //DEFAULT_TIME;
        parScoreEasy       = -1;
        parScoreDifficult  = -1;
        numSolvedEasy      = 0;
        numSolvedDifficult = 0;
        pdmSolvedEasy      = 0;
        pdmSolvedDifficult = 0;
        averageRating      = -1;
    }


    RatingManager *RatingManager::theSingleton = 0;
    
    RatingManager* RatingManager::instance() {
        if (theSingleton == 0) {
            theSingleton = new RatingManager();
        }
        return theSingleton;
    }

    
    
    RatingManager::RatingManager() : didAddRatings (false), didEditRatings (false) {
        std::tm time;
        time.tm_year = 2006 - 1900; // the past - all ratings are newer as this
        time.tm_mon  = 02;   //   is the date it was coded
        time.tm_mday = 30;
        time.tm_hour = 20;
        time.tm_min  = 10;
        time.tm_sec  = 00;
        time.tm_isdst = false;
        ratingVersion = std::mktime(&time);
        if(app.state->getString("RatingsUpdateTime").empty()) {
            // guarantee usable "RatingsUpdateTime"
            char str[22];
            std::strftime(str, 22, "%Y-%m-%dT%H:%M:%SZ", std::gmtime(&ratingVersion));
            std::string currentTimeString = str;
            app.state->setProperty("RatingsUpdateTime", currentTimeString);
        }
        std::string sysRatingPath;
        std::string userRatingPath;
        if (app.systemFS->findFile(RATINGSFILENAME, sysRatingPath)) {
            // preload cache with application distributed ratings
            loadURI(sysRatingPath);
        }
        if (app.resourceFS->findFile(RATINGSFILENAME, userRatingPath) &&
                userRatingPath != sysRatingPath) {
            // update with user saved ratings - ignore cache changes as there
            // is no reason to store the cache.
            loadURI(userRatingPath);
        }
        if (app.prefs->getBool("RatingsAutoUpdate") == true)
            update();
    }
    
    RatingManager::~RatingManager() {
    }
    
    RatingManager::loadResult RatingManager::loadURI(std::string path) {
        RatingManager::loadResult status = checked;
        try {
            app.domParserErrorHandler->resetErrors();
            app.domParserErrorHandler->reportToErr();
            app.domParserSchemaResolver->resetResolver();
            app.domParserSchemaResolver->addSchemaId("ratings.xsd","ratings.xsd");
            DOMDocument *doc = app.domParser->parseURI(path.c_str());
            if (doc != NULL && !app.domParserErrorHandler->getSawErrors()) {
                DOMElement *updateElem = 
                        dynamic_cast<DOMElement *>(doc->getElementsByTagName(
                        Utf8ToXML("update").x_str())->item(0));
                std::time_t newVersion = timeValue(updateElem->getAttribute(Utf8ToXML("date").x_str()));
                bool isUpdate = (std::difftime(newVersion, ratingVersion) > 0) ? true : false;
                if (isUpdate) {
                    ratingVersion = newVersion;
                    ratingVersionString = XMLtoUtf8(updateElem->getAttribute(Utf8ToXML("date").x_str())).c_str();
                    urlFullUpdate = XMLtoUtf8(updateElem->getAttribute(Utf8ToXML("urlFull").x_str())).c_str();
                    urlIncrementalUpdate = XMLtoUtf8(updateElem->getAttribute(Utf8ToXML("urlIncremental").x_str())).c_str();
                    updateMinDelay = XMLString::parseInt(updateElem->getAttribute(Utf8ToXML("delay").x_str()));
                }
                DOMNodeList *levelList = doc->getElementsByTagName( 
                            Utf8ToXML("level").x_str());
                for (int i = 0, l = levelList-> getLength();  i < l; i++) {
                    DOMElement *levelElem = dynamic_cast<DOMElement *>(levelList->item(i));
                    const XMLCh * attr = levelElem->getAttribute(Utf8ToXML("id").x_str());
                    std::string id = XMLtoUtf8(attr).c_str();
                    attr = levelElem->getAttribute(Utf8ToXML("sv").x_str());
                    short sv = XMLString::stringLen(attr) > 0 ? XMLString::parseInt(attr) : 0;
                    std::string cacheKey = id + "#" + ecl::strf("%d", sv);
                    std::map<std::string, Rating *>::iterator it = cache.find(cacheKey);
                    Rating * theRating;
                    bool isNewRating = false;
                    if (it != cache.end()) {
                        if (isUpdate)
                            // update the outdated entry
                            theRating = it->second;
                        else
                            // keep the newer entry
                            continue;
                    } else {
                        // add not existing entries in all cases
                        theRating = new Rating();
                        isNewRating = true;
                    }
                    status = updated;
                    attr = levelElem->getAttribute(Utf8ToXML("int").x_str());
                    theRating->intelligence = XMLString::stringLen(attr) > 0 ? XMLString::parseInt(attr) : 0;
                    attr = levelElem->getAttribute(Utf8ToXML("dex").x_str());
                    theRating->dexterity = XMLString::stringLen(attr) > 0 ? XMLString::parseInt(attr) : 0;
                    attr = levelElem->getAttribute(Utf8ToXML("pat").x_str());
                    theRating->patience = XMLString::stringLen(attr) > 0 ? XMLString::parseInt(attr) : 0;
                    attr = levelElem->getAttribute(Utf8ToXML("kno").x_str());
                    theRating->knowledge = XMLString::stringLen(attr) > 0 ? XMLString::parseInt(attr) : 0;
                    attr = levelElem->getAttribute(Utf8ToXML("spe").x_str());
                    theRating->speed = XMLString::stringLen(attr) > 0 ? XMLString::parseInt(attr) : 0;
                    attr = levelElem->getAttribute(Utf8ToXML("bse").x_str());
                    theRating->bestScoreEasy = XMLString::stringLen(attr) > 0 ? XMLString::parseInt(attr) : -1;
                    attr = levelElem->getAttribute(Utf8ToXML("bseh").x_str());
                    theRating->bestScoreEasyHolder = XMLtoUtf8(attr).c_str();
                    attr = levelElem->getAttribute(Utf8ToXML("bsd").x_str());
                    theRating->bestScoreDifficult = XMLString::stringLen(attr) > 0 ? XMLString::parseInt(attr) : -1;
                    attr = levelElem->getAttribute(Utf8ToXML("bsdh").x_str());
                    theRating->bestScoreDifficultHolder = XMLtoUtf8(attr).c_str();
                    attr = levelElem->getAttribute(Utf8ToXML("pare").x_str());
                    theRating->parScoreEasy = XMLString::stringLen(attr) > 0 ? XMLString::parseInt(attr) : -1;
                    attr = levelElem->getAttribute(Utf8ToXML("pard").x_str());
                    theRating->parScoreDifficult = XMLString::stringLen(attr) > 0 ? XMLString::parseInt(attr) : -1;
                    attr = levelElem->getAttribute(Utf8ToXML("solvne").x_str());
                    theRating->numSolvedEasy = XMLString::stringLen(attr) > 0 ? XMLString::parseInt(attr) : 0;
                    attr = levelElem->getAttribute(Utf8ToXML("solvpe").x_str());
                    theRating->pdmSolvedEasy = XMLString::stringLen(attr) > 0 ? XMLString::parseInt(attr) : 0;
                    attr = levelElem->getAttribute(Utf8ToXML("solvnd").x_str());
                    theRating->numSolvedDifficult = XMLString::stringLen(attr) > 0 ? XMLString::parseInt(attr) : 0;
                    attr = levelElem->getAttribute(Utf8ToXML("solvpd").x_str());
                    theRating->pdmSolvedDifficult = XMLString::stringLen(attr) > 0 ? XMLString::parseInt(attr) : 0;
                    attr = levelElem->getAttribute(Utf8ToXML("avgur").x_str());
                    theRating->averageRating = XMLString::stringLen(attr) > 0 ? XMLString::parseInt(attr) : -1;
                    if (isNewRating)
                        cache.insert(std::make_pair(cacheKey, theRating));
                }
                doc->release();
            } else {
                status = error;
            }
            
        }
        catch (const XMLException& toCatch) {
            char* message = XMLString::transcode(toCatch.getMessage());
            cerr << "Exception on load of preferences: "
                 << message << "\n";
            XMLString::release(&message);
            return error;
        }
        catch (const DOMException& toCatch) {
            char* message = XMLString::transcode(toCatch.msg);
            cerr << "Exception on load of preferences: "
                 << message << "\n";
            XMLString::release(&message);
            return error;
        }
        catch (...) {
            cerr << "Unexpected Exception on load of preferences\n" ;
            return error;
        }
        return status;
    }
    
    void RatingManager::update() {
        std::time_t currentTime = std::time(NULL);

        // check if an update is allowed
        std::time_t currentWeirdTime = std::mktime(gmtime(&currentTime));
        double updateDelay = std::difftime(currentWeirdTime, 
                timeValue(app.state->getString("RatingsUpdateTime"))); 
        if (updateDelay/86400 < updateMinDelay) {
            Log << "Rating update not yet allowed: " <<updateDelay<< " s\n";
            return;
        }
        Log << "Rating update allowed\n";
        
        // do update
        bool didUpdate = false;
        loadResult result;
        std::string oldUrlIncrementalUpdate;
        std::string oldUrlFullUpdate;
        do {
            Log << "Ratings update from '" << urlIncrementalUpdate << "'\n";
            oldUrlIncrementalUpdate = urlIncrementalUpdate;
            oldUrlFullUpdate = urlFullUpdate;
            result = loadURI(urlIncrementalUpdate);
            if (result == updated)
                didUpdate = true;
        } while (result != error && 
                oldUrlIncrementalUpdate != urlIncrementalUpdate &&
                oldUrlIncrementalUpdate != oldUrlFullUpdate);
                

        if (result == error) {
            // check if we are allowed for an full update
            if (updateDelay/86400 > 3 * updateMinDelay) {
                result = loadURI(urlFullUpdate);
            }
        }
        
        if (result != error) {
            // save current time as rating update time
            char str[22];
            std::strftime(str, 22, "%Y-%m-%dT%H:%M:%SZ", std::gmtime(&currentTime));
            std::string currentTimeString = str;
            app.state->setProperty("RatingsUpdateTime", currentTimeString);
        }
        
        if (didUpdate)
            didAddRatings = true;  // we need to save the cache
        else if (result == checked)
            ;   // the update had no new ratings
        
    }
    
    // the following local vars should be ivars or arguments to doit - but
    // as <algorithm> is anti OO it does not allow to transfer a context.
    DOMDocument *saveDoc;
    DOMElement *levelsElem;
    void saveLevelRating(const std::map<std::string, Rating *>::value_type info) {
        DOMElement *level = saveDoc->createElement(Utf8ToXML("level").x_str());
        std::string id =  info.first.substr(0, info.first.rfind('#'));
        std::string sv =  info.first.substr(info.first.rfind('#')+1);
        level->setAttribute(Utf8ToXML("id").x_str(), Utf8ToXML(id.c_str()).x_str());
        level->setAttribute(Utf8ToXML("sv").x_str(), Utf8ToXML(sv.c_str()).x_str());
        level->setAttribute(Utf8ToXML("int").x_str(), Utf8ToXML(ecl::strf("%d",info.second->intelligence).c_str()).x_str());
        level->setAttribute(Utf8ToXML("dex").x_str(), Utf8ToXML(ecl::strf("%d",info.second->dexterity).c_str()).x_str());
        level->setAttribute(Utf8ToXML("pat").x_str(), Utf8ToXML(ecl::strf("%d",info.second->patience).c_str()).x_str());
        level->setAttribute(Utf8ToXML("kno").x_str(), Utf8ToXML(ecl::strf("%d",info.second->knowledge).c_str()).x_str());
        level->setAttribute(Utf8ToXML("spe").x_str(), Utf8ToXML(ecl::strf("%d",info.second->speed).c_str()).x_str());
        level->setAttribute(Utf8ToXML("bse").x_str(), Utf8ToXML(ecl::strf("%d",info.second->bestScoreEasy).c_str()).x_str());
        level->setAttribute(Utf8ToXML("bsd").x_str(), Utf8ToXML(ecl::strf("%d",info.second->bestScoreDifficult).c_str()).x_str());
        level->setAttribute(Utf8ToXML("bseh").x_str(), Utf8ToXML(info.second->bestScoreEasyHolder.c_str()).x_str());
        level->setAttribute(Utf8ToXML("bsdh").x_str(), Utf8ToXML(info.second->bestScoreDifficultHolder.c_str()).x_str());
        level->setAttribute(Utf8ToXML("pare").x_str(), Utf8ToXML(ecl::strf("%d",info.second->parScoreEasy).c_str()).x_str());
        level->setAttribute(Utf8ToXML("pard").x_str(), Utf8ToXML(ecl::strf("%d",info.second->parScoreDifficult).c_str()).x_str());
        level->setAttribute(Utf8ToXML("solvne").x_str(), Utf8ToXML(ecl::strf("%d",info.second->numSolvedEasy).c_str()).x_str());
        level->setAttribute(Utf8ToXML("solvpe").x_str(), Utf8ToXML(ecl::strf("%d",info.second->pdmSolvedEasy).c_str()).x_str());
        level->setAttribute(Utf8ToXML("solvnd").x_str(), Utf8ToXML(ecl::strf("%d",info.second->numSolvedDifficult).c_str()).x_str());
        level->setAttribute(Utf8ToXML("solvpd").x_str(), Utf8ToXML(ecl::strf("%d",info.second->pdmSolvedDifficult).c_str()).x_str());
        level->setAttribute(Utf8ToXML("avgur").x_str(), Utf8ToXML(ecl::strf("%d",info.second->averageRating).c_str()).x_str());
        levelsElem->appendChild(level);
    }
    
    void RatingManager::save() {
        if (!(didAddRatings || didEditRatings))
            return;
        if (didEditRatings) {
            // set the rating version to the current time
            char date[25];
            std::time_t current = std::time(NULL);
            strftime(date, 25, "%Y-%m-%dT%H:%M:%SZ",gmtime(&current));
            ratingVersionString = date;
        }
        std::string ratTemplatePath;
        if (!app.systemFS->findFile("schemas/ratings.xml" , ratTemplatePath)) {
            cerr << "Ratings: no template found\n";
            return;
        }
        try {
            app.domParserErrorHandler->resetErrors();
            app.domParserErrorHandler->reportToErr();
            app.domParserSchemaResolver->resetResolver();
            app.domParserSchemaResolver->addSchemaId("ratings.xsd","ratings.xsd");
            saveDoc = app.domParser->parseURI(ratTemplatePath.c_str());
            if (saveDoc != NULL && !app.domParserErrorHandler->getSawErrors()) {
                DOMElement * updateElem = dynamic_cast<DOMElement *>(saveDoc->getElementsByTagName(
                        Utf8ToXML("update").x_str())->item(0));
                updateElem->setAttribute(Utf8ToXML("date").x_str(), Utf8ToXML(ratingVersionString.c_str()).x_str());
                updateElem->setAttribute(Utf8ToXML("urlFull").x_str(), Utf8ToXML(urlFullUpdate.c_str()).x_str());
                updateElem->setAttribute(Utf8ToXML("urlIncremental").x_str(), Utf8ToXML(urlIncrementalUpdate.c_str()).x_str());
                updateElem->setAttribute(Utf8ToXML("delay").x_str(), Utf8ToXML(ecl::strf("%d",updateMinDelay).c_str()).x_str());
                levelsElem = dynamic_cast<DOMElement *>(saveDoc->getElementsByTagName(
                        Utf8ToXML("levels").x_str())->item(0));
                std::for_each(cache.begin(), cache.end(), saveLevelRating);
                std::string writePath = app.userPath + "/" + RATINGSFILENAME;
#if _XERCES_VERSION >= 30000
                app.domSer->writeToURI(saveDoc, LocalToXML(writePath.c_str()).x_str());
#else
                XMLFormatTarget *myFormTarget = new LocalFileFormatTarget(writePath.c_str());
                app.domSer->writeNode(myFormTarget, *saveDoc);            
#endif
                saveDoc->release();
            }
        }
        catch (const XMLException& toCatch) {
            char* message = XMLString::transcode(toCatch.getMessage());
            cerr << "Exception on load of preferences: "
                 << message << "\n";
            XMLString::release(&message);
            return;
        }
        catch (const DOMException& toCatch) {
            char* message = XMLString::transcode(toCatch.msg);
            cerr << "Exception on load of preferences: "
                 << message << "\n";
            XMLString::release(&message);
            return;
        }
        catch (...) {
            cerr << "Unexpected Exception on load of preferences\n" ;
            return;
        }        
        
    }

    void RatingManager::registerRating(std::string levelId, short levelScoreVersion,
                short intelligence, short dexterity, short patience,
                short knowledge, short speed, short bestScoreEasy, 
                std::string bestScoreEasyHolder, short bestScoreDifficult,
                std::string bestScoreDifficultHolder) {
        std::string cacheKey = levelId + "#" + ecl::strf("%d", levelScoreVersion);
        std::map<std::string, Rating *>::iterator i = cache.find(cacheKey);
        if (i != cache.end()) {
//            Log << "Rating cache hit for " << levelId <<"\n";
            return;
        }
        Rating *theRating = new Rating();
        theRating->intelligence = intelligence;
        theRating->dexterity = dexterity;
        theRating->patience = patience;
        theRating->knowledge = knowledge;
        theRating->speed = speed;
        theRating->bestScoreEasy = bestScoreEasy;
        theRating->bestScoreEasyHolder = bestScoreEasyHolder;
        theRating->bestScoreDifficult = bestScoreDifficult;
        theRating->bestScoreDifficultHolder = bestScoreDifficultHolder;
        cache.insert(std::make_pair(cacheKey, theRating));
        didAddRatings = true;
        return;
    }
    
    Rating * RatingManager::registerNewRating(Proxy * levelProxy) {
        std::string cacheKey = levelProxy->getId() + "#" + 
                ecl::strf("%d", levelProxy->getScoreVersion());
        std::map<std::string, Rating *>::iterator it = cache.find(cacheKey);
        if (it != cache.end()) {
            return it->second;
        }
        Rating *theRating = new Rating();
        cache.insert(std::make_pair(cacheKey, theRating));
        didAddRatings = true;
        return theRating;        
    }

    Rating * RatingManager::findRating(Proxy * levelProxy) {
        std::string cacheKey = levelProxy->getId() + "#" + 
                ecl::strf("%d", levelProxy->getScoreVersion());
        std::map<std::string, Rating *>::iterator it = cache.find(cacheKey);
        if (it != cache.end()) {
            Rating * theRating = it->second;
            return theRating;
        } else {
            return NULL;
        }
    }
    
    short RatingManager::getIntelligence(Proxy *levelProxy) {
        Rating * theRating = findRating(levelProxy);
        if (theRating != NULL) {
            return theRating->intelligence;
        }
        return 0;
    }

    void RatingManager::setIntelligence(Proxy *levelProxy, short intelligence) {
        Rating * theRating = registerNewRating(levelProxy);
        if (theRating != NULL) {
            theRating->intelligence = intelligence;
            didEditRatings = true;
        }
    }

    short RatingManager::getDexterity(Proxy *levelProxy) {
        Rating * theRating = findRating(levelProxy);
        if (theRating != NULL) {
            return theRating->dexterity;
        }
        return 0;
    }
    
    void RatingManager::setDexterity(Proxy *levelProxy, short dexterity) {
        Rating * theRating = registerNewRating(levelProxy);
        if (theRating != NULL) {
            theRating->dexterity = dexterity;
            didEditRatings = true;
        }
    }

    short RatingManager::getPatience(Proxy *levelProxy) {
        Rating * theRating = findRating(levelProxy);
        if (theRating != NULL) {
            return theRating->patience;
        }
       
        return 0;
    }
    
    void RatingManager::setPatience(Proxy *levelProxy, short patience) {
        Rating * theRating = registerNewRating(levelProxy);
        if (theRating != NULL) {
            theRating->patience = patience;
            didEditRatings = true;
        }
    }

    short RatingManager::getKnowledge(Proxy *levelProxy) {
        Rating * theRating = findRating(levelProxy);
        if (theRating != NULL) {
            return theRating->knowledge;
        }
       
        return 0;
    }
    
    void RatingManager::setKnowledge(Proxy *levelProxy, short knowledge) {
        Rating * theRating = registerNewRating(levelProxy);
        if (theRating != NULL) {
            theRating->knowledge = knowledge;
            didEditRatings = true;
        }
    }

    short RatingManager::getSpeed(Proxy *levelProxy) {
        Rating * theRating = findRating(levelProxy);
        if (theRating != NULL) {
            return theRating->speed;
        }
       
        return 0;
    }

    void RatingManager::setSpeed(Proxy *levelProxy, short speed) {
        Rating * theRating = registerNewRating(levelProxy);
        if (theRating != NULL) {
            theRating->speed = speed;
            didEditRatings = true;
        }
    }

    short RatingManager::getDifficulty(Proxy *levelProxy) {
        int difficulty = 7*getIntelligence(levelProxy) + 
                6*getDexterity(levelProxy) +
                4*getPatience(levelProxy) +
                3*getKnowledge(levelProxy) +
                4*getSpeed(levelProxy) - 23;
        return difficulty > 0 ? difficulty : 0;
    }
    
    short RatingManager::getBestScore(Proxy *levelProxy, int difficulty) {
        if (difficulty == DIFFICULTY_EASY && !levelProxy->hasEasymode())
            difficulty = DIFFICULTY_HARD;
        switch (difficulty) {
            case DIFFICULTY_EASY:
                return getBestScoreEasy(levelProxy);
            case DIFFICULTY_HARD:
                return getBestScoreDifficult(levelProxy);
            default:
                ecl::Assert <XFrontend> (false, "RatingManager::getBestScore illegal difficulty");
        }
        return -1;
    }
    
    std::string RatingManager::cutHolders(std::string org, int factor) {
        if (factor <= 0)
            return org;
        
        int others = 0;
        if (org.rfind(" others") == org.length() - 7) {
            // ratings string is already cut
            std::string::size_type lastPlus = org.rfind('+');
            std::istringstream othersString(org.substr(lastPlus + 1));
            othersString >> std::dec >> others;
            org = org.substr(0, lastPlus);
        }
        std::string::size_type cutPlus;
        for (int i=0; i< factor; i++) {
            cutPlus = org.rfind('+');
            if (cutPlus == std::string::npos)
                return "";
            org = org.substr(0, cutPlus);
            others++;
        }
        return ecl::strf("%s+%d others", org.c_str(), others);
    }
    
    std::string RatingManager::getBestScoreHolder(Proxy *levelProxy, int difficulty, int cut) {
        if (difficulty == DIFFICULTY_EASY && !levelProxy->hasEasymode())
            difficulty = DIFFICULTY_HARD;
        switch (difficulty) {
            case DIFFICULTY_EASY:
                return getBestScoreEasyHolder(levelProxy, cut);
            case DIFFICULTY_HARD:
                return getBestScoreDifficultHolder(levelProxy, cut);
            default:
                ecl::Assert <XFrontend> (false, "RatingManager::getBestScoreHolder illegal difficulty");
        }
        return "";
    }

    short RatingManager::getBestScoreEasy(Proxy *levelProxy) {
        Rating * theRating = findRating(levelProxy);
        if (theRating != NULL) {
            return theRating->bestScoreEasy;
        }
        return -1;
    }

    std::string RatingManager::getBestScoreEasyHolder(Proxy *levelProxy, int cut) {
        Rating * theRating = findRating(levelProxy);
        if (theRating != NULL) {
            return cutHolders(theRating->bestScoreEasyHolder, cut);
        }
        return "";
    }

    short RatingManager::getBestScoreDifficult(Proxy *levelProxy) {
        Rating * theRating = findRating(levelProxy);
        if (theRating != NULL) {
            return theRating->bestScoreDifficult;
        }
        return -1;
    }

    std::string RatingManager::getBestScoreDifficultHolder(Proxy *levelProxy, int cut) {
        Rating * theRating = findRating(levelProxy);
        if (theRating != NULL) {
            return cutHolders(theRating->bestScoreDifficultHolder, cut);
        }
        return "";
    }
    
    
    short RatingManager::getParScore(Proxy *levelProxy, int difficulty) {
        if (difficulty == DIFFICULTY_EASY && !levelProxy->hasEasymode())
            difficulty = DIFFICULTY_HARD;
        switch (difficulty) {
            case DIFFICULTY_EASY:
                return getParScoreEasy(levelProxy);
            case DIFFICULTY_HARD:
                return getParScoreDifficult(levelProxy);
            default:
                ecl::Assert <XFrontend> (false, "RatingManager::getParScore illegal difficulty");
        }
        return -1;
    }
    
    short RatingManager::getParScoreEasy(Proxy *levelProxy) {
        Rating * theRating = findRating(levelProxy);
        if (theRating != NULL) {
            return theRating->parScoreEasy;
        }
        return -1;
    }

    short RatingManager::getParScoreDifficult(Proxy *levelProxy) {
        Rating * theRating = findRating(levelProxy);
        if (theRating != NULL) {
            return theRating->parScoreDifficult;
        }
        return -1;
    }

    short RatingManager::getNumSolvedEasy(Proxy *levelProxy) {
        Rating * theRating = findRating(levelProxy);
        if (theRating != NULL) {
            return theRating->numSolvedEasy;
        }
        return 0;
    }

    short RatingManager::getNumSolvedDifficult(Proxy *levelProxy) {
        Rating * theRating = findRating(levelProxy);
        if (theRating != NULL) {
            return theRating->numSolvedDifficult;
        }
        return 0;
    }

    short RatingManager::getPdmSolvedEasy(Proxy *levelProxy) {
        Rating * theRating = findRating(levelProxy);
        if (theRating != NULL) {
            return theRating->pdmSolvedEasy;
        }
        return 0;
    }

    std::string RatingManager::getPcSolvedEasy(Proxy *levelProxy) {
        std::string s = ecl::strf("%5.3d", getPdmSolvedEasy(levelProxy));
        s.insert(3,1,'.');
        return s;
    }

    short RatingManager::getPdmSolvedDifficult(Proxy *levelProxy) {
        Rating * theRating = findRating(levelProxy);
        if (theRating != NULL) {
            return theRating->pdmSolvedDifficult;
        }
        return 0;
    }

    std::string RatingManager::getPcSolvedDifficult(Proxy *levelProxy) {
        std::string s = ecl::strf("%5.3d", getPdmSolvedDifficult(levelProxy));
        s.insert(3,1,'.');
        return s;
    }

    short RatingManager::getDAverageRating(Proxy *levelProxy) {
        Rating * theRating = findRating(levelProxy);
        if (theRating != NULL) {
            return theRating->averageRating;
        }
        return -1;
    }
    
    std::string RatingManager::getAverageRating(Proxy *levelProxy) {
        short rat = getDAverageRating(levelProxy);
        std::string s;
        if ( rat >= 0 ) {
            s = ecl::strf("%3.2d", rat);
            s.insert(2,1,'.');
        } else {
            s = "-";
        }
        return s;
    }
}} // namespace enigma::lev
