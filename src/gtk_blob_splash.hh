/* -*-	Mode:C++ -*- */

/*
 * gtk_blob_splash.hh
 * Copyright (C) 2003 by John Heidemann
 * $Id: gtk_blob_splash.hh,v 1.1 2003/06/22 22:50:02 johnh Exp $
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

#ifndef lavaps_gtk_blob_splash_h
#define lavaps_gtk_blob_splash_h

#ifdef USE_GTK_BLOB

#include "gtk_blob.hh"


class gtk_blob_splash {
protected:
	GtkWidget *canvas_w_;

	bool live_;
	GnomeCanvasItem *id_;
	int x_, y_;
	int gray_;
	string text_;
	string text_without_last_;
	string last_msg_;
	int last_msg_count_;

	void revive(const char *msg);
public:
	gtk_blob_splash(GtkWidget *canvas_w) : canvas_w_(canvas_w),
				    live_(false), id_(NULL),
				    x_(50), y_(50), gray_(0),
				    last_msg_count_(0)
		{};
	void splash(const char *msg);
	bool heartbeat();
	void slay();
	void update_wh(int w, int h);
	bool item_is_this_splash(GnomeCanvasItem *i) { return (i != NULL) && (i == id_); };
};


#endif /* USE_GTK_BLOB */

#endif /* ! lavaps_gtk_blob_splash_h */
