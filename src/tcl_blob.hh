/* -*-	Mode:C++ -*- */

/*
 * tcl_blob.hh
 * Copyright (C) 1999-2000 by John Heidemann
 * $Id: tcl_blob.hh,v 1.31 2003/07/15 04:03:06 johnh Exp $
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

#ifndef lavaps_tcl_blob_h
#define lavaps_tcl_blob_h

#ifdef USE_TCL_BLOB

#include "tcl.h"

#include "blob.hh"
#include "scaled_blob.hh"
#include "const_str.hh"

#include <list>

class tcl_blob : public scaled_blob {
private:
	tcl_blob(const tcl_blob& tb);
	tcl_blob& operator=(const tcl_blob& tb);

protected:
	static int lava_tick_proc(ClientData clientData, Tcl_Interp *interp, int argc, const char *argv[]);
	static int lava_id_to_info_proc(ClientData clientData, Tcl_Interp *interp, int argc, const char *argv[]);
	static int lava_menu_proc(ClientData clientData, Tcl_Interp *interp, int argc, const char *argv[]);
	static int lava_resize_proc(ClientData clientData, Tcl_Interp *interp, int argc, const char *argv[]);
	static int lava_dump_proc(ClientData clientData, Tcl_Interp *interp, int argc, const char *argv[]);
	static int lava_version_proc(ClientData clientData, Tcl_Interp *interp, int argc, const char *argv[]);
	static int lava_default_resources_proc(ClientData clientData, Tcl_Interp *interp, int argc, const char *argv[]);


protected:
	static Tcl_Interp *interp_;
	static char *canvas_;
#define TCL_BUF_LEN 256
#define LONG_TCL_BUF_LEN 2048
	static char result_buf_[TCL_BUF_LEN];
	typedef list<tcl_blob*> tcl_blobs_t;
	static tcl_blobs_t tcl_blobs_;
	tcl_blobs_t::iterator me_in_blobs_;

	static int frame_window_id_;
	static int pixel_size_;   // number of drawing units that make up a pixel on the screen

	friend class blobs_find_id_p;
	int id_;
	int did_;   // dark id
	const_str light_color_;
	const_str dark_color_;
	const_str bd_color_;

	static bool shaped_window_ok();
	static void shape_window();
	static char *list_point(char *buf, char *eobuf, int x, int y);
	char *list_points(char *buf, char *eobuf, int pct = 100, int offset = 0);
	virtual void redraw();
public:
	tcl_blob(process_view *pv);
	virtual ~tcl_blob();

	virtual void set_hsb(double h, double s, double b);

	static void splash(const char *msg);
	static void init(int argc, char **argv);
	static void mainloop();
	static void size_divisor_commit();
};

#endif /* USE_TCL_BLOB */

#endif /* ! lavaps_tcl_blob_h */
