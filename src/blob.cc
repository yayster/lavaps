
/*
 * blob.cc
 * Copyright (C) 1999-2003 by John Heidemann
 * $Id: blob.cc,v 1.39 2004/12/16 05:12:38 johnh Exp $
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

#include <stdlib.h>  // atoi

#include <iostream>
#include <algorithm> // swap

#include "main.hh"
#include "process_view.hh"
#include "blob.hh"


#if 0
#define BLOB_ARRAY_DEBUG(x) x
#define BLOB_AUTOSIZE_DEBUG(x) x
#else /* ! 0 */
#define BLOB_ARRAY_DEBUG(x) 
#define BLOB_AUTOSIZE_DEBUG(x) 
#endif /* 0 */



bool blob::next_right_tending_ = false;
blob::blob_queue_t blob::all_queue_;
blob::blob_queue_t blob::redraw_queue_;

const int blob::DELTA_LIMIT = 5;


blob::blob(process_view *pv) :
	pv_(pv),
	dark_percentage_(0),
	dark_offset_(50),
	H_(0.), S_(0.), B_(0.),
	dk_H_(0.), dk_S_(0.), dk_B_(0.),
	size_(0),
	y_lows_(NULL), y_highs_(NULL),
	num_(0), 
	alloced_num_(0), 
	sign_(1),
	reached_limit_(0),
	all_place_(all_queue_.end()),
	redraw_place_(redraw_queue_.end())
{
	ENTRY_TRACE(__FILE__,__LINE__);
	// start with a small square
	adjust_num(2);
	x_ = 0;
	y_lows_[0] = 0;
	y_highs_[0] = INITIAL_SLOP;
	y_lows_[1] = 0;
	y_highs_[1] = INITIAL_SLOP;
	size_ = INITIAL_SLOP * 2;
	right_tending_ = next_right_tending_;
	next_right_tending_ = !next_right_tending_;

	all_queue_.push_front(this);
	all_place_ = all_queue_.begin();

	validate();
}

blob::~blob()
{
	ENTRY_TRACE(__FILE__,__LINE__);
	if (y_lows_)
		delete y_lows_;
	if (y_highs_)
		delete y_highs_;
	if (all_place_ != all_queue_.end())
		all_queue_.erase(all_place_);
	if (redraw_place_ != redraw_queue_.end())
		redraw_queue_.erase(redraw_place_);
}


void
blob::flush_drawings()
{
	int flush_count = 0;
	int debug = 0;
	char ch = '(';
	if (debug)
		cout << O_("flush: ");

	blob_queue_t::iterator bi;
	while ((bi = redraw_queue_.begin()) != redraw_queue_.end()) {
		flush_count++;
		(*bi)->redraw();
		(*bi)->redraw_place_ = redraw_queue_.end();
		if (debug)
			cout << ch << (*bi)->pv_->pid();  ch = ',';
		redraw_queue_.pop_front();
	};
	if (debug)
		cout << O_("): ") << flush_count << endl;
}

struct redraw_all_f : public unary_function<blob*,void> {
	void operator()(blob *b) { b->schedule_redraw(); }
};

void
blob::redraw_all()
{
	// Some extra work here (in putting them on the redraw queue
	// then flushing them),
	// but it's rare, and I get to reuse the diagnostics
	// in flush_drawings().
	for_each(all_queue_.begin(), all_queue_.end(), redraw_all_f());
	flush_drawings();
}

void
blob::resize_int_array(int **iap, int on, int nn)
{
	ENTRY_TRACE(__FILE__,__LINE__);
	if (!*iap)
		*iap = new int[nn];
	else if (nn > on) {
		int *nia = new int[nn], *oia = *iap;
		for (int i = 0; i < on; i++)
			nia[i] = oia[i];
		*iap = nia;
		delete oia;
	};
	if (nn > on)
		for (int j = on; j < nn; j++)
			(*iap)[j] = 0;
}

void
blob::adjust_num(int new_num)
{
	ENTRY_TRACE(__FILE__,__LINE__);
	// allocate
	if (new_num > alloced_num_) {
		resize_int_array(&y_lows_, num_, new_num);
		resize_int_array(&y_highs_, num_, new_num);
		alloced_num_ = new_num;
	};
	num_ = new_num;
}

void
blob::shift_to_left(int start)
{
	ENTRY_TRACE(__FILE__,__LINE__);
	int i;
	pedantic_assert(start >= 0 && start < num_);
	for (i = start + 1; i < num_; i++) {
		y_lows_[i-1] = y_lows_[i];
		y_highs_[i-1] = y_highs_[i];
	};
}

void
blob::shift_to_right()
{
	ENTRY_TRACE(__FILE__,__LINE__);
	int i;
	for (i = num_-1; i > 0; i--) {
		y_lows_[i] = y_lows_[i-1];
		y_highs_[i] = y_highs_[i-1];
	};
}

void
blob::widen_on_left()
{
	ENTRY_TRACE(__FILE__,__LINE__);
	BLOB_ARRAY_DEBUG(cout << O_("blob::widen_on_left\n"));
	x_ -= x_step_;
	adjust_num(num_ + 1);
	shift_to_right();
	// y_lows_[0] = y_highs_[0] = lava_random(y_highs_[0] - y_lows_[0]) + y_lows_[0];   // random
	y_lows_[0] = y_highs_[0] = find_rank_midpoint(1);
}

void
blob::widen_on_right()
{
	ENTRY_TRACE(__FILE__,__LINE__);
	BLOB_ARRAY_DEBUG(cout << O_("blob::widen_on_right\n"));
	adjust_num(num_ + 1);
	int last = num_-1,
		last_last = last - 1;
	assert(last_last >= 0);
	// y_lows_[last] = y_highs_[last] = lava_random(y_highs_[last_last] - y_lows_[last_last]) + y_lows_[last_last]; // random
	y_lows_[last] = y_highs_[last] = find_rank_midpoint(last_last);
}

bool
blob::narrow_in_middle(int where)
{
	ENTRY_TRACE(__FILE__,__LINE__);
	if (num_ <= 1)
		return false;
	if (where == num_ - 1)
		return narrow_on_right();
	if (y_lows_[where] < y_highs_[where])
		return false;
	// where is unnecessary
	BLOB_ARRAY_DEBUG(cout << O_("blob::narrow_in_middle ") << where << endl);
	assert(y_lows_[where] == y_highs_[where]);
	x_ += x_step_ / 2;
	shift_to_left(where);
	adjust_num(num_ - 1);
	return true;
}

bool
blob::narrow_on_left()
{
	ENTRY_TRACE(__FILE__,__LINE__);
	if (narrow_in_middle(0)) {
		// fully shift edge
		BLOB_ARRAY_DEBUG(cout << O_("really blob::narrow_on_left\n"));
		x_ -= x_step_ / 2;  // (try to correct for possible rounding errors)
		x_ += x_step_;
		return true;
	} else return false;
}

bool
blob::narrow_on_right()
{
	ENTRY_TRACE(__FILE__,__LINE__);
	int last = num_-1;
	if (num_ <= 1)
		return false;
	if (y_lows_[last] < y_highs_[last])
		return false;
	BLOB_ARRAY_DEBUG(cout << O_("blob::narrow_on_right\n"));
	// last is unnecessary
	adjust_num(num_ - 1);
	return true;
}

int
blob::calc_real_size()
{
	int i;
	int rs = 0;
	for (i = 0; i < num_; i++)
		rs += y_highs_[i] - y_lows_[i];
	return rs;
}

// advance rank returns the amount of magnitude it was
// NOT able to use.  (0 == success)
int
blob::advance_rank(blob_op op, int num, int magnitude, int direction)
{
	ENTRY_TRACE(__FILE__,__LINE__);
	int *backp, *frontp;
	int unused_magnitude = 0;

	pedantic_assert(num >= 0 && num < num_);
	assert(magnitude >= 0);
	pedantic_assert(direction == -1 || direction == 1);
	validate_rank(num, false);

	if (magnitude == 0)
		return 1;
	if (direction > 0) {
		backp = &y_lows_[num];
		frontp = &y_highs_[num];
	} else {
		frontp = &y_lows_[num];
		backp = &y_highs_[num];
	};
	if (op == SHRINK_OP && y_highs_[num] - y_lows_[num] < magnitude) {
		// Don't shrink a column into negativeness.
		int new_magnitude = y_highs_[num] - y_lows_[num];
		unused_magnitude += magnitude - new_magnitude;
		magnitude = new_magnitude;
	};
	if (op == GROW_OP && y_highs_[num] - y_lows_[num] + 1 + magnitude >= Y_MAX) {
		// Don't grow a column larger than possible.
		int new_magnitude = Y_MAX - (y_highs_[num] - y_lows_[num] + 1);
		unused_magnitude += magnitude - new_magnitude;
		magnitude = new_magnitude;
	};

	// move it
	int move = magnitude * direction;
	switch (op) {
	case MOVE_OP:
		*frontp += move;
		*backp += move;
		break;
	case GROW_OP:
		*frontp += move;
		size_ += magnitude;
		break;
	case SHRINK_OP:
		*backp += move;
		size_ -= magnitude;
		break;
	};

	// check limits and fix things up if needed
	int remove = 0;
	if (direction < 0 && *frontp < 0) {
		remove = -*frontp;
		assert(remove > 0);
		unused_magnitude += remove;
	} else if (direction > 0 && *frontp >= Y_MAX) {
		remove = - (*frontp - Y_MAX + 1);
		assert(remove < 0);
		unused_magnitude += -remove;
	};
	if (remove) {
		*frontp += remove;
		*backp += remove;
	};

	validate_rank(num, false);
	return unused_magnitude;
}

int
blob::find_rank_midpoint(int num)
{
	if (num < 0)
		num = 0;
	if (num >= num_)
		num = num_ - 1;
	return (y_highs_[num] - y_lows_[num]) / 2 + y_lows_[num];
}

// return the amount two (presumably adjacent) ranks overlap
int
blob::ranks_overlap(int a, int b)
{
	assert(a >= 0 && a < num_);
	assert(b >= 0 && b < num_);
	// first, eliminate no overlap cases
	if (y_highs_[a] < y_lows_[b])
		return y_lows_[b] - y_highs_[a];
	if (y_lows_[a] > y_highs_[b])
		return y_highs_[b] - y_lows_[a];
	// must be overlap
	return 0;
}

void
blob::reverse_direction()
{
	ENTRY_TRACE(__FILE__,__LINE__);
	BLOB_ARRAY_DEBUG(cout << O_("blob::reverse_direction\n"));
	reached_limit_ = 0;
	sign_ *= -1;
}

// partial up returns the amount of unused delta, like advance_rank
// (0==success)
int
blob::partial_op(blob_op op, int delta)
{
	ENTRY_TRACE(__FILE__,__LINE__);
	assert(delta >= 0);

	// pick source
	int src = lava_random(num_);

	// figure direction
	// 50% straight, 25% right or left
	int dest_dir = lava_random(4) - 1; if (dest_dir == 2) dest_dir = 0;
	if (src + dest_dir < 0) {
		if (x_ - x_step_ < 0) {  // don't go off edge!
			dest_dir = 0;
		} else {
			widen_on_left();
			src++;
		};
	};
	if (src + dest_dir >= num_) {
		if (x_ + x_step_ * num_ >= X_MAX) {  // watch edge
			dest_dir = 0;
		} else {
			widen_on_right();
		};
	};
	int adj_src = src + dest_dir;

	// second chance---if two adjacent rows are
	// largely askew, move the one that makes things better.
	// Randomly try one side or the other.
	// Initial results suggest that this section of code
	// makes things look a *lot* less stringy than without.
	// (Gee, aparently approximate algorithms based on 
	// local information *are* able to effect globally
	// useful properties... what a good idea :-)
	//
	// TODO: we shouldn't always give things a 2nd chance,
	// but make it porportional to something interesting (like
	// disk IO).
	static int old_first = 1, old_second = -1;
	int first = old_first, second = old_second;
	swap(old_first, old_second);

	int diff;
	if (valid_num(adj_src + first) && (diff = ranks_overlap(adj_src + first, adj_src))) {
		if (sign(diff) == sign_)
			adj_src += first;
	} else if (valid_num(adj_src + second) && (diff = ranks_overlap(adj_src + second, adj_src))) {
		if (sign(diff) == sign_)
			adj_src += second;
	};

	// advance it
	int unused_magnitude = advance_rank(op, adj_src, delta, sign_);
	if (unused_magnitude && (op == MOVE_OP || op == GROW_OP)) {
		BLOB_ARRAY_DEBUG(cout << O_("bounce\n"));
		if (++reached_limit_ >= num_) {
			reverse_direction();
		};
	};

	// check for empty edges
	if (adj_src > 0)
		narrow_in_middle(adj_src);
	while (narrow_on_right()) ;
	while (narrow_on_left()) ;

	return unused_magnitude;
}

void
blob::op(blob_op op, int delta)
{
	ENTRY_TRACE(__FILE__,__LINE__);
	assert(delta >= 0);
	validate();

	if (op != MOVE_OP) {
		// static const char *ops[] = { "MOVE", "GROW", "SHRINK" };
		BLOB_AUTOSIZE_DEBUG(cout << O_("blob::op: action ") << ops[op] << O_(", delta=") << delta << O_(", pre_s=") << size_ << endl);
	};

	int i = 0;
	int pre_size = size_;
	int post_size = pre_size;
	while (delta > 0) {
		int this_delta = min(DELTA_LIMIT, delta);
		int used_delta = this_delta - partial_op(op, this_delta);
		delta -= used_delta;
		switch (op) {
		case GROW_OP:
			post_size += used_delta;
			break;
		case SHRINK_OP:
			post_size -= used_delta;
			break;
		default:
			break;
		};
		if (++i > 200) {
			static int uid = -1;
			if (uid == -1)
				uid = getuid();
#if 0
			if (0 && uid == 2274)  // i.e., me
				cerr << _("blob::op: looping limit exceeded, blob ") << (void*)this << endl;
#endif /* 0 */
			break;
		};
	};

	if (op != MOVE_OP) {
		BLOB_AUTOSIZE_DEBUG(int crs = calc_real_size());
		BLOB_AUTOSIZE_DEBUG(cout << O_("    post_s_=") << size_ << O_(", ps=") << post_size << O_(", crs=") << crs << endl);
	};

	schedule_redraw();
	validate();
}

void
blob::print()
{
	ENTRY_TRACE(__FILE__,__LINE__);
	cout << O_("blob: ") << (unsigned)this << O_(" x=") << x_ << O_(" step=") << x_step_ << O_(" num=") << num_ << O_(":");
	for (int i = 0; i < num_; i++) {
		cout << O_(" ") << y_lows_[i] << O_("-") << y_highs_[i];
	};
	cout << O_(" size=") << size_ << O_(" cs=") << calc_real_size() <<endl;
}


void
blob::validate_rank(int rank, bool strict_less)
{
	assert(y_lows_[rank] >= 0 && y_lows_[rank] < Y_MAX);
	assert(y_highs_[rank] >= 0 && y_highs_[rank] < Y_MAX);
	if (strict_less)
		assert(y_lows_[rank] < y_highs_[rank]);
	else assert(y_lows_[rank] <= y_highs_[rank]);
}

void
blob::validate()
{
	// basic sanity checks
	int i;
	for (i = 0; i < num_; i++)
		validate_rank(i, true);
	assert(x_ >= 0 && x_ < X_MAX);
	int end_x = x_ + (num_ - 1) * x_step_;
	assert(end_x >= 0 && end_x < X_MAX);
}

void
blob::set_hsb(double h, double s, double b)
{
	ENTRY_TRACE(__FILE__,__LINE__);
	H_ = h;
	S_ = s;
	B_ = b;

	/*
	 * Pick a dark color;
	 * Constants:
	 * 0.0625 is too subtle.
	 * 0.1 is too much.
	 */
	double dark_h = H_ + 0.07;
	if (dark_h > 1.0)
		dark_h -= 1.0;
	double dark_s = S_ + 0.1;
	if (dark_s > 1.0)
		dark_s = 1.0;
	dk_H_ = dark_h;
	dk_S_ = dark_s;
	dk_B_ = B_;
}

// not guaranteed to necessarily set it exactly
void
blob::set_size(int size)
{
	ENTRY_TRACE(__FILE__,__LINE__);
	int delta = size - size_;
	if (delta > 0)
		op(GROW_OP, delta);
	else if (delta < 0)
		op(SHRINK_OP, -delta);
}

void
blob::set_position(int x, int y)
{
	ENTRY_TRACE(__FILE__,__LINE__);
	// fix the size
	x_ += x;
	for (int i = 0; i < num_; i++) {
		y_lows_[i] += y;
		y_highs_[i] += y;
	};
	validate();
}

void
blob::info_to_string(string &s)
{
	ENTRY_TRACE(__FILE__,__LINE__);
	pv_->info_to_string(s);
}

blob *
blob::superior()
{
	process_view *superior_view = pv_->superior();
	return superior_view ?  superior_view->get_blob() : NULL;
}

void
blob::set_dark_percentage(int pct, int offset)
{
	dark_percentage_ = pct;
	if (offset != -1) {
		dark_offset_ = offset;
	};
}


