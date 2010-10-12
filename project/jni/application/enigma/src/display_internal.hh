#ifndef DISPLAY_INTERNAL_HH
#define DISPLAY_INTERNAL_HH

#include "display.hh"

namespace display
{
    using ecl::V2;

    class DisplayLayer;
    class StatusBarImpl;
    class Model;

    typedef ecl::Rect          ScreenArea;
    typedef ecl::Rect          WorldArea;
    typedef std::list<Model*> ModelList;


    class Window {
    public:
        Window() {}
        Window (const ScreenArea &area) : m_area(area)
        {}

        const ScreenArea &get_area() const { return m_area; }
    private:
        ScreenArea m_area;
    };


    class TextDisplay {
    public:
        TextDisplay(ecl::Font &f);

        void set_text(const std::string &t, bool scrolling, double duration = -1);

        void tick(double dtime);
        bool has_changed() const { return changedp; }
        bool has_finished() const { return finishedp; }

        void draw(ecl::GC &gc, const ecl::Rect &r);
    private:
        ecl::Rect                area;
        std::string             text;
        bool                    changedp, finishedp;
        bool                    pingpong;
        bool                    showscroll;
        double                  xoff;
        double                  scrollspeed; // pixels per second
        std::auto_ptr<ecl::Surface>  textsurface;
        ecl::Font               &font;
        double                  time, maxtime;
    };

    class StatusBarImpl : public StatusBar, public Window {
    public:
        StatusBarImpl (const ScreenArea &area);
        ~StatusBarImpl();

        bool has_changed() const { return m_changedp; }
        void redraw (ecl::GC &gc, const ScreenArea &r);
        void tick (double dtime);
        void new_world();

        // StatusBar interface.
        void set_time (double time);
        void set_inventory (const std::vector<std::string> &modelnames);
        void show_text (const std::string &str, bool scrolling, double duration);
        void hide_text();

        void show_move_counter (bool active);
        void show_odometer (bool active);

        void set_speed (double speed);
        void set_travelled_distance (double distance);
        void set_counter (int new_counter);

    private:
        ScreenArea     m_itemarea;
        std::vector<Model*> m_models;
        bool           m_changedp;
        TextDisplay    m_textview;

        double m_leveltime;
        bool   m_showtime_p;
        int    m_counter;
        bool   m_showcounter_p;
        bool   m_showodometer_p;
        bool   m_interruptible; // Current text message may be interrupted
        bool m_text_active;
    };


}

#endif
