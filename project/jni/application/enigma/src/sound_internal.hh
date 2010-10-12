namespace
{

/* -------------------- SoundEvent -------------------- */

    struct SoundEvent {
        // Variables
        SoundName name;
        bool      has_position;
        ecl::V2   position;
        int       priority;
        double    volume;           // Volume between 0.0 and 1.0
        int       left;
        int       right;

        // Variables used internally by sound engine
        bool      active;
        double    playing_time;

        // Constructor
        SoundEvent ();
    };

/* -------------------- SoundEffect and SoundEffectRepository ---------------- */

/*! This class stores information about how to play sound events
    and provides methods to play them. */

    class SoundEffect {
    public:
        SoundEffect(string name_, string soundset_key_, string filename_,
                       double volume_, bool loop_, bool global_, int priority_,
                       double damp_max_, double damp_inc_, double damp_mult_,
                       double damp_min_, double damp_tick_, string silence_string_)
        : name(name_), soundset_key(soundset_key_), filename(filename_), volume(volume_),
          loop(loop_), global(global_), priority(priority_),
          silence_string(silence_string_) {
            damp.maxi = damp_max_;
            damp.incr = damp_inc_;
            damp.mult = damp_mult_;
            damp.mini = damp_min_;
            damp.tick = damp_tick_;
        }

        SoundEffect()  // standard empty data set, compare sound-defaults.lua
        : name("EMPTY_EVENT"), filename(""), soundset_key(""), volume(1.0),
          loop(false), global(false), priority(1), silence_string("") {
            damp.maxi = 20.0;
            damp.incr =  1.0;
            damp.mult =  1.0;
            damp.mini =  0.5;
            damp.tick =  0.9;
        }

        void setFilename(string filename_) { filename = filename_; }
        bool play(const ecl::V2 &pos = ecl::V2(), double vol = 1.0, bool glob = false);
        DampingData getDampingData() { return damp; }
        string getSoundSetKey() { return soundset_key; }
        string getSilenceString() { return silence_string; }

    private:
        string name;
        string filename;
        string soundset_key;
        string silence_string;
        double volume;
        bool loop;
        bool global;
        int priority;
        DampingData damp;
    };

    class SoundSet {
    public:
        SoundSet(string soundset_key_, int button_position_, OxydLib::OxydVersion oxyd_ver_)
        : soundset_key(soundset_key_), is_oxyd(true), oxyd_ver(oxyd_ver_),
          button_position(button_position_) {}

        SoundSet(string soundset_key_, int button_position_)
        : soundset_key(soundset_key_), is_oxyd(false),
          oxyd_ver(OxydLib::OxydVersion_Invalid), button_position(button_position_) {}

        SoundSet()
        : soundset_key(""), is_oxyd(false),
          oxyd_ver(OxydLib::OxydVersion_Invalid), button_position(-1) {}

        bool activate();
        OxydLib::OxydVersion getOxydVersion() { return oxyd_ver; }
        bool isOxyd() { return is_oxyd; }
        string getSoundSetKey() { return soundset_key; }
        int getButtonPosition() { return button_position; }
        void setButtonPosition(int pos) { button_position = pos; }

    private:
        string soundset_key;
        bool is_oxyd;
        OxydLib::OxydVersion oxyd_ver;
        int button_position;
    };

    typedef map<string, SoundEffect> SoundEffectRepository;
    typedef map<string, SoundSet> SoundSetRepository;
        
/* -------------------- SoundEngine -------------------- */

    class SoundEngine {
    public:
        virtual ~SoundEngine() {}
        
        //! Returns true if successful.
        virtual bool init() = 0;
        virtual void shutdown() = 0;
        virtual bool is_initialized() const = 0;

        virtual void set_sound_volume (double soundvol) = 0;
        virtual void set_music_volume (double musicvol) = 0;

        // ---------- Music ----------

        virtual bool play_music (const std::string &filename) = 0;
        virtual void stop_music() = 0;
        virtual void fadeout_music() = 0;

        // ---------- Sound effects ----------

        virtual void clear_cache() = 0;
        virtual void define_sound (const SoundName &, const std::string &filename)=0;
        virtual void define_sound (const SoundName &, const SoundData &)=0;
        virtual bool play_sound (const SoundEvent &s) = 0;
        virtual void set_listenerpos (ecl::V2 pos) = 0;
        virtual void tick (double dtime) {}

        // ---------- Sound effect repository and sound sets ----------

        void setActiveSoundSetKey(string soundset_key) {active_sound_set_key=soundset_key;}
        string getActiveSoundSetKey() { return active_sound_set_key; }
        void setDefaultSoundSet(string soundset_name) {default_sound_set=soundset_name;}
        string getDefaultSoundSet() { return default_sound_set; }
        string effectKey(string effect_name, string soundset_name = "");
        DampingData getDampingData(string effect_name) {
            return sound_effects[effectKey(effect_name)].getDampingData(); }
        void defineSoundEffect(string soundset_key, string name, SoundEffect se) {
            sound_effects[effectKey(name, soundset_key)] = se;
        }
        bool emitSoundEvent (const std::string &eventname, const ecl::V2 &pos = ecl::V2 (), 
                             double volume = 1.0, bool force_global = false);
        void writeSilenceString (const std::string &eventname);
        void initSoundSets();
        bool defineSoundSet(string soundset_name, string soundset_key, int button_position);
        bool defineSoundSetOxyd(string soundset_name, string soundset_key,
                                OxydLib::OxydVersion oxyd_ver, int button_position);
        string getOxydSoundSet(OxydLib::OxydVersion oxyd_ver);
        int convertToOldSoundSetNumber(string soundset_name);
        string convertFromOldSoundSetNumber(int soundset_number);
        void setActiveSoundSet(string soundset_name);
        void setSoundSetCount(int count) { sound_set_count = count; }
        int getSoundSetCount() { return sound_set_count; }
        int getButtonPosition(string soundset_name) {
            return sound_sets[soundset_name].getButtonPosition();
        }
        string getSoundSetByPosition(int button_position);

    private:
        SoundSetRepository       sound_sets;
        SoundEffectRepository    sound_effects;
        string                   active_sound_set_key;
        string                   default_sound_set;
        int                      sound_set_count;
    };

    class SoundEngine_Null : public SoundEngine {
    public:

        // SoundEngine interface
        bool init() { return true; }
        void shutdown() {}
        void clear_cache() {}
        bool is_initialized() const { return true; }
        void set_sound_volume(double /*soundvol*/) {}
        void set_music_volume(double /*musicvol*/) {}
        bool play_music (const std::string &/*filename*/) { return false; }
        void stop_music() {}
        void fadeout_music() {}
        bool play_sound (const SoundEvent &) { return false;}
        void define_sound (const SoundName &, const std::string &/*filename*/) {}
        void define_sound (const SoundName &, const SoundData &) {}
        void set_listenerpos (ecl::V2 pos) {}
    };

    class SoundEngine_SDL : public SoundEngine {
    public:
        SoundEngine_SDL();
        ~SoundEngine_SDL();

        // ---------- SoundEngine interface ----------
        bool init();        
        void shutdown();

        bool is_initialized() const { return m_initialized; }
        void set_sound_volume(double soundvol);
        void set_music_volume(double musicvol);

        bool play_music (const std::string &filename);
        void stop_music();
        void fadeout_music();

        bool play_sound(const SoundEvent &s);
        void clear_cache();
        void define_sound (const SoundName &, const std::string &filename);
        void define_sound (const SoundName &, const SoundData &);


        void set_listenerpos (ecl::V2 pos) { m_listenerpos = pos; }
        void tick (double dtime);
    private:
        // ---------- Private methods ----------
        Mix_Chunk *cache_sound(const std::string &name);

        void update_channel (int channel);
        int already_playing (const SoundEvent &s);


        static void channel_finished (int channel);


        // ---------- Variables ----------
        bool       m_initialized;
        int        m_soundvolume;
        int        m_musicvolume;
        Mix_Music *m_current_music;
        int        m_freq;
        Uint16     m_format;
        int        m_channels;
        ecl::Dict<Mix_Chunk*> wav_cache;
        vector<SoundEvent> m_channelinfo;
        ecl::V2      m_listenerpos;
        SDL_mutex  *m_mutex;
        static SoundEngine_SDL *m_instance;
    };
}
