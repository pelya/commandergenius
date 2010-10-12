/*
 * Copyright (C) 2002,2004,2005 Daniel Heck
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

#ifndef NETWORK_HH_INCLUDED
#define NETWORK_HH_INCLUDED

#include "ecl_fwd.hh"

#include "SDL_types.h"
#include "enet/enet.h"
#include <list>
#include <iosfwd>

namespace enigma
{
    class Peer {
    public:
        virtual ~Peer() {}
        virtual void send_message (const ecl::Buffer &b, int channel) = 0;
        virtual void send_reliable (const ecl::Buffer &b, int channel) = 0;
        virtual bool poll_message (ecl::Buffer &b, int &player_no) = 0;
        virtual void disconnect(int timeout=1000) = 0;
        virtual bool is_connected() const = 0;
    };

    class Peer_Enet : public Peer {
    public:
        Peer_Enet (ENetHost *host, ENetPeer *peer, int playerno);
        ~Peer_Enet();

    private:
        // Peer interface
        virtual void send_message (const ecl::Buffer &b, int channel);
        virtual void send_reliable (const ecl::Buffer &b, int channel);
        virtual bool poll_message (ecl::Buffer &b, int &player_no); 
        virtual void disconnect(int timeout);
        virtual bool is_connected() const;

    private:
        // Variables
        ENetHost *m_host;
        ENetPeer *m_peer;
        int m_playerno;
        bool m_connected;
    };

    class Peer_Local : public Peer {
    public:
        virtual void send_message (const ecl::Buffer &b, int channel) ;
        virtual void send_reliable (const ecl::Buffer &b, int channel);
        virtual bool poll_message (ecl::Buffer &b, int &player_no);
    private:
        // Variables
        std::list<ecl::Buffer> m_queue;
    };


}

#endif
