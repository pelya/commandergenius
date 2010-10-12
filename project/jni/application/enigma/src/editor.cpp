/*
 * Copyright (C) 2002,2003,2004 Daniel Heck
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
#include "editor.hh"

#include "ecl_sdl.hh"
#include "ecl_video.hh" // set_color, line
#include "ecl_util.hh"  // set_flags

#include "main.hh"
#include "world.hh"
#include "lua.hh"
#include "video.hh"
#include "gui/widgets.hh"
#include "display_internal.hh" // WorldArea

#include <iostream>
#include <cassert>

#include "display_internal.hh"
#include "d_engine.hh"

#ifndef CXXLUA
extern "C" {
#include "lualib.h"
#include "tolua++.h"
}
#else
#include "lualib.h"
#include "tolua++.h"
#endif 

#include "lua-editor.hh"

using namespace std;
using namespace enigma;
using namespace editor;
using namespace ecl;

using display::ScreenArea;
using display::DisplayEngine;
using display::Model;
//using world::ObjectTraits;

using world::ItemID;

#include "editor_impl.hh"


/* -------------------- Editor state -------------------- */

EditorState::EditorState()
{
}


EditorState::~EditorState()
{
}


void EditorState::save( std::ostream & /*os*/ )
{
}

void EditorState::load( std::istream & /*is*/ )
{
}



/* -------------------- Editor display engine -------------------- */

EditorDisplay::EditorDisplay()
{
	DisplayEngine *engine = get_engine();
	engine->set_screen_area( video::GetInfo()->gamearea );
	engine->add_layer( new DL_Editor );
}

void
EditorDisplay::redraw()
{
	display::CommonDisplay::redraw();
}


/* -------------------- Editor tools -------------------- */

void
GridObjectTool::advance_in_group( int offset )
{
	int newidx = m_objectidx + offset;
	if( newidx >= 0 && newidx < (int)m_groups[ m_groupidx ].entries.size() ) {
		m_objectidx = newidx;
	}
}

void
GridObjectTool::advance_group( int offset )
{
	int newidx = m_groupidx + offset;
	if( newidx >= 0 && newidx < (int)m_groups.size() ) {
		m_groupidx = newidx;
		m_objectidx = 0;
	}
}

void
GridObjectTool::on_mousebutton( const SDL_Event &e, V2 worldpos )
{
	Editor *ed = Editor::get_instance();
	string obj = current_object();

	round_coordinates( &worldpos );
	int x = (int)worldpos[ 0 ];
	int y = (int)worldpos[ 1 ];

	if( e.button.type == SDL_MOUSEBUTTONDOWN ) {
		if( e.button.button == 1 ) {
			// left mb -> set object
			switch( m_layer ) {
			case GRID_FLOOR: ed->set_floor( x, y, obj ); break;
			case GRID_ITEMS: ed->set_item( x, y, obj ); break;
			case GRID_STONES: ed->set_stone( x, y, obj ); break;
			default: break;
			}
		}
		else if( e.button.button == 3 ) {
			// right mb
		}
	}
}

void
GridObjectTool::object_menu()
{
}

void
ActorTool::on_mousebutton( const SDL_Event &e, V2 worldpos )
{
	Editor *ed = Editor::get_instance();
	string obj = current_object();

	if( e.button.type == SDL_MOUSEBUTTONDOWN ) {
		if( e.button.button == 1 ) {
			// left mb -> set object
			ed->set_actor( worldpos[0], worldpos[1], obj );
		}
		else if( e.button.button == 3 ) {
			// right mb
		}
	}
}


/* -------------------- The Editor -------------------- */

Editor *Editor::m_instance = 0;

Editor::Editor()
	: m_editarea( 0,0,640,13*32 )
	, m_iconarea( 0,13*32,640,64 )
	, m_display()
	, m_iconbar( m_iconarea, 2, 640/32 )
	, m_quit_editor( false )
	, m_cursor()
	, m_lua( luaL_newstate() )
	, m_editmode( MODE_FLOOR )
	, m_tools()
{
        luaL_openlibs(m_lua);
	tolua_open( m_lua );
	tolua_editor_open( m_lua );
}

Editor::~Editor()
{
	lua_close( m_lua );
}

void
Editor::init()
{
	delete_sequence( m_tools.begin(), m_tools.end() );
	m_tools.clear();
	m_tools.resize( MODE_COUNT, NULL );
	m_tools[ MODE_FLOOR ] = m_floortool = new FloorTool;
	m_tools[ MODE_ITEMS ] = m_itemtool = new ItemTool;
	m_tools[ MODE_STONES ] = m_stonetool = new StoneTool;
	m_tools[ MODE_ACTORS ] = m_actortool = new ActorTool;

	new_world( 20, 13 );

    // TODO - just printing a message is not enough - the app will crash on missing editor.lua
    if (lua::DoSysFile(m_lua, "compat.lua") != lua::NO_LUAERROR) {
        std::string message = "While processing 'compat.lua':\n" +lua::LastError(m_lua);
        fprintf( stderr, message.c_str() );
    }
    if (lua::DoSysFile(m_lua, "editor.lua") != lua::NO_LUAERROR) {
        std::string message = "Error loading 'editor.lua'\n" +lua::LastError(m_lua);
        fprintf( stderr, message.c_str() );
    }
}

void
Editor::run()
{
	m_quit_editor = false;
	while( !m_quit_editor ) {
		video::HideMouse();
		m_display.redraw();
		video::ShowMouse();
		video::GetScreen()->flush_updates();

		SDL_Event e;
		if( SDL_PollEvent( &e ) ) {
			dispatch_event (e);
		}

		SDL_Delay( 10 );
	}
}

void Editor::set_mode( EditMode m )
{
	m_editmode = m;
	update_cursor();
}

void Editor::update_cursor()
{
	// set proper position
	int mx;
	int my;
	SDL_GetMouseState( &mx, &my );
	DisplayEngine *engine = m_display.get_engine();
	V2 worldpos = engine->to_world( V2( mx, my ) );
	current_tool()->round_coordinates( &worldpos );
	m_cursor.move( worldpos );

	// set proper face
	set_cursor( current_tool()->current_object() );
}

Tool *Editor::current_tool()
{
	if( Tool *tool = m_tools[ m_editmode ] ) {
		return tool;
	}
	else {
		fprintf( stderr, "undefined tool\n" );
		assert( m_stonetool != NULL );
		return m_stonetool;
	}
}


void
Editor::set_floor( int x, int y, std::string const& name )
{
	m_display.set_floor( x, y, display::MakeModel( name ) );
}

void
Editor::set_item( int x, int y, std::string const& name )
{
	 m_display.set_item( x, y, display::MakeModel( name ) );
}

void
Editor::set_stone( int x, int y, std::string const& name )
{
	m_display.set_stone( x, y, display::MakeModel( name ) );
}

void
Editor::set_actor( double x, double y, std::string const& name )
{
	m_display.add_sprite( V2( x, y ), display::MakeModel( name ) );
}


void
Editor::new_world( int w, int h )
{
	m_display.new_world( w, h );
	for( int x=0; x<w; ++x ) {
		for( int y=0; y<h; ++y ) {
			set_floor (x, y, "fl-normal");
		}
	}
	m_cursor = m_display.add_effect( V2(), display::MakeModel( "it-hammer" ) );
}

void
Editor::set_cursor( const string &name )
{
	m_cursor.replace_model( display::MakeModel( name ) );
}


void
Editor::scroll( double xoff, double yoff )
{
	DisplayEngine *engine = m_display.get_engine();
	V2 newoffset = engine->get_offset() + V2( xoff, yoff );
	video::HideMouse();
	engine->move_offset( newoffset );
	video::ShowMouse();
}

void
Editor::scroll_abs( double x, double y )
{
	DisplayEngine *engine = m_display.get_engine();
	video::HideMouse();
	engine->move_offset( V2( x, y ) );
	video::ShowMouse();
}

bool
Editor::on_mousemotion( SDL_Event &e )
{
	DisplayEngine *engine = m_display.get_engine();
	V2 worldpos = engine->to_world( V2( e.motion.x, e.motion.y ) );
	current_tool()->round_coordinates( &worldpos );
	m_cursor.move( worldpos );
	return true;
}

bool
Editor::on_mousebutton( SDL_Event &e )
{
	DisplayEngine *engine = m_display.get_engine();
	V2 worldpos = engine->to_world( V2( e.motion.x, e.motion.y ) );

	if( Tool *tool = current_tool() ) {
		tool->on_mousebutton( e, worldpos );
	}
	return true;
}

bool
Editor::on_keydown( SDL_Event &e )
{
	bool ctrl_pressed  = e.key.keysym.mod & KMOD_CTRL;
	bool shift_pressed = e.key.keysym.mod & KMOD_SHIFT;
	int hoff = 19;
	int voff = 12;

	Tool *tool = current_tool();

	switch( e.key.keysym.sym ) {
	case SDLK_ESCAPE:
		m_quit_editor = true;
		break;
	case SDLK_LEFT:  scroll( ctrl_pressed ? -hoff : -1, 0 ); break;
	case SDLK_RIGHT: scroll( ctrl_pressed ? +hoff : +1, 0 ); break;
	case SDLK_DOWN:  scroll( 0, ctrl_pressed ? +voff : +1 ); break;
	case SDLK_UP:    scroll( 0, ctrl_pressed ? -voff : -1 ); break;
	case SDLK_HOME:  scroll_abs( 0, 0 ); break;

	case SDLK_PAGEUP:
		tool->advance_group( -1 );
		update_cursor();
		break;
	case SDLK_PAGEDOWN:
		tool->advance_group( +1 );
		update_cursor();
		break;

	case SDLK_PLUS:
	case SDLK_KP_PLUS:
		tool->advance_in_group( +1 );
		update_cursor();
		break;

	case SDLK_MINUS:
	case SDLK_KP_MINUS:
		tool->advance_in_group( -1 );
		update_cursor();
		break;

	case SDLK_a:
		if( shift_pressed )
			;
		set_mode( MODE_ACTORS );
		break;

	case SDLK_f:
		if( shift_pressed )
			;
		set_mode( MODE_FLOOR );
		break;

	case SDLK_i:
		if( shift_pressed )
			;
		set_mode( MODE_ITEMS );
		break;

	case SDLK_s:
		set_mode( MODE_STONES );
		if( shift_pressed ) {
			tool = current_tool();
			tool->object_menu();
		}
		break;

	default:
		return false;
	}
	return true;
}

void editor::DefineFloorGroup( char const* name, char const* descr,
	int nentries, char ** entries )
{
	Editor *ed = Editor::get_instance();

	vector<string> entryvec(entries, entries+nentries);
	ObjectGroup group(name, descr, entryvec);
	ed->add_floor_group (group);
}

void editor::DefineItemGroup( const char *name, const char *descr,
	int nentries, char **entries )
{
	Editor *ed = Editor::get_instance();

	vector<string> entryvec( entries, entries + nentries );
	ObjectGroup group( name, descr, entryvec );
	ed->add_item_group( group );
}

void editor::DefineStoneGroup( const char *name, const char *descr,
	int nentries, char **entries )
{
	Editor *ed = Editor::get_instance();

	vector<string> entryvec( entries, entries+nentries );
	ObjectGroup group( name, descr, entryvec );
	ed->add_stone_group( group );
}


void editor::Run()
{
	Editor *ed = Editor::get_instance();
	ed->init();
	ed->run();
}
