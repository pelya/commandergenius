namespace
{
	class EditorState {
	public:
		EditorState();
		~EditorState();

		void save (std::ostream &os);
		void load (std::istream &is);

	private:

	};

	class DL_Editor
		: public display::DisplayLayer
	{
	public:
		DL_Editor()
		{
		}

		~DL_Editor()
		{
		}

		void prepare_draw( const display::WorldArea & )
		{
		}

		void draw( ecl::GC &gc, const display::WorldArea &a, int x, int y )
		{
		}

		void draw_onepass( ecl::GC & gc )
		{
			//just a test...
			//ecl::set_color( gc, 240, 140, 20, 255 );
			//ecl::set_flags( gc.flags, GS_ANTIALIAS );
			//ecl::line( gc, 0, 0, 320, 240 );
		}

		void tick( double dtime )
		{
		}

		void new_world( int w, int h )
		{
		}
	};

	class EditorDisplay
		: public display::CommonDisplay
	{
	public:
		EditorDisplay();
		~EditorDisplay() {}

		void redraw();

		void tick(double /*dtime*/ ) {}

		Model *make_model( const std::string &name );
	private:

	};

	class IconBar
		: public gui::Container
	{
	public:
		IconBar( const ScreenArea &a, int rows, int cols )
			: m_area( a )
			, m_rows( rows )
			, m_cols( cols )
			, m_bgcolor( 150, 150, 150 )
		{
		}

		~IconBar() {}

		ScreenArea get_area() const
		{
			return m_area;
		}

		// Widget interface.
		void draw( ecl::GC &gc, const ecl::Rect &area_ )
		{
			set_color( gc, m_bgcolor );
			box( gc, area_ );
		}

	private:
		// Variables.
		ScreenArea m_area;
		int        m_rows, m_cols;
		ecl::RGB   m_bgcolor;
	};


	enum EditMode {
		MODE_FLOOR,
		MODE_ITEMS,
		MODE_STONES,
		MODE_ACTORS,
		MODE_COUNT
	};

	struct ObjectGroup {
		string         name;
		string         descr;
		vector<string> entries;

		ObjectGroup( const string &name_,
			const string &descr_, const vector<string> entries_
		)
			: name( name_ )
			, descr( descr_ )
			, entries( entries_ )
		{
		}
	};


	class Tool {
	public:
		virtual ~Tool() {}

		virtual void object_menu() = 0;

		virtual void advance_in_group( int offset ) = 0;
		virtual void advance_group( int offset ) = 0;
		virtual string current_object() = 0;

		virtual void on_mousebutton( const SDL_Event &e, V2 worldpos ) = 0;

		/// Rounds coordinates for item display.  Actor-like tools
		/// will want to leave the coordinates intact, grid-like tools
		/// could want to round to integers.
		virtual void round_coordinates( V2 * worldpos ) = 0;
	};

	class GridObjectTool
		: public Tool
	{
	public:

		// Tool interface
		void object_menu();

		void advance_in_group( int offset );
		void advance_group( int offset );

		string current_object()
		{
			return m_groups[ m_groupidx ].entries[ m_objectidx ];
		}

		void add_group( const ObjectGroup &g )
		{
			m_groups.push_back( g );
		}

		// Constructors
		GridObjectTool( GridLayer layer )
		{
			m_groupidx = m_objectidx = 0;
			m_layer = layer;
		}

		void round_coordinates( V2 * worldpos )
		{
			(*worldpos)[ 0 ] = round_down<int>( (*worldpos)[ 0 ] );
			(*worldpos)[ 1 ] = round_down<int>( (*worldpos)[ 1 ] );
		}

		void on_mousebutton( const SDL_Event &e, V2 worldpos );

	protected:

		// Variables
		vector<ObjectGroup> m_groups;
		size_t              m_groupidx;
		size_t              m_objectidx;
		GridLayer           m_layer;
	};

	class FloorTool
		: public GridObjectTool
	{
	public:
		FloorTool()
			: GridObjectTool(GRID_FLOOR)
		{}
	};

	class ItemTool
		: public GridObjectTool
	{
	public:
		ItemTool()
			: GridObjectTool( GRID_ITEMS )
		{
		}
	};

	class StoneTool
		: public GridObjectTool
	{
	public:
		StoneTool ()
			: GridObjectTool( GRID_STONES )
		{
		}
	};

	class ActorTool
		: public Tool
	{
	public:
		// Tool interface.
		void object_menu() {}
		void advance_in_group( int /*offset*/ ) {}
		void advance_group( int /*offset*/ ) {}
		string current_object() { return "ac-blackball"; }
		void on_mousebutton( const SDL_Event & e, V2 worldpos );
		void round_coordinates( V2 * /*worldpos*/ ) {}
	private:
	};


	class Level {
	public:
		Level( EditorDisplay *display )
			: m_display( display )
		{
		}

	private:
		// Variables.
		EditorDisplay *m_display;
	};



	class Editor
		: private sdl::EventHandler
	{
	public:
		static Editor * get_instance()
		{
			if( m_instance == NULL ) {
				m_instance = new Editor;
			}
			return m_instance;
		}
		~Editor();

		void init();
		void run();

		Level *get_level() const
		{
			return m_level.get();
		}

		void add_floor_group( const ObjectGroup &g )
		{
			m_floortool->add_group( g );
		}

		void add_item_group( const ObjectGroup &g )
		{
			m_itemtool->add_group( g );
		}

		void add_stone_group( const ObjectGroup &g )
		{
			m_stonetool->add_group( g );
		}

		void set_floor( int x, int y, string const& name );
		void set_item( int x, int y, string const& name );
		void set_stone( int x, int y, string const& name );
		void set_actor( double x, double y, string const& name );

	private:
		Editor();


		/* ---------- Private methods ---------- */
		void set_cursor( const string &name );

		/// Change cursor face and position based on active tool.
		void update_cursor();

		void set_mode( EditMode m );
		Tool *current_tool();

		void new_world( int w, int h );

		void scroll( double xoff, double yoff );
		void scroll_abs( double x, double y );

		/*
		** EventHandler interface.
		*/
		bool on_mousemotion( SDL_Event &e );
		bool on_mousebutton( SDL_Event &e );
		bool on_keydown( SDL_Event &e );

		/*
		** Variables.
		*/
		ScreenArea    m_editarea;
		ScreenArea    m_iconarea;
		EditorDisplay m_display;
		IconBar       m_iconbar;
		bool          m_quit_editor;
		display::SpriteHandle  m_cursor;

		lua_State      *m_lua;

		FloorTool      *m_floortool;
		ItemTool       *m_itemtool;
		StoneTool      *m_stonetool;
		ActorTool      *m_actortool;
		EditMode        m_editmode;
		vector<Tool *>  m_tools;
		auto_ptr<Level> m_level;

		static Editor  *m_instance;
	};
}
