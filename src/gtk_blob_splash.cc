
/*
 * gtk_blob_splash.cc
 * Copyright (C) 1999-2003 by John Heidemann
 * $Id: gtk_blob_splash.cc,v 1.4 2003/12/02 01:11:42 johnh Exp $
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


static gboolean
splash_heartbeat(gpointer data)
{
	gtk_blob_splash *splash = (gtk_blob_splash *)data;
	return splash->heartbeat() ? TRUE : FALSE;
}

void
gtk_blob_splash::revive(const char *msg)
{
	if (live_) {
		assert(id_ != NULL);
		return;
	};
	assert(canvas_w_ != NULL);
	id_ = gnome_canvas_item_new(gnome_canvas_root(GNOME_CANVAS(canvas_w_)),
					   gnome_canvas_text_get_type(),
					   O_("text"), msg,
					   O_("x"), (double)x_,
					   O_("y"), (double)y_,
					   O_("anchor"), GTK_ANCHOR_S,
					   O_("fill_color_rgba"), 0xffffffff,
					   NULL);
	gnome_canvas_item_raise_to_top(id_);

	gray_ = 255;
	text_ = msg;
	text_without_last_ = "";
	last_msg_ = msg;
	last_msg_count_ = 1;

	// schedule 4s timer event
	(void) g_timeout_add(1000, splash_heartbeat, this);

	live_ = true;
}

void
gtk_blob_splash::slay()
{
	if (!live_) {
		assert(id_ == NULL);
		return;
	};
	gtk_object_destroy(GTK_OBJECT(id_));
	id_ = NULL;
	live_ = false;
}

bool
gtk_blob_splash::heartbeat()
{
	/*
	 * Note: there's a sort of race condition, in that slay()
	 * can be called, killing the blob, if a user clicks on the splash.
	 * But the timer is left running, so if some event makes a new
	 * splash quickly we'll have two timers going.
	 * The Right Way to stop this would be to cancel the pending
	 * timer in slay() (except that we don't keep its id, and
	 * slay would need to do this only if called directly, not if
	 * called from heartbeat), or to add a generation number to heartbeat
	 * so we reject duplicate calls (but it's a pain to pass both 
	 * the object and the generation number to the heartbeat callback,
	 * and the generation number can't be static because that
	 * defeats the purpose.
	 *
	 * Solution: ignore the problem.  The only result of the race
	 * is that the text decays twice as fast as usual.
	 */
	if (!live_)
		return false;
	gray_ -= 6;  // 6/256 ~= 2%
	if (gray_ < 0) {
		slay();
		return false;
	} else {
		int rgba = (((((gray_ << 8) + gray_) << 8) + gray_) << 8) + gray_;
		gnome_canvas_item_set(id_, O_("fill_color_rgba"), rgba, NULL);
		gnome_canvas_item_raise_to_top(id_);
		// reschedule ourselves
		return true;
	};
	
}

void
gtk_blob_splash::update_wh(int w, int h)
{
	x_ = w / 2;
	y_ = h * 5 / 6;  // assumes base exists
	if (id_ != NULL) {
		// move it
		gnome_canvas_item_set(id_,
				      "x", (double)x_,
				      "y", (double)y_,
				      NULL);
	};
	if (live_) {
		int font_size = 12;
		if (w < 90)
			font_size = 8;
		else if (w < 180)
			font_size = 10;
		else if (w < 270)
			font_size = 12;
		else if (w < 360)
			font_size = 14;
		else font_size = 16;
		ostringstream oss;
		oss << O_("Sans Bold ") << font_size;
		gnome_canvas_item_set(id_, O_("font"), oss.str().c_str(), NULL);
	};
}

void
gtk_blob_splash::splash(const char *msg)
{
	if (!live_) {
		revive(msg);
		return;
	};
	
	gray_ = 255; // will propagate next tick
	if (msg == last_msg_) {
		// old text, bump the count
		last_msg_count_++;
		stringstream whole;
		string msg_nonl = msg;
		// xxx: this doesn't succsesfully trim the \n!
		msg_nonl.erase(msg_nonl.find_last_not_of('\n') + 1);
		whole << text_without_last_ 
		      << msg_nonl << " " << last_msg_count_ << endl;
		text_ = whole.str();
	} else {
		// new text, add it
		text_without_last_ = text_;
		text_ += msg;
		last_msg_ = msg;
		last_msg_count_ = 1;
	};
	gnome_canvas_item_set(id_, O_("text"), text_.c_str(), NULL);
}

#endif /* USE_GTK_BLOB */
