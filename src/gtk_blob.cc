
/*
 * gtk_blob.cc
 * Copyright (C) 1999-2003 by John Heidemann
 * $Id: gtk_blob.cc,v 1.63 2004/12/17 19:59:13 johnh Exp $
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
#include <math.h>  // for M_SQRT2 and M_SQRT1_2

#include <iostream>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <string>

#include <gnome.h>
#include <libgnomeui/libgnomeui.h>

#include <libintl.h>

#include "main.hh"
#include "gtk_blob.hh"
#include "gtk_lava_help.hh"
#include "gtk_compat.hh"
#include "process_list.hh"
#include "lava_signal.hh"


//
// hooks to superclass
//

blob *
blob::create_real_blob(process_view *pv)
{
	ENTRY_TRACE(__FILE__,__LINE__);
	return (blob*) new gtk_blob(pv);
}
void
blob::splash(const char *msg)
{
	gtk_blob::splash(msg);
}
void
blob::init(int argc, char **argv)
{
	gtk_blob::init(argc, argv);
}
void
blob::mainloop()
{
	gtk_blob::mainloop();
}
void
blob::size_divisor_commit()
{
	gtk_blob::size_divisor_commit();
}


//
// static member variables
//
gtk_blob::gtk_blobs_t gtk_blob::gtk_blobs_;
map<GnomeCanvasItem*,gtk_blob*> gtk_blob::item_to_blob_map_;
//
GnomeProgram *gtk_blob::program_;
GtkWidget *gtk_blob::app_;
GtkWidget *gtk_blob::canvas_w_;
//
GtkWidget *gtk_blob::popup_menu_w_;
GtkAccelGroup *gtk_blob::popup_accel_group_;
//
gtk_blob *gtk_blob::info_blob_hit_ = NULL;
gtk_blob *gtk_blob::menu_blob_hit_ = NULL;
int gtk_blob::last_x_ = 0;
int gtk_blob::last_y_ = 0; 
//
int gtk_blob::spline_spacing_ = 4; 
//
bool gtk_blob::mouse_down_ = false;
GtkWindow *gtk_blob::info_w_ = 0;
GtkLabel *gtk_blob::info_color_label_ = 0;
GtkLabel *gtk_blob::info_text_label_ = 0;
//
gtk_blob_splash *gtk_blob::splash_ = NULL;
gtk_blob_base *gtk_blob::base_ = NULL;
GList *gtk_blob::icon_pixbufs_glist_ = NULL;
//
gtk_blob_conf_double *gtk_blob::size_mem_to_screen_scale_conf_ = NULL;
gtk_blob_conf_double *gtk_blob::check_interval_conf_ = NULL;
explicit_change_tracking<double> gtk_blob::check_interval_seconds_(2.0);



// Shouldn't this be a static class variable?
// No.  This way it gets created.
static process_list pl;

extern GnomeUIInfo proc_menu[];


gboolean
gtk_blob::find_canvas_hit(GtkObject *obj, GdkEvent *event, gtk_blob **blob_hit)
{
	*blob_hit = NULL; 

	GnomeCanvasItem *item_hit = NULL;
	bool tried_again = false;

try_again:
	switch (event->type) {
	case GDK_BUTTON_PRESS: 
	case GDK_BUTTON_RELEASE:
		item_hit = gnome_canvas_get_item_at(GNOME_CANVAS(obj), event->button.x, event->button.y);
		break;
	case GDK_MOTION_NOTIFY:
		item_hit = gnome_canvas_get_item_at(GNOME_CANVAS(obj), event->motion.x, event->motion.y);
		break;
	default:
		return FALSE;
	};
	/*
	 * As a side-effect, get rid of the splash,
	 * but not if we're just moving across it.
	 */
	if (splash_->item_is_this_splash(item_hit) &&
		!(event->type == GDK_MOTION_NOTIFY && !mouse_down_)) {
		assert(!tried_again); // catch infinite loops
		splash_->slay();
		goto try_again;  // could be a loop, but that's actually uglier
	};
	/*
	 * With bases, call the base event routine,
	 * which may or may not handle the event for us.
	 */
	if (base_->item_hit_is_base(item_hit)) {
		return base_->event(event, item_hit);
	};
	if (item_hit == NULL)
		return FALSE;
	map<GnomeCanvasItem*,gtk_blob*>::iterator it = 
		item_to_blob_map_.find(item_hit);
	if (it == item_to_blob_map_.end())
		return FALSE;
	*blob_hit = (*it).second;
	return FALSE;
}


void
gtk_blob::shape_window(int w, int h)
{
	static int old_w, old_h;
	if (w == old_w && h == old_h) // already did it
		return;
	if (w == 0 && h == 0) {
		// signal for lozenge change (hack)
		w = old_w;
		h = old_h;
	};


	if (app_ == NULL) {
		// cerr << "gtk_blob::shape_window: way too early\n";
		return;
	};

	GdkWindow *gdk_window = app_->window;
	// could be too early?
	if (gdk_window == NULL) {
		// cerr << "gtk_blob::shape_window: too early\n";
		return;
	};

	static GdkBitmap *bitmap = NULL;
	if (bitmap != NULL && (old_w != w || old_h != h)) {
		// free mis-matched bitmap; will realloc it below
		g_object_unref(bitmap);
		bitmap = NULL;
	};
	if (bitmap == NULL) {
		// cerr << "gtk_blob::shape_window: making " << w << " x " << h <<endl;
		bitmap = gdk_pixmap_new(gdk_window, w, h, 1);
	};
	assert(bitmap != NULL);

	// erase the bitmap
	static GdkGC *white_gc = NULL, *black_gc = NULL;
	if (white_gc == NULL) {
		white_gc = gdk_gc_new(bitmap);
		black_gc = gdk_gc_new(bitmap);
		GdkColor white, black;
		gdk_color_parse(O_("#ffffff"), &white);
		gdk_color_parse(O_("#000000"), &black);
		(void) gdk_colormap_alloc_color(gdk_colormap_get_system(), &white, FALSE, TRUE);
		(void) gdk_colormap_alloc_color(gdk_colormap_get_system(), &black, FALSE, TRUE);
		gdk_gc_set_foreground(white_gc, &white);
		gdk_gc_set_background(white_gc, &black);
		gdk_gc_set_foreground(black_gc, &black);
		gdk_gc_set_background(black_gc, &white);
	};
	assert(white_gc != NULL);
	assert(black_gc != NULL);
        gdk_draw_rectangle(bitmap, black_gc, TRUE, 0, 0, w, h);
//        gdk_draw_rectangle(bitmap, white_gc, TRUE, 20, 20, w-20, h-20);

	// draw the bounding shape
#define NPOINTS 9
	GdkPoint points[NPOINTS];
	double screen_x, screen_y;
#define assign_point(I,X,Y) \
		{ int myi = (I); scale_point((X),(Y), &screen_x, &screen_y); \
		points[myi].x = (gint)screen_x; points[myi].y = (gint)screen_y; }
	int i = 0;
	int buldge_peak_y = Y_MAX * lozenge_y_buldge_peak_pct_ / 100;
	assign_point(i++, 0, 0);
	assign_point(i++, 0, buldge_peak_y);
	assign_point(i++, 0, Y_MAX);
	if (has_base_) {
		assign_point(i++, 0, Y_MAX + BASE_EXTENSION);
		assign_point(i++, X_MAX, Y_MAX + BASE_EXTENSION);
	};
	assign_point(i++, X_MAX, Y_MAX);
	assign_point(i++, X_MAX, buldge_peak_y);
	assign_point(i++, X_MAX, 0);
	assign_point(i++, 0, 0);  // close polygon in paranoia
	assert(i == NPOINTS || i == NPOINTS-2);
	gdk_draw_polygon(bitmap, white_gc, TRUE, points, i);

	// now apply it to the top widget
	gtk_widget_shape_combine_mask(GTK_WIDGET(app_), bitmap, 0, 0);

	old_w = w;
	old_h = h;
}

gboolean
gtk_blob::configure_event(GtkObject *obj, GdkEvent *event, gpointer data)
{
	assert(event->type == GDK_CONFIGURE);
	GdkEventConfigure *config_event = (GdkEventConfigure *)event;
	// assert(config_event->window == GDK_WINDOW(app_));
	int w = config_event->width;
	int h = config_event->height;
	if (w < 1 || h < 1) {
		cerr << _("bogus gtk_blob::configure_event\n");
		return FALSE;
	};
	// fix the canvas
	gnome_canvas_set_scroll_region(GNOME_CANVAS(canvas_w_), 0.0, 0.0,
				       (double)w, (double)h);
	// and figure out how smooth to be
	spline_spacing_ = int(  double(2.0 * min(w, h)) / 100.0);
	spline_spacing_ = max(spline_spacing_, 3);
	spline_spacing_ = min(spline_spacing_, 12);
	// cerr << "spline_spacing_ " << spline_spacing_ << endl;

	// ...and tell the generic blob code
	resize_window(w, h);

	lozenge_need_recompute_ = true;
	shape_window(w, h);
	splash_->update_wh(w, h);
	base_->update_wh(w, h);

	// pass on
	return FALSE;
}

/*
 * Event processing.
 * Sigh, basically I have to roll my own demultiplexing :-(.
 */
gboolean
gtk_blob::blob_event(GtkObject *obj, GdkEvent *event, gpointer data)
{
	gtk_blob *blob_hit;
	// find what we hit, maybe handling the event in the process
	if (find_canvas_hit(obj, event, &blob_hit)) {
		mouse_down_ = false;
		return TRUE;  // already handled
	};

	// ignore non-button events
	switch (event->type) {
	case GDK_BUTTON_PRESS: 
		// menu events first
		// xxx: always runs first, that's bad
		if (event->button.button != 1) {
			return popup_menu_event(obj, (GdkEventButton*)event, blob_hit);
		};
		mouse_down_ = true;
		// fall through
	case GDK_MOTION_NOTIFY:
		if (!mouse_down_) {
			// cerr << "!mouse_down_\n";
			return FALSE;
		};
		return post_info_event(obj, (GdkEventButton*)event, blob_hit);

	case GDK_BUTTON_RELEASE:
		// cerr << "gtk_blob::blob_event: release\n";
		if (info_w_) {
			// cerr << "withdraw it\n";
			gtk_widget_hide(GTK_WIDGET(info_w_));
		};
		info_blob_hit_ = 0;
		mouse_down_ = false;
		return TRUE;

	case GDK_EXPOSE:
		return FALSE;

	case GDK_CONFIGURE:
		assert(0);  // handle configures at the app_ level
		// cerr << "gtk_blob::blob_event configure " << event->type << " " << (unsigned)data << endl;
		return TRUE;

	case GDK_MAP:
	case GDK_ENTER_NOTIFY:
	case GDK_LEAVE_NOTIFY:
	case GDK_FOCUS_CHANGE:
	case GDK_VISIBILITY_NOTIFY:
		/* ignore these */
		return FALSE;

	default:
		// cerr << "gtk_blob::blob_event " << event->type << " " << (unsigned)data << endl;
		return FALSE;
		;
	};
	assert(0);
}

// find_longest_line after http://www.sunsite.ualberta.ca/Documentation/Gnu/libstdc++-2.90.8/html/21_strings/stringtok_std_h.txt
static string 
find_longest_line(string &s)
{
	const string::size_type len = s.length();
	const char delimiter = '\n';
	string::size_type i = 0;
	string champion = "";
	while (i < len) {
		// find eoln
		string::size_type j = s.find_first_of(delimiter, i);
		string challenger;
		if (j == string::npos)
			challenger = s.substr(i);  // no hit
		else challenger = s.substr(i, j - i);
		if (challenger.length() > champion.length())
			champion = challenger;
		if (j == string::npos)  // at eos
			break;
		else i = j + 1; // go to next token
	};
	return champion;
}


void
gtk_blob::setup_info_for_me()
{
	// cerr << "  new blob " << (unsigned)blob_hit << endl;
	assert(info_w_ != NULL);

	// set it up
	// set the text widget to match
	string istr;
	info_to_string(istr);
	istr.erase(istr.find_last_not_of("\n") + 1);
	gtk_label_set_text(info_text_label_, istr.c_str());

	// set the color widget to match
	/*
	 * Hack: I can't (don't know how to) make the label
	 * expand properly, so instead we're going to fill it
	 * with the longest line of text and make fg == bg.
	 */
	string longest_line = find_longest_line(istr);
	string str_color = rgb16_to_string(fg_color_.red,
					   fg_color_.green,
					   fg_color_.blue);
	ostringstream oss;
	oss << O_("<span foreground=\"") << str_color << "\" "
	    << O_("background=\"") << str_color << "\">"
	    << "M" // extra spacing
	    << longest_line
	    << "M"
	    << O_("</span>");
	gtk_label_set_markup(info_color_label_, oss.str().c_str());

	// show it!
	gtk_widget_show_all(GTK_WIDGET(info_w_));
}


gboolean
gtk_blob::post_info_event(GtkObject *obj, GdkEventButton *button, gtk_blob *blob_hit)
{
	bool blob_hit_p = (blob_hit != NULL);

	// if not on a blob, use the last one we hit
	if (!blob_hit_p) {
		if (!info_blob_hit_)
			return FALSE;
		blob_hit = info_blob_hit_;
	};
	assert(blob_hit != NULL);

	// create or change new info window, if necessary
	if (info_blob_hit_ != blob_hit) {
		info_blob_hit_ = blob_hit;
		info_blob_hit_->setup_info_for_me();
	};

	// move info window to right place, if necessary
	int root_x, root_y;
	gdk_window_get_root_origin(button->window, &root_x, &root_y);
	root_x += (int)button->x;
	root_y += (int)button->y;
	if (last_x_ != root_x || last_y_ != root_y) {
		last_x_ = root_x;
		last_y_ = root_y;

		// cerr << "  new x/y " << last_x_ << " " << last_y_ << endl;
		// While nice, gtk_window_set_position centers the window on the mouse, not what we want (since that obscures the blob).
		assert(info_w_ != NULL);

		// handle the case of the window being near the screen edge, approximately
		GdkWindow *info_gtk_w = GTK_WIDGET(info_w_)->window;
		gint info_width, info_height;
		gdk_drawable_get_size(GDK_DRAWABLE(info_gtk_w), &info_width, &info_height);
		const int mouse_offset = 15;
		info_width += mouse_offset;
		info_height += mouse_offset;
		GdkScreen *screen = gdk_drawable_get_screen(GDK_DRAWABLE(info_gtk_w));
		assert(screen != NULL);
		gint screen_width = gdk_screen_get_width(screen);
		gint screen_height = gdk_screen_get_height(screen);
		int tweaked_x = (int)last_x_;
		int tweaked_y = (int)last_y_;
		if (tweaked_x < screen_width - info_width) {
			// on screen
			tweaked_x += mouse_offset;
		} else {
			tweaked_x -= info_width;
		};
		if (tweaked_y < screen_height - info_height) {
			// on screen
			tweaked_y += mouse_offset;
		} else {
			tweaked_y -= info_height;
		};

		gtk_window_move(GTK_WINDOW(info_w_), tweaked_x, tweaked_y);
	};
	return TRUE;
}

gboolean
gtk_blob::popup_menu_event(GtkObject *obj, GdkEventButton *button, gtk_blob *blob_hit)
{
	bool blob_hit_p = (blob_hit != NULL);
	bool desired_menu_sensitivity = false;

	// cerr << "posting menu for " << (unsigned) blob_hit <<endl;

	// configure the details about the hot process
	if (blob_hit_p) {
		// save our target for later
		menu_blob_hit_ = blob_hit;
		// color and name the proc
		GtkWidget *pid_menu_entry = proc_menu[0].widget;
		assert(pid_menu_entry != NULL);
		gchar *quoted_cmd = g_markup_escape_text(blob_hit->pv_->cmd().c_str(), blob_hit->pv_->cmd().length());
		ostringstream oss;
		GdkColor *menu_color = blob_hit->fg_is_whitish_ ?
			&blob_hit->fg_color_ : &blob_hit->bd_color_;
		oss << O_("<span background=\"")
		    << rgb16_to_string(menu_color->red,
				       menu_color->green,
				       menu_color->blue)
		    << O_("\" foreground=\"")
		    << (blob_hit->fg_is_whitish_ ? "#000000" : "#ffffff")
		    << "\">"
		    << "     "
		    << blob_hit->pv_->pid() << ": " << quoted_cmd
		    << "     "
		    << O_("</span>");
		g_free(quoted_cmd);
		gtk_menu_item_set_label_markup(GTK_MENU_ITEM(pid_menu_entry), oss.str().c_str());

		// check menu status
		int me = getuid();
		desired_menu_sensitivity = (me == 0 || blob_hit->pv_->uid() == me);
	} else {
		// not on anything => disable menus
		desired_menu_sensitivity = false;
	};

	// enable or disable the proc menu items
	for (int i = 0; proc_menu[i].type != GNOME_APP_UI_ENDOFINFO; i++) {
		if (proc_menu[i].user_data != NULL) {
			// cerr << "sensitive: " << (unsigned)proc_menu[i].widget << " to " << foo <<endl;
			gtk_widget_set_sensitive(proc_menu[i].widget, desired_menu_sensitivity);
		};
	};

	// pop it up!
	gtk_menu_popup(GTK_MENU(popup_menu_w_), NULL, NULL, NULL, NULL,
		       button->button, button->time);
	return TRUE;
}


void
gtk_blob::send_signal_callback(GtkWidget *widget, char *sig)
{
	assert(menu_blob_hit_ != NULL);
	int pid = menu_blob_hit_->pv_->pid();
	int e;
	if (strcmp(O_("nice"), sig) == 0) {
		e = lava_renice(pid, 5);
	} else if (strcmp(O_("renice"), sig) == 0) {
		e = lava_renice(pid, 5);
	} else {
		e = lava_named_signal(pid, sig);
	};
	if (e != 0) {
		GtkWidget *dialog, *label;
		dialog = gtk_dialog_new_with_buttons(_("LavaPs: Signal Error"),
					    GTK_WINDOW(app_),
					    GTK_DIALOG_DESTROY_WITH_PARENT,
					    GTK_STOCK_OK,
					    GTK_RESPONSE_ACCEPT,
					    NULL);
		ostringstream oss;
		oss << _("\nSignal ")
		    << sig
		    << _(" to process ")
		    << menu_blob_hit_->pv_->pid() << ": "
		    << menu_blob_hit_->pv_->cmd().c_str() << _(" failed.\n");
		label = gtk_label_new(oss.str().c_str());
		g_signal_connect_swapped(GTK_OBJECT(dialog), 
				 O_("response"), 
				 G_CALLBACK(gtk_widget_destroy),
				 GTK_OBJECT (dialog));
		gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox),
				  label);
		gtk_widget_show_all(dialog);
		splash_->splash(_("signal failed"));
	} else {
		// splash message
		splash_->splash(_("signaled"));
	};
}

void
gtk_blob::menu_set_callback(GtkWidget *widget, menu_callback_info *mci)
{
	switch (mci->menu_) {
	case menu_callback_info::WHO_MENU:
		mci->conf_->from_ui();
		assert(filter_style != FILTER_BY_PID); // not supported in gtk mode
		break;
	case menu_callback_info::WHAT_MENU:
		mci->conf_->from_ui();
		break;
	case menu_callback_info::HOW_MENU:
		switch (mci->modifier_) {
		case menu_callback_info::HOW_AUTOSIZE:
			mci->conf_->from_ui();
			break;
		case menu_callback_info::HOW_LOZENGE:
			mci->conf_->from_ui();
			shape_window(0, 0);  // sort of hack
			break;
		case menu_callback_info::HOW_BASE:
			mci->conf_->from_ui();
			shape_window(0, 0);  // sort of hack
			break;
		case menu_callback_info::HOW_STICKY:
			mci->conf_->from_ui();
			if (lava_wm_sticky)
				gtk_window_stick(GTK_WINDOW(app_));
			else gtk_window_unstick(GTK_WINDOW(app_));
			break;
		case menu_callback_info::HOW_SHRINK:
			if (allow_autosize)
				process_view::mem_target_adjust(-1);
			else process_view::mem_adjust(1);
			break;
		case menu_callback_info::HOW_GROW:
			if (allow_autosize)
				process_view::mem_target_adjust(1);
			else process_view::mem_adjust(-1);
			break;
		case menu_callback_info::HOW_SMOOTHER:
			check_interval_seconds_.tick_scale(M_SQRT1_2);
			break;
		case menu_callback_info::HOW_JUMPIER:
			check_interval_seconds_.tick_scale(M_SQRT2);
			break;
		default:
			assert(0);
		};
		break;
	default:
		assert(0);
	};
	const char *msg = mci->desc_;
	if (mci->modifier_ == menu_callback_info::HOW_BASE &&
	    !has_base_ &&
	    (lava_random(2) == 0)) {
		// We're having fun now!
		msg = O_("all your base\nare belong to us\n");
	};

	if (mci->desc_)
		splash_->splash(_(msg)); // translate it here!
}

void
gtk_blob::menu_help_callback(GtkWidget *widget, char *details)
{
	static gtk_lava_help_text glht_error(O_("error"), _("Internal help system error.\n"), _("Error"));  // fallback
	gtk_lava_help_text *glht = gtk_lava_help_text::find_by_key(details);
	if (glht == NULL)
		glht = &glht_error;

	GtkWidget *dialog, *label;
	dialog = gtk_dialog_new_with_buttons(glht->title(),
					     GTK_WINDOW(app_),
					     GTK_DIALOG_DESTROY_WITH_PARENT,
					     GTK_STOCK_OK,
					     GTK_RESPONSE_ACCEPT,
					     NULL);
	label = gtk_label_new(NULL);
	// make it wider than it might otherwise be
	// xxx: 500 is bad magic number
        gtk_widget_set_size_request(label, 500, -1);
	gtk_label_set_line_wrap(GTK_LABEL(label), TRUE);
	gtk_label_set_markup(GTK_LABEL(label), glht->body());
	g_signal_connect_swapped(GTK_OBJECT(dialog), 
				 O_("response"), 
				 G_CALLBACK(gtk_widget_destroy),
				 GTK_OBJECT (dialog));
	gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox),
				  label);
	gtk_widget_show_all(dialog);
}


// static
gboolean
gtk_blob::tick(gpointer data)
{
	// xxx: handle raising splash
	// cerr << "tick\n";
	pl.scan();
	blob::flush_drawings();
	// implement any change to check frequency
	if (!check_interval_seconds_.changed())
		return TRUE;  // no changes
	// save it!
	check_interval_conf_->to_conf();
	(void) g_timeout_add((int)(check_interval_seconds_.current() * 1000.0), // timeout in ms
				     gtk_blob::tick,
				     NULL);
	check_interval_seconds_.commit();  // remember we've changed it
	return FALSE;  // we've rescheduled ourselves

}

/*
 * initialization/setup
 */

gtk_blob::gtk_blob(process_view *pv) :
	scaled_blob(pv), group_(NULL), id_(0), did_(0), did_shown_(true)
{
	ENTRY_TRACE(__FILE__,__LINE__);
	// xxx: assume blob initializer called first!
	gtk_blobs_.push_front(this);
	me_in_blobs_ = gtk_blobs_.begin();
	assert(me_in_blobs_ != gtk_blobs_.end());
	group_ = (GnomeCanvasGroup*) 
		gnome_canvas_item_new(gnome_canvas_root(GNOME_CANVAS(canvas_w_)),
				       gnome_canvas_group_get_type(),
				       NULL);
	// start with SOME points
	GnomeCanvasPoints *points = gnome_canvas_points_new(3);
	points->coords[0] = 0.0;
	points->coords[1] = 0.0;
	points->coords[2] = 1.0;
	points->coords[3] = 1.0;
	points->coords[4] = 1.0;
	points->coords[5] = 0.0;
	id_ = gnome_canvas_item_new(group_,
			      gnome_canvas_polygon_get_type(),
			      O_("points"), points,
			      O_("fill_color"), O_("blue"),
			      O_("width_units"), (double)1.0,
			      NULL);
	assert(id_ != 0);
	item_to_blob_map_[id_] = this;
	did_ = gnome_canvas_item_new(group_,
			      gnome_canvas_polygon_get_type(),
			      O_("points"), points,
			      O_("fill_color"), O_("blue"),
			      O_("width_units"), (double)0.0,
			      NULL);
	assert(did_ != 0);
	did_shown_ = true;
	item_to_blob_map_[did_] = this;
	gnome_canvas_points_free(points);

	// do binding
	// no binding, we dispatch everything at the canvas level


	// xxx: should also make the shadow
	schedule_redraw();
}

gtk_blob::~gtk_blob()
{
	ENTRY_TRACE(__FILE__,__LINE__);

	gtk_blobs_.erase(me_in_blobs_);

	// gtk-canvas-xxx: is this really the only way to destroy
	// something?  surely one can destroy canvas objects, too?
	if (group_)
		gtk_object_destroy(GTK_OBJECT(group_));
	if (id_ != 0)
		item_to_blob_map_.erase(id_);
	if (did_ != 0)
		item_to_blob_map_.erase(did_);
	id_ = 0;
	did_ = 0;
}

// static
void
gtk_blob::init_canvas()
{
	/* make the canvas */
	canvas_w_ = gnome_canvas_new();
	GdkColor bg_color;
	gdk_color_parse("#000000", &bg_color);
	gtk_widget_modify_bg(canvas_w_, GTK_STATE_NORMAL, &bg_color);
	// xxx
	gnome_canvas_set_scroll_region(GNOME_CANVAS(canvas_w_), 0.0, 0.0,
				       (double) CANVAS_DEFAULT_WIDTH,
				       (double) CANVAS_DEFAULT_HEIGHT);
	// xxx: should read default size
	gtk_widget_set_usize(canvas_w_, CANVAS_DEFAULT_WIDTH, CANVAS_DEFAULT_HEIGHT);
	gtk_container_set_resize_mode(GTK_CONTAINER(canvas_w_), GTK_RESIZE_PARENT);
	// link canvas to app window
	gnome_app_set_contents(GNOME_APP(app_), canvas_w_);

	// trap resize events?
	g_signal_connect(GTK_OBJECT(app_), O_("configure_event"),
			   GTK_SIGNAL_FUNC(gtk_blob::configure_event),
			   NULL);

	// xxx: how to trap resize events?
	// trap button presses
	//   Sigh, in tcl binding lets us select events,
	//   but here we have to do it in the callback.
	// xxx: what's the diff between _set_events and the 2nd arg of connect
	g_signal_connect_after(GTK_OBJECT(canvas_w_), O_("event"),
			   GTK_SIGNAL_FUNC(gtk_blob::blob_event),
			   NULL);
}

void
gtk_blob::init_info()
{
	const int spacing_size = 5;  // xxx: should be about 1ex width

	assert(info_w_ == 0);
	info_w_ = (GtkWindow*)gtk_window_new(GTK_WINDOW_POPUP);
	gtk_window_set_title(info_w_, _("LavaPs Info"));
	// in spite of the manual for gtk_window_set_policy,
	// we want allow_shrink, because our internal widgets will react.
	gtk_window_set_resizable(info_w_, FALSE);
	gtk_window_set_transient_for(info_w_, GTK_WINDOW(app_));  // xxx do this?
	gtk_window_set_decorated(info_w_, FALSE);  //  xxx needed?
	gtk_window_set_gravity(info_w_, GDK_GRAVITY_STATIC);

	gtk_container_set_border_width(GTK_CONTAINER(info_w_), spacing_size / 2);

	// bypass the signals (do I need to do this?)
	g_signal_connect (GTK_OBJECT(info_w_), O_("delete-event"), 
			  G_CALLBACK (gtk_true), NULL);
	g_signal_connect (GTK_OBJECT(info_w_), O_("destroy"), 
			  G_CALLBACK (gtk_true), NULL);

	GtkWidget *vbox_parent_w = GTK_WIDGET(info_w_);
#if 1
	// put a frame in it
	GtkWidget *frame_w = gtk_frame_new(NULL);
	gtk_frame_set_shadow_type(GTK_FRAME(frame_w), GTK_SHADOW_OUT);
	gtk_container_add(GTK_CONTAINER(info_w_), GTK_WIDGET(frame_w));
	vbox_parent_w = frame_w;
#endif


	// populate it with its pieces: color bar and text
	GtkBox *vbox = (GtkBox*)gtk_vbox_new(FALSE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 0);
	// gtk_container_add(GTK_CONTAINER(frame_w), GTK_WIDGET(vbox));
	gtk_container_add(GTK_CONTAINER(vbox_parent_w), GTK_WIDGET(vbox));

	info_color_label_ = (GtkLabel*)gtk_label_new(O_("placeholder"));
	// changing from label to button shows proper resizing behavior
	// info_color_label_ = (GtkLabel*)gtk_button_new_with_label("placeholder");
	gtk_label_set_selectable(info_color_label_, FALSE);
	// will set color later
	// gtk_box_pack_start_defaults(vbox, GTK_WIDGET(info_color_label_));
	gtk_box_pack_start(vbox, GTK_WIDGET(info_color_label_), TRUE, TRUE, 0);

	info_text_label_ = (GtkLabel*)gtk_label_new(O_("trial"));
	gtk_label_set_selectable(info_color_label_, TRUE);
	gtk_box_pack_start_defaults(vbox, GTK_WIDGET(info_text_label_));

	// don't show it yet
}

GnomeUIInfo proc_menu[] = {
	GNOMEUIINFO_ITEM_DATA(N_("no process selected"), NULL, NULL, NULL, NULL),
	GNOMEUIINFO_SEPARATOR,
	GNOMEUIINFO_ITEM_DATA(N_("_Nice"), NULL, gtk_blob::send_signal_callback, (void*)O_("nice"), NULL),
	GNOMEUIINFO_ITEM_DATA(N_("_Unnice"), NULL, gtk_blob::send_signal_callback, (void*)O_("unnice"), NULL),
	GNOMEUIINFO_SEPARATOR,
	GNOMEUIINFO_ITEM_DATA(N_("_Terminate (politely)"), NULL, gtk_blob::send_signal_callback, (void*)O_("TERM"), NULL),
	GNOMEUIINFO_ITEM_DATA(N_("_Kill (forcefully)"), NULL, gtk_blob::send_signal_callback, (void*)O_("KILL"), NULL),
	GNOMEUIINFO_ITEM_DATA(N_("_Stop (suspend)"), NULL, gtk_blob::send_signal_callback, (void*)O_("STOP"), NULL),
	GNOMEUIINFO_ITEM_DATA(N_("_Continue (unsuspend)"), NULL, gtk_blob::send_signal_callback, (void*)O_("CONT"), NULL),
	GNOMEUIINFO_ITEM_DATA(N_("_Hang up"), NULL, gtk_blob::send_signal_callback, (void*)O_("HUP"), NULL),
	GNOMEUIINFO_END
};

static menu_callback_info who_me_mci(menu_callback_info::WHO_MENU, menu_callback_info::WHO_ME, N_("my jobs\n"));
static menu_callback_info who_everyone_mci(menu_callback_info::WHO_MENU, menu_callback_info::WHO_EVERYONE, N_("all jobs\n"));

static GnomeUIInfo who_menu_radio[] = {
	GNOMEUIINFO_RADIOITEM_DATA(N_("_Me"), NULL, gtk_blob::menu_set_callback, (void*)&who_me_mci, NULL),
	GNOMEUIINFO_RADIOITEM_DATA(N_("_Everyone"), NULL, gtk_blob::menu_set_callback, (void*)&who_everyone_mci, NULL),
	GNOMEUIINFO_END
};

static GnomeUIInfo who_menu[] = {
	GNOMEUIINFO_RADIOLIST(who_menu_radio),
	GNOMEUIINFO_END
};	

static menu_callback_info what_virt_mci(menu_callback_info::WHAT_MENU, menu_callback_info::WHAT_VIRTUAL, N_("virtual mem\n"));
static menu_callback_info what_phys_mci(menu_callback_info::WHAT_MENU, menu_callback_info::WHAT_PHYSICAL, N_("physical mem\n"));
static menu_callback_info what_both_mci(menu_callback_info::WHAT_MENU, menu_callback_info::WHAT_BOTH, N_("both mem\n"));

static GnomeUIInfo what_menu_radio[] = {
	GNOMEUIINFO_RADIOITEM_DATA(N_("_Virtual Memory"), NULL, gtk_blob::menu_set_callback, (void*)&what_virt_mci, NULL),
	GNOMEUIINFO_RADIOITEM_DATA(N_("_Physical Memory"), NULL, gtk_blob::menu_set_callback, (void*)&what_phys_mci, NULL),
	GNOMEUIINFO_RADIOITEM_DATA(N_("_Both"), NULL, gtk_blob::menu_set_callback, (void*)&what_both_mci, NULL),
	GNOMEUIINFO_END
};

static GnomeUIInfo what_menu[] = {
	GNOMEUIINFO_RADIOLIST(what_menu_radio),
	GNOMEUIINFO_END
};	

static menu_callback_info how_autosize_mci(menu_callback_info::HOW_MENU, menu_callback_info::HOW_AUTOSIZE, N_("toggle autosizing\n"));
static menu_callback_info how_shrink_mci(menu_callback_info::HOW_MENU, menu_callback_info::HOW_SHRINK, N_("shrink mem\n"));
static menu_callback_info how_grow_mci(menu_callback_info::HOW_MENU, menu_callback_info::HOW_GROW, N_("grow mem\n"));
static menu_callback_info how_smoother_mci(menu_callback_info::HOW_MENU, menu_callback_info::HOW_SMOOTHER, N_("smoother\n"));
static menu_callback_info how_jumpier_mci(menu_callback_info::HOW_MENU, menu_callback_info::HOW_JUMPIER, N_("jumpier\n"));
static menu_callback_info how_lozenge_mci(menu_callback_info::HOW_MENU, menu_callback_info::HOW_LOZENGE, N_("toggle lozenge\n"));
static menu_callback_info how_base_mci(menu_callback_info::HOW_MENU, menu_callback_info::HOW_BASE, N_("toggle base\n"));
static menu_callback_info how_sticky_mci(menu_callback_info::HOW_MENU, menu_callback_info::HOW_STICKY, N_("toggle stickiness\n"));

static GnomeUIInfo how_menu[] = {
	GNOMEUIINFO_TOGGLEITEM_DATA(N_("_Autosizing"), NULL, gtk_blob::menu_set_callback, (void*)&how_autosize_mci, NULL),
	GNOMEUIINFO_ITEM_DATA(N_("_Shrink"), NULL, gtk_blob::menu_set_callback, (void*)&how_shrink_mci, NULL),
	GNOMEUIINFO_ITEM_DATA(N_("_Grow"), NULL, gtk_blob::menu_set_callback, (void*)&how_grow_mci, NULL),
	GNOMEUIINFO_SEPARATOR,
	GNOMEUIINFO_ITEM_DATA(N_("S_moother"), NULL, gtk_blob::menu_set_callback, (void*)&how_smoother_mci, NULL),
	GNOMEUIINFO_ITEM_DATA(N_("_Jumpier"), NULL, gtk_blob::menu_set_callback, (void*)&how_jumpier_mci, NULL),
	GNOMEUIINFO_SEPARATOR,
	GNOMEUIINFO_TOGGLEITEM_DATA(N_("_Lozenge"), NULL, gtk_blob::menu_set_callback, (void*)&how_lozenge_mci, NULL),
	GNOMEUIINFO_TOGGLEITEM_DATA(N_("_Based"), NULL, gtk_blob::menu_set_callback, (void*)&how_base_mci, NULL),
	GNOMEUIINFO_TOGGLEITEM_DATA(N_("_Sticky"), NULL, gtk_blob::menu_set_callback, (void*)&how_sticky_mci, NULL),
	GNOMEUIINFO_END
};

static GnomeUIInfo help_menu[] = {
	GNOMEUIINFO_ITEM_DATA(N_("_About..."), NULL, gtk_blob::menu_help_callback, (void*)O_("about"), NULL),
	GNOMEUIINFO_ITEM_DATA(N_("_Basics..."), NULL, gtk_blob::menu_help_callback, (void*)O_("basics"), NULL),
	GNOMEUIINFO_ITEM_DATA(N_("_Menus..."), NULL, gtk_blob::menu_help_callback, (void*)O_("menus"), NULL),
//	GNOMEUIINFO_ITEM_DATA("_Resources...", NULL, gtk_blob::menu_help_callback, (void*)"resources", NULL),
//	GNOMEUIINFO_ITEM_DATA("_How do I...", NULL, gtk_blob::menu_help_callback, (void*)"howdoi", NULL),
	GNOMEUIINFO_ITEM_DATA(N_("_Copyright..."), NULL, gtk_blob::menu_help_callback, (void*)O_("copyright"), NULL),
	GNOMEUIINFO_END
};

static GnomeUIInfo popup_menu[] = {
	GNOMEUIINFO_SUBTREE(N_("_Proc"), proc_menu),
	GNOMEUIINFO_SUBTREE(N_("_Who"), who_menu),
	GNOMEUIINFO_SUBTREE(N_("Wh_at"), what_menu),
	GNOMEUIINFO_SUBTREE(N_("H_ow"), how_menu),
	GNOMEUIINFO_MENU_HELP_TREE(help_menu),
	GNOMEUIINFO_SEPARATOR,
	GNOMEUIINFO_MENU_QUIT_ITEM(GTK_SIGNAL_FUNC(gtk_main_quit), NULL),
	GNOMEUIINFO_END
};

// static
void
gtk_blob::init_menu()
{
	popup_menu_w_ = gtk_menu_new();
	popup_accel_group_ = gtk_accel_group_new();
	gnome_app_fill_menu((GtkMenuShell*)popup_menu_w_, popup_menu, popup_accel_group_, TRUE, 0);
	gtk_window_add_accel_group(GTK_WINDOW(app_), popup_accel_group_);
}

// static
void
gtk_blob::init_conf()
{
	// who
	static list<gtk_blob_conf_enum_mapping*> who_mapping;
	who_mapping.push_back(new gtk_blob_conf_enum_mapping(FILTER_NOTHING,
							     O_("everyone"),
							     GTK_RADIO_MENU_ITEM(who_menu_radio[1].widget)));
	who_mapping.push_back(new gtk_blob_conf_enum_mapping(FILTER_BY_UID,
							     O_("me"),
							     GTK_RADIO_MENU_ITEM(who_menu_radio[0].widget)));
	who_me_mci.conf_ =
		new gtk_blob_conf_enum((int*)&filter_style,
				       (int)FILTER_BY_UID,
				       O_("/apps/lavaps/who"),
				       &who_mapping);
	who_everyone_mci.conf_ = 
		new gtk_blob_conf_enum((int*)&filter_style,
				       (int)FILTER_NOTHING,
				       O_("/apps/lavaps/who"),
				       &who_mapping);

	// what
	static list<gtk_blob_conf_enum_mapping*> what_mapping;
	what_mapping.push_back(new gtk_blob_conf_enum_mapping(BOTH_MEM,
							     O_("both"),
							     GTK_RADIO_MENU_ITEM(what_menu_radio[2].widget)));
	what_mapping.push_back(new gtk_blob_conf_enum_mapping(VIRTUAL_MEM,
							     O_("virtual"),
							     GTK_RADIO_MENU_ITEM(what_menu_radio[0].widget)));
	what_mapping.push_back(new gtk_blob_conf_enum_mapping(PHYSICAL_MEM,
							     O_("physical"),
							     GTK_RADIO_MENU_ITEM(what_menu_radio[1].widget)));
	what_virt_mci.conf_ =
		new gtk_blob_conf_enum((int*)(report_vm.address()),
				       (int)VIRTUAL_MEM,
				       O_("/apps/lavaps/what"),
				       &what_mapping);
	what_phys_mci.conf_ =
		new gtk_blob_conf_enum((int*)(report_vm.address()),
				       (int)PHYSICAL_MEM,
				       O_("/apps/lavaps/what"),
				       &what_mapping);
	what_both_mci.conf_ =
		new gtk_blob_conf_enum((int*)(report_vm.address()),
				       (int)BOTH_MEM,
				       O_("/apps/lavaps/what"),
				       &what_mapping);

	// others
	how_autosize_mci.conf_ = 
		new gtk_blob_conf_bool(&allow_autosize, true,
				       O_("/apps/lavaps/autosize"), 
				       GTK_CHECK_MENU_ITEM(how_menu[0].widget));
	how_lozenge_mci.conf_ = 
		new gtk_blob_conf_bool(&lozenge_shaped_, true,
				       O_("/apps/lavaps/lozenge"), 
				       GTK_CHECK_MENU_ITEM(how_menu[7].widget));
	how_base_mci.conf_ = 
		new gtk_blob_conf_bool(&has_base_, true,
				       O_("/apps/lavaps/base"), 
				       GTK_CHECK_MENU_ITEM(how_menu[8].widget));
	how_sticky_mci.conf_ = 
		new gtk_blob_conf_bool(&lava_wm_sticky, true,
				       O_("/apps/lavaps/sticky"), 
				       GTK_CHECK_MENU_ITEM(how_menu[9].widget));
	// some non-GUI things
	size_mem_to_screen_scale_conf_ = new gtk_blob_conf_double(process_view::size_mem_to_screen_scale_address(),
						   1.0, 1e-8, 1e8, O_("/apps/lavaps/size_mem_to_screen_scale"));
	check_interval_conf_ = new gtk_blob_conf_double(check_interval_seconds_.address(),
						       2.0, 0.01, 10.0,
						       O_("/apps/lavaps/check_interval"));
}


// static
void
gtk_blob::init_ticker()
{
	// int event_id = 
	(void) g_timeout_add((int)(check_interval_seconds_.current() * 1000.0), // timeout in ms
				     gtk_blob::tick,
				     NULL);
}

// static
void
gtk_blob::init_icons()
{
	extern const char **icon_xpms[];
	const char ***xpms = &icon_xpms[0];
	assert(icon_pixbufs_glist_ == NULL);
	int num_icons = 0;
	while (*xpms != NULL) {
		GdkPixbuf *pixbuf = gdk_pixbuf_new_from_xpm_data(*xpms);
		assert(pixbuf != NULL);
		icon_pixbufs_glist_ = g_list_append(icon_pixbufs_glist_, pixbuf);
		xpms++;
		num_icons++;
	};
	assert(num_icons > 0);
	gtk_window_set_default_icon_list(icon_pixbufs_glist_);
}

// static
void
gtk_blob::init(int argc, char **argv)
{
	/* gettext first */
	setlocale(LC_ALL, "");  // set up locale based on env vars
	if (NULL == bindtextdomain(GETTEXT_PACKAGE, GNOMELOCALEDIR))
		cerr << _("warning: bindtextdomain failed.\n");
	if (NULL == bind_textdomain_codeset(GETTEXT_PACKAGE, O_("UTF-8")))
		cerr << _("warning: bind_textdomain_codeset failed.\n");
	if (NULL == textdomain(GETTEXT_PACKAGE))
		cerr << _("warning: textdomain failed.\n");

	/* init gnome and make the window */
	program_ = gnome_program_init(O_("lavaps"), "2.0",
				  LIBGNOMEUI_MODULE,
	                          argc, argv,
				  GNOME_PARAM_POPT_TABLE, popt_options,
				  GNOME_PARAM_NONE);
	app_ = gnome_app_new(O_("lavaps"), _("LavaPS"));
	/*
	 * Set up minimum size (otherise gtk's default refuses to let
	 * it be shrunk.
	 */
	GdkGeometry geometry;
	geometry.min_width = geometry.min_height = (2*5*16) / 2;
	gtk_window_set_geometry_hints(GTK_WINDOW(app_), GTK_WIDGET(app_),
				      &geometry, GDK_HINT_MIN_SIZE);
	gtk_window_set_decorated(GTK_WINDOW(app_), FALSE);  // get rid of title
	if (lava_wm_sticky)
		gtk_window_stick(GTK_WINDOW(app_));
	else gtk_window_unstick(GTK_WINDOW(app_));
	g_signal_connect(GTK_OBJECT(app_), O_("delete_event"),
			   GTK_SIGNAL_FUNC(gtk_main_quit),
			   NULL);

	init_icons();

	init_canvas();
	assert(canvas_w_ != NULL);
	splash_ = new gtk_blob_splash(canvas_w_);
	assert(splash_ != NULL);
	base_ = new gtk_blob_base(app_, canvas_w_);
	assert(base_ != NULL);
	init_menu();
	init_conf();
	init_info();

	gtk_widget_show_all(GTK_WIDGET(app_));

	if (lava_default_geometry) {
		if (!gtk_window_parse_geometry(GTK_WINDOW(app_), lava_default_geometry))
			cerr << _("warning: could not parse --geometry option: ") << lava_default_geometry << endl;
	};

	init_ticker();
}

void
gtk_blob::mainloop()
{
	gtk_main();
}

void
gtk_blob::splash(const char *msg)
{
	splash_->splash(msg);
}

void
gtk_blob::set_point(GnomeCanvasPoints *points, int i, double x, double y)
{
	scale_point(x, y, &x, &y);  // convert to scren coords
	points->coords[2*i] = x;
	points->coords[2*i+1] = y;
}

GnomeCanvasPoints *
gtk_blob::set_points(int pct, int offset)
	/*
	 * Return a points array 
	 * for the blob.  Scale y-values by PCT.
	 * OFFSET is not used.
	 */
{
	int max_points = max(4, 2*num_ + 1);
	GnomeCanvasPoints *points = gnome_canvas_points_new(max_points);
	int i, npoints = 0;
	double x, y;

	// Scratch space for scaling.
	static double *ranges = NULL, *midpoints = NULL;
	static int len = 0;
	if (len < num_) {
		delete[] ranges;
		delete[] midpoints;
		ranges = new double[num_];
		midpoints = new double[num_];
		len = num_;
	};

	for (i = 0, x = x_; i < num_; i++) {
		if (pct != 100) {
			/*
			 * Scale things: occupy PCT of the space
			 * around the center.
			 *
			 * Note: DON'T scale the first point (i==0)
			 * to ensure that we have a closed polygon.
			 */
			ranges[i] = y_highs_[i] - y_lows_[i];
			midpoints[i] = y_lows_[i] + ranges[i] / 2.0;
			y = midpoints[i] - (ranges[i] * pct) / 200.0;
			if (y <= y_lows_[i])
				y += 1; // pixel_size_;
		};
		if (pct == 100 || i == 0) {
			y = y_lows_[i];
		};
		// if (pct != 100 && this->pv_->pid() == 1910) cerr << "low: " << y_lows_[i] << "->" << y << endl;
		set_point(points, npoints++, x, y);
		x += x_step_;
	};
	for (; --i >= 0; ) {
		if (pct != 100) {
			y = midpoints[i] + (ranges[i] * pct) / 200.0;
			if (y >= y_highs_[i])
				y -= 1; //pixel_size_;
			if (y_lows_[i] > y_highs_[i])
				swap(y_lows_[i], y_highs_[i]);
		};
		if (pct == 100 || i == 0) {
			y = y_highs_[i];
		};
		x -= x_step_;
		set_point(points, npoints++, x, y);
	};
	// must always have 3 points
	if (num_ == 1) {
		// make one up
		x = x_ + x_step_ / 2 * (right_tending_ ? 1 : -1);
		int y = (y_highs_[0] - y_lows_[0]) / 2 + y_lows_[0];
		set_point(points, npoints++, x, y);
	};
	// finally, close it
	set_point(points, npoints++, (double)x_, y_lows_[0]);

	/*
	 * Now smooth them.
	 */
#if 1
	GnomeCanvasPoints *smooth_points =
		gnome_canvas_points_smooth(points, spline_spacing_);
	gnome_canvas_points_free(points);

	return smooth_points;
#else
	return points;
#endif
}

void
gtk_blob::redraw()
{
	/*
	 * first, program the points
	 */
	GnomeCanvasPoints *points = set_points();
	gnome_canvas_item_set(id_,
			      O_("points"), points,
			      NULL);
	gnome_canvas_points_free(points);
	// shading
	if (dark_percentage_) {
		if (!did_shown_) {
			gnome_canvas_item_show(did_);
			did_shown_ = true;
		};
		GnomeCanvasPoints *dark_points = set_points(100 - dark_percentage_, 50);
		assert(did_ != 0);
		gnome_canvas_item_set(did_,
				      O_("points"), dark_points,
				      NULL);
		gnome_canvas_points_free(dark_points);
	} else {
		if (did_shown_) {
			gnome_canvas_item_hide(did_);
			did_shown_ = false;
		};
	};
	/*
	 * now set the correct z-stacking
	 */
	blob *superior_blob = superior();
	GnomeCanvasItem *superior_id = superior_blob ? ((gtk_blob*)superior_blob)->id_ : NULL;
	if (superior_id) {
		// put is below our superior
		gnome_canvas_item_lower_below(did_, superior_id);
		gnome_canvas_item_lower_below(id_, did_);
	} else {
		// we're on top
		gnome_canvas_item_raise_to_top(id_);
		gnome_canvas_item_raise_to_top(did_);
	};
}

static void
hsb_to_gdk(double h, double s, double b, GdkColor *color)
{
	int red, green, blue; 	// copy because they're not int's in GdkColor
	hsb_to_rgb(h, s, b, 0xffff, &red, &green, &blue);
	color->red = red;
	color->green = green;
	color->blue = blue;
	(void) gdk_colormap_alloc_color(gdk_colormap_get_system(),
					color, FALSE, TRUE);
}

void
gtk_blob::set_hsb(double h, double s, double b)
{
	blob::set_hsb(h, s, b);

	hsb_to_gdk(H_, S_, B_, &fg_color_);
	fg_is_whitish_ = (B_ > 0.5);  // remember so we can tweak menu fg
	hsb_to_gdk(H_, S_, 1.0 - B_, &bd_color_);
	gnome_canvas_item_set(id_,
			      O_("fill_color_gdk"), &fg_color_,
			      O_("outline_color_gdk"), &bd_color_,
			      NULL);

	if (did_ != 0) {
		hsb_to_gdk(dk_H_, dk_S_, dk_B_, &dk_color_);
		gnome_canvas_item_set(did_,
			      O_("fill_color_gdk"), &dk_color_,
			      NULL);
	};

	schedule_redraw();
}

void
gtk_blob::size_divisor_commit()
{
	size_mem_to_screen_scale_conf_->to_conf();
}

#endif /* USE_GTK_BLOB */
