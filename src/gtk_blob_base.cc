
/*
 * gtk_blob_base.cc
 * Copyright (C) 1999-2000 by John Heidemann
 * $Id: gtk_blob_base.cc,v 1.9 2003/07/04 01:53:06 johnh Exp $
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

#ifdef USE_GTK_BLOB

#include <stdlib.h>  // atoi
#include <signal.h>

#include <iostream>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <string>

#include <gnome.h>
#include <libgnomeui/libgnomeui.h>

#include "main.hh"
#include "gtk_blob.hh"

extern const char **gripper_xpms[];
extern const char **menu_xpms[];


gtk_blob_base::gtk_blob_base(GtkWidget *app, GtkWidget *canvas_w) :
	app_(app),
	canvas_w_(canvas_w),
	base_id_(NULL),
	menu_id_(NULL),
	gripper_id_(NULL)
//	tracking_id_(NULL)
{
	base_id_ = gnome_canvas_item_new(gnome_canvas_root(GNOME_CANVAS(canvas_w_)),
					 gnome_canvas_rect_get_type(),
					 "x1", (double)0,
					 "y1", (double)(blob::Y_MAX),
					 "x2", (double)(blob::X_MAX),
					 "y2", (double)(blob::Y_MAX + blob::BASE_EXTENSION),
					 "fill_color_rgba", 0x451515ff,
					 "outline_color", "black",
					 "width_units", 2.0,
					 NULL);

	fill_pixbufs(gripper_xpms, &gripper_pixbufs_);
	fill_pixbufs(menu_xpms, &menu_pixbufs_);
	assert(gripper_pixbufs_.size() == menu_pixbufs_.size());
	assert(gripper_pixbufs_.size() == 3);

	gripper_id_ = gnome_canvas_item_new(gnome_canvas_root(GNOME_CANVAS(canvas_w_)),
					    gnome_canvas_pixbuf_get_type (),
					    "pixbuf", gripper_pixbufs_[0], 
					    "x", (double)(blob::X_MAX),
					    "y", (double)(blob::Y_MAX + blob::BASE_EXTENSION),
					    "width", (double)gdk_pixbuf_get_width(gripper_pixbufs_[0]),
					    "height", (double) gdk_pixbuf_get_height(gripper_pixbufs_[0]),
					    "anchor", GTK_ANCHOR_SE,
					    NULL);

	menu_id_ = gnome_canvas_item_new(gnome_canvas_root(GNOME_CANVAS(canvas_w_)),
					    gnome_canvas_pixbuf_get_type (),
					    "pixbuf", menu_pixbufs_[0], 
					    "x", (double)0,
					    "y", (double)(blob::Y_MAX + blob::BASE_EXTENSION),
					    "width", (double)gdk_pixbuf_get_width(menu_pixbufs_[0]),
					    "height", (double) gdk_pixbuf_get_height(menu_pixbufs_[0]),
					    "anchor", GTK_ANCHOR_SW,
					    NULL);
}

void
gtk_blob_base::fill_pixbufs(xpm_t *xpms, vector<GdkPixbuf*> *v)
{
	while (*xpms != NULL) {
		GdkPixbuf *pixbuf = gdk_pixbuf_new_from_xpm_data(*xpms);
		assert(pixbuf != NULL);
		v->push_back(pixbuf);
		xpms++;
	};
}

void
gtk_blob_base::init()
{
}

void
gtk_blob_base::update_wh(int w, int h)
{
	/*
	 * pick a size:
	 * currently 0, 1, 2 == 16x16, 32x32, 64x64
	 * wait until the max-dimension is bigger than 8x the icon size,
	 * and the min-dimension is bigger than 6x size.
	 */
	int max_dimension = max(w, h);
	int size = max_dimension < 16*8*2 ? 0 : 
		max_dimension < 32*8*2 ? 1 :
		2;
	int min_dimension = min(w, h);
	size = min(size, min_dimension < 16*3*2 ? 0 :
		   min_dimension < 32*3*2 ? 1 :
		   2);
	
	assert(size < int(gripper_pixbufs_.size()));

	double s_right_x, s_bottom_y;
	double s_left_x, s_top_y;
	scaled_blob::scale_point(blob::X_MAX, blob::Y_MAX + blob::BASE_EXTENSION,
				 &s_right_x, &s_bottom_y);
	scaled_blob::scale_point(0.0, blob::Y_MAX,
				 &s_left_x, &s_top_y);

	gnome_canvas_item_set(base_id_,
			      "x1", s_left_x, "y1", s_top_y,
			      "x2", s_right_x, "y2", s_bottom_y,
			      NULL);
	gnome_canvas_item_set(gripper_id_,
			      "x", s_right_x, "y", s_bottom_y,
			      "pixbuf", gripper_pixbufs_[size],
			      NULL);
	gnome_canvas_item_set(menu_id_,
			      "x", (scaled_blob::is_vertical() ? s_left_x : s_right_x),
			      "y", (scaled_blob::is_vertical() ? s_bottom_y : s_top_y),
			      "anchor", (scaled_blob::is_vertical() ? GTK_ANCHOR_SW : GTK_ANCHOR_NE),
			      "pixbuf", menu_pixbufs_[size],
			      NULL);
}


gboolean
gtk_blob_base::event(GdkEvent *event, GnomeCanvasItem *item_hit)
{
	assert(item_hit != NULL);
	assert(item_hit == menu_id_ || item_hit == gripper_id_ || item_hit == base_id_ );

	if (event->type != GDK_BUTTON_PRESS)
		return FALSE;
	GdkEventButton *button = (GdkEventButton*)event;
	if (item_hit == menu_id_) {
		return gtk_blob::popup_menu_event(NULL, (GdkEventButton*)event, NULL);
	} else if (item_hit == gripper_id_) {
		gtk_window_present(GTK_WINDOW(app_));  // raise it
		gtk_window_begin_resize_drag(GTK_WINDOW(app_),
					     GDK_WINDOW_EDGE_SOUTH_EAST,
					     button->button,
					     (int)button->x_root,
					     (int)button->y_root,
					     button->time);
		return TRUE;
	} else if (item_hit == base_id_) {
		gtk_window_present(GTK_WINDOW(app_));  // raise it
		gtk_window_begin_move_drag(GTK_WINDOW(app_),
					   button->button,
					   (int)button->x_root,
					   (int)button->y_root,
					   button->time);
		return TRUE;
	}
	assert(0);
}


#endif /* USE_GTK_BLOB */
