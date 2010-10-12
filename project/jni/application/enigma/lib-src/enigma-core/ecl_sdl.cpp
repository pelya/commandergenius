#include "ecl_sdl.hh"

using namespace sdl;

// Dispatch an event to the suitable virtual function. Returns
// true if event was handled.
bool
EventHandler::dispatch_event(SDL_Event &e)
{
    bool handled = false;
    
    switch (e.type) {
    case SDL_KEYDOWN:
        handled=on_keydown(e); break;
    case SDL_KEYUP:
        handled=on_keyup(e); break;
    case SDL_MOUSEMOTION: 
        handled=on_mousemotion(e); break;
    case SDL_MOUSEBUTTONDOWN:
    case SDL_MOUSEBUTTONUP:
        handled=on_mousebutton(e);
        break;
    case SDL_ACTIVEEVENT:
        // TODO
        break;
    case SDL_QUIT:
        handled=on_quit(e); break;
    }
    return handled || on_event(e);
}

void 
sdl::FlushEvents()
{
    SDL_Event e;
    while (SDL_PollEvent(&e))
        ;
}


