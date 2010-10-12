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

#include "lev/ScoreManager.hh"
#include "enigma.hh"
#include "errors.hh"
#include "DOMErrorReporter.hh"
#include "DOMSchemaResolver.hh"
#include "LocalToXML.hh"
#include "Utf8ToXML.hh"
#include "utilXML.hh"
#include "XMLtoLocal.hh"
#include "XMLtoUtf8.hh"
#include "ecl_system.hh"
#include "gui/ErrorMenu.hh"
#include "nls.hh"
#include "file.hh"

#include "main.hh"
#include "options.hh"

#include <cmath>
#include <ctime>
#include <iostream>
#include <fstream>
#include <sstream>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/util/XMLDouble.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/XMLUniDefs.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/XercesVersion.hpp>
#include <xercesc/framework/MemBufInputSource.hpp>
#include <xercesc/framework/Wrapper4InputSource.hpp>
#if _XERCES_VERSION < 30000
#include <xercesc/framework/LocalFileFormatTarget.hpp>
#include <xercesc/framework/MemBufFormatTarget.hpp>
#endif


using namespace std;
using namespace enigma;
XERCES_CPP_NAMESPACE_USE 

namespace {
#if _XERCES_VERSION >= 30000
    class ScoreDomSerFilter : public DOMLSSerializerFilter {
        public:
            virtual DOMNodeFilter::FilterAction acceptNode(const DOMNode *node) const;
#else
    class ScoreDomSerFilter : public DOMWriterFilter {
        public:
            virtual short acceptNode(const DOMNode *node) const;
#endif
            virtual unsigned long getWhatToShow () const {
                return DOMNodeFilter::SHOW_ALL;
            }
            virtual void setWhatToShow (unsigned long toShow) {}
    };
    
#if _XERCES_VERSION >= 30000
    DOMNodeFilter::FilterAction ScoreDomSerFilter::acceptNode(const DOMNode *node) const {
#else
    short ScoreDomSerFilter::acceptNode(const DOMNode *node) const {
#endif
        if (node->getNodeType () == DOMNode::ELEMENT_NODE &&
                 std::string(XMLtoUtf8(node->getNodeName()).c_str()) == "level") {
            const DOMElement *e = dynamic_cast<const DOMElement *>(node);
            std::string id = XMLtoUtf8(e->getAttribute(Utf8ToXML("id").x_str())).c_str();
            if (id.find("Import ") == 0) {
                // reject scores for levels imported from dat files
                return DOMNodeFilter::FILTER_REJECT;
            }
        }
        return DOMNodeFilter::FILTER_ACCEPT;
    }
}

namespace enigma { namespace lev {
    ScoreManager *ScoreManager::theSingleton = 0;
    unsigned ScoreManager::ctab[256];
    unsigned ScoreManager::pol = 0x1021;
    
    ScoreManager* ScoreManager::instance() {
        if (theSingleton == 0) {
            for (unsigned i = 0; i < 256; i++) {
                unsigned r = i << 8;
                for (int j = 0; j < 8; j++) {
                    bool bit = (r & 0x8000) != 0;
                    r <<= 1;
                    if (bit)
                        r ^= pol;
                }
                ctab[i] = r & 0xFFFF;
            }
            theSingleton = new ScoreManager();
        }
        return theSingleton;
    }

    ScoreManager::ScoreManager() : hasValidUserId (false), isModified (false) {
        sec("");
        ratingMgr = lev::RatingManager::instance();
        std::string scorePath;
        std::string errMessage;
        bool isTemplate = false;
        bool hasValidStateUserId = false;
        std::string stateUserId = app.state->getString("UserId"); 
        
        if (stateUserId.length() == 16 && 
                stateUserId.find_first_not_of("0123456789ABCDEF") == std::string::npos) {
            unsigned i1, i2, i3, i4; 
            std::istringstream s1(stateUserId.substr(0, 4));
            std::istringstream s2(stateUserId.substr(4, 4));
            std::istringstream s3(stateUserId.substr(8, 4));
            std::istringstream s4(stateUserId.substr(12, 4));
            s1 >> std::hex >> i1;
            s2 >> std::hex >> i2;
            s3 >> std::hex >> i3;
            s4 >> std::hex >> i4;
            if ((i4 == (i1 ^ i2 ^ i3)) && stateUserId != "0000000000000000") {
                hasValidStateUserId = true;
                Log << "User id '" << stateUserId << "'\n";
            } else 
                Log << "Bad user id '" << ecl::strf("%.4lX %.4lX %.4lX %.4lX",i1, i2,i3, i4) << "'\n";
                
        }
        
        if (!app.resourceFS->findFile( "enigma.score" , scorePath)) {
            isTemplate = true;
            if (!app.systemFS->findFile( "schemas/score.xml" , scorePath)) {
                throw XFrontend("Cannot load application score template!");
            }
        }

        try {
            std::ostringstream errStream;
            app.domParserErrorHandler->resetErrors();
            app.domParserErrorHandler->reportToOstream(&errStream);
            app.domParserSchemaResolver->resetResolver();
            app.domParserSchemaResolver->addSchemaId("score.xsd","score.xsd");

            if (isTemplate)
                doc = app.domParser->parseURI(scorePath.c_str());
            else {
                std::basic_ifstream<char> ifs(scorePath.c_str(), ios::binary | ios::in);
                std::string zipFile;
                char c;
                while (ifs.get(c))
                    zipFile += (char)(c ^ 0xE5);
                std::istringstream zipStream(zipFile);
                std::ostringstream content;
                readFromZipStream(zipStream, content);
                std::string score = content.str();
 #if _XERCES_VERSION >= 30000
                std::auto_ptr<DOMLSInput> domInputScoreSource ( new Wrapper4InputSource(
                        new MemBufInputSource(reinterpret_cast<const XMLByte *>(score.c_str()),
                        score.size(), "", false)));
                doc = app.domParser->parse(domInputScoreSource.get());
#else    
                std::auto_ptr<Wrapper4InputSource> domInputScoreSource ( new Wrapper4InputSource(
                        new MemBufInputSource(reinterpret_cast<const XMLByte *>(score.c_str()),
                        score.size(), "", false)));
                doc = app.domParser->parse(*domInputScoreSource);
#endif
                
            }
            if (doc != NULL && !app.domParserErrorHandler->getSawErrors()) {
                propertiesElem = dynamic_cast<DOMElement *>(doc->getElementsByTagName(
                        Utf8ToXML("properties").x_str())->item(0));
                levelsElem = dynamic_cast<DOMElement *>(doc->getElementsByTagName(
                        Utf8ToXML("levels").x_str())->item(0));
                levelList = levelsElem->getElementsByTagName(Utf8ToXML("level").x_str());
                
                userId = getString("UserId");
                if (hasValidStateUserId && userId == stateUserId) {
                    hasValidUserId = true;
                    for (int i = 0, l = levelList->getLength(); i < l; i++) {
                        DOMElement * levelElem  = dynamic_cast<DOMElement *>(levelList->item(i));
                        std::string levelId = XMLtoUtf8(levelElem->getAttribute(Utf8ToXML("id").x_str())).c_str();
                        std::string scoreVersion = XMLtoUtf8(levelElem->getAttribute(Utf8ToXML("version").x_str())).c_str();
                        DOMNamedNodeMap * attrXMap = levelElem->getAttributes();
                        std::map<std::string, std::string> attrMap;
                        for (int j = 0, k = attrXMap->getLength();  j < k; j++) {
                            DOMAttr * levelAttr = dynamic_cast<DOMAttr *>(attrXMap->item(j));
                            std::string attrName = XMLtoUtf8(levelAttr->getName()).c_str();
                            if (attrName != "sec" && levelAttr->getSpecified())
                                attrMap[attrName] = XMLtoUtf8(levelAttr->getValue()).c_str();
                        }
                        std::string target;
                        std::map<std::string, std::string>::iterator it;
                        for (it = attrMap.begin(); it != attrMap.end(); it++)
                            target += (*it).second;
                        target += userId;
                        if (sec(target) != XMLtoUtf8(levelElem->getAttribute(Utf8ToXML("sec").x_str())).c_str()) {
                            Log << "Faulty score entry deletion: " << levelId << " - version: " << scoreVersion << "\n";
                            levelsElem->removeChild(levelElem);
                            --i; --l;
                            continue;
                        }

                        std::string cacheKey = levelId + "#" + scoreVersion;
                        allLevelScores[cacheKey] = levelElem;
                        std::map<std::string,  DOMElement *>::iterator its;
                        its = curLevelScores.find(levelId);
                        if (its == curLevelScores.end()) {
                            curLevelScores[levelId] = levelElem;
                        } else {
                            // another scoreversion for this level exists - compare versions
                            int previousVersion = XMLString::parseInt(its->second->getAttribute(
                                    Utf8ToXML("version").x_str()));
                            int thisVersion = XMLString::parseInt(levelElem->getAttribute(
                                    Utf8ToXML("version").x_str()));
                            if (thisVersion > previousVersion)
                                curLevelScores[levelId] = levelElem;
                        }
                    }
                } else if (isTemplate && userId.empty()) {
                    isModified = true;
                    if (hasValidStateUserId) {
                        userId = stateUserId;
                        setProperty("UserId", userId);
                        hasValidUserId = true;
                    } else {
                        // create first part of user id based on time stamp
                        unsigned id;
                        if (RAND_MAX > 0x10000) {
                            id = std::rand() & 0xFFFFFFFF;
                        } else {
                            // MinGW 3.4.2 and maybe other configs
                            unsigned t = std::time(NULL);
                            std::srand(t >> 15);
                            id = (std::rand() << 16) ^ (t & 0xFFFF);
                            enigma::Randomize();
                        }
                        userId = ecl::strf("%.8lX",id);
                        // we need a second random part as 2 users may start Enigma
                        // within the same second - we postpone this part till we save
                    }
                    // do not allow existing level entries
                    int levelCount = levelList->getLength();
                    for (int i = 0; i < levelCount; i++) {
                        levelsElem->removeChild(levelList->item(0));
                    }
                } else {
                    doc->release();
                    // rename faulty score file as backup and give Enigma a chance
                    // to recreate a new good score file
                    std::string scoreBasePath = app.userPath + "/enigma.score";
                    std::string backupPath = scoreBasePath + "~s1";
                    int i = 1;
                    while (ecl::FileExists(backupPath)) {
                        backupPath = scoreBasePath + ecl::strf("~s%d", ++i);
                    }       
                    std::rename(scoreBasePath.c_str(), backupPath.c_str());
                    
                    errMessage = "Mismatch of state.xml and enigma.score.\n";
                    errMessage += "Your current faulty score file is backed up to:\n";
                    errMessage += backupPath + "\n";
                    errMessage += "Restore both from your backup or just restart\n";
                    errMessage += "Enigma to retry with an empty score file\n";
                    throw XFrontend("");
                }
                // update from 0.92
                
                // check if already updated
                didUpgrade = (getString("Upgrade") == "true") || 
                        (!isTemplate && (upgradeSum() != getString("Upgrade")));
                if (!didUpgrade) {
                    if (!hasValidUserId) {
                        unsigned id3 = idFromLegacyScore();
                        if (id3 <= 0xFFFF) {
                            Log << "idFromLegacyScore\n";
                            finishUserId(id3);
                        }
                    }
                    if (hasValidUserId)
                        if (upgradeLegacy()) {
                            isModified = true;
                            didUpgrade = true;
                        }
                }
                
            }
            if(app.domParserErrorHandler->getSawErrors()) {
                errMessage = errStream.str();
            }
            app.domParserErrorHandler->reportToNull();  // do not report to errStream any more
        }
        catch (...) {
            if (errMessage.empty())
                errMessage = "Unexpected XML Exception on load of score\n";
        }
        if (!errMessage.empty()) {
            throw XFrontend("Cannot load application score file: " + scorePath +
                    "\nError: " + errMessage);
        }
        
        
    }

    ScoreManager::~ScoreManager() {
        if (doc != NULL)
            shutdown();
    }

    void ScoreManager::markModified() {
        isModified = true;
    }
    
    void ScoreManager::finishUserId(unsigned id3) {
        unsigned i1, i2, i3, i4; 
        std::istringstream s1(userId.substr(0, 4));
        std::istringstream s2(userId.substr(4, 4));
        s1 >> std::hex >> i1;
        s2 >> std::hex >> i2;
        i3 = id3 & 0xFFFF;
        i4 = (i1 ^ i2 ^ i3);
        userId += ecl::strf("%.4lX%.4lX",i3, i4);
        app.state->setProperty("UserId", userId);
        setProperty("UserId", userId);
        hasValidUserId = true;
    }
    
    std::string ScoreManager::sec(std::string target) {
        int len = target.size();
        unsigned r = 0;
        const char *p = target.c_str();
        
        while (len--)
            r = (r<<8 & 0xFFFF) ^ ctab[(r >> 8) ^ *p++];
        return ecl::strf("%.4lX", r); 
    }

    bool ScoreManager::save() {
        bool result = true;
        std::string errMessage;
        
        if (doc == NULL || !isModified)
            return true;

        int count = getInt("Count");
        setProperty("Count", ++count);
        
        setProperty("UserName", app.state->getString("UserName"));
        
        if (!hasValidUserId) {
            finishUserId(std::time(NULL) & 0xFFFF);
        }
        
        if (userId.find("0000") == 0) {
            Log << "ReId Windows 1.00 User Id: " << userId << "\n";
            setProperty("UserId1.00", userId);
            app.state->setProperty("UserId1.00", userId);
            unsigned id1 = std::rand() & 0xFFFF;
            userId.replace(0, 4, ecl::strf("%.4lX",id1));
            unsigned id2, id3, id4;
            std::istringstream s2(userId.substr(4, 4));
            std::istringstream s3(userId.substr(8, 4));
            s2 >> std::hex >> id2;
            s3 >> std::hex >> id3;
            id4 = (id1 ^ id2 ^ id3);
            userId.replace(12, 4, ecl::strf("%.4lX",id4));
            app.state->setProperty("UserId", userId);
            setProperty("UserId", userId);
            Log << "new id: " << userId << "\n";
        }
        
        for (int i = 0, l = levelList->getLength(); i < l; i++) {
            DOMElement * levelElem  = dynamic_cast<DOMElement *>(levelList->item(i));
            DOMNamedNodeMap * attrXMap = levelElem->getAttributes();
            std::map<std::string, std::string> attrMap;
            for (int j = 0, k = attrXMap->getLength();  j < k; j++) {
                DOMAttr * levelAttr = dynamic_cast<DOMAttr *>(attrXMap->item(j));
                std::string attrName = XMLtoUtf8(levelAttr->getName()).c_str();
                if (attrName != "sec" && levelAttr->getSpecified())
                    attrMap[attrName] = XMLtoUtf8(levelAttr->getValue()).c_str();
            }
            std::string target;
            std::map<std::string, std::string>::iterator it;
            for (it = attrMap.begin(); it != attrMap.end(); it++)
                target += (*it).second;
            target += userId;
            levelElem->setAttribute(Utf8ToXML("sec").x_str(), 
                    Utf8ToXML(sec(target)).x_str());
        }
        
        if (!didUpgrade)
            setProperty("Upgrade", upgradeSum());
        else
            setProperty("Upgrade", std::string("true"));

        stripIgnorableWhitespace(doc->getDocumentElement());
//        std::string path = app.userPath + "/score.xml";
        std::string zipPath = app.userPath + "/enigma.score";
        std::string zipPathNoDat = app.userPath + "/enigma_nodat.score";
        std::string zipPathBackup = app.userPath + "/backup/enigma.score";
        
        // backup score every 10th save as we save after each level completion once
        if (count%10 == 0) {
            std::remove((zipPathBackup + "~2").c_str());
            std::remove((zipPath + "~2").c_str()); // 1.00 bakups
            if (ecl::FileExists(zipPath + "~1")) {
                if (std::difftime(ecl::FileModTime(zipPath + "~1"),
                        ecl::FileModTime(zipPathBackup + "~1")) > 0) {
                    // backup 1 from 1.00 is newer than backup 1 on backup path
                    if (Copyfile(zipPath + "~1", zipPathBackup + "~2"))
                        std::remove((zipPath + "~1").c_str()); // 1.00 bakup
                } else {
                    // just in case off previous copy failure
                    std::rename((zipPathBackup + "~1").c_str(), (zipPathBackup + "~2").c_str());
                    std::remove((zipPath + "~1").c_str()); // 1.00 bakup
                }
            } else {
                std::rename((zipPathBackup + "~1").c_str(), (zipPathBackup + "~2").c_str());
            }
            Copyfile(zipPath, zipPathBackup + "~1");
        }
        
        try {
            ScoreDomSerFilter serialFilter;
            for (int j=0; j < 2; j++) { // save twice: first all, then without dat scores
#if _XERCES_VERSION >= 30000
//            result = app.domSer->writeToURI(doc, LocalToXML(& path).x_str());
                if (j==1)
                    (app.domSer)->setFilter(&serialFilter);
                std::string contents(XMLtoUtf8(app.domSer->writeToString(doc)).c_str());
                if (j==1)
                    (app.domSer)->setFilter(NULL);
                contents.replace(contents.find("UTF-16"), 6, "UTF-8"); // adapt encoding info
#else
//            XMLFormatTarget *myFormTarget = new LocalFileFormatTarget(path.c_str());
//            result = app.domSer->writeNode(myFormTarget, *doc);            
//            delete myFormTarget;   // flush
            
                MemBufFormatTarget *memFormTarget = new MemBufFormatTarget();
                if (j==1)
                    app.domSer->setFilter(&serialFilter);
                result = app.domSer->writeNode(memFormTarget, *doc);
                if (j==1)
                    app.domSer->setFilter(NULL);
                std::string contents(
                        reinterpret_cast<const char *>(memFormTarget->getRawBuffer()),
                        memFormTarget->getLen());
                delete memFormTarget;
#endif
                std::istringstream contentStream(contents);
                std::ostringstream zippedStream;
                writeToZip(zippedStream, "score.xml", contents.size(), contentStream);
                std::ofstream of( j==0 ? zipPath.c_str() : zipPathNoDat.c_str(),
                        ios::out | ios::binary );
                std::string zipScore = zippedStream.str();
                
                // patch zipios++ malformed output
                // assumptions: just one file named "score.xml" (9 chars)
                if ((zipScore[0x06] & 0x08) == 0) {
                    Log << "Fixing Zipios++ output\n";
                    unsigned cdirOffset = zipScore.size() - 22 - 46 - 9;
                    unsigned compressedSize = cdirOffset - 30 - 9;
                    zipScore.replace(cdirOffset + 20, 1, 1, (char)(compressedSize & 0xFF));
                    zipScore.replace(cdirOffset + 21, 1, 1, (char)((compressedSize & 0xFF00) >> 8));
                    zipScore.replace(cdirOffset + 22, 1, 1, (char)((compressedSize & 0xFF0000) >> 16));
                    zipScore.replace(cdirOffset + 23, 1, 1, (char)((compressedSize & 0xFF000000) >> 24));
                    std::string dataDescr = "\x50\x4B\x07\x08" + zipScore.substr(cdirOffset + 16, 12);
                    zipScore.replace(6, 1 , 1, '\x08'); // general purpose bit flag
                    zipScore.replace(14, 12, 12, '\x00');
                    zipScore.replace(cdirOffset + 8, 1 , 1, '\x08'); // general purpose bit flag
                    zipScore.replace(cdirOffset + 38, 8, 8, '\x00'); // external file attr, offset local header
                    zipScore.insert(cdirOffset, dataDescr);
                    cdirOffset += 16;
                    zipScore.replace(zipScore.size() - 6, 1, 1, (char)(cdirOffset & 0xFF));
                    zipScore.replace(zipScore.size() - 5, 1, 1, (char)((cdirOffset & 0xFF00) >> 8));
                    zipScore.replace(zipScore.size() - 4, 1, 1, (char)((cdirOffset & 0xFF0000) >> 16));
                    zipScore.replace(zipScore.size() - 3, 1, 1, (char)((cdirOffset & 0xFF000000) >> 24));
                }
                
                for (int i=0; i<zipScore.size(); i++)
                    of << (char)(zipScore[i] ^ 0xE5);
            }
        } catch (const XMLException& toCatch) {
            errMessage = std::string("Exception on save of score: \n") + 
                    XMLtoUtf8(toCatch.getMessage()).c_str() + "\n";
            result = false;
        } catch (const DOMException& toCatch) {
            errMessage = std::string("Exception on save of score: \n") + 
                    XMLtoUtf8(toCatch.getMessage()).c_str() + "\n";
            result = false;
        } catch (...) {
            errMessage = "Unexpected exception on save of score\n" ;
            result = false;
        }

        if (!result) {
            if (count%10 == 0) {
                // restore backup in case of error
                if (Copyfile(zipPathBackup + "~1", zipPath)) {
                    std::remove((zipPathBackup + "~1").c_str());
                    std::rename((zipPathBackup + "~2").c_str(), (zipPathBackup + "~1").c_str());
                }
            }
            cerr << XMLtoLocal(Utf8ToXML(errMessage.c_str()).x_str()).c_str();
            gui::ErrorMenu m(errMessage, N_("Continue"));
            m.manage();          
        } else
            Log << "Score save o.k.\n";
        
        return result;
    }

    void ScoreManager::shutdown() {
        save();
        if (doc != NULL)
            doc->release();
        doc = NULL;
    }


    bool ScoreManager::isSolved(lev::Proxy *levelProxy, int difficulty) {
        ecl::Assert <XFrontend> (difficulty >= DIFFICULTY_EASY &&  
                difficulty <= DIFFICULTY_ANY, "ScoreManager::isSolved illegal difficulty");
        if (difficulty == DIFFICULTY_EASY && !levelProxy->hasEasymode())
            difficulty = DIFFICULTY_HARD;
        
        DOMElement * level = getLevel(levelProxy);
        if (level != NULL) {
            const XMLCh *attr = level->getAttribute(Utf8ToXML((difficulty == DIFFICULTY_HARD) ? "diff1" : "easy1").x_str());
            int score = (XMLString::stringLen(attr) > 0) ? XMLString::parseInt(attr) : -1;
            return score != SCORE_UNSOLVED;
        } else
            return false;
    }

    bool ScoreManager::isOutdated(lev::Proxy *levelProxy, int difficulty) {
        ecl::Assert <XFrontend> (difficulty >= DIFFICULTY_EASY &&  
                difficulty <= DIFFICULTY_HARD, "ScoreManager::isOutdated illegal difficulty");
        if (difficulty == DIFFICULTY_EASY && !levelProxy->hasEasymode())
            difficulty = DIFFICULTY_HARD;
        
        DOMElement * level = getLevel(levelProxy);
        if (level != NULL && XMLString::parseInt(level->getAttribute(
                Utf8ToXML("version").x_str())) == levelProxy->getScoreVersion()) {
            const XMLCh *attr = level->getAttribute(Utf8ToXML((difficulty == DIFFICULTY_HARD) ? "diff1" : "easy1").x_str());
            int score = (XMLString::stringLen(attr) > 0) ? XMLString::parseInt(attr) : -1;
            return score == SCORE_SOLVED;
        } else if (level != NULL && XMLString::parseInt(level->getAttribute(
                Utf8ToXML("version").x_str())) != levelProxy->getScoreVersion()){
            return true;
        } else
            return false;
    }

    int ScoreManager::getBestUserScore(lev::Proxy *levelProxy, int difficulty) {
        ecl::Assert <XFrontend> (difficulty >= DIFFICULTY_EASY &&  
                difficulty <= DIFFICULTY_HARD, "ScoreManager::getBestUserScore illegal difficulty");
        if (difficulty == DIFFICULTY_EASY && !levelProxy->hasEasymode())
            difficulty = DIFFICULTY_HARD;
        
        DOMElement * level = getLevel(levelProxy);
        if (level == NULL || XMLString::parseInt(level->getAttribute(
                Utf8ToXML("version").x_str())) != levelProxy->getScoreVersion()) {
            return SCORE_UNSOLVED;
        }
        const XMLCh *attr = level->getAttribute(Utf8ToXML((difficulty == DIFFICULTY_HARD) ? "diff1" : "easy1").x_str());
        int score = (XMLString::stringLen(attr) > 0) ? XMLString::parseInt(attr) : -1;
        return (score < 0) ? SCORE_UNSOLVED : score;
    }

    void ScoreManager::updateUserScore(lev::Proxy *levelProxy, int difficulty, 
            int score, double enigmaRelease) {
        ecl::Assert <XFrontend> (difficulty >= DIFFICULTY_EASY &&  
                difficulty <= DIFFICULTY_HARD, "ScoreManager::updateUserScore illegal difficulty");
        if (levelProxy->getLevelStatus() != lev::STATUS_RELEASED)
            return;
        
        if (difficulty == DIFFICULTY_EASY && !levelProxy->hasEasymode())
            difficulty = DIFFICULTY_HARD;
        
        if (score > SCORE_MAX)
            score = SCORE_MAX;  // distinguish from SCORE_SOLVED levels
            
        if (!hasValidUserId) {
            finishUserId(std::time(NULL) & 0xFFFF);
        }
        
        DOMElement * level = getCreateLevel(levelProxy);
        
        // get the current newest scoreversion of the level - it exists now
        DOMElement * curLevel = curLevelScores[levelProxy->getId()];
        if (curLevel != level) {
            // this levelversion is older than the current one
            // update the current one concerning the solved status
            const XMLCh * attr = curLevel->getAttribute(Utf8ToXML((difficulty == DIFFICULTY_HARD) ? "diff1" : "easy1").x_str());
            int curScore1 = (XMLString::stringLen(attr) > 0) ? XMLString::parseInt(attr) : -1;
            if (curScore1 == SCORE_UNSOLVED) {
                isModified = true;
                curLevel->setAttribute(Utf8ToXML((difficulty == DIFFICULTY_HARD) ? "diff1" : "easy1").x_str(),
                        Utf8ToXML(ecl::strf("%d",SCORE_SOLVED)).x_str());
                curLevel->setAttribute(Utf8ToXML((difficulty == DIFFICULTY_HARD) ? "diff1rel" : "easy1rel").x_str(),
                        Utf8ToXML(ecl::strf("%.2f",enigmaRelease)).x_str());              
            }
        }
        // update this levelversion
        const XMLCh *attr;
        // read attributes - for new elements the XML defaults are not yet given
        attr = level->getAttribute(Utf8ToXML((difficulty == DIFFICULTY_HARD) ? "diff1" : "easy1").x_str());
        int score1 = (XMLString::stringLen(attr) > 0) ? XMLString::parseInt(attr) : -1;
        attr = level->getAttribute(Utf8ToXML((difficulty == DIFFICULTY_HARD) ? "diff2" : "easy2").x_str());
        int score2 = (XMLString::stringLen(attr) > 0) ? XMLString::parseInt(attr) : -1;
        attr = level->getAttribute(Utf8ToXML((difficulty == DIFFICULTY_HARD) ? "diff1rel" : "easy1rel").x_str());
        double rel1 = 0.92; // default
        if (XMLString::stringLen(attr) > 0) {
            XMLDouble * result = new XMLDouble(attr);
            rel1 = result->getValue();
            delete result;
        }
        attr = level->getAttribute(Utf8ToXML((difficulty == DIFFICULTY_HARD) ? "diff2rel" : "easy2rel").x_str());
        double rel2 = 0.92; // default
        if (XMLString::stringLen(attr) > 0) {
            XMLDouble * result = new XMLDouble(attr);
            rel2 = result->getValue();
            delete result;
        }
        
        bool score1mod = false;
        bool score2mod = false;
        if (score1 < 0) {
            // the level has not yet been solved in this scoreversion
            score1 = score;
            rel1 = enigmaRelease;
            score1mod = true;
        } else if (score < score1) {
            // new best score
            if (rel1 > rel2 && rel1 > enigmaRelease) {
                // remember old best score if it is played with newest Enigma version
                score2 = score1;
                rel2 = rel1;
                score2mod = true;                
            }
            score1 = score;
            rel1 = enigmaRelease;
            score1mod = true;
        } else if (score == score1) {
            if (rel1 < enigmaRelease) {
                // update the Enigma release version for top score
                rel1 = enigmaRelease;
                score1mod = true;
            }
        } else if (rel1 < enigmaRelease) {
            // score > score1 but played with a more recent Enigma version
            // check if we should save it as score2
            if (score2 < 0) {
                // second best score and no valid score2 yet
                score2 = score;
                rel2 = enigmaRelease;
                score2mod = true;
            } else if (score < score2) {
                // score better than previous second best score
                if (rel2 <= enigmaRelease) {
                    score2 = score;
                    rel2 = enigmaRelease;
                    score2mod = true;
                }
            } else if (rel2 < enigmaRelease) {
                // we remember the second best score played with the most recent 
                // Enigma version
                score2 = score;
                rel2 = enigmaRelease;
                score2mod = true;
            }
        }
        if (score1mod) {
            isModified = true;
            level->setAttribute(Utf8ToXML((difficulty == DIFFICULTY_HARD) ? "diff1" : "easy1").x_str(),
                    Utf8ToXML(ecl::strf("%d",score1)).x_str());
            level->setAttribute(Utf8ToXML((difficulty == DIFFICULTY_HARD) ? "diff1rel" : "easy1rel").x_str(),
                    Utf8ToXML(ecl::strf("%.2f",rel1)).x_str());
        }
        if (score2mod) {
            isModified = true;
            level->setAttribute(Utf8ToXML((difficulty == DIFFICULTY_HARD) ? "diff2" : "easy2").x_str(),
                    Utf8ToXML(ecl::strf("%d",score2)).x_str());
            level->setAttribute(Utf8ToXML((difficulty == DIFFICULTY_HARD) ? "diff2rel" : "easy2rel").x_str(),
                    Utf8ToXML(ecl::strf("%.2f",rel2)).x_str());
        }
        
    }
    
    bool ScoreManager::bestScoreReached(lev::Proxy *levelProxy, int difficulty) {
        if (difficulty == DIFFICULTY_EASY && !levelProxy->hasEasymode())
            difficulty = DIFFICULTY_HARD;
        
        int bestUserScore = getBestUserScore(levelProxy, difficulty);
        int bestScore = ratingMgr->getBestScore(levelProxy, difficulty);
        return bestUserScore>=0 && (bestScore<0 || bestUserScore <= bestScore);
    }
    
    bool ScoreManager::parScoreReached(lev::Proxy *levelProxy, int difficulty) {
        if (difficulty == DIFFICULTY_EASY && !levelProxy->hasEasymode())
            difficulty = DIFFICULTY_HARD;
        
        int bestUserScore = getBestUserScore(levelProxy, difficulty);
        int parScore = ratingMgr->getParScore(levelProxy, difficulty);
        return bestUserScore>=0 && (parScore<0 || bestUserScore <= parScore);
    }
    
    void ScoreManager::markUnsolved(lev::Proxy *levelProxy, int difficulty) {
        ecl::Assert <XFrontend> (difficulty >= DIFFICULTY_EASY &&  
                difficulty <= DIFFICULTY_ANY, "ScoreManager::markUnsolved illegal difficulty");
        DOMElement * level = getLevel(levelProxy);
        if (level != NULL && XMLString::parseInt(level->getAttribute(
                Utf8ToXML("version").x_str())) == levelProxy->getScoreVersion()) {
            const XMLCh *attr = level->getAttribute(Utf8ToXML((difficulty == DIFFICULTY_HARD) ? "diff1" : "easy1").x_str());
            int score = (XMLString::stringLen(attr) > 0) ? XMLString::parseInt(attr) : -1;
            if (score != SCORE_UNSOLVED) {
                isModified = true;
                level->setAttribute(Utf8ToXML((difficulty == DIFFICULTY_HARD) ? "diff1" : "easy1").x_str(),
                    Utf8ToXML(ecl::strf("%d",SCORE_UNSOLVED)).x_str());
            }
        }
    }
    
    void ScoreManager::markSolved(lev::Proxy *levelProxy, int difficulty) {
        ecl::Assert <XFrontend> (difficulty >= DIFFICULTY_EASY &&  
                difficulty <= DIFFICULTY_ANY, "ScoreManager::markSolved illegal difficulty");
        if (enigma::WizardMode) {
            if (!hasValidUserId) {
                finishUserId(std::time(NULL) & 0xFFFF);
            }
            DOMElement * level = getLevel(levelProxy);
            if (level != NULL && XMLString::parseInt(level->getAttribute(
                    Utf8ToXML("version").x_str())) == levelProxy->getScoreVersion()) {
                const XMLCh *attr = level->getAttribute(Utf8ToXML((difficulty == DIFFICULTY_HARD) ? "diff1" : "easy1").x_str());
                int score = (XMLString::stringLen(attr) > 0) ? XMLString::parseInt(attr) : -1;
                if (score != SCORE_UNSOLVED)
                    return;  // avoid deleting existing scores
            }
            // reset the score to solved but no score value
            updateUserScore(levelProxy, difficulty, 99*60+59); // store max possible score
            level = getLevel(levelProxy);
            // check if score is created - it may be NULL if level is not released
            if (level != NULL && XMLString::parseInt(level->getAttribute(
                    Utf8ToXML("version").x_str())) == levelProxy->getScoreVersion()) {
                isModified = true;
                level->setAttribute(Utf8ToXML((difficulty == DIFFICULTY_HARD) ? "diff1" : "easy1").x_str(),
                        Utf8ToXML(ecl::strf("%d",SCORE_SOLVED)).x_str());
            }
        }
    }

    int ScoreManager::countSolved(lev::Index *ind, int difficulty) {
        int i;
        int size = ind->size();
        int num = 0;
        for (i=0 ; i < size; i++) {
            if (isSolved(ind->getProxy(i),difficulty))
                num++;
        }
        return num;
    }
    
    int ScoreManager::countBestScore(lev::Index *ind, int difficulty) {
        int i;
        int size = ind->size();
        int num = 0;
        for (i=0 ; i < size; i++) {
            if (bestScoreReached(ind->getProxy(i),difficulty))
                num++;
        }
        return num;
    }  
    
    int ScoreManager::countParScore(lev::Index *ind, int difficulty) {
        int i;
        int size = ind->size();
        int num = 0;
        for (i=0 ; i < size; i++) {
            if (parScoreReached(ind->getProxy(i),difficulty))
                num++;
        }
        return num;
    }  
    
    double ScoreManager::calcHCP(lev::Index *ind, int difficulty) {
        static double log_2 = log(2.0);
        int i;
        int size = ind->size();
        double hcp = 0;
        for (i=0 ; i < size; i++) {
            double score = getBestUserScore(ind->getProxy(i), difficulty);
            double par = ratingMgr->getParScore(ind->getProxy(i), difficulty);
            double dhcp = 0;
            if (score == SCORE_UNSOLVED) {
                dhcp = 1;
            } else if (score == SCORE_SOLVED) {
                dhcp = 0.7;
            } else if (score >= par && par > 0) {
                dhcp = log10(score/par);
                if (dhcp > 0.7)
                    dhcp = 0.7;
            } else if (score < par && par > 0) {
                dhcp = log(score/par) / log_2;
                if (dhcp < -3)
                    dhcp = -3;
            } else { // par <= 0 no par
                dhcp = -3;
            }
            hcp += dhcp;
        }
        return hcp*100.0/size;
    }  
    
    void ScoreManager::setRating(lev::Proxy *levelProxy, int rating) {
        if (!hasValidUserId) {
            finishUserId(std::time(NULL) & 0xFFFF);
        }
        if (rating == -1) {
            DOMElement *level = getLevel(levelProxy);
            if (level == NULL)
                // no level score entry for this level - -1 is default anyway
               return;
            else if (XMLString::parseInt(level->getAttribute(
                    Utf8ToXML("version").x_str())) == levelProxy->getScoreVersion()) {
                const XMLCh *attr = level->getAttribute(Utf8ToXML("rating").x_str());
                int oldRating = (XMLString::stringLen(attr) > 0) ? XMLString::parseInt(attr) : -1;
                if (oldRating != -1) {
                    isModified = true;
                    level->setAttribute(Utf8ToXML("rating").x_str(),
                        Utf8ToXML(ecl::strf("%d",rating)).x_str());
                }
                DOMAttr *irAttr = level->getAttributeNode(Utf8ToXML("rating-inherited").x_str());
                if ((irAttr != NULL) && irAttr->getSpecified()) {
                    // delete any inherited rating that may shadow a default of -1
                    level->removeAttribute(Utf8ToXML("rating-inherited").x_str());
                    isModified = true;
                }
                return;
            } else {
                // no level score entry for this score version exists - the user
                // did set the rating to undefined.
                DOMElement *level = getCreateLevel(levelProxy);
                isModified = true;
                level->setAttribute(Utf8ToXML("rating").x_str(),
                        Utf8ToXML(ecl::strf("%d",rating)).x_str());
                level->removeAttribute(Utf8ToXML("rating-inherited").x_str());
                return;
            }
        } else if (rating != getRating(levelProxy)) {
            DOMElement *level = getCreateLevel(levelProxy);
            isModified = true;
            level->setAttribute(Utf8ToXML("rating").x_str(),
                    Utf8ToXML(ecl::strf("%d",rating)).x_str());
            level->removeAttribute(Utf8ToXML("rating-inherited").x_str());
            return;
        }
    }
    
    int ScoreManager::getRating(lev::Proxy *levelProxy) {
        DOMElement * level = getLevel(levelProxy);
        if (level == NULL)
            return -1;
        else {
            const XMLCh *attr = level->getAttribute(Utf8ToXML("rating").x_str());
            int rating = (XMLString::stringLen(attr) > 0) ? XMLString::parseInt(attr) : -1;
            if (rating == -1) {
                attr = level->getAttribute(Utf8ToXML("rating-inherited").x_str());
                int ratingInherited = (XMLString::stringLen(attr) > 0) ? XMLString::parseInt(attr) : -1;
                if (ratingInherited >= 0)
                    rating = ratingInherited;
            }
            return rating;
        }
    }
    
    bool ScoreManager::isRatingInherited(lev::Proxy *levelProxy) {
        DOMElement * level = getLevel(levelProxy);
        if (level == NULL)
            return false;
        if (XMLString::parseInt(level->getAttribute(
                Utf8ToXML("version").x_str())) == levelProxy->getScoreVersion()) {
            const XMLCh *attr = level->getAttribute(Utf8ToXML("rating").x_str());
            int rating = (XMLString::stringLen(attr) > 0) ? XMLString::parseInt(attr) : -1;
            if (rating == -1) {
                attr = level->getAttribute(Utf8ToXML("rating-inherited").x_str());
                int ratingInherited = (XMLString::stringLen(attr) > 0) ? XMLString::parseInt(attr) : -1;
                if (ratingInherited >= 0)
                    return true;
            }
            return false;
        } else {
            const XMLCh *attr = level->getAttribute(Utf8ToXML("rating").x_str());
            int rating = (XMLString::stringLen(attr) > 0) ? XMLString::parseInt(attr) : -1;
            if (rating == -1)
                return false;
            else
                return true;
        }
    }
    
    DOMElement * ScoreManager::getLevel(lev::Proxy *levelProxy) {
        std::map<std::string,  DOMElement *>::iterator it;
        std::string cacheKey = levelProxy->getId() + "#" + ecl::strf("%d", levelProxy->getScoreVersion());
        it = allLevelScores.find(cacheKey);
        if (it != allLevelScores.end()) {
            return it->second;
        } else {
            it = curLevelScores.find(levelProxy->getId());
            if (it != curLevelScores.end()) {
                return it->second;
            } else {
                return NULL;
            }
        }
    }
    
    DOMElement * ScoreManager::getCreateLevel(lev::Proxy *levelProxy) {
        DOMElement * level = getLevel(levelProxy);
        if (level == NULL || XMLString::parseInt(level->getAttribute(
                Utf8ToXML("version").x_str())) != levelProxy->getScoreVersion()) {
            // no level score entry for this scoreversion exists - create it
            isModified = true;
            DOMElement * newLevel = doc->createElement (Utf8ToXML("level").x_str());
            newLevel->setAttribute(Utf8ToXML("id").x_str(), Utf8ToXML(levelProxy->getId()).x_str());
            newLevel->setAttribute(Utf8ToXML("version").x_str(),  Utf8ToXML(ecl::strf("%d",levelProxy->getScoreVersion())).x_str());
            newLevel->setAttribute(Utf8ToXML("sec").x_str(), Utf8ToXML("crc").x_str());
            
            levelsElem->appendChild(newLevel);
            std::string cacheKey = levelProxy->getId() + "#" + ecl::strf("%d", levelProxy->getScoreVersion());
            allLevelScores[cacheKey] = newLevel;
            if (level != NULL && XMLString::parseInt(level->getAttribute(
                    Utf8ToXML("version").x_str())) <  levelProxy->getScoreVersion()) {
                // this new levelversion is newer than the previous current one
                // update solved status form previous current one
                const XMLCh * attr = level->getAttribute(Utf8ToXML("diff1").x_str());
                int scoreDiff = (XMLString::stringLen(attr) > 0) ? XMLString::parseInt(attr) : -1;
                if (scoreDiff != SCORE_UNSOLVED) {
                    newLevel->setAttribute(Utf8ToXML("diff1").x_str(), Utf8ToXML(ecl::strf("%d",SCORE_SOLVED)).x_str());
                    newLevel->setAttribute(Utf8ToXML("diff1rel").x_str(),
                           level->getAttribute(Utf8ToXML("diff1rel").x_str()));
                }
                attr = level->getAttribute(Utf8ToXML("easy1").x_str());
                int scoreEasy = (XMLString::stringLen(attr) > 0) ? XMLString::parseInt(attr) : -1;
                if (scoreEasy != SCORE_UNSOLVED) {
                    newLevel->setAttribute(Utf8ToXML("easy1").x_str(), Utf8ToXML(ecl::strf("%d",SCORE_SOLVED)).x_str());
                    newLevel->setAttribute(Utf8ToXML("easy1rel").x_str(),
                           level->getAttribute(Utf8ToXML("easy1rel").x_str()));
                }
                attr = level->getAttribute(Utf8ToXML("rating").x_str());
                int oldRating = (XMLString::stringLen(attr) > 0) ? XMLString::parseInt(attr) : -1;
                if (oldRating != -1) {
                    newLevel->setAttribute(Utf8ToXML("rating-inherited").x_str(),
                        Utf8ToXML(ecl::strf("%d",oldRating)).x_str());
                }
                curLevelScores[levelProxy->getId()] = newLevel;
            } else if (level == NULL) {
                curLevelScores[levelProxy->getId()] = newLevel;
            }
            level = newLevel;
        }
        return level;
    }

    // Legacy 0.92 
    unsigned ScoreManager::idFromLegacyScore() {
        unsigned id3 = 0;
        int bits = 0;
        bool hasLevels = false; 
        for (int mode = 0; mode<2; mode++) {
            bool withEasy = (mode == 0) ? false : true;
            std::set<std::string> levelIds = Proxy::getLevelIds(withEasy);
            for (std::set<std::string>::iterator it = levelIds.begin(); it != levelIds.end(); it++) {
                options::LevelStatus levelstat;
                if (options::GetLevelStatus (*it, levelstat)) {
                    hasLevels = true;
                    int diffScore = levelstat.time_hard;
                    if (diffScore > 0) {
                        id3 <<= 1;
                        id3 |= (diffScore & 1);
                        bits++;
                        if (bits >= 16)
                            return id3 & 0xFFFF;
                    }
                }
            }
        }
        if (hasLevels)
            return id3 & 0xFFFF;
        else
            return 0x10000;
    }
    
    std::string ScoreManager::upgradeSum() {
        std::string scores;
        for (int i = 0, l = levelList->getLength(); i < l; i++) {
            DOMElement * level  = dynamic_cast<DOMElement *>(levelList->item(i));
            const XMLCh *attr = level->getAttribute(Utf8ToXML("diff1").x_str());
            int score = (XMLString::stringLen(attr) > 0) ? XMLString::parseInt(attr) : -1;
            if (score >= 0)
                scores += ecl::strf("%d", score);
            attr = level->getAttribute(Utf8ToXML("easy1").x_str());
            score = (XMLString::stringLen(attr) > 0) ? XMLString::parseInt(attr) : -1;
            if (score >= 0)
                scores += ecl::strf("%d", score);
        }
        return sec(scores);
    }
    
    bool ScoreManager::upgradeLegacy() {
        bool result = false;
        // it is likely that the user has tested 1.00 and solved a few levels
        // before he updates from his enigma.rc2
        for (int mode = 0; mode<2; mode++) {
            // first levels without easy, then those with easy
            bool withEasy = (mode == 0) ? false : true;
            std::set<std::string> levelIds = Proxy::getLevelIds(withEasy);
            for (std::set<std::string>::iterator it = levelIds.begin(); it != levelIds.end(); it++) {
                options::LevelStatus levelstat;
                if (options::GetLevelStatus (*it, levelstat)) {
                    result = true;
                    int diffScore = levelstat.time_hard;
                    if (!withEasy) {
                        // 0.92 scores could be saved at either place
                        if (levelstat.time_easy > 0 && (levelstat.time_easy < diffScore || diffScore<0))
                            diffScore = levelstat.time_easy;
                    }
                    bool solvedDiff = ((levelstat.finished & DIFFICULTY_HARD) ||
                            (!withEasy && levelstat.finished)) &&
                            diffScore != SCORE_UNSOLVED;
                    int easyScore = levelstat.time_easy;
                    bool solvedEasy = (levelstat.finished & DIFFICULTY_EASY) &&
                            easyScore != SCORE_UNSOLVED;
                    
                    // limit scores to 99*60+59 - they need to fit in a short for XML
                    if (diffScore > SCORE_MAX)
                        diffScore = SCORE_MAX;
                    if (easyScore > SCORE_MAX)
                        easyScore = SCORE_MAX;
                    
                    std::map<std::string,  DOMElement *>::iterator itx;
                    // do we have to update a score entry for a higher version ?
                    itx = curLevelScores.find(*it);
                    bool isMostRecent = true;
                    if (itx != curLevelScores.end() &&
                            levelstat.solved_revision < XMLString::parseInt(
                                (*itx).second->getAttribute(Utf8ToXML("version").x_str()))) {
                        isMostRecent = false;
                        if (solvedDiff) {
                            const XMLCh *attr = (*itx).second->getAttribute(Utf8ToXML("diff1").x_str());
                            int curDiff = (XMLString::stringLen(attr) > 0) ? XMLString::parseInt(attr) : -1;
                            if (curDiff == SCORE_UNSOLVED) {
                                (*itx).second->setAttribute(Utf8ToXML("diff1").x_str(),
                                        Utf8ToXML(ecl::strf("%d",SCORE_SOLVED)).x_str());
                                (*itx).second->setAttribute(Utf8ToXML("diff1rel").x_str(),
                                        Utf8ToXML("0.92").x_str());
                            }
                        }
                        if (withEasy && solvedEasy) {
                            const XMLCh *attr = (*itx).second->getAttribute(Utf8ToXML("easy1").x_str());
                            int curEasy = (XMLString::stringLen(attr) > 0) ? XMLString::parseInt(attr) : -1;
                            if (curEasy == SCORE_UNSOLVED) {
                                (*itx).second->setAttribute(Utf8ToXML("easy1").x_str(),
                                        Utf8ToXML(ecl::strf("%d",SCORE_SOLVED)).x_str());
                                (*itx).second->setAttribute(Utf8ToXML("easy1rel").x_str(),
                                        Utf8ToXML("0.92").x_str());
                            }
                        }
                    } 
                    // do we have already an score entry for this version?
                    std::string cacheKey = *it + "#" + ecl::strf("%d", levelstat.solved_revision);
                    itx = allLevelScores.find(cacheKey);
                    if (itx != allLevelScores.end() &&
                            levelstat.solved_revision == XMLString::parseInt(
                                (*itx).second->getAttribute(Utf8ToXML("version").x_str()))) {
                        // check if old score is better than current one
                        if (solvedDiff) {
                            const XMLCh *attr = (*itx).second->getAttribute(Utf8ToXML("diff1").x_str());
                            int curDiff = (XMLString::stringLen(attr) > 0) ? XMLString::parseInt(attr) : -1;
                            if (diffScore < curDiff || curDiff < 0) {
                                if (curDiff >= 0) {
                                    // backup current best score as second
                                    (*itx).second->setAttribute(Utf8ToXML("diff2").x_str(),
                                        Utf8ToXML(ecl::strf("%d",curDiff)).x_str());
                                    (*itx).second->setAttribute(Utf8ToXML("diff2rel").x_str(),
                                        (*itx).second->getAttribute(Utf8ToXML("diff1rel").x_str()));
                                }
                                (*itx).second->setAttribute(Utf8ToXML("diff1").x_str(),
                                        Utf8ToXML(ecl::strf("%d",diffScore)).x_str());
                                (*itx).second->setAttribute(Utf8ToXML("diff1rel").x_str(),
                                        Utf8ToXML("0.92").x_str());
                            }
                        }
                        if (withEasy && solvedEasy) {
                            const XMLCh *attr = (*itx).second->getAttribute(Utf8ToXML("easy1").x_str());
                            int curEasy = (XMLString::stringLen(attr) > 0) ? XMLString::parseInt(attr) : -1;
                            if (easyScore < curEasy || curEasy < 0) {
                                if (curEasy >= 0) {
                                    // backup current best score as second
                                    (*itx).second->setAttribute(Utf8ToXML("easy2").x_str(),
                                        Utf8ToXML(ecl::strf("%d",curEasy)).x_str());
                                    (*itx).second->setAttribute(Utf8ToXML("easy2rel").x_str(),
                                        (*itx).second->getAttribute(Utf8ToXML("easy1rel").x_str()));
                                }
                                (*itx).second->setAttribute(Utf8ToXML("easy1").x_str(),
                                        Utf8ToXML(ecl::strf("%d",easyScore)).x_str());
                                (*itx).second->setAttribute(Utf8ToXML("easy1rel").x_str(),
                                        Utf8ToXML("0.92").x_str());
                            }
                        }                        
                    } else if (solvedDiff || (withEasy && solvedEasy)) {
                        // we need a new entry for this version
                        DOMElement * newLevel = doc->createElement (Utf8ToXML("level").x_str());
                        newLevel->setAttribute(Utf8ToXML("id").x_str(), Utf8ToXML(*it).x_str());
                        newLevel->setAttribute(Utf8ToXML("version").x_str(),  Utf8ToXML(ecl::strf("%d",levelstat.solved_revision)).x_str());
                        if (solvedDiff) {
                            newLevel->setAttribute(Utf8ToXML("diff1").x_str(),
                                    Utf8ToXML(ecl::strf("%d",diffScore)).x_str());
                            newLevel->setAttribute(Utf8ToXML("diff1rel").x_str(),
                                    Utf8ToXML("0.92").x_str());
                        }
                        if (withEasy && solvedEasy) {
                            newLevel->setAttribute(Utf8ToXML("easy1").x_str(),
                                    Utf8ToXML(ecl::strf("%d",easyScore)).x_str());
                            newLevel->setAttribute(Utf8ToXML("easy1rel").x_str(),
                                    Utf8ToXML("0.92").x_str());
                        }
                        newLevel->setAttribute(Utf8ToXML("sec").x_str(), Utf8ToXML("crc").x_str());
                        levelsElem->appendChild(newLevel);
                        allLevelScores[cacheKey] = newLevel;
                        if (isMostRecent)
                            curLevelScores[*it] = newLevel;
                    }
                }
            }
        }
        return result;
    }
}} // namespace enigma::lev
