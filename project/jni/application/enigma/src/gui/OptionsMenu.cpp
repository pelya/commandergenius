/*
 * Copyright (C) 2002,2003,2004,2005,2006 Daniel Heck
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
 
#include "gui/OptionsMenu.hh"
#include "ecl.hh"
#include "enigma.hh"
#include "lev/ScoreManager.hh"
#include "LocalToXML.hh"
#include "main.hh"
#include "nls.hh"
#include "options.hh"
#include "oxyd.hh"
#include "sound.hh"
#include "Utf8ToXML.hh"
#include "video.hh"
#include "XMLtoLocal.hh"
#include "XMLtoUtf8.hh"

#include <cassert>

using namespace ecl;
using namespace std;

namespace enigma { namespace gui {

/* -------------------- Buttons for Options -------------------- */

    class MouseSpeedButton : public ValueButton {
        int get_value() const     { 
            return ecl::round_nearest<int>(options::GetMouseSpeed());
        }
        void set_value(int value) { 
            options::SetMouseSpeed (value);
        }

        string get_text(int value) const  {
            return strf("%d", value);
        }
    public:
        MouseSpeedButton()
        : ValueButton(options::MIN_MouseSpeed, options::MAX_MouseSpeed)
        { init(); }
    };

    class SoundVolumeButton : public ValueButton {
        int get_value() const     { 
            return round_nearest<int>(options::GetDouble("SoundVolume")*10.0); 
        }
        void set_value(int value) {
            options::SetOption("SoundVolume", value/10.0);
            options::UpdateVolume();
        }

        string get_text(int value) const {
            if (value == 0) {
                return _("muted");
            }
            else {
                return strf("%d", value);
            }
        }
    public:
        SoundVolumeButton() : ValueButton(0, 10) { init(); }
    };

    class MusicVolumeButton : public ValueButton {
        int get_value() const { 
            return round_nearest<int> (options::GetDouble("MusicVolume")*10.0); 
        }
        void set_value(int value) {
            options::SetOption("MusicVolume", value/10.0);
            options::UpdateVolume();
        }

        string get_text(int value) const {
            if (value == 0)
                return _("muted");
            else
                return strf("%d", value);
        }
    public:
        MusicVolumeButton() : ValueButton(0, 10) { init(); }
    };

    class InGameMusicButton : public BoolOptionButton {
        void on_action(Widget *) {
            if (toggle())
                sound::PlayMusic (options::GetString("LevelMusicFile"));
            else
                sound::StopMusic (options::GetString("LevelMusicFile"));
        }
    public:
        InGameMusicButton() :
            BoolOptionButton("InGameMusic", N_("Music in game"), N_("No music in game"), this)
        { }
    };

    struct SkipSolvedButton : public BoolOptionButton {
        SkipSolvedButton() : BoolOptionButton("SkipSolvedLevels", N_("Yes"), N_("No"), this) {}
    };

    struct TimeHuntButton : public BoolOptionButton {
        TimeHuntButton() : BoolOptionButton("TimeHunting", N_("Yes"), N_("No"), this) {}
    };

    struct RatingsUpdateButton : public BoolOptionButton {
        RatingsUpdateButton() : BoolOptionButton("RatingsAutoUpdate", N_("Auto"), N_("Never"), this) {}
    };

    class VideoModeButton : public TextButton {

        video::VideoModes get_mode() const {
            int mode = Clamp(options::GetInt("VideoMode"), 0, int(video::VM_COUNT));
            return static_cast<video::VideoModes>(mode);
        }
        string get_text() const {
            return GetInfo(get_mode())->name;
        }
        void on_action(Widget *) {
            int mode = get_mode();

            // cycle at most once through all available video modes
            do {
                mode += 1;
                if (mode >= video::VM_COUNT) 
                    mode = 0;

                const video::VMInfo *vminfo = GetInfo (static_cast<video::VideoModes>(mode));
                if (vminfo->available) {
                    options::SetOption("VideoMode", mode);
                    invalidate();
                    break;
                }
            } while (mode != get_mode());
        }
    public:
        VideoModeButton() : TextButton(this) { }
    };


    /* -------------------- SoundSetButton -------------------- */
    
    SoundSetButton::SoundSetButton() : ValueButton(0, 1) {
        int numAvail = sound::GetOptionSoundSetCount();
        setMaxValue(numAvail - 1);
        init();
    }

    int SoundSetButton::get_value() const {
        return sound::GetOptionSoundSet();
    }

    void SoundSetButton::set_value(int value) {
        sound::SetOptionSoundSet(value);
    }

    string SoundSetButton::get_text(int value) const {
        return _(sound::GetOptionSoundSetText(value).c_str());
    }

    
    /* -------------------- StereoButton -------------------- */
    
    StereoButton::StereoButton() : ValueButton(-1, 1)
    {
        init();
    }
    
    int StereoButton::get_value() const 
    {
        double separation = options::GetDouble("StereoSeparation");
        if (separation == 0)
            return 0;
        else
            return (separation > 0) ? 1 : -1; 
    }
    void StereoButton::set_value(int value)  
    {
        if (value == 0) 
            options::SetOption("StereoSeparation", 0.0);
        else if (value > 0) 
            options::SetOption("StereoSeparation", 10.0);
        else 
            options::SetOption("StereoSeparation", -10.0);
    }
    
    string StereoButton::get_text(int value) const 
    {
        switch (value) {
        case -1: return _("reversed");
        case 0: return _("mono");
        case 1: return _("normal");
        }
        assert(0);
        return string();
    }
    
    
    /* -------------------- FullscreenButton -------------------- */
    
    FullscreenButton::FullscreenButton()
        : BoolOptionButton("FullScreen", N_("Yes"), N_("No"), this)
    {
    }
    
    /* -------------------- LanguageButton -------------------- */
    
    struct Language {
        const char *name;
        const char *localename;
    };
    
    Language languages[] = {
        { "default",                "" },
        { "Deutsch",                "de_DE" },
        { "English",                "en_EN" },
        { "Español",         "es_ES" },
        { "Français",        "fr_FR" },
        { "Italiano",               "it_IT" },
        { "Nederlands",             "nl_NL" },
        { "Svenska",             "sv_SE" },
        { "Русский",             "ru_RU" },
        { "Magyar",             "hu_HU" },
        { "Português",             "pt_BR" },
        { "Suomi",             "fi_FI" },
    };
    
    int LanguageButton::get_value() const
    {
        string localename; //  = ecl::DefaultMessageLocale ();
        options::GetOption ("Language", localename);
    
        int lang = 0;                  // unknown language
        for (size_t i=0; i<NUMENTRIES(languages); ++i) {
            if (localename == languages[i].localename)
                lang = int(i);
        }
        return lang;
    }
    
    void LanguageButton::set_value(int value)
    {
        options::SetOption ("Language", languages[value].localename);
        
        if ( not inInit) {
            // change language only on user action
            app.setLanguage(languages[value].localename);
            myListener->on_action(this);
        }
    }
    
    string LanguageButton::get_text(int value) const
    {
        if (value == -1)
            return _("unknown");
        else
            return languages[value].name;
    }
    
    LanguageButton::LanguageButton (ActionListener *al)
    : ValueButton(0, NUMENTRIES(languages)-1), myListener(al)
    {
        inInit = true;
        init();
        inInit = false;
    }
    
    /* -------------------- GammaButton -------------------- */
    
    
    GammaButton::GammaButton()
    : ValueButton(1, 10)
    {
        init();
    }
    
    void GammaButton::set_value(int value) 
    { 
        double gamma = double(value) / 5.0;
        options::SetOption ("Gamma", gamma);
        video::UpdateGamma();
    }
    
    int GammaButton::get_value() const
    { 
        double gamma = options::GetDouble ("Gamma");
        int value = round_down<int>(gamma * 5.0);
        return value;
    }
    
    string GammaButton::get_text(int value) const
    {
        return ecl::strf ("%d", value-5);
    }
    
    
    
    /* -------------------- Options Menu -------------------- */
    
    OptionsMenu::OptionsMenu(ecl::Surface *background_)
    : back(new StaticTextButton(N_("Back"), this)),
      fullscreen(new FullscreenButton),
      m_restartinfo (new Label("")),
      background(background_),
      previous_caption(video::GetCaption())
    {
        const int spacing     = 5;
        const int big_spacing = 60;
        const int label_width = 180;
        const int but_width   = 100;
        const int but_height  = 30;
        const video::VMInfo *vminfo = video::GetInfo();
        int hmargin = vminfo->width < 660 ? 10 : (vminfo->width < 900 ? 20 : 80);
        int midspacing = vminfo->width - 2*hmargin - 2*but_width - 2*label_width;
    
        BuildVList leftlabels (this, Rect(-label_width, 0, label_width, but_height), spacing);
        BuildVList left (this, Rect(0, 0, but_width, but_height), spacing);
        BuildVList rightlabels (this, Rect(but_width+midspacing, 0, label_width, but_height), spacing);
        BuildVList right(this, Rect(but_width+midspacing+label_width, 0, but_width, but_height), spacing);
        leftlabels.add (new Label(N_("Language: "), HALIGN_RIGHT));
        leftlabels.add (new Label(N_("Fullscreen: "), HALIGN_RIGHT));
        leftlabels.add (new Label(N_("Video mode: "), HALIGN_RIGHT));
        leftlabels.add (new Label(N_("Gamma correction: "), HALIGN_RIGHT));
        leftlabels.add (new Label(N_("Mouse speed: "), HALIGN_RIGHT));
    
        language = new LanguageButton(this);
        left.add (language);
        left.add (fullscreen);
        left.add (new VideoModeButton);
        left.add (new GammaButton);
        left.add (new MouseSpeedButton);
    
        rightlabels.add (new Label(N_("Sound volume: "), HALIGN_RIGHT));
        rightlabels.add (new Label(N_("Sound set: "), HALIGN_RIGHT));
        rightlabels.add (new Label(N_("Music volume: "), HALIGN_RIGHT));
        rightlabels.add (new Label(N_("Stereo: "), HALIGN_RIGHT));
        rightlabels.add (new Label(N_("Ratings update: "), HALIGN_RIGHT));
    
        right.add (new SoundVolumeButton);
        right.add (new SoundSetButton);
        right.add (new MusicVolumeButton);
//        right.add (new InGameMusicButton);Über
        right.add (new StereoButton);
        right.add (new RatingsUpdateButton);
        
        Rect l = left.pos();
        Rect r = right.pos();

        BuildVList bottomlabels (this, Rect(-label_width, Max(l.y, r.y), label_width, but_height), spacing);
        BuildVList bottom (this, Rect(0, Max(l.y, r.y), vminfo->width - 2*hmargin - label_width, but_height), spacing);
        bottomlabels.add (new Label(N_("User name: "), HALIGN_RIGHT));
        bottomlabels.add (new Label(N_("User path: "), HALIGN_RIGHT));
        bottomlabels.add (new Label(N_("User image path: "), HALIGN_RIGHT));
        userNameTF = new TextField(app.state->getString("UserName"));
        userNameTF->setMaxChars(20);
        userNameTF->setInvalidChars("+");
        bottom.add (userNameTF);
        userPathTF = new TextField(XMLtoUtf8(LocalToXML(app.userPath.c_str()).x_str()).c_str());
        bottom.add (userPathTF);
        userImagePathTF = new TextField(XMLtoUtf8(LocalToXML(app.userImagePath.c_str()).x_str()).c_str());
        bottom.add (userImagePathTF);

//            add (m_restartinfo, Rect (l.x, l.y + 15, 400, 20));
//            m_restartinfo->set_alignment (HALIGN_LEFT);
//            update_info();
    
        Rect b = bottom.pos();
        l.x = (l.x+r.x)/2;
        l.y = b.y+big_spacing;
    
        add(back, l);
    }
    
    OptionsMenu::~OptionsMenu() {
        video::SetCaption(previous_caption.c_str());
    }
    
//    void OptionsMenu::update_info() 
//    {
//        if (options::MustRestart)
//            m_restartinfo->set_text (
//                N_("Please restart Enigma to activate your changes!"));
//        else
//            m_restartinfo->set_text ("");
//    }
    
    void OptionsMenu::quit() {
        std::string tfUserPathLocal = XMLtoLocal(Utf8ToXML(userPathTF->getText().c_str()).x_str()).c_str();
        std::string tfUserImageLocal = XMLtoLocal(Utf8ToXML(userImagePathTF->getText().c_str()).x_str()).c_str();
        if ((app.state->getString("UserName") != userNameTF->getText())
                || (app.userPath != tfUserPathLocal ) || (app.userImagePath != tfUserImageLocal)) {
            // ensure that enigma.score is saved with new Username or to new location
            lev::ScoreManager::instance()->markModified();
        }
        // strip off leading and trailing whitespace from user name
        std::string userName = userNameTF->getText();
        std::string::size_type firstChar = userName.find_first_not_of(" ");
        std::string::size_type lastChar = userName.find_last_not_of(" ");
        if (firstChar != std::string::npos)
            app.state->setProperty("UserName", userName.substr(firstChar, lastChar - firstChar + 1));
        else
            app.state->setProperty("UserName", std::string(""));
        app.setUserPath(tfUserPathLocal.c_str());
        app.setUserImagePath(tfUserImageLocal.c_str());
        Menu::quit();
    }

    bool OptionsMenu::on_event (const SDL_Event &e)
    {
        bool handled=false;
        if (e.type == SDL_MOUSEBUTTONDOWN
            && e.button.button == SDL_BUTTON_RIGHT)
        {
            quit();
            handled = true;
        }
        else if (e.type == SDL_KEYUP) {
            if ((e.key.keysym.sym==SDLK_RETURN) &&
                (e.key.keysym.mod & KMOD_ALT))
            {
                // update state of FullscreenButton :
                fullscreen->invalidate();
                handled = true;
            }
        }
        return handled;
    }
    
    void OptionsMenu::on_action(Widget *w)
    {
        if (w == back)
            quit();
        else if (w == language)
            // language changed - retranslate and redraw everything
            invalidate_all();
    }
    
    void OptionsMenu::tick (double)
    {
//        update_info();
    }
    
    void OptionsMenu::draw_background(ecl::GC &gc)
    {
        video::SetCaption(("Enigma - Options Menu"));
    //     blit(gc, 0,0, enigma::GetImage("menu_bg"));
        blit(gc, 0,0, background);
    }
    
/* -------------------- Functions -------------------- */

    void ShowOptionsMenu(Surface *background) {
        if (background == 0)
            background = enigma::GetImage("menu_bg", ".jpg");
        OptionsMenu m(background);
        m.center();
        m.manage();
    }

}} // namespace enigma::gui

