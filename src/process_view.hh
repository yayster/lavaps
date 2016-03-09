/* -*-	Mode:C++ -*- */

/*
 * process_view.hh
 * Copyright (C) 1999 by John Heidemann
 * $Id: process_view.hh,v 1.30 2004/12/16 05:16:13 johnh Exp $
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


#ifndef lavaps_process_view_h
#define lavaps_process_view_h

#include <unistd.h>  // pid_t

#include <string>
#include <functional>
#include <set>

#include "blob.hh"
#include "change_tracking.hh"
#include "const_str.hh"

class process_model;
class process_view;

struct process_view_ordering {
	bool operator()(process_view *a, process_view *b) const;
};


/*
 * process_view: a process-level representation of 
 * how we'll render the process.
 * This is independent of how we'll actually render it
 * (i.e., Tcl, GNOME, etc.).
 */
class process_view {
	friend class process_view_ordering;

private:
	process_view(const process_view& tb);
	process_view& operator=(const process_view& tb);

protected:
	process_model *pm_;

	blob *b_;

	// keep an ordering of processes
	typedef set<process_view *, process_view_ordering> process_view_order_t;
	static process_view_order_t orders_;
	process_view_order_t::iterator order_;
	void reorder();

	// static stuff:
	double H_, S_, B_;
	// int start_x_, start_y_;

	struct memory_sum_t { static int sum_; };  // in screen linear units
	sum_change_tracking<int,memory_sum_t> size_; // size, scaled to screen linear units, computed from memory
	void compute_size();
	change_tracking<int> time_;
	change_tracking<int> age_;

	// sizing concepts:
	// try to match memory size to screen real-estate
	static const double dir_to_multiplier(int dir);
	static explicit_change_tracking<double> size_mem_to_screen_scale_;
	static int size_low_water_, size_high_water_; // high & low watermarks, in screen linear units
	static double fraction_of_total_screen_area_for_target_size_;  // range [0,1] of screen linear units to hit
	int scale_size(int size);  // convert from mem to screen (via size_divisor_)

	static const int time_divisor_ = 10;  // msec->100ths of a sec
	int scale_time(int time);

	void compute_color();
	void init_position();
public:
	process_view(process_model *pm);
	~process_view();
	void update();
	void info_to_string(string &s);
	int pid();
	int uid();
	const_str cmd();

	blob *get_blob() { return b_; }
	process_view *superior();

	// expose size_divisor_ so the GUI can save it as a preference
	// (a bit of a hack)
	static double *size_mem_to_screen_scale_address() { return size_mem_to_screen_scale_.address(); }

	static int show_size_;
	static void autosize();
	static void mem_adjust(int dir);
	static void mem_target_adjust(int dir);
};


#endif /* lavaps_process_view_h */

