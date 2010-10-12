/*
 * Copyright (C) 2002,2003,2004,2005,2006 Daniel Heck, Ronald Lamprecht
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
 
#include "gui/LevelPreviewCache.hh"
#include "ecl.hh"
#include "file.hh"
#include "game.hh"
#include "main.hh"
#include "video.hh"
#include "lev/Proxy.hh"

#include "SDL.h"
#include <fstream>

using namespace ecl;
using namespace std;

namespace enigma { namespace gui {
    /* -------------------- LevelPreviewCache -------------------- */
    
    LevelPreviewCache *LevelPreviewCache::theSingleton = 0;
    
    LevelPreviewCache* LevelPreviewCache::instance() {
        if (theSingleton == 0) {
            theSingleton = new LevelPreviewCache();
        }
        return theSingleton;
    }

    LevelPreviewCache::LevelPreviewCache() : cachedIndex (NULL) {
    }
    
    void LevelPreviewCache::clear() { 
        Log << "LevelPreviewCache clear\n";
        cache.clear(); 
        imgCache.clear();
    }
        
    Surface *LevelPreviewCache::getPreview(lev::Proxy *levelProxy, 
            bool allowGeneration, bool &didGenerate) {
        didGenerate = false;
        std::string mapIndex = levelProxy->getId() + 
                ecl::strf("#%d", levelProxy->getReleaseVersion());
        // first look in cache
        PreviewMap::iterator i = cache.find(mapIndex);
        if (i != cache.end())
            return i->second;
    
        // we have to cache a new preview - check if we should clear cache first
        if (cachedIndex == NULL) {
            // remember the index we are caching
            cachedIndex = lev::Index::getCurrentIndex();
        } else if (cachedIndex != lev::Index::getCurrentIndex()) {
            //the index changed - cache only previews from new index
            clear();
            cachedIndex = lev::Index::getCurrentIndex();
        }
            
        std::string previewSubPath = makePreviewPath(levelProxy);
        Surface *surface = 0;
        
        // load preview from file bundled with the level itself
        std::string absLevelPath ;
        std::auto_ptr<std::istream> isptr;
        ByteVec imageData;
        if(levelProxy->getNormPathType() == lev::Proxy::pt_resource && 
                app.resourceFS->findFile ("levels/" + levelProxy->getNormLevelPath() + ".png", 
                        absLevelPath, isptr)) {
            // load plain image file or zipped image
            if (isptr.get() != NULL) {
                // zipped file
                Readfile (*isptr, imageData);
            } else {
                // plain file
                basic_ifstream<char> ifs(absLevelPath.c_str(), ios::binary | ios::in);
                Readfile (ifs, imageData);
            }
            SDL_RWops *imageHandle = SDL_RWFromMem(&(imageData[0]), imageData.size());
            surface = ecl::LoadImage(imageHandle, 1);
            imgCache.store(previewSubPath, surface); // insert in imgCache
        }
                        
        // load preview from stored file if possible
        string previewFullPath;
        if (!surface && app.resourceFS->findFile(previewSubPath, previewFullPath))
            surface = imgCache.get(previewFullPath);

        // generate new preview otherwise
        if (!surface && allowGeneration) {
            surface = newPreview(levelProxy);
            if (surface) {
                imgCache.store(previewSubPath, surface); // insert in imgCache
                savePreview(levelProxy, surface);        // save on filesystem
            } else {
                surface = enigma::GetImage("error");
            }
            didGenerate = true;
        }

        // update index
        if (surface)
            cache[mapIndex] = surface;
        return surface;
    }
    
    ecl::Surface *LevelPreviewCache::newPreview (lev::Proxy *levelProxy) {
        const video::VMInfo &vminfo = *video::GetInfo();
        Surface *surface = 0;
        ecl::GC gc(video::BackBuffer());
        if (game::DrawLevelPreview (gc, levelProxy)) { 
            surface = Resample (video::BackBuffer(), 
                        vminfo.gamearea, vminfo.thumbw, vminfo.thumbh); 
        }
        return surface;
    }
    
    Surface *LevelPreviewCache::updatePreview (lev::Proxy *levelProxy) {
        if (Surface *surface = newPreview (levelProxy)) {
            std::string idx = levelProxy->getId() + 
                    ecl::strf("#%d", levelProxy->getReleaseVersion());
            cache[idx] = surface;
    
            string preview_name = makePreviewPath(levelProxy);
            imgCache.store (preview_name, surface);  // insert in imgCache
            savePreview(levelProxy, surface);        // save on filesystem
            return surface;
        }
        return 0;
    }
    
    std::string LevelPreviewCache::makePreviewPath(lev::Proxy *levelProxy) {
        return "thumbs/" +
                levelProxy->getLocalSubstitutionLevelPath() +
                ecl::strf("#%d", levelProxy->getReleaseVersion()) + ".png";
    }
    
    void LevelPreviewCache::savePreview(lev::Proxy *levelProxy, ecl::Surface *s) {
        std::string savePath = app.userImagePath + "/" + makePreviewPath(levelProxy);
        Log << "savePreview to " << savePath << "\n";
        // auto-create the directory if necessary
        string directory;
        if (ecl::split_path (savePath, &directory, 0) && !ecl::FolderExists(directory)) {
            ecl::FolderCreate (directory);
        }
        ecl::SavePNG(s, savePath);
    }
    
    void LevelPreviewCache::makeSystemPreview(lev::Proxy *levelProxy, std::string systemDataPath) {
        std::string savePath = systemDataPath + "/levels/" + levelProxy->getNormLevelPath() + ".png";
        // auto-create the directory if necessary -- on an installed Enigma
        // distribution this is of course unnecessary, but you start Enigma
        // without prior installation. This is useful to get a directory with
        // just the previews.
        string directory;
        if (ecl::split_path (savePath, &directory, 0) && !ecl::FolderExists(directory)) {
            ecl::FolderCreate (directory);
        }
        ecl::Surface * s = newPreview(levelProxy);
        if (s != NULL)
            ecl::SavePNG(s, savePath);
    }
}} // namespace enigma::gui
