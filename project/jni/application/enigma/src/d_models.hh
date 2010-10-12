/*
 * Copyright (C) 2002,2003 Daniel Heck
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
#ifndef D_MODELS_HH
#define D_MODELS_HH

#include "ecl_fwd.hh"
#include <vector>
#include <string>
#include <cassert>

namespace display
{
    using std::string;
    using std::vector;
    using ecl::Surface;

/* -------------------- Image -------------------- */

    struct Image {
	// Variables.
	ecl::Surface *surface;
	ecl::Rect     rect;      // location of image inside surface
	int          refcount;  // reference count, initialized to 1

        // Constructors.
	Image(ecl::Surface *sfc);
	Image(ecl::Surface *sfc, const ecl::Rect &r);
    };

    void incref (Image *i);
    void decref (Image *i);
    void draw_image (Image *i, ecl::GC &gc, int x, int y);

/* -------------------- ImageModel -------------------- */

    class ImageModel : public Model {
	Image* image;
	int xoff, yoff;         // relative origin of the image
    public:
        // Constructors
	ImageModel (Image *i, int xo, int yo);
	ImageModel (Surface *s, int xo, int yo);
	ImageModel (Surface *s, const ecl::Rect &r, int xo, int yo);
	~ImageModel();
	
        // Model interface
        void   draw(ecl::GC &gc, int x, int y);
	Model *clone();
        void   get_extension (ecl::Rect &r);
        Image *get_image() { return image; }
    };

/* -------------------- ShadowModel -------------------- */

    class ShadowModel : public Model {
    public:
        ShadowModel(Model *m, Model *sh);
        ~ShadowModel();

        // Model interface
        void expose (ModelLayer *ml, int vx, int vy);
        void remove (ModelLayer *ml);

	void set_callback(ModelCallback *cb);
	void reverse();
        void restart();
        void draw (ecl::GC &gc, int x, int y);
        void draw_shadow (ecl::GC &gc, int x, int y);
        Model *get_shadow() const;
        Model *clone();

        void   get_extension (ecl::Rect &r);

    private:
        Model *model, *shade;
    };

/* -------------------- CompositeModel -------------------- */

    class CompositeModel : public Model {
        Model *bg, *fg;
    public:
        CompositeModel(Model *b, Model *f) : bg(b), fg(f) {}
        ~CompositeModel() {
            delete bg; delete fg;
        }

        // Animation interface
        void set_callback(ModelCallback *cb) {
//            bg->set_callback(cb);
            fg->set_callback(cb);
        }
        void reverse() {
//            bg->reverse();
            fg->reverse();
        }
        void restart() {
            fg->restart();
        }

        // Model interface
	Model *get_shadow() const { return bg->get_shadow(); }
        virtual void expose (ModelLayer *ml, int vx, int vy) {
            fg->expose (ml, vx, vy);
//            bg->expose (ml, vx, vy);
        }
        virtual void remove (ModelLayer *ml) {
            fg->remove (ml);
//            bg->remove (ml);
        }
        void draw(ecl::GC &gc, int x, int y) {
            bg->draw(gc,x,y);
            fg->draw(gc,x,y);
        }
        void draw_shadow(ecl::GC &gc, int x, int y) {
            bg->draw_shadow(gc,x,y);
        }
        Model *clone() {
            return new CompositeModel(bg->clone(), fg->clone());
        }

        void   get_extension (ecl::Rect &r) {
            fg->get_extension (r);
//             ecl::Rect r1, r2;
//             bg->get_extension (r1);
//             fg->get_extension (r2);
//             r = boundingbox (r1, r2);
        }
    };

/* -------------------- RandomModel -------------------- */

    /* Creates new models randomly from a set of template models. */
    class RandomModel : public Model {
        std::vector<std::string> modelnames;
    public:
        void add_model(const std::string &name) {modelnames.push_back(name);}
        Model *clone();
    };

/* -------------------- AliasModel -------------------- */

    class AliasModel : public Model {
        string name;
    public:
        AliasModel(const string &modelname) : name(modelname) {}
        Model *clone();
    };

/* -------------------- Animations -------------------- */

    struct AnimFrame : public ecl::Nocopy {
        // Variables
        Model *model;
        double  duration;

        // Constructor and Destructor
        AnimFrame(Model *m, double dur)
        : model(m), duration(dur)
        {}

        ~AnimFrame() { delete model; }
    };

    struct AnimRep {
        // Variables
        vector<AnimFrame*>  frames;
        bool                loop;
        int                 refcount;

        // Constructor and Destructor
        AnimRep(bool l) : loop(l), refcount(1) {}

        ~AnimRep() { 
            delete_sequence(frames.begin(), frames.end()); 
        }
    };

    class Anim2d : public Model, public ecl::Nocopy {
    public:
        Anim2d (bool loop);
        ~Anim2d();
        void set_callback(ModelCallback *cb) { callback = cb; }

        void add_frame(Model *m, double duration);

        /* ---------- Model interface ---------- */
        void draw(ecl::GC &gc, int x, int y);
        void draw_shadow(ecl::GC &gc, int x, int y);
        Model *clone() { return new Anim2d(rep); }
        void reverse() { reversep = !reversep; }
        void restart ();

        void expose (ModelLayer *ml, int vx, int vy);
        void remove (ModelLayer *ml);

        void tick(double dtime);
        bool has_changed(ecl::Rect &changed_region);
        bool is_garbage() const { return finishedp; }

        void move (int newx, int newy);
        void get_extension (ecl::Rect &r);

    private:
        Anim2d(AnimRep *r);

        /* ---------- Variables ---------- */
        AnimRep *rep;
        unsigned curframe;      // Current frame number
        double  frametime;      // Elapsed time since frame was activated
        bool    finishedp;      // Animation has finished
        bool    changedp;       // Model state has changed since last redraw
        bool    reversep;       // Play the animation in reverse direction

        int videox, videoy;     // Video coordinates of sprite
        ModelCallback *callback;
    };

    ecl::Surface *GetSurface (const std::string &filename);
    ecl::Surface *CropSurface (const ecl::Surface *s, ecl::Rect r);
    void DefineModel (const char *name, Model *m);
    void DefineImageModel (const char *name, ecl::Surface *s);
}

#endif
