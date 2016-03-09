/* -*-	Mode:C++ -*- */

/*
 * gtk_blob.hh
 * Copyright (C) 2003 by John Heidemann
 * $Id: gtk_blob.hh,v 1.34 2004/12/16 05:16:17 johnh Exp $
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

#ifndef lavaps_gtk_blob_h
#define lavaps_gtk_blob_h

#ifdef USE_GTK_BLOB

#include <gnome.h>
#include <libgnomecanvas/libgnomecanvas.h>

#include <list>
#include <map>

#include <string>

#include "scaled_blob.hh"
#include "const_str.hh"
#include "gtk_blob_splash.hh"
#include "gtk_blob_base.hh"
#include "gtk_blob_conf.hh"


#define LABEL_BUF_LEN 256  // buffer big enough to contain process names



/*
 * structure to keep track of details for the menu callback
 */
class menu_callback_info {
public:
	enum menus { UNDEFINED_MENU, WHO_MENU, WHAT_MENU, HOW_MENU };
	enum modifiers { UNDEFINED_MODIFIER, WHO_ME, WHO_EVERYONE,
			 WHAT_VIRTUAL, WHAT_PHYSICAL, WHAT_BOTH, 
			 HOW_SHRINK, HOW_GROW, HOW_AUTOSIZE, HOW_LOZENGE,
			 HOW_BASE, HOW_STICKY,
			 HOW_SMOOTHER, HOW_JUMPIER
	};
	enum menus menu_;
	enum modifiers modifier_;
	const char *desc_;
	gtk_blob_conf *conf_;

	menu_callback_info(enum menus menu, enum modifiers modifier, const char *desc) :
		menu_(menu), modifier_(modifier), desc_(desc), conf_(NULL) {}
};


class gtk_blob : public scaled_blob {
private:
	gtk_blob(const gtk_blob& tb);
	gtk_blob& operator=(const gtk_blob& tb);

protected:
	// constants
	enum { CANVAS_DEFAULT_WIDTH = 100,
	       CANVAS_DEFAULT_HEIGHT = (CANVAS_DEFAULT_WIDTH * (Y_MAX + BASE_EXTENSION) / X_MAX) };

	// keep track of blobs, and us in them
	typedef list<gtk_blob*> gtk_blobs_t;
	static gtk_blobs_t gtk_blobs_;
	gtk_blobs_t::iterator me_in_blobs_;
	// and map between items and blobs (many to one)
	static map<GnomeCanvasItem*,gtk_blob*> item_to_blob_map_;

	// a blob is two parts
	GnomeCanvasGroup *group_;  // group for the blob and its shadow
	GnomeCanvasItem *id_, *did_;
	bool did_shown_;

	// basic stuff
	static GnomeProgram *program_;
	static GtkWidget *app_;
	static GtkWidget *canvas_w_;

	// user interaction
	static GtkWidget *popup_menu_w_;
	static GtkAccelGroup *popup_accel_group_;
	static gtk_blob *info_blob_hit_;
	static gtk_blob *menu_blob_hit_;
	static int last_x_, last_y_;
	//
	static int spline_spacing_;
	//
	GdkColor fg_color_, bd_color_, dk_color_;
	bool fg_is_whitish_;

	// info window
	static bool mouse_down_;
	static GtkWindow *info_w_;
	static GtkLabel *info_color_label_, *info_text_label_;
	// splash
	static gtk_blob_splash *splash_;
	static gtk_blob_base *base_;
	// icons
	static void init_icons();
	static GList *icon_pixbufs_glist_;
	// config stuff
	static gtk_blob_conf_double *size_mem_to_screen_scale_conf_;
	static gtk_blob_conf_double *check_interval_conf_;
	static explicit_change_tracking<double> check_interval_seconds_;

	static void init_menu();
	static void init_conf();
	static void init_canvas();
	static void init_ticker();
	static void init_info();

	void set_point(GnomeCanvasPoints *points, int i, double x, double y);
	GnomeCanvasPoints *set_points(int pct = 100, int offset = 0);

	virtual void redraw();
	static gboolean tick(gpointer data);  // check for processes

	static gboolean find_canvas_hit(GtkObject *obj, GdkEvent *event, gtk_blob **blob_hit);

	static gboolean post_info_event(GtkObject *obj, GdkEventButton *button, gtk_blob *blob_hit);
	void setup_info_for_me();
	static gboolean blob_event(GtkObject *obj, GdkEvent *event, gpointer data);
	static gboolean configure_event(GtkObject *obj, GdkEvent *event, gpointer data);

	static void shape_window(int w, int h);

public:
	gtk_blob(process_view *pv);
	virtual ~gtk_blob();

	virtual void set_hsb(double h, double s, double b);

	static void splash(const char *msg);
	static void init(int argc, char **argv);
	static void mainloop();
	static void size_divisor_commit();

	static void send_signal_callback(GtkWidget *widget, char *signal);
	static void menu_set_callback(GtkWidget *widget, menu_callback_info *signal);
	static void menu_help_callback(GtkWidget *widget, char *signal);

	static gboolean popup_menu_event(GtkObject *obj, GdkEventButton *button, gtk_blob *blob_hit);
};

#endif /* USE_GTK_BLOB */

#endif /* ! lavaps_gtk_blob_h */
