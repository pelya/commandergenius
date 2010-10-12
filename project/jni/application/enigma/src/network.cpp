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

#include "network.hh"
#include "ecl_buffer.hh"

#include "enet/enet.h"

#include <cassert>
#include <cstring>
#include <string>
#include <cstdio>

#include "SDL_types.h"


using namespace std;
using namespace enigma;
using ecl::Buffer;

/* -------------------- Peer_Enet implementation -------------------- */

Peer_Enet::Peer_Enet (ENetHost *host, ENetPeer *peer, int playerno) 
    : m_host (host),
      m_peer (peer), 
      m_playerno (playerno),
      m_connected (true)
{
}


Peer_Enet::~Peer_Enet() 
{
    disconnect(0);
}


void Peer_Enet::disconnect(int timeout = 1000) 
{
    ENetEvent event;
    
    enet_peer_disconnect (m_peer, 0);

    while (enet_host_service (m_host, & event, timeout) > 0) {
        switch (event.type) {
        case ENET_EVENT_TYPE_RECEIVE:
            enet_packet_destroy (event.packet);
            break;

        case ENET_EVENT_TYPE_DISCONNECT:
            printf ("Disconnection succeeded.\n");
            return;
        case ENET_EVENT_TYPE_NONE:
        case ENET_EVENT_TYPE_CONNECT:
            break;
        }
    }
    
    enet_peer_reset (m_peer);
    m_connected = false;
}

bool Peer_Enet::is_connected() const 
{
    return m_connected;
}

bool Peer_Enet::poll_message (Buffer &b, int &player_no) 
{
    if (!m_connected)
        return false;

    ENetEvent event;
    if (enet_host_service (m_host, & event, 0) > 0) {
        switch (event.type) {
        case ENET_EVENT_TYPE_RECEIVE: {
            b.assign (reinterpret_cast<char*> (event.packet->data), 
                      event.packet->dataLength);
            player_no = m_playerno;
            enet_packet_destroy (event.packet);
            return true;
        }
           
        case ENET_EVENT_TYPE_DISCONNECT:
            m_connected = false;
            event.peer -> data = NULL;
            break;

        default:
            break;
        }
    }
    return false;
}

void Peer_Enet::send_message (const Buffer &b, int channel)
{
    ENetPacket *packet = enet_packet_create (b.data(), b.size(), 0);
    enet_peer_send (m_peer, channel, packet);
}

void Peer_Enet::send_reliable (const Buffer &b, int channel)
{
    ENetPacket *packet = enet_packet_create (b.data(), b.size(),
                                             ENET_PACKET_FLAG_RELIABLE);
    enet_peer_send (m_peer, channel, packet);
}




/* -------------------- Peer_Local implementation -------------------- */

bool Peer_Local::poll_message (Buffer &b, int &player_no)
{
    if (!m_queue.empty()) {
        
    }
    return false;
}

void Peer_Local::send_message (const Buffer &b, int channel) 
{
}

void Peer_Local::send_reliable (const Buffer &b, int channel)
{
    send_message (b, channel);
}

