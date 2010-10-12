#include "gui/Menu.hh"
#include "video.hh"
#include "ecl_buffer.hh"

namespace
{
    using ecl::Buffer;
    using std::string;

/* -------------------- Server -> Client messages -------------------- */

    enum ClientCommand {
        CLMSG_NOOP,
        CLMSG_NEW_WORLD,
        CLMSG_LEVEL_LOADED,
        CLMSG_CHANGE_FIELD,
        CLMSG_ADD_ACTOR,
        CLMSG_MOVE_ACTOR,
        CLMSG_FOCUS_ACTOR,
        CLMSG_CHANGE_LINE,
        CLMSG_PLAY_SOUND,
        CLMSG_SHOW_TEXT,
        CLMSG_ERROR                // error occurred
    };

    struct Message {
        Message (ClientCommand type_ = CLMSG_NOOP) : type (type_) {
        }

        ClientCommand type;
    };

    struct Cl_NewWorld {
        std::string levelname;
        int width;
        int height;
    };

    struct Cl_LevelLoaded : public Message {
        Cl_LevelLoaded() : Message (CLMSG_LEVEL_LOADED) {
        }
    };
    Buffer &operator << (Buffer &b, const Cl_LevelLoaded &m) {
        return b << Uint8 (CLMSG_LEVEL_LOADED);
    }

    struct Cl_ChangeField {


    };

    struct Cl_AddActor {

    };

    struct Cl_MoveActor {

    };

    struct Cl_FocusActor {
    
    };

    struct Cl_ShowText : public Message {
        Cl_ShowText() : Message(CLMSG_SHOW_TEXT) {
        }

        string text;
        float duration;
        bool scrolling;
        bool interruptible;
    };

    struct Cl_AddEffect {
    
        float x, y;
    };

    struct Cl_PlaySound {
        string soundname;
        float x, y;
        int priority;
    };

/* -------------------- Client class -------------------- */

    enum ClientState {
        cls_idle,
        cls_preparing_game,     // level loaded, currently updating the screen
        cls_game,
        cls_finished,            // level finished, waiting for next one
        cls_gamehelp,
        cls_gamemenu,
        cls_abort,
        cls_error
    };


    class Client {
    public:
        Client();
        ~Client();

        void tick (double dtime);
        void stop() { m_state = cls_idle; }
        bool network_start();
        void network_stop();


        void handle_message(Message *msg);

        void level_loaded(bool isRestart);
        void level_finished();

        void error (const std::string &text);

        void abort() { m_state = cls_abort; }
        bool abort_p() const { return m_state == cls_abort; }

        void mark_cheater() { m_cheater = true; }
        void easy_going() { m_hunt_against_time = false; }
    private:

        std::string init_hunted_time();


        /* ---------- Private methods ---------- */

        void show_menu();
        void show_help();

        // Screen update (state dependant)
        void draw_screen();

        // Event handling
        void handle_events();
        void on_keydown(SDL_Event &e);
        void on_mousebutton(SDL_Event &e);
        void update_mouse_button_state();

        // Inventory & command line
        void rotate_inventory(int direction);
        
        void process_userinput();
        void user_input_append (char c);
        void user_input_backspace ();
        void user_input_previous ();
        void user_input_next ();


        // Variables
        ClientState m_state;
        string      m_levelname;
        double      m_timeaccu;

        double      m_total_game_time;
        int         m_hunt_against_time;
        bool        m_cheater;

        string      m_user_input;
        string      m_error_message;

        std::auto_ptr<video::TransitionEffect> m_effect;
        ENetHost   *m_network_host;
        ENetPeer   *m_server;

    private:
        Client (const Client&);
        Client &operator = (const Client &);
    };
}

