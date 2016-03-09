/* -*-	Mode:C++ -*- */

/*
 * gtk_blob_base.hh
 * Copyright (C) 2003 by John Heidemann
 * $Id: gtk_blob_base.hh,v 1.3 2003/06/24 05:39:37 johnh Exp $
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

#ifndef lavaps_gtk_blob_base_h
#define lavaps_gtk_blob_base_h

#ifdef USE_GTK_BLOB

#include <vector>

#include <gnome.h>
#include <libgnomecanvas/libgnomecanvas.h>

#include "gtk_blob.hh"

class gtk_blob_base {
protected:
	GtkWidget *app_;
	GtkWidget *canvas_w_;

	GnomeCanvasItem *base_id_;
	GnomeCanvasItem *menu_id_;
	GnomeCanvasItem *gripper_id_;

	vector<GdkPixbuf*>gripper_pixbufs_;
	vector<GdkPixbuf*>menu_pixbufs_;

	typedef const char **xpm_t;
	void fill_pixbufs(xpm_t *xpms, vector<GdkPixbuf*> *v);
	void init();
public:
	gtk_blob_base(GtkWidget *app, GtkWidget *canvas_w);
	void update_wh(int w, int h);
	gboolean event(GdkEvent *event, GnomeCanvasItem *item_hit);
	bool item_hit_is_base(GnomeCanvasItem *item) {
		return (item != NULL && (item == base_id_ ||
					 item == menu_id_ ||
					 item == gripper_id_));
	};
};



#endif /* USE_GTK_BLOB */

#endif /* ! lavaps_gtk_blob_base_h */
