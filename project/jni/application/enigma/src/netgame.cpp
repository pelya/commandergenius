/*
 * Copyright (C) 2005 Daniel Heck
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
#include "client.hh"
#include "main.hh"
#include "netgame.hh"
#include "network.hh"
#include "options.hh"
#include "server.hh"

#include "enet/enet.h"

#include "SDL.h"
#include <string>

using namespace enigma;

#include "client_internal.hh"
#include "server_internal.hh"


//======================================================================
// SERVER
//======================================================================

void handle_client_packet (Buffer &b, int player_no)
{
    using namespace enigma_server;
    Uint8 code; 
    while (b >> code) {
        switch (code) {
        case SVMSG_NOOP:
            break;              // no nothing

// not yet used -- rewrite to index/proxy usage
//        case SVMSG_LOADLEVEL: {
//            Uint16 levelno;
//            string levelpack;
//            if (b >> levelno >> levelpack) {
//                printf ("SV: Loading level %d from levelpack %s\n", int(levelno), 
//                        levelpack.c_str());
//                server::Msg_SetLevelPack (levelpack);
//                server::Msg_LoadLevel (levelno);
//            }
//
//            break;
//        }

        case SVMSG_MOUSEFORCE: {
            printf ("mouse force\n");
            float dx, dy;
            if (b >> dx >> dy) {
                printf ("-- yei!\n");
                server::Msg_MouseForce (ecl::V2(dx, dy));
            }
            break;
        }

        case SVMSG_ACTIVATEITEM: {
            server::Msg_ActivateItem ();
            break;
        }

        default:
            enigma::Log << "SV: received undefined packet: " << int(code) << "\n";
        }
    }
}

namespace
{
    Uint32      last_tick_time;
}


void server_loop (Peer *m_peer)
{
    printf ("SV: Entered server loop\n");
    server::InitNewGame();
    Buffer buf;
    buf << Cl_LevelLoaded ();
    m_peer->send_reliable (buf, 1);

    double dtime = 0;
    while (!client::AbortGameP() && m_peer->is_connected()) {
        last_tick_time=SDL_GetTicks();


        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type && e.key.keysym.sym == SDLK_ESCAPE)
                goto done;
        }

        try {
            client::Tick (dtime);
            server::Tick (dtime);
        }
        catch (XLevelRuntime& err) {        
            client::Msg_Error (string("Server Error: level runtime error:\n")
                               + err.what());
            server::Msg_Panic(true);
        }

        Buffer buf;
        int player_no;
        while (m_peer->poll_message (buf, player_no)) {
            printf ("SV: Received message from client %d\n", player_no);
            handle_client_packet (buf, player_no);
        }


        int sleeptime = 10 - (SDL_GetTicks()-last_tick_time);
        if (sleeptime >= 3)      // only sleep if relatively idle
            SDL_Delay(sleeptime);
        dtime = (SDL_GetTicks()-last_tick_time)/1000.0;
        if (fabs(1-dtime/0.01) < 0.2) {
            // less than 20% deviation from desired frame time?
            dtime = 0.01;
        }

	if (dtime > 500.0) /* Time has done something strange, perhaps
			      run backwards */
            dtime = 0.0;

    }

  done:
    return;
}

void netgame::Start ()
{

    // ---------- Create network host ----------
    ENetHost *network_host;
    ENetAddress network_address;

    network_address.host = ENET_HOST_ANY;
    network_address.port = 12345;

    network_host = enet_host_create (&network_address, 1, 0, 0);
    if (network_host == NULL) {
        fprintf (stderr, 
                 "SV: An error occurred while trying to create an ENet server host.\n");
        return;
    }

    // ---------- Wait for client(s) ----------
    ENetEvent event;
    Peer *m_peer = 0;
    printf ("SV: Waiting for client...\n");

    while (!m_peer) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type && e.key.keysym.sym == SDLK_ESCAPE)
                return;
        }
        
        while (enet_host_service (network_host, & event, 0) > 0) {
            if (event.type == ENET_EVENT_TYPE_CONNECT) {
                printf ("SV: Connected to client\n");
                m_peer = new Peer_Enet (network_host, event.peer, 2);
            }
        }
        SDL_Delay (10);
    }

    server_loop (m_peer);

    m_peer->disconnect();
    delete m_peer;
}

//======================================================================
// CLIENT
//======================================================================

namespace
{
    struct MovementCommand {
        float time_stamp;
        float force_x;
        float force_y;
    };

    typedef std::list<MovementCommand> MovementList;

    MovementList movement_list;


    Peer *server_peer;
}

void handle_server_packet (Buffer &buf)
{
    if (buf.size() > 0) {
        Uint8 code;
        buf >> code;
        switch (code) {
        case CLMSG_LEVEL_LOADED:
            printf ("cl_level_loaded\n");
            break;

        }
    }
            
    Buffer obuf;
    obuf << Uint8(enigma_server::SVMSG_LOADLEVEL);
    obuf << Uint16(84);
    obuf << string("Enigma");
    server_peer->send_reliable (obuf, 1);
    printf ("CL: sending message %u\n", obuf.size());
}

void netgame::Join (std::string hostname, int port)
{
    printf ("CL: trying to join remote game\n");



    // ---------- Create network host ----------
    ENetHost *m_network_host;
    m_network_host = enet_host_create (NULL,
                                       1 /* only allow 1 outgoing connection */,
                                       57600 / 8 /* 56K modem with 56 Kbps downstream bandwidth */,
                                       14400 / 8 /* 56K modem with 14 Kbps upstream bandwidth */);

    if (m_network_host == NULL) {
        fprintf (stderr, "CL: An error occurred while trying to create an ENet client host.\n");
        return;
    }


    // ---------- Connect to server ----------

    ENetAddress sv_address;
    ENetPeer *m_server;

    /* Connect to some.server.net:1234. */
    enet_address_set_host (&sv_address, hostname.c_str());
    sv_address.port = port;

    /* Initiate the connection, allocating the two channels 0 and 1. */
    int numchannels = 2;
    m_server = enet_host_connect (m_network_host, &sv_address, numchannels);
    
    if (m_server == NULL) {
        fprintf (stderr, 
                 "CL: No available peers for initiating an ENet connection.\n");
        return;
    }
    

    server_peer = 0;
    ENetEvent event;
    if (enet_host_service (m_network_host, &event, 5000) > 0 &&
        event.type == ENET_EVENT_TYPE_CONNECT)
    {
        fprintf (stderr, "CL: Connection to some.server.net:12345 succeeded.\n");
        if (m_server != event.peer)
            printf ("CL: peers differ!?!\n");
        server_peer = new Peer_Enet (m_network_host, m_server, 0);
    }
    else
        return;


    while (server_peer->is_connected()) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type && e.key.keysym.sym == SDLK_ESCAPE)
                goto done;
            else if (e.type ==SDL_MOUSEMOTION) {
                Buffer buf;
                float mouseforce = options::GetDouble("MouseSpeed");
                buf << Uint8 (enigma_server::SVMSG_MOUSEFORCE)
                    << float (e.motion.xrel * mouseforce)
                    << float (e.motion.yrel * mouseforce);
                server_peer->send_reliable (buf,1);
            }
        }

        Buffer buf;
        int peerno;
        while (server_peer->poll_message (buf, peerno)) {
            handle_server_packet (buf);
        }
        SDL_Delay (10);
    }

  done:
    server_peer->disconnect();
    delete server_peer;
    server_peer = 0;
    return;

}
