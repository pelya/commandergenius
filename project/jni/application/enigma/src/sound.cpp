/*
 * Copyright (C) 2002,2003,2004 Daniel Heck
 * Copyright (C) 2007 Andreas Lochmann
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
#include "errors.hh"
#include "enigma.hh"
#include "options.hh"
#include "sound.hh"
#include "main.hh"
#include "oxyd.hh"
#include "oxydlib/OxydVersion.h"
#include "nls.hh"
#include "client.hh"

#include "SDL.h"
#include "SDL_mixer.h"

#include <string>
#include <iostream>
#include <cassert>
#include <memory>

using namespace std;
using namespace enigma;
using namespace sound;
using namespace OxydLib;

#include "sound_internal.hh"

/*! This function converts raw audio data with a specified format to
  the mixer's audio format.  This is used for converting the original
  Oxyd sounds to a format usable by Enigma. */
Mix_Chunk *ChunkFromRaw (const Uint8 *buf, Uint32 len,
                         int freq, int format, int channels);


/* -------------------- SoundEvent implementation -------------------- */
SoundEvent::SoundEvent ()
: name(""), has_position(false), position(),
  priority (0), volume (0.0),
  left (0), right(0), active (false),
  playing_time (0)
{
}

bool not_active (const SoundEvent &s)
{
    return !s.active;
}

bool lower_priority (const SoundEvent &s, const SoundEvent &t)
{
    return s.priority < t.priority;
}



/* -------------------- SoundEngine_SDL implementation -------------------- */

class MutexLock {
public:
    MutexLock (SDL_mutex *m) {
        mutex = m;
        SDL_mutexP (mutex);
    }
    ~MutexLock () {
        SDL_mutexV (mutex);
    };
private:
    SDL_mutex *mutex;
};

SoundEngine_SDL *SoundEngine_SDL::m_instance = 0;

SoundEngine_SDL::SoundEngine_SDL()
: m_initialized(false),
  m_soundvolume (MIX_MAX_VOLUME),
  m_musicvolume (MIX_MAX_VOLUME),
  m_current_music (0),
  m_freq (MIX_DEFAULT_FREQUENCY),
  m_format (MIX_DEFAULT_FORMAT),
  m_channels (MIX_DEFAULT_CHANNELS)
{
    assert (m_instance == 0);
    m_instance = this;
}

SoundEngine_SDL::~SoundEngine_SDL()
{
    shutdown();
    m_instance = 0;
}

bool SoundEngine_SDL::init() 
{
    if (!m_initialized) {
        // Initialize SDL audio subsystem
        if (SDL_InitSubSystem (SDL_INIT_AUDIO) == -1) {
            fprintf(stderr, "Couldn't open SDL audio subsystem: %s\n", SDL_GetError());
            return false;
        }

        const SDL_version* vi = Mix_Linked_Version();
        Log <<  ecl::strf("SDL_mixer Version: %u.%u.%u\n", vi->major, vi->minor, vi->patch);
#ifdef SDL_MIX_INIT
        int mix_flags = MIX_INIT_MOD;
        if (Mix_Init(mix_flags) & mix_flags != mix_flags) {
            Log << ecl::strf( "Couldn't initialize SDL_mixer: %s\n", Mix_GetError());
            return false;
        }
#endif

        // Initialize SDL_mixer lib
        if (Mix_OpenAudio(m_freq, m_format, m_channels, 1024) < 0) {
            fprintf(stderr, "Couldn't open mixer: %s\n", Mix_GetError());
            return false;
        }

        // Update number of available channels
        m_channels = Mix_GroupCount (-1);
        m_channelinfo.resize (m_channels);

        Mix_ChannelFinished (&channel_finished);
        m_mutex = SDL_CreateMutex();
        if (!m_mutex)
            return false;

        m_initialized = true;
    }
    return true;
}

void SoundEngine_SDL::shutdown() 
{
    if (m_initialized) {
	Mix_FreeMusic(m_current_music);
        Mix_CloseAudio();
        clear_cache();
        SDL_DestroyMutex (m_mutex);
#ifdef SDL_MIX_INIT
        Mix_Quit();
#endif
        m_initialized = false;
    }
}


void SoundEngine_SDL::clear_cache() 
{
    for (ecl::Dict<Mix_Chunk*>::iterator it = wav_cache.begin(); it != wav_cache.end(); ++it)
        Mix_FreeChunk(it->second);
    wav_cache.clear();
}

void SoundEngine_SDL::set_sound_volume (double soundvol) 
{
    if (!m_initialized)
        return;                 // SDL_mixer crashes without this check

    m_soundvolume = ecl::round_down<int>(ecl::Clamp(soundvol, 0.0, 1.0) * MIX_MAX_VOLUME);
    Mix_Volume (-1, m_soundvolume);
}

void SoundEngine_SDL::set_music_volume (double musicvol) 
{
    if (!m_initialized)
        return;                 // SDL_mixer crashes without this check

    m_musicvolume = ecl::round_down<int>(ecl::Clamp(musicvol, 0.0, 1.0) * MIX_MAX_VOLUME);
    Mix_VolumeMusic (m_musicvolume);
}


void SoundEngine_SDL::stop_music() 
{
    Mix_HaltMusic();
    Mix_FreeMusic(m_current_music);
    m_current_music = 0;
}

bool SoundEngine_SDL::play_music (const std::string &filename) 
{
    if (Mix_Music *music = Mix_LoadMUS(filename.c_str())) {
        if (m_current_music)
            Mix_FreeMusic (m_current_music);
        m_current_music = music;
        Mix_PlayMusic (m_current_music, -1);
        Mix_VolumeMusic (m_musicvolume);
        return true;
    }
    return false;
}

void SoundEngine_SDL::fadeout_music()
{
    while (Mix_FadingMusic() != MIX_NO_FADING)
        SDL_Delay(10);

    if (Mix_PlayingMusic()) {
        Mix_FadeOutMusic(500);
        SDL_Delay(400);
    }
    while (Mix_PlayingMusic())
        SDL_Delay(10);
}

void SoundEngine_SDL::update_channel (int channel)
{
    // If distance sound origin <= this value, play at full volume
    const double fullvol_range = 0.2;

    // How far can sound travel?
    const double range = 30;    

    SoundEvent &se = m_channelinfo[channel];

    double volume;
    int    left;
    int    right;
    if (se.has_position) {
        ecl::V2 distv = se.position - m_listenerpos;
        double dist = max(0.0, length(distv) - fullvol_range);

        int xdist = int(distv[0] * options::GetDouble("StereoSeparation"));

        left  = ecl::Clamp (255 - xdist, 0, 255);
        right = ecl::Clamp (255 + xdist, 0, 255);
        volume = (1 - dist/range) * se.volume;
    }
    else
    {
        volume = se.volume;
        left = se.left;
        right = se.right;
    }

    Mix_SetPanning (channel, left, right);

    int mixvol = ecl::round_down<int>(volume * MIX_MAX_VOLUME);
    Mix_Volume(channel, ecl::Clamp(mixvol, 0, MIX_MAX_VOLUME));
}

int SoundEngine_SDL::already_playing (const SoundEvent &s)
{
    for (size_t i=0; i<m_channelinfo.size(); ++i) {
        const SoundEvent &se = m_channelinfo[i];

        if (se.active && se.name == s.name && se.playing_time < 0.05
            && (!se.has_position || !s.has_position ||
		ecl::length(se.position - s.position) < 30))
            return static_cast<int> (i);
    }
    return -1;
}


Mix_Chunk *SoundEngine_SDL::cache_sound(const std::string &name)
{
    ecl::Dict<Mix_Chunk*>::iterator i=wav_cache.find(name);
    if (i == wav_cache.end()) {
        Mix_Chunk *ch = 0;
        string filename;
        if (app.resourceFS->findFile("soundsets/" + name + ".wav", filename))
             ch = Mix_LoadWAV(filename.c_str());
        if (ch != 0)
            wav_cache.insert(name, ch);
        else
            enigma::Log << "Couldn't load sample '" << name << "': "
                        << Mix_GetError() << endl;
        return ch;
    } else
        return i->second;
}

bool SoundEngine_SDL::play_sound (const SoundEvent &s)
{
    int channel = already_playing (s);
    if (channel != -1) {
        MutexLock (m_instance->m_mutex);
        SoundEvent &se = m_channelinfo [channel];
        if (se.has_position) {
            se.position = (se.position + s.position) / 2;
            update_channel (channel);
            return true;
        }
    }
    
    if (Mix_Chunk *chunk = cache_sound(s.name)) {
        channel = -1; //Mix_GroupOldest(-1);

        channel = Mix_PlayChannel(channel, chunk, 0);

        if (channel != -1) {
            {
                MutexLock (m_instance->m_mutex);
                SoundEvent &se = m_channelinfo[channel];
                se = s;
                se.active       = true;
                se.playing_time = 0.0;
            }
            update_channel (channel);
        }
        return true; // even if no free channel was found
    } else
        return false;
}

void SoundEngine_SDL::tick (double dtime)
{
    MutexLock (m_instance->m_mutex);
    for (size_t i=0; i<m_channelinfo.size(); ++i) {
        SoundEvent &se = m_channelinfo[i];
        if (se.active)
            se.playing_time += dtime;
    }
}


void SoundEngine_SDL::define_sound (
    const SoundName &/*name*/,
    const std::string &/*filename*/)
{
}

void SoundEngine_SDL::define_sound (
    const SoundName &name, 
    const SoundData &data)
{
    Uint32 bufsize = static_cast<Uint32> (data.buf.size());
    Mix_Chunk *ch= ChunkFromRaw (&data.buf[0], bufsize,
                                 data.freq, AUDIO_S8, data.nchannels);
    if (ch != 0)
        wav_cache.insert(name, ch);
}

void SoundEngine_SDL::channel_finished (int channel)
{
    MutexLock (m_instance->m_mutex);
    SoundEvent &se = m_instance->m_channelinfo[channel];
    se.active = false;
}




/* -------------------- Local variables -------------------- */
namespace
{
    auto_ptr<SoundEngine> sound_engine;

    bool sound_enabled      = true;
    bool music_enabled      = true;
    bool sound_enabled_temp = false;

    string     current_music_name;
}



/* -------------------- Functions -------------------- */

void sound::Init(bool withMusic, bool withSound) 
{
    sound_enabled = withSound;
    music_enabled = withMusic;
    if (!sound_engine.get()) {
        sound_engine.reset(new SoundEngine_SDL);
    }

    if (sound_engine->init()) {
        options::UpdateVolume();
    }
    else {
        sound_enabled = false;
        music_enabled = false;
        sound_engine.reset(new SoundEngine_Null);
    }
}

void sound::Shutdown() 
{
    if (sound_engine.get())
        sound_engine->shutdown();
}

void sound::Tick (double dtime)
{
    sound_engine->tick (dtime);
}

void sound::TempDisableSound() {
    sound_enabled_temp = sound_enabled;
    sound_enabled      = false;
}

void sound::TempReEnableSound() {
    sound_enabled = sound_enabled_temp;
}

void sound::SetListenerPosition (const ecl::V2 &pos) 
{
    sound_engine->set_listenerpos (pos);
}

bool sound::PlaySound (const SoundName &name, const ecl::V2 &pos, double volume, int priority) 
{
    if (!sound_enabled)
        return false;

    SoundEvent se;
    se.name         = name;
    se.has_position = true;
    se.position     = pos;
    se.priority     = priority;
    se.volume       = volume * options::GetDouble("SoundVolume");
    se.left = se.right = 0;

    return sound_engine->play_sound (se);
}

bool sound::PlaySoundGlobal (const SoundName &name, double volume, int priority) 
{
    SoundEvent se;
    se.name         = name;
    se.has_position = false;
    se.position     = ecl::V2();
    se.priority     = priority;
    se.volume       = volume * options::GetDouble("SoundVolume");
    se.left         = 255;
    se.right        = 255;

    return sound_engine->play_sound (se);
}

void sound::FadeoutMusic() 
{
    sound_engine->fadeout_music();
}

void sound::PlayMusic (const std::string &name) 
{
    if (!sound_enabled || !music_enabled || name==current_music_name)
        return;

    FadeoutMusic();

    string fname;
    if (app.resourceFS->findFile (name, fname) && sound_engine->play_music (fname))
        current_music_name = name;
    else
        current_music_name = "";

}

void sound::StopMusic() {
    sound_engine->stop_music();
    current_music_name = "";
}

void sound::StopMusic (const std::string &name) {
    if (name==current_music_name)
        StopMusic();
}

void sound::ClearCache()
{
    sound_engine->clear_cache();
}

void sound::DefineSound (const SoundName &name, const SoundData &data)
{
    sound_engine->define_sound (name, data);
}

void sound::SetSoundVolume (double vol)
{
    sound_engine->set_sound_volume (vol);
}

void sound::SetMusicVolume (double vol)
{
    sound_engine->set_music_volume (vol);
}

/* SDL_ConvertAudio is only capable of changing the sound frequency by
   integer powers of 2 (i.e., by a factor of ... 1/4 1/2 1 2 ...).
   The sound files used by Oxyd are sampled at 6kHz which we must
   convert to roughly 22kHz.  This function resamples between any two
   frequencies using simple linear interpolation.  It is not capable
   of changing the sample format or dealing with more than one
   channel.

   FIXME: We should apply a lowpass filter after reampling to get rid
   of the artifacts introduced by linear interpolation or use a better
   interpolator.
*/
namespace
{
    Sint8 *resample (const Sint8 *data, Uint32 len, int oldfreq, int newfreq, 
                     Uint32 *newlen_)
    {
        assert (data);
        assert (len>0);
        assert (oldfreq > 0);
        assert (newfreq > 0);
        const int sample_size = 1;    //  8bit sample data

        float ratio = float(oldfreq) / float(newfreq);
        Uint32 newlen = ecl::round_down<int> (len / ratio);
        *newlen_ = newlen;
        Sint8 *newdata = (Sint8*) malloc (sample_size * newlen);
        if (!newdata)
            return 0;

        const Sint8 *src = data;
        Sint8       *dst = newdata;

        float srcinc = float (len-1) / float (newlen); 
        for (unsigned i=0; i<newlen; ++i) {
            int srcidx = ecl::round_down <int> (i * srcinc); // srcpos);
            float a2 = i*srcinc - srcidx;
            float a1 = 1.0f - a2;
            dst[i] = static_cast<Sint8> ((a1*src[srcidx] + a2*src[srcidx+1])/2);
        }
        return newdata;
    }

//     typedef float (*FilterFunc)(float);

//     inline double sinc(double x)
//     {
//         if (x == 0)
//             return 1;
//         x *= M_PI;
//         return sin(x)/x;
//     }

//     class Sinc {
//     public:
//         static const int support = 4;

//         static double func (double x) {
//             if (x < -4 || x > 4)
//                 return 0;
//             else
//                 return sinc(x);
//         }
//     };


//     class Lanczos2 {
//     public:
//         static const int support = 2;

//         static double func (double x) {
//             if (x < -2 || x > 2)
//                 return 0;
//             else
//                 return sinc(x) * sinc(x/2);
//         }
//     };

//     class Lanczos3 {
//     public:
//         static const int support = 3;

//         static double func (double x) {
//             if (x < -3 || x > 3)
//                 return 0;
//             else
//                 return sinc(x) * sinc(x/3);
//         }
//     };


//     template <class Filter, class SampleT>
//     class Resampler {
//         double ratio;
//         vector<double> G;
//     public:
//         Resampler (double ratio_)
//         : ratio (ratio_)
//         {
//         }

//         size_t calc_destlen (size_t sourcelen)
//         {
//             return static_cast<size_t> ((sourcelen / ratio + 1) 
//                                         * sizeof (SampleT));
//         }

//         void resample (const SampleT *source, size_t sourcelen, 
//                        SampleT *dest, size_t destlen)
//         {
//             double        factor  = (double)sourcelen / destlen;
//             int           support = Filter::support;
//             double         dx      = ecl::Max(factor, 1.0);
//             vector<double> F (2*support + 1);

//             SampleT *dp = dest;
//             for (size_t t=0; t<destlen; ++t) 
//             {
//                 double center = (t+0.5)*factor; // kernel center in the source
//                 int   start  = ecl::Max<int>(0, center-support + 0.5);
//                 int   stop   = ecl::Min<int>(sourcelen, center+support+0.5);
//                 int   n      = stop-start;

//                 // Calculate filter coefficients
//                 double c      = 0;
//                 double pos = start-center+0.5;
//                 for (int j=0; j<n; ++j) {
//                     F[j] = Filter::func(pos/dx);
//                     c += F[j];
//                     pos++;
//                 }
//                 // Normalize filter
//                 if (c != 0 && c!=1)
//                     for (int j=0; j<n; ++j)
//                         F[j] /= c;

//                 // Calculate next destination value
//                 const SampleT *sp = source + start;
//                 double accu = 0;
//                 for (int j=n-1; j>=0; --j)
//                     accu += F[j] * sp[j];
//                 *dp++ = static_cast<SampleT> (accu); 
// //                 *dp++ = static_cast<SampleT> (ecl::Clamp<double> (accu, -128, 127));
//             }
//         }

//     };
}

Mix_Chunk * ChunkFromRaw (const Uint8 *buf, Uint32 len,
                          int sfreq, int sformat, int schannels)
{
    if (!sound_enabled || !buf)
        return 0;

    // Get destination format
    int dfreq, dchannels;
    Uint16 dformat;
    Mix_QuerySpec (&dfreq, &dformat, &dchannels);

    // Resample
//     Resampler<Sinc, Sint8> r (double(sfreq)/dfreq);
//     size_t newlen = r.calc_destlen(len);
//     Uint8 *newbuf = (Uint8*) malloc (newlen);
//     r.resample ((Sint8*)buf, len, (Sint8*)newbuf, newlen);

    // Resample
    Uint32 newlen=0;
    Uint8 *newbuf = (Uint8*)resample((const Sint8*)buf, len, sfreq, dfreq, &newlen);

    // Convert audio data
    SDL_AudioCVT cvt;
    if (!SDL_BuildAudioCVT (&cvt, sformat, schannels, dfreq,
                            dformat, dchannels, dfreq))
        return 0;

    cvt.buf = (Uint8*) malloc(newlen * cvt.len_mult);
    cvt.len = newlen;
    memcpy(cvt.buf, newbuf, newlen);
    free(newbuf);

    SDL_ConvertAudio(&cvt);

    Mix_Chunk *chunk = Mix_QuickLoad_RAW(cvt.buf, cvt.len_cvt);
    chunk->allocated = 1;
    return chunk;
}


/* ------------- Conversion and helper functions ------------- */

/*! This function defines how to put soundset_key and eventname together to
  create an eventkey. */

string SoundEngine::effectKey(string effect_name, string soundset_name)
{
    if (soundset_name == "")
        return getActiveSoundSetKey() + "#" + effect_name;
    else
        return soundset_name + "#" + effect_name;
}

/*! This function searches the known sound sets for a sound set with given
  oxyd version, and returns the sound set name (or empty string if none). */

string SoundEngine::getOxydSoundSet(OxydVersion oxyd_ver)
{
    for (SoundSetRepository::iterator i = sound_sets.begin();
             i != sound_sets.end(); ++i)
        if((*i).second.getOxydVersion() == oxyd_ver)
            return (*i).first;
    return "";
}

string sound::GetOxydSoundSet(OxydVersion oxyd_ver)
{
    return sound_engine->getOxydSoundSet(oxyd_ver);
}

/*! Enigma 1.00 only knew the option "SoundSet", which was an integer. This
  was quite unhandy if one wanted to add additional sound sets. Since 1.01
  Enigma features the option "SoundSetName". Still, old "SoundSet" is needed
  if user wants to switch to <= 1.00 again; so here are the conversion
  functions. Any user sound set is mapped to 0 ("Default").  */

int SoundEngine::convertToOldSoundSetNumber(string soundset_name)
{
    if(soundset_name == "Default")  return 0;
    if(soundset_name == "Enigma")   return 1;
    SoundSet sd = sound_sets[soundset_name];
    if(sd.isOxyd())
        return ((int) sd.getOxydVersion()) + 2;
    return 0;
}

string SoundEngine::convertFromOldSoundSetNumber(int soundset_number)
{
    if(soundset_number == 0)  return "Default";
    if(soundset_number == 1)  return "Enigma";
    return getOxydSoundSet((OxydVersion) (soundset_number - 2));
}

/* -------------------- Sound set handling -------------------- */

/*! These functions fill in data for the sound sets, initialises and
  activates them. Return false, if something went wrong, e.g. when an
  oxyd sound set is mentioned to not accessible oxyd version. */

bool SoundEngine::defineSoundSet(string soundset_name, string soundset_key,
                                 int button_position)
{
    sound_sets[soundset_name] = SoundSet(soundset_key, button_position);
    Log << "Added sound set '" << soundset_name << "' (key '" << soundset_key
        << "') on position " << button_position << ".\n";
    return true;
}

bool SoundEngine::defineSoundSetOxyd(string soundset_name, string soundset_key,
                                     OxydVersion oxyd_ver, int button_position)
{
    if(oxyd::FoundOxyd(oxyd_ver)) {
        sound_sets[soundset_name] =
            SoundSet(soundset_key, button_position, (OxydVersion) oxyd_ver);
        Log << "Added sound set '" << soundset_name << "' (key '" << soundset_key
            << "') on position " << button_position << ".\n";
        return true;
    } else {
        Log << "Skipped sound set '" << soundset_name << "'.\n";
        return false;
    }
}

bool SoundSet::activate()
{
    if(getSoundSetKey() == "")
        return false;
    if(isOxyd() &&  !oxyd::InitOxydSoundSet(getOxydVersion()))
        return false;
    sound_engine->setActiveSoundSetKey(getSoundSetKey());
    return true;
}

void sound::InitSoundSets() {
    sound_engine->initSoundSets();
}

void SoundEngine::initSoundSets()
{
    // Define sound sets
    sound_sets.clear();
    assert(sound_sets.empty());
    assert(defineSoundSet ("Enigma",   "Enigma",  1));
    int pos = 2; // position in options menu button
    if (defineSoundSetOxyd ("Oxyd",     "Oxyd*",   OxydVersion_Oxyd1,          pos))  pos++;
    if (defineSoundSetOxyd ("Magnum",   "Magnum*", OxydVersion_OxydMagnum,     pos))  pos++;
    if (defineSoundSetOxyd ("Mag.Gold", "Magnum*", OxydVersion_OxydMagnumGold, pos))  pos++;
    if (defineSoundSetOxyd ("Per.Oxyd", "Oxyd*",   OxydVersion_PerOxyd,        pos))  pos++;
    if (defineSoundSetOxyd ("Extra",    "Oxyd*",   OxydVersion_OxydExtra,      pos))  pos++;
    // Define user sound sets, as given by sound_effects
    for (SoundEffectRepository::iterator i = sound_effects.begin();
         i != sound_effects.end(); ++i) {
        string soundset_key = (*i).second.getSoundSetKey();
        bool found = false;
        // ignore Oxyd* and Magnum* sound effects, if no 
        if ((soundset_key != "Oxyd*") && (soundset_key != "Magnum*")) {
            for (SoundSetRepository::iterator j = sound_sets.begin();
                 j != sound_sets.end(); ++j)
                if((*j).second.getSoundSetKey() == soundset_key)
                    found = true;
            if(!found)
                if (defineSoundSet (soundset_key, soundset_key, pos))  pos++;
        }
    }
    Log << "Found " << pos - 1 << " different sound sets.\n";
    setSoundSetCount(pos - 1);
    setDefaultSoundSet("Enigma");
    // Extract sound set names and keys from options; activate!
    string soundset_name = app.state->getString("SoundSetName");
    if (soundset_name == "") { // just switched from 1.00 to higher
        soundset_name = convertFromOldSoundSetNumber(options::GetInt("SoundSet"));
        app.state->setProperty("SoundSetName", soundset_name);
    }
    if (soundset_name == "Default")
        soundset_name = getDefaultSoundSet();
    clear_cache();
    if (sound_sets[soundset_name].activate()) 
        Log << "Activated sound set '" << soundset_name << "'.\n";
    else {
        // Fallback, happens e.g. when oxyd sound set can't be established or 
        // a user soundset is given which doesn't exist anymore.
        Log << "Warning: Soundset '" << soundset_name << "' not available.\n";
        if (sound_sets["Enigma"].activate()) {
            app.state->setProperty("SoundSetName", "Enigma");
            options::SetOption("SoundSet", convertToOldSoundSetNumber("Enigma"));
        } else
            ASSERT(false, XFrontend,
                "Soundsets defect and fallback 'Enigma' not available.");
    }
}

void SoundEngine::setActiveSoundSet(string soundset_name)
{
    string soundset_key = sound_sets[soundset_name].getSoundSetKey();
    if (soundset_key == getActiveSoundSetKey())
        return;
    if (soundset_key == "Default") {
        Log << "Warning: Tried to choose 'Default' as effective sound set.\n";
        return;
    }
    if (soundset_key == "") {
        Log << "Warning: Tried to choose empty sound set key as effective sound set.\n";
        return;
    }
    clear_cache();
    if (sound_sets[soundset_name].activate()) {
        Log << "Switched to sound set '" << soundset_name << "' (key '" 
            << soundset_key << "').\n";
    } else
        Log << "Warning: Problems loading sound set '" << soundset_name << "' (key'"
            << soundset_key << "').\n";
}

void sound::SetActiveSoundSet(string soundset_name)
{
    sound_engine->setActiveSoundSet(soundset_name);
}

void sound::SetDefaultSoundSet(string soundset_name)
{
    sound_engine->setDefaultSoundSet(soundset_name);
    if(app.state->getString("SoundSetName") == "Default")
        SetActiveSoundSet(soundset_name);
}

string SoundEngine::getSoundSetByPosition(int button_position)
{
    for (SoundSetRepository::iterator i = sound_sets.begin();
             i != sound_sets.end(); ++i)
        if((*i).second.getButtonPosition() == button_position)
            return (*i).first;
    return "";
}

/* -------------------- Playing sound events -------------------- */

/*! The first function creates an interface to add sound events to Enigma.
  It is accessed via sound-defaults.lua and user sound definitions.
  The second method is the interface between the formal SoundEffect
  and the sound engine (via PlaySound[Global]). The last two functions
  define the interface between level objects and SoundEffect. */

void sound::DefineSoundEffect(string soundset_key, string name, string filename,
                              double volume, bool loop, bool global, int priority,
                              double damp_max, double damp_inc, double damp_mult,
                              double damp_min, double damp_tick, string silence_string) {
    assert(sound_engine.get());
    if(soundset_key == "") {
        Log << "Warning: Tried to define sound event '" << name
            << "' without sound set key. Skipped.\n";
        return;
    }
    sound_engine->defineSoundEffect(soundset_key, name,
        SoundEffect(name, soundset_key, filename, volume, loop, global, priority,
        damp_max, damp_inc, damp_mult, damp_min, damp_tick, silence_string));
}

bool SoundEffect::play(const ecl::V2 &pos, double vol, bool glob)
{
    if (filename == "") {
        Log << "No soundfile given for sound event " << name << ".\n";
        return false;
    }
    if (glob || global)
        return PlaySoundGlobal (filename, volume * vol, priority);
    else
        return PlaySound (filename, pos, volume * vol, priority);
}

bool SoundEngine::emitSoundEvent (const std::string &eventname, const ecl::V2 &pos,
                            double volume, bool force_global)
{
    string effectkey = effectKey(eventname);
    SoundEffectRepository::iterator i = sound_effects.find(effectkey);
    if (i == sound_effects.end()) {
        Log << "Undefined sound event " << effectkey << " @ " 
            << pos[0] << "," << pos[1] << "\n";
        return false;
    } else {
        return (*i).second.play(pos, volume, force_global);
    }
}

bool sound::EmitSoundEvent (const std::string &eventname, const ecl::V2 &pos,
                            double volume, bool force_global)
{
    return sound_engine->emitSoundEvent(eventname, pos, volume, force_global);
}

bool sound::EmitSoundEventGlobal (const std::string &eventname, double volume)
{
    return sound_engine->emitSoundEvent(eventname, ecl::V2(), volume, true);
}

void SoundEngine::writeSilenceString (const std::string &eventname)
{
    string effectkey = effectKey(eventname);
    SoundEffectRepository::iterator i = sound_effects.find(effectkey);
    if (i != sound_effects.end()) {
        string silence_string = (*i).second.getSilenceString();
        if (silence_string != "")
            client::Msg_ShowText (silence_string, true);
    }
}

void sound::WriteSilenceString (const std::string &eventname)
{
    sound_engine->writeSilenceString(eventname);
}

/* -------------------- Sound damping implementation -------------------- */

/*! These methods are connected to the sound damping mechanism, designed
  to reduce the noise created by some objects like st-lightpassenger. */

SoundDamping::SoundDamping(std::string effect_name_, const void *origin_)
: effect_name(effect_name_), origin(origin_)
{
    damp = sound_engine->getDampingData(effect_name_);
    factor = damp.incr;
    //Log << "New damping entry " << effect_name << " with " << damp.incr << ".\n";
}

float SoundDamping::get_volume(float def_volume) {
    if (factor < damp.maxi)
        factor += damp.incr;
    //Log << "  Found entry " << effect_name << ". Factor is now " << i->factor << ".\n";
    float q = factor * damp.mult;
    if (q > 1.0)
        return def_volume / q;
    return def_volume;
}

bool SoundDamping::tick() {
    // return true, if this entity is to be destroyed.
    factor *= damp.tick;
    return (factor <= damp.mini);
}

/* -------------------- Sound option helpers -------------------- */

/*! These functions are used in OptionsMenu.cc for the Soundset-Button. */

int sound::GetOptionSoundSetCount()
{        
    return sound_engine->getSoundSetCount() + 1;
}

int sound::GetOptionSoundSet()
{
    string soundSet = app.state->getString("SoundSetName");
    if (soundSet == "Default")
        return 0;
    int pos = sound_engine->getButtonPosition(soundSet);
    assert(pos > 0);
    return pos;
}

void sound::SetOptionSoundSet(int value)
{
    if(value == 0) {
        // settting to default sound set
        if (app.state->getString("SoundSetName") == "Default")
            return;
        app.state->setProperty("SoundSetName", "Default");
        options::SetOption("SoundSet", sound_engine->convertToOldSoundSetNumber("Default"));
        SetActiveSoundSet(sound_engine->getDefaultSoundSet());
    } else {
        string newSet = sound_engine->getSoundSetByPosition(value);
        assert(newSet != "");
        if (app.state->getString("SoundSetName") == newSet)
            return;
        app.state->setProperty("SoundSetName", newSet);
        options::SetOption("SoundSet", sound_engine->convertToOldSoundSetNumber(newSet));
        SetActiveSoundSet(newSet);
    }
}

string sound::GetOptionSoundSetText(int value)
{
    if(value == 0)
        return N_("Default");
    string soundset_name = sound_engine->getSoundSetByPosition(value);
    if(soundset_name == "")
        return "INVALID";
    return soundset_name;
}

