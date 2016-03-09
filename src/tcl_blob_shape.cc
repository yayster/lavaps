
/*
 * tcl_blob_shape.cc
 * Copyright (C) 2000-2003 by John Heidemann
 * $Id: tcl_blob_shape.cc,v 1.14 2003/12/01 01:05:57 johnh Exp $
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
 *
 */

#include "config.h"

#ifdef USE_TCL_BLOB

#include <stdlib.h>  // atoi
#include <tcl.h>

#include <iostream>

#include <tcl.h>
#include <tk.h>

#include "main.hh"
#include "tcl_blob.hh"

#define HAVE_X11_SHAPE  // xxx: until autoconf support is worked out

#ifdef HAVE_X11_SHAPE
extern "C" {
#include <X11/Xos.h>
#include <X11/Xutil.h> // for Region
#include <X11/extensions/shape.h>
};
#endif /* HAVE_X11_SHAPE */



// Low-level X11 calls... we're having fun now.

bool
tcl_blob::shaped_window_ok()
{
#ifdef HAVE_X11_SHAPE
	// check on the extension
	static int have_extension = -1;
	static int shape_event_base, shape_error_base;
	if (have_extension == -1) {
		Tk_Window tk_main_window = Tk_MainWindow(interp_);
		have_extension = XShapeQueryExtension (Tk_Display(tk_main_window),
						       &shape_event_base, 
						       &shape_error_base);
	};
	return have_extension != 0;
#else /* ! HAVE_X11_SHAPE */
	return false;
#endif /* HAVE_X11_SHAPE */
}


void
tcl_blob::shape_window()
{
	if (!shaped_window_ok())
		return;

	if (!lozenge_shaped_)
		return;

#ifdef HAVE_X11_SHAPE
	// set up tk
	static bool inited = false;
	static Tk_Window tk_main_window, tk_canvas;
	if (!inited) {
		inited = true;
		tk_main_window = Tk_MainWindow(interp_);
		tk_canvas = Tk_NameToWindow(interp_, ".c", tk_main_window);
	};

	// allocate a pixmap to draw shapes in
	Pixmap shape_mask = XCreatePixmap (Tk_Display(tk_main_window),
					    Tk_WindowId(tk_main_window),
					    raw_width_, raw_height_, 1);
	assert(shape_mask != None);
	XGCValues xgcv;
	static GC shape_gc = 0;
	if (!shape_gc)
		shape_gc = XCreateGC(Tk_Display(tk_main_window), shape_mask, 0, &xgcv);

	// erase the pixmap
    	XSetForeground (Tk_Display(tk_main_window), shape_gc, 0);
    	XFillRectangle (Tk_Display(tk_main_window), shape_mask, shape_gc,
			0, 0, raw_width_, raw_height_);
    	XSetForeground (Tk_Display(tk_main_window), shape_gc, 1);

	// draw the bounding shape
	// should we account for border here?
	XPoint points[6];  // XFillPolygon will close shape
	double screen_x, screen_y;
#define assign_point(I,X,Y) \
		scale_point((X),(Y), &screen_x, &screen_y); \
		points[(I)].x = (int)screen_x; points[(I)].y = (int)screen_y;
	int buldge_peak_y = Y_MAX * lozenge_y_buldge_peak_pct_ / 100;
	assign_point(0, 0, 0);
	assign_point(1, 0, buldge_peak_y);
	assign_point(2, 0, Y_MAX);
	assign_point(3, X_MAX, Y_MAX);
	assign_point(4, X_MAX, buldge_peak_y);
	assign_point(5, X_MAX, 0);
	XFillPolygon(Tk_Display(tk_main_window), shape_mask, shape_gc,
		     points, 6, Convex, CoordModeOrigin);

	// Find the highest enclosing widget and shape it
	Tk_Window top_tk_window;
	int x = 0, y = 0;
	// this all really is a complicated way to do:
	// 	top_tk_window = tk_main_window;
	// but this is what xeyes does.
	for (top_tk_window = tk_canvas; Tk_Parent(top_tk_window); top_tk_window = Tk_Parent(top_tk_window)) {
		/* x = x + parent->core.x + parent->core.border_width; */
		x += Tk_InternalBorderWidth(top_tk_window);
		y += Tk_InternalBorderWidth(top_tk_window);
	}
	// finally clip
	XShapeCombineMask (Tk_Display(top_tk_window),
			   Tk_WindowId(top_tk_window),
			   ShapeBounding,
			   x, y, shape_mask, ShapeSet);
	XShapeCombineMask (Tk_Display(top_tk_window),
			   Tk_WindowId(top_tk_window),
			   ShapeClip,
			   x, y, shape_mask, ShapeSet);

	/*
	 * clip the frame?
	 * xeyes and such don't seem to do this,
	 * but I couldn't get it to work under fvwm without it.
	 */
	if (frame_window_id_ != 0) {
		/*
		 * Borders are a crock.
		 * The window manager puts an arbitrary decoration
		 * around our window that we *must* know about to
		 * make the shaping work right, but there's no 
		 * API to find out how big it is.
		 *
		 * To work around, we get the frame and our toplevel
		 * and map both coords to the root window and figure the 
		 * difference.
		 *
		 * Near as I can tell this approach isn't documented
		 * anywhere; I figured it out by looking at 
		 * Tk's tkUnixWm.c.
		 */
		XWindowAttributes top_x_attrs, frame_x_attrs;
		(void) XGetWindowAttributes(Tk_Display(top_tk_window), Tk_WindowId(top_tk_window), &top_x_attrs);
		(void) XGetWindowAttributes(Tk_Display(top_tk_window), frame_window_id_, &frame_x_attrs);

		Window x_root_window = RootWindow(Tk_Display(top_tk_window), Tk_ScreenNumber(top_tk_window));
		Window top_child_window, frame_child_window;
		int top_root_x, top_root_y;
		if (!XTranslateCoordinates(Tk_Display(top_tk_window),
					   Tk_WindowId(top_tk_window),
					   x_root_window, 
					   0, 0,
					   &top_root_x, &top_root_y,
					   &top_child_window))
			die(_("XTranslateCoordinates failed\n"));
		int frame_root_x, frame_root_y;
		if (!XTranslateCoordinates(Tk_Display(top_tk_window),
					   frame_window_id_,
					   x_root_window, 
					   0, 0,
					   &frame_root_x, &frame_root_y,
					   &frame_child_window))
			die(_("XTranslateCoordinates failed\n"));
#if 0
		printf (O_("top/frame/diff root x %d %d %d  y %d %d %d\n"),
			top_root_x, frame_root_x, top_root_x - frame_root_x,
			top_root_y, frame_root_y, top_root_y - frame_root_y);
#endif /* 0 */
		int frame_x_offset = top_root_x - frame_root_x;
		int frame_y_offset = top_root_y - frame_root_y;

		XShapeCombineMask (Tk_Display(top_tk_window),
			   frame_window_id_,
			   ShapeBounding,
			   x + frame_x_offset, y + frame_y_offset, shape_mask, ShapeSet);
#if 1
		XShapeCombineMask (Tk_Display(top_tk_window),
			   frame_window_id_,
			   ShapeClip,
			   x + frame_x_offset, y + frame_y_offset, shape_mask, ShapeSet);
#endif
	};
#if 0
	// clip ourselves --- not needed
	XShapeCombineMask (Tk_Display(tk_canvas),
			   Tk_WindowId(tk_canvas),
			   ShapeBounding,
			   0, 0, shape_mask, ShapeSet);
	XSync(Tk_Display(top_tk_window), false);
#endif
	// clean up
	XFreePixmap(Tk_Display(tk_main_window), shape_mask);
#endif /* HAVE_X11_SHAPE */
}

#endif /* USE_TCL_BLOB */
