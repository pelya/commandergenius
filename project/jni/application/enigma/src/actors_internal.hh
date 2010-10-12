namespace
{
    // helper class to find good respawn positions

    struct ExaminedLocation : public GridPos {
    public:
        ExaminedLocation(GridPos p) : GridPos(p) {}

        bool operator<(const ExaminedLocation& other) const {
            return (x == other.x) ? y<other.y : x<other.x;
        }
    };

    typedef std::set<ExaminedLocation> ExaminedLocations;

    class FreeRespawnLocationFinder
    {
        ExaminedLocations checked;
        ExaminedLocations blocked;
        ExaminedLocations candidates;

        const Actor &actor_to_set;
        bool         actor_is_marble;
        V2           preferred_position;

        double max_enemy_gap;
        V2     max_gap_pos;

        static const double MAX_DISTANCE_WANTED;

        static bool is_marble(const string& k) {
            // true if kind 'k' is a marble
            return k == "ac-blackball" || k == "ac-whiteball" || k == "ac-whiteball-small";
        }

        static bool is_respawn_floor(const string& k) {
            // true if marble may appear on floors of kind 'k'
            return
                k != "fl-abyss" &&
                k != "fl-water" &&
                k != "fl-space"; // player cannot be moved on fl-space
        }

        static bool is_respawn_item(const string& k) {
            // true if marble may appear on items of kind 'k'
            return
                k != "it-laserbeam" &&
                k != "it-burnable-ignited";
        }

        static bool search_through_stone(const Stone& st) {
            if (st.is_movable() || st.is_floating()) return true;

            const string& k = st.get_kind();
            return k == "st-puzzle";
        }

        static double wanted_distance_to(const string& k) {
            // returns the size of the gap wanted between a marble and an actor of kind 'k'

            if (k == "ac-rotor") return MAX_DISTANCE_WANTED;
            if (k == "ac-top") return 3.0;
            if (k == "ac-killerball" || k == "ac-bug") return 1.5;
            return 0.3;
        }

        double distance_wanted_to(const Actor& a) {
            double dist = 0.3;

            if      (actor_is_marble)         dist = wanted_distance_to(a.get_kind());
            else if (is_marble(a.get_kind())) dist = wanted_distance_to(actor_to_set.get_kind());

            ASSERT(dist <= MAX_DISTANCE_WANTED, XLevelRuntime, "FreeRespawnLocationFinder: distance_wanted_to too large ");
            return dist;
        }

        bool enemyActorAt(const V2& p) {
            vector<Actor*> found_actors;
            double         range = get_radius (&actor_to_set) + MAX_DISTANCE_WANTED + Actor::get_max_radius();

            if (GetActorsInRange(p, range, found_actors)) {
                bool   found_near_enemy = false;
                double min_enemy_gap    = 1000.0;

                for (vector<Actor*>::const_iterator ai = found_actors.begin();
                     ai != found_actors.end();
                     ++ai)
                {
                    Actor *a = *ai;

                    if (a != &actor_to_set) {
                        double distance    = length(p - a->get_pos());
                        double gap_between = distance - get_radius (&actor_to_set) - get_radius(a);
                        double wanted_gap  = distance_wanted_to(*a);

                        if (gap_between < wanted_gap)
                            found_near_enemy = true;

                        if (gap_between < min_enemy_gap)
                            min_enemy_gap = gap_between;
                    }
                }

                if (found_near_enemy) {
                    if (min_enemy_gap<999.0) {
                        if (min_enemy_gap > max_enemy_gap) {
                            max_enemy_gap = min_enemy_gap;
                            max_gap_pos   = p;
                        }
                    }
                }

                return found_near_enemy;
            }

            return false;
        }

        void examine(GridPos p) {
            if (checked.find(p) != checked.end()) return; // already examined
            checked.insert(p);  // never check again

            Floor *fl = GetFloor(p);
            if (!fl || !is_respawn_floor(fl->get_kind())) return; // bad floor

            bool may_respawn     = true;
            bool continue_search = true;

            Item *it = GetItem(p);
            if (it && !is_respawn_item(it->get_kind())) may_respawn = false; // bad item

            Stone *st = GetStone(p);
            if (st) {
                if (!search_through_stone(*st)) continue_search = false;
                may_respawn = false;
            }

            if (may_respawn) { // may be a candidate -> check for enemy actors
                if (enemyActorAt(p.center())) may_respawn = false;
            }

            if (continue_search) blocked.insert(p);
            if (may_respawn) candidates.insert(p);
        }

    public:

        FreeRespawnLocationFinder(V2 p, const Actor& actor)
            : actor_to_set(actor)
            , preferred_position(p)
            , max_enemy_gap(-1000.0)
        {
            actor_is_marble = is_marble(actor_to_set.get_kind());
            ExaminedLocations affected; // all locations affected by current respawn position
            {
                double radius = get_radius (&actor_to_set);
                int    xmin   = int(p[0]-radius);
                int    xmax   = int(p[0]+radius);
                int    ymin   = int(p[1]-radius);
                int    ymax   = int(p[1]+radius);

                for (int x = xmin; x <= xmax; ++x) {
                    for (int y = ymin; y <= ymax; ++y) {
                        affected.insert(GridPos(x, y));
                    }
                }
            }

            for (ExaminedLocations::const_iterator ai = affected.begin(); ai != affected.end(); ++ai) {
                examine(*ai);
            }

            if (candidates.size() != affected.size()) { // if any affected location may not be used for respawning
                // choose alternate respawn location

                blocked = affected; // start with all affected positions

                while (candidates.empty()) {
                    ExaminedLocations curr_blocked;
                    swap(curr_blocked, blocked);

                    if (curr_blocked.empty()) {
                        break;  // no chance to find a candidate
                    }

                    for (ExaminedLocations::const_iterator bl = curr_blocked.begin(); bl != curr_blocked.end(); ++bl) {
                        examine(move(*bl, NORTH));
                        examine(move(*bl, SOUTH));
                        examine(move(*bl, EAST));
                        examine(move(*bl, WEST));
                    }
                }

                if (candidates.empty()) { // no better location -> take least worse tested location
                    if (max_enemy_gap > 0.0) {
                        preferred_position = max_gap_pos;
                    }
                }
                else { // a better location has been found
                    ExaminedLocations::const_iterator c = candidates.begin();
                    advance(c, IntegerRand(0, int (candidates.size()-1)));

                    ASSERT(c != candidates.end(), XLevelRuntime, "FreeRespawnLocationFinder: list of candidates corrupt");
                    preferred_position = c->center();
                }
            }
        }

        V2 get_position() const { return preferred_position; }
    };
}

namespace
{
    const double FreeRespawnLocationFinder::MAX_DISTANCE_WANTED = 5.0;
}
