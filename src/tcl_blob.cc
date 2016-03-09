
/*
 * tcl_blob.cc
 * Copyright (C) 1999-2003 by John Heidemann
 * $Id: tcl_blob.cc,v 1.63 2003/12/02 01:11:32 johnh Exp $
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

#include <signal.h>

#include <tcl.h>
#include <tk.h>

#include <iostream>
#include <algorithm>
#include <string>

#include "main.hh"
#include "tcl_blob.hh"
#include "process_list.hh"
#include "lava_signal.hh"

extern "C" {
	extern const char *lava_code;
};


/*
 * hook blob proper into tcl_blob
 */
blob *
blob::create_real_blob(process_view *pv)
{
	ENTRY_TRACE(__FILE__,__LINE__);
	return (blob*) new tcl_blob(pv);
}
void
blob::splash(const char *msg)
{
	tcl_blob::splash(msg);
}
void
blob::init(int argc, char **argv)
{
	tcl_blob::init(argc, argv);
}
void
blob::mainloop()
{
	tcl_blob::mainloop();
}
void
blob::size_divisor_commit()
{
	// ignore it
}





// static stuff
Tcl_Interp *tcl_blob::interp_ = NULL;
char *tcl_blob::canvas_ = NULL;
tcl_blob::tcl_blobs_t tcl_blob::tcl_blobs_;
char tcl_blob::result_buf_[TCL_BUF_LEN];

int tcl_blob::pixel_size_ = 1;

int tcl_blob::frame_window_id_ = 0;

static process_list pl;


// static
int
tcl_blob::lava_tick_proc(ClientData clientData, Tcl_Interp *interp, int argc, const char *argv[])
{
	ENTRY_TRACE(__FILE__,__LINE__);
	if (argc != 1) {
		Tcl_SetResult(interp, _("wrong # args: should be \"lava_tick_\""), NULL);
		return TCL_ERROR;
	};
	pl.scan();
	blob::flush_drawings();
	return TCL_OK;
}

// static
int
tcl_blob::lava_dump_proc(ClientData clientData, Tcl_Interp *interp, int argc, const char *argv[])
{
	ENTRY_TRACE(__FILE__,__LINE__);
	pl.dump();
	return TCL_OK;
}

// static
int
tcl_blob::lava_resize_proc(ClientData clientData, Tcl_Interp *interp, int argc, const char *argv[])
{
	ENTRY_TRACE(__FILE__,__LINE__);
	if (! (argc == 1 || argc == 3 || argc == 4)) {
		Tcl_SetResult(interp, _("wrong # args: should be \"lava_resize [w h [frameWindow]]\""), NULL);
		return TCL_ERROR;
	};
	static int old_w, old_h;
	int w, h;
	if (argc == 1) {
		// suppress calls to this routine before there's a default
		if (old_w == 0 || old_h == 0)
			return TCL_OK;
		w = old_w;
		h = old_h;
	} else {
		old_w = w = atoi(argv[1]);
		old_h = h = atoi(argv[2]);
	};

	if (argc >= 3)
		frame_window_id_ = strtol(argv[3], NULL, 16);
	resize_window(w, h);
	shape_window();

	double w_scale = x_scale_, h_scale = y_scale_;
	if (!vertical_)
		swap(w_scale, h_scale);
	snprintf(result_buf_, TCL_BUF_LEN, "%f %f",  w_scale, h_scale);
	Tcl_SetResult(interp, result_buf_, NULL);
	return TCL_OK;
}


// static
int
tcl_blob::lava_version_proc(ClientData clientData, Tcl_Interp *interp, int argc, const char *argv[])
{
	ENTRY_TRACE(__FILE__,__LINE__);
	strncpy(result_buf_, VERSION, TCL_BUF_LEN);
	Tcl_SetResult(interp, result_buf_, NULL);
	return TCL_OK;
}

// static
int
tcl_blob::lava_default_resources_proc(ClientData clientData, Tcl_Interp *interp, int argc, const char *argv[])
{
	ENTRY_TRACE(__FILE__,__LINE__);
	strncpy(result_buf_, default_resources.c_str(), TCL_BUF_LEN);
	Tcl_SetResult(interp, result_buf_, NULL);
	return TCL_OK;
}


// xxx: probably a better way to do this using bind2nd or something,
// but that requires too many brain cells.
struct blobs_find_id_p : public unary_function<tcl_blob *,bool> {
	blobs_find_id_p(int id) : id_(id) {};
	int id_;
	bool operator()(tcl_blob *p) { return p->id_ == id_ || p->did_ == id_; };
};

// static
int
tcl_blob::lava_id_to_info_proc(ClientData clientData, Tcl_Interp *interp, int argc, const char *argv[])
{
	ENTRY_TRACE(__FILE__,__LINE__);
	if (argc != 2) {
		Tcl_SetResult(interp, _("wrong # args: should be \"lava_id_to_info id\""), NULL);
		return TCL_ERROR;
	};
	int id = atoi(argv[1]);
	// find it
	tcl_blobs_t::iterator result = find_if(tcl_blobs_.begin(), tcl_blobs_.end(), blobs_find_id_p(id));
	if (result == tcl_blobs_.end()) {
		Tcl_SetResult(interp, _("cannot find id"), NULL);
		return TCL_ERROR;
	};
	// copy result
	string s;
	(*result)->info_to_string(s);
	assert(s.length() < TCL_BUF_LEN);
	strcpy(result_buf_, s.c_str());
	Tcl_SetResult(interp, result_buf_, NULL);
	return TCL_OK;
}

// static
int
tcl_blob::lava_menu_proc(ClientData clientData, Tcl_Interp *interp, int argc, const char *argv[])
{
	ENTRY_TRACE(__FILE__,__LINE__);
	const char *key, *value, *value2;  // before goto to satisfy gcc 2.95.2
	if (argc < 2)
		goto wrong_number_args;
	key = argv[1];
	value = (argc >= 3 ? argv[2] : NULL);
	value2 = (argc >= 4 ? argv[3] : NULL);
	if (strcmp(key, O_("who")) == 0) {
		if (argc != 3)
			goto wrong_number_args;
		filter_style = (strcmp(value, O_("me")) == 0 ? FILTER_BY_UID :
				strcmp(value, O_("everyone")) == 0 ? FILTER_NOTHING :
				FILTER_BY_PID );
		if (filter_style == FILTER_BY_PID)
			filter_good_pid = atoi(value);
	} else if (strcmp(key, O_("what")) == 0) {
		if (argc != 3)
			goto wrong_number_args;
		vm_style_enum style =
			strcmp(value, O_("virtual")) == 0 ? VIRTUAL_MEM :
			strcmp(value, O_("physical")) == 0 ? PHYSICAL_MEM :
			BOTH_MEM;
		report_vm = style;
	} else if (strcmp(key, O_("how")) == 0) {
		if (argc != 3)
			goto wrong_number_args;
		if (strcmp(value, "1") == 0 ||
		    strcmp(value, "0") == 0) {
			allow_autosize = (strcmp(value, "1") == 0);
		} else if (strcmp(value, O_("shrink")) == 0) {
			if (allow_autosize)
				process_view::mem_target_adjust(-1);
			else process_view::mem_adjust(1);
		} else if (strcmp(value, O_("grow")) == 0) {
			if (allow_autosize)
				process_view::mem_target_adjust(1);
			else process_view::mem_adjust(-1);
		} else {
			Tcl_SetResult(interp, _("unknown VALUE for how"), NULL);
			return TCL_ERROR;
		};
	} else if (strcmp(key, O_("debug")) == 0) {
		if (argc != 3)
			goto wrong_number_args;
		lava_debug = (strcmp(value, "1") == 0);
	} else if (strcmp(key, O_("shaped_window_ok")) == 0) {
		if (argc != 2)
			goto wrong_number_args;
		// The cast on this next line is to work around a 
		// compiler problem with gcc 2.95.2 on Madrake Linux:
		// with the ?: it's const char*, without it's just char*.
		// (The compiler doesn't want to drop the const when
		// calling Tcl_SetResult.)
		Tcl_SetResult(interp, (char*)(shaped_window_ok() ? "1" : "0"), NULL);
	} else if (strcmp(key, O_("shape")) == 0) {
		if (argc != 3)
			goto wrong_number_args;
		if ((strcmp(value, O_("unshaped")) == 0)) {
			lozenge_shaped_ = false;
		} else {
			lozenge_shaped_ = true;
			lozenge_x_shrink_pct_ = atoi(value);
		};
		// Hackily call lava_resize to effect the change.
		lava_resize_proc(NULL, interp, 1, argv);
	} else if (strcmp(key, O_("signal")) == 0) {
		if (argc != 4)
			goto wrong_number_args;
		int pid = atoi(value2);
		if (pid == 0) {
			Tcl_SetResult(interp, _("will not signal pid 0"), NULL);
			return TCL_ERROR;
		};
		int e;
		if (strcmp(O_("nice"), value) == 0) {
			e = lava_renice(pid, 5);
		} else if (strcmp(O_("renice"), value) == 0) {
			e = lava_renice(pid, 5);
		} else {
			e = lava_named_signal(pid, value);
		};
		if (e == -1) {
			Tcl_SetResult(interp, _("signalling error"), NULL);
			return TCL_ERROR;
		};
	} else if (strcmp(key, O_("whoami")) == 0) {
		if (argc != 2)
			goto wrong_number_args;
		snprintf(result_buf_, TCL_BUF_LEN, "%d", (int)getuid());
		Tcl_SetResult(interp, result_buf_, NULL);
	} else {
		Tcl_SetResult(interp, _("unknown KEY"), NULL);
		return TCL_ERROR;
	};
	return TCL_OK;
 wrong_number_args:
	Tcl_SetResult(interp, _("wrong # args: should be \"lava_menu KEY VALUE [VALUE2]\""), NULL);
	return TCL_ERROR;
}

// static
void
tcl_blob::init(int argc, char **argv)
{
	ENTRY_TRACE(__FILE__,__LINE__);
	// const char* appname = "lavaps";
	interp_ = Tcl_CreateInterp();
	if (Tcl_Init(interp_) == TCL_ERROR)
		die(Tcl_GetStringResult(interp_));
	Tk_Window tk = NULL;
	if (Tk_Init(interp_) == TCL_OK)
		tk = Tk_MainWindow(interp_);
	if (tk == NULL)
		die(_("cannot start tk"));

	/*
	 * add our new commands
	 */
	(void) Tcl_CreateCommand(interp_, O_("lava_tick"), (Tcl_CmdProc*)lava_tick_proc, NULL, NULL);
	(void) Tcl_CreateCommand(interp_, O_("lava_id_to_info"), (Tcl_CmdProc*)lava_id_to_info_proc, NULL, NULL);
	(void) Tcl_CreateCommand(interp_, O_("lava_menu"), (Tcl_CmdProc*)lava_menu_proc, NULL, NULL);
	(void) Tcl_CreateCommand(interp_, O_("lava_resize"), (Tcl_CmdProc*)lava_resize_proc, NULL, NULL);
	(void) Tcl_CreateCommand(interp_, O_("lava_dump"), (Tcl_CmdProc*)lava_dump_proc, NULL, NULL);
	(void) Tcl_CreateCommand(interp_, O_("lava_version"), (Tcl_CmdProc*)lava_version_proc, NULL, NULL);
	(void) Tcl_CreateCommand(interp_, O_("lava_default_resources"), (Tcl_CmdProc*)lava_default_resources_proc, NULL, NULL);

	/*
	 * source some files
	 */
	char *mutable_code = strdup(lava_code);  // sigh, tcl actually changes the code while running
	assert(mutable_code != NULL);
	if (TCL_OK != Tcl_Eval(interp_, mutable_code)) {
		cerr << _("error loading bootstrap: ") << Tcl_GetStringResult(interp_) << endl;
		exit(1);
	};
	free(mutable_code);
	if (TCL_OK != Tcl_Eval(interp_, O_("main"))) {
		cerr << _("error evaling main: ") << Tcl_GetStringResult(interp_) << endl;
		exit(1);
	};
	canvas_ = ".c";

	has_base_ = false;
}


// static
void
tcl_blob::mainloop()
{
	ENTRY_TRACE(__FILE__,__LINE__);
	Tk_MainLoop();
}


tcl_blob::tcl_blob(process_view *pv) :
	scaled_blob(pv), id_(0), did_(0)
{
	ENTRY_TRACE(__FILE__,__LINE__);
	// xxx: assume blob initializer called first!
	tcl_blobs_.push_front(this);
	me_in_blobs_ = tcl_blobs_.begin();
	assert(me_in_blobs_ != tcl_blobs_.end());
	bd_color_ = O_("white");
	light_color_ = O_("black");
	dark_color_ = O_("grey");
	schedule_redraw();
}

tcl_blob::~tcl_blob()
{
	ENTRY_TRACE(__FILE__,__LINE__);
	char *bufp = result_buf_;

	tcl_blobs_.erase(me_in_blobs_);

	if (id_) {
		sprintf(bufp, O_("%s delete %d\n"), canvas_, id_);   bufp += strlen(bufp);
	};
	if (did_) {
		sprintf(bufp, O_("%s delete %d\n"), canvas_, did_);   bufp += strlen(bufp);
	};
	if (TCL_OK != Tcl_Eval(interp_, result_buf_)) {
		cerr << result_buf_ << endl << _("tcl_blob::~tcl_blob: eval error\n");;
		exit (1);
	};
	id_ = 0;
	did_ = 0;
}

char *
tcl_blob::list_point(char *bufp, char *eobuf, int x, int y)
	/*
	 * Map from x,y into screen_x,screen_y.
	 * Adjust for distortion and scale to screen.
	 */
{
	double screen_x, screen_y;
	scale_point((double)x, (double)y, &screen_x, &screen_y);
	snprintf(bufp, eobuf-bufp, "%d %d ", (int)screen_x, (int)screen_y);
	bufp += strlen(bufp);
	return bufp;
}

char *
tcl_blob::list_points(char *bufp, char *eobuf, int pct, int offset)
	/*
	 * Populate BUFP with an ASCII version of the point list
	 * for the blob.  Scale y-values by PCT.
	 * OFFSET is not used.
	 */
{
	int i, x, y;

	// Scratch space for scaling.
	static int *ranges = NULL, *midpoints = NULL;
	static int len = 0;
	if (len < num_) {
		delete[] ranges;
		delete[] midpoints;
		ranges = new int[num_];
		midpoints = new int[num_];
		len = num_;
	};

	for (i = 0, x = x_; i < num_; i++, x += x_step_) {
		if (pct != 100) {
			// Scale things: occupy PCT of the space
			// around the center.
			ranges[i] = y_highs_[i] - y_lows_[i];
			midpoints[i] = y_lows_[i] + ranges[i] / 2;
			y = midpoints[i] - ranges[i] * pct / 200;
			if (y <= y_lows_[i])
				y += pixel_size_;
		} else {
			y = y_lows_[i];
		};
		bufp = list_point(bufp, eobuf, x, y);
	};
	for (; --i >= 0; ) {
		if (pct != 100) {
			y = midpoints[i] + ranges[i] * pct / 200;
			if (y >= y_highs_[i])
				y -= pixel_size_;
		} else {
			y = y_highs_[i];
		};
		x -= x_step_;
		bufp = list_point(bufp, eobuf, x, y);
	};
	// must always have 3 points
	if (num_ == 1) {
		// make one up
		x = x_ + x_step_ / 2 * (right_tending_ ? 1 : -1);
		int y = (y_highs_[0] - y_lows_[0]) / 2 + y_lows_[0];
		bufp = list_point(bufp, eobuf, x, y);
	};
	return bufp;
}

void
tcl_blob::redraw()
{
	ENTRY_TRACE(__FILE__,__LINE__);
	char buf[LONG_TCL_BUF_LEN], *bufp = buf;
	char *eobuf = &buf[LONG_TCL_BUF_LEN];  // betcha no compiler optimizes this away :-(
	if (id_) {
		snprintf(bufp, eobuf-bufp, O_("%s delete %d\n"), canvas_, id_);   bufp += strlen(bufp);
	};
	if (did_) {
		snprintf(bufp, eobuf-bufp, O_("%s delete %d\n"), canvas_, did_);   bufp += strlen(bufp);
	};
	snprintf(bufp, eobuf-bufp, O_("set id [%s create polygon "), canvas_);   bufp += strlen(bufp);
	bufp = list_points(bufp, eobuf);
	snprintf(bufp, eobuf-bufp, O_("-fill %s -outline %s -smooth 1 -tag p]\n"), light_color_.c_str(), bd_color_.c_str());   bufp += strlen(bufp);

	//
	// do shading
	//
	if (dark_percentage_) {
		snprintf(bufp, eobuf-bufp, O_("set did [%s create polygon "), canvas_);   bufp += strlen(bufp);
		bufp = list_points(bufp, eobuf, 100 - dark_percentage_, 50);
		snprintf(bufp, eobuf-bufp, O_("-fill %s -smooth 1 -tag p]\n"), dark_color_.c_str());   bufp += strlen(bufp);
		// snprintf(bufp, eobuf-bufp, "-fill %s -outline %s -smooth 1 -tag p]\n", dark_color_.c_str(), bd_color_.c_str());   bufp += strlen(bufp);
	};

	blob *b = superior();
	int superior_id = b ? ((tcl_blob*)b)->id_ : 0;
	if (superior_id)
		snprintf(bufp, eobuf-bufp, O_("%s lower $id %d\n"), canvas_, superior_id);   bufp += strlen(bufp);
	if (dark_percentage_) {
		if (superior_id)
			snprintf(bufp, eobuf-bufp, O_("%s lower $did %d\n"), canvas_, superior_id);   bufp += strlen(bufp);
		snprintf(bufp, eobuf-bufp, O_("return \"{$id $did}\"\n"));   bufp += strlen(bufp);
	} else {
		snprintf(bufp, eobuf-bufp, O_("return $id\n"));   bufp += strlen(bufp);
	};

	if (eobuf - bufp < 5) {
		int cont = 0;
		cerr << _("tcl_blob::redraw: buffer overrun.\n");
		if (!cont)
			exit (1);
	};
	int e;
	if ((e = Tcl_Eval(interp_, buf)) != TCL_RETURN) {
		cerr << Tcl_GetStringResult(interp_) << endl << _("tcl_blob::redraw: eval error\nscript:\n") << buf << endl << endl;;
		exit (1);
	};
	if (dark_percentage_) {
		const char *cp = Tcl_GetStringResult(interp_);
		assert(cp != NULL);
		assert(*cp != 0);
		id_ = atoi(cp+1);
		cp = strchr(cp, ' ');
		assert(cp != NULL);
		did_ = atoi(cp);
	} else {
		id_ = atoi(Tcl_GetStringResult(interp_));
		did_ = 0;
	};
}

void
tcl_blob::set_hsb(double h, double s, double b)
{
	ENTRY_TRACE(__FILE__,__LINE__);
	blob::set_hsb(h, s, b);

	string cs = hsb_to_rgb_string(H_, S_, B_);
	light_color_ = cs.c_str();

	cs = hsb_to_rgb_string(H_, S_, 1.0 - B_);
	bd_color_ = cs.c_str();

	cs = hsb_to_rgb_string(dk_H_, dk_S_, dk_B_);
	dark_color_ = cs.c_str();

//	if (pv_->pid() == 1913) {
//		cerr << "test: " << light_color_.c_str() << " " << dark_color_.c_str() << " did " << did_ << " dark_percentage_ " << dark_percentage_  << endl;
//	};

	schedule_redraw();
}

void
tcl_blob::splash(const char *msg)
{
	// cout << msg << endl;
	char buf[TCL_BUF_LEN];
	snprintf(buf, TCL_BUF_LEN, O_("splash_refresh {%s}"), msg);
	if (TCL_OK != Tcl_Eval(interp_, buf)) {
		cerr << _("tcl_blob::tcl_splash: eval error: ") << Tcl_GetStringResult(interp_) << endl;
	};
}

#endif /* USE_TCL_BLOB */
