/* -*-	Mode:C++ -*- */

/*
 * blob.hh
 * Copyright (C) 1999-2000 by John Heidemann
 * $Id: blob.hh,v 1.34 2004/12/16 05:12:32 johnh Exp $
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

/*
 * Blob provides the virtual rendering of blobs on a X_MAX x Y_MAX
 * area.  See scaled_blob to add mapping to screen coordinates and 
 * lozenging.
 */

#ifndef lavaps_blob_h
#define lavaps_blob_h


#include "config.h"

#include <string>
#include <list>


class process_view;

class blob {
private:
	blob(const blob& tb);
	blob& operator=(const blob& tb);

public:
	enum { X_MAX = 100, Y_MAX = 200, BASE_EXTENSION = 25, INITIAL_SLOP = 10 };

protected:
	process_view *pv_;

	// blob characteristics.
	int dark_percentage_;
	int dark_offset_;

	double H_, S_, B_,
		dk_H_, dk_S_, dk_B_;
	int size_;  // size in "screen linear units"
	// set from process_view

	static bool next_right_tending_;
		// When rendering blobs with only one column,
		// you may need to fake a 3rd point.
		// It's either right or left per blob,
		// and they're about 50% one way or the other.
	bool right_tending_;

	static const int x_step_ = INITIAL_SLOP;
	int x_;
	int *y_lows_, *y_highs_;
	int num_;
	int alloced_num_;

	int sign_;  // travel direction
	int reached_limit_;

	typedef list<blob*> blob_queue_t;
	static blob_queue_t all_queue_;
	blob_queue_t::iterator all_place_;

	static blob_queue_t redraw_queue_;
	blob_queue_t::iterator redraw_place_;
	void schedule_redraw() { if (redraw_place_ != redraw_queue_.end()) return; redraw_queue_.push_front(this); redraw_place_ = redraw_queue_.begin(); }
	static void flush_drawings();
	friend class redraw_all_f;
	static void redraw_all();

	virtual void redraw() = 0;

	void resize_int_array(int **iap, int on, int nn);
	void adjust_num(int new_num);
	inline bool valid_num(int n) { return n >= 0 && n < num_; }

	static const int DELTA_LIMIT;
	enum blob_op { MOVE_OP, GROW_OP, SHRINK_OP };

	blob *superior();

	// note: deltas must be > 0, use SHRINK/GROW to show sign
	int partial_op(blob_op op, int delta);
	virtual void op(blob_op op, int delta);

	void shift_to_right();
	void shift_to_left(int start);
	void widen_on_right();
	void widen_on_left();
	bool narrow_on_right();
	bool narrow_in_middle(int where);
	bool narrow_on_left();

	int advance_rank(blob_op op, int num, int magnitude, int direction);
	int find_rank_midpoint(int num);
	int ranks_overlap(int a, int b);

	void reverse_direction();

public:
	blob(process_view *pv);
	virtual ~blob();

	static const int available_area() { return Y_MAX * X_MAX / x_step_; };

	static blob *create_real_blob(process_view *pv); // instantiate a back-end blob

	void move(int delta) { op(MOVE_OP, delta); };
	void print();
	void validate_rank(int rank, bool strict_less);
	void validate();

	virtual void set_hsb(double h, double s, double b);
	virtual void set_size(int size);   // only guaranteed to get "close"
	virtual void set_position(int x, int y);
	virtual void set_dark_percentage(int pct, int offset = -1);
	void reordered() { schedule_redraw(); };

	int calc_real_size();
	void info_to_string(string &s);

	int get_real_size() { return size_; }

	// the following should be virtual but can't because they're static
	static void init(int argc, char **argv);
	static void mainloop();
	static void splash(const char *msg);
	static void size_divisor_commit();
	static void size_mem_to_screen_commit() {};
};

#endif /* lavaps_blob_h */

