
/*
 * process_view.cc
 * Copyright (C) 1999-2003 by John Heidemann
 * $Id: process_view.cc,v 1.51 2004/12/17 19:58:48 johnh Exp $
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

// #include <stdio.h>  // snprintf

#include <iostream>
#include <sstream>

#include <math.h>

#include "main.hh"
#include "process_view.hh"
#include "process_model.hh"
#include "user.hh"


#if 0
#define PV_AUTOSIZE_DEBUG(X) X
#else /* ! 0 */
#define PV_AUTOSIZE_DEBUG(X) 
#endif /* 0 */


int process_view::memory_sum_t::sum_ = 0;
explicit_change_tracking<double> process_view::size_mem_to_screen_scale_(1.0);
int process_view::size_low_water_ = -1;
int process_view::size_high_water_ = -1;
double process_view::fraction_of_total_screen_area_for_target_size_ = 0.75;  // range [0,1] of screen linear units to hit
int process_view::show_size_ = 1;


bool
process_view_ordering::operator()(process_view *a, process_view *b) const
{
	// For some reason we get called with a null...
	// if we do reorders() in age().
	// Recover from that.
	if (!a || !b)
		return false;
	if (a->age_.current() != b->age_.current())
		return a->age_.current() < b->age_.current();
	if (a->size_.current() != b->size_.current())
		return a->size_.current() < b->size_.current();
	// force a total ordering
	return a->pm_->pid() < b->pm_->pid();
}


process_view::process_view_order_t process_view::orders_;


process_view::process_view(process_model *pm) :
	pm_(pm),
	order_(orders_.end())
{
	ENTRY_TRACE(__FILE__,__LINE__);
	b_ = blob::create_real_blob(this);
	init_position();  // must preceed compute_size() or the bigger blob moves off the field
	compute_size();
	time_.tick_set(scale_time(pm_->utime_.current() + pm_->stime_.current()));
	compute_color();
	update();   // set dark percentage
}

process_view::~process_view()
{
	ENTRY_TRACE(__FILE__,__LINE__);
	if (order_ != orders_.end())
		orders_.erase(order_);
	order_ = orders_.end();
	delete b_;
}

void
process_view::reorder()
{
	// optimize for not moving
	process_view_order_t::iterator prev_order_(order_);
	if (order_ != orders_.end()) {
		--prev_order_;
		orders_.erase(order_);
	};
	process_view_order_t::iterator next_order_(order_);
	if (next_order_ != orders_.end())
		++next_order_;

	order_ = orders_.insert(prev_order_, this);
	// If we changed order, tell the blob to redraw.
	// (Boy, it's a shame there isn't an iterator where ++end == end!)
	process_view_order_t::iterator new_prev_order_(order_);
	if (order_ != orders_.end())
		--new_prev_order_;
	process_view_order_t::iterator new_next_order_(order_);
	if (new_next_order_ != orders_.end())
		++new_next_order_;
	if (prev_order_ != new_prev_order_ || next_order_ != new_next_order_)
		b_->reordered();
}

void
process_view::compute_color()
{
	ENTRY_TRACE(__FILE__,__LINE__);
	S_ = 0.6;
	B_ = 0.8 - (pm_->age() * 0.06);
	const double MIN_B = 0.15;
	if (B_ < MIN_B)
		B_ = MIN_B;

	H_ = (pm_->cmd_hash_.current() % 2048) / 2048.0;

	b_->set_hsb(H_, S_, B_);
}

void
process_view::init_position()
{
	ENTRY_TRACE(__FILE__,__LINE__);
	int x = pm_->pid_ % (blob::X_MAX - blob::INITIAL_SLOP);
	int y = pm_->uid_ % (blob::Y_MAX - blob::INITIAL_SLOP);
	b_->set_position(x, y);
}

int
process_view::scale_size(int size)
{
	ENTRY_TRACE(__FILE__,__LINE__);
	// return size / size_divisor_.current();
	assert(size_mem_to_screen_scale_.current() > 0.0);
	return int(size * size_mem_to_screen_scale_.current());
}

int
process_view::scale_time(int time)
{
	ENTRY_TRACE(__FILE__,__LINE__);
	// covert from msec to 100ths of a second
	return time / time_divisor_;
}

/*
 * compute the scaled size from whatever memory size is desired
 */
void
process_view::compute_size()
{
	assert(report_vm.current() != ERROR_MEM);

	int size = scale_size(report_vm.current() != PHYSICAL_MEM ? 
			      pm_->virtual_size_.current() :
			      pm_->resident_.current());
	size = max(size, 3);   // work around not handling small sizes well.
	size_.tick_set(size);
	if (size_.changed())
		b_->set_size(size_.current());

	if (report_vm.current() == BOTH_MEM) {
		if (pm_->inmem_pct_.changed() || report_vm.changed())
			b_->set_dark_percentage(100 - pm_->inmem_pct_.current());
	} else if (report_vm.changed()) {
		b_->set_dark_percentage(0);
	};
}

void
process_view::update()
{
	ENTRY_TRACE(__FILE__,__LINE__);
	age_.tick_set(pm_->age());
	if (age_.changed() || pm_->cmd_hash_.changed())
		compute_color();

	if (report_vm.changed() ||
	    size_mem_to_screen_scale_.changed() ||
	    (report_vm.current() == BOTH_MEM ?
	     pm_->virtual_size_.changed() || pm_->resident_.changed() :
	     report_vm.current() == VIRTUAL_MEM ?
	     pm_->virtual_size_.changed() : pm_->resident_.changed())) {
		compute_size();
	};

	time_.tick_set(scale_time(pm_->utime_.current() + pm_->stime_.current()));
	if (time_.changed()) {
		int change = time_.change();
		if (change < -400000)  // hack around integer overflow
			change = 0;
		else assert(change > 0);
		b_->move(change);
	};

	if (age_.changed() || size_.changed())
		reorder();
}

static void
stream_mem(ostream &os, int kb)
{
	// kb is already in kibibytes, not bytes
	/* for what a KiB is, see http://physics.nist.gov/cuu/Units/binary.html */
	if (kb < 1) { // xxx
		os << kb << "B";
	} else if (kb < 10 * 1024) {
		os << kb << O_("KiB");
	} else {
		os << int(kb/1024) << O_("MiB");
	};
}


void
process_view::info_to_string(string &s)
{
	ENTRY_TRACE(__FILE__,__LINE__);
	ostringstream oss;
	oss 
	    << _("cmd: ") << pm_->cmd_.c_str() << endl
	    << _("pid: ") << pm_->pid_ << endl
	    << _("user: ") << uid_to_name(pm_->uid_) << " (" << pm_->uid_ << ")\n"
	    << _("nice: ") << pm_->nice_ << endl;
	oss << _("virt mem: ");
	stream_mem(oss, pm_->virtual_size_.current());
	oss << endl;
	oss << _("phys mem: ");
	stream_mem(oss, pm_->resident_.current());
	oss << " (" << pm_->inmem_pct_.current() << "%)\n";
	if (lava_debug) {
		int rs = b_->get_real_size();
		int crs = b_->calc_real_size();
		oss << O_("(pvs=") << size_.current() 
		    << O_(", rs=") << rs
		    << O_(", crs=") << crs
		    << ")\n";
	};
	s = oss.str();
}	

#if 0
void
process_view::info_to_buf(char *buf, int len)
{
	ENTRY_TRACE(__FILE__,__LINE__);
	string s;
	info_to_string(s);
	if (s.length() < unsigned(len))
		strcpy(buf, s.c_str());
	else {
		strncpy(buf, _("[error: overflow]\n"), len);
		buf[len-1] = 0;  // terminate
	};
}
#endif /* 0 */


process_view *
process_view::superior()
{
	if (order_ == orders_.end())
	    return NULL;
	// I don't know how to check for moving our iterator
	// off the front.  I assumed it would return end(),
	// but maybe not.  The next check is paranoia.
	if (order_ == orders_.begin())
	    return NULL;
	process_view_order_t::iterator superior(order_);
	--superior;
	return superior == orders_.end() ? NULL : *superior;
}

// static
const double
process_view::dir_to_multiplier(int dir)
{
	static double step = pow(2.0, 1.0/8.0);  // so 8 steps doubles in area
	switch (dir) {
	case 1: return step;
	case -1: return 1.0/step;
	case 0: return 1.0;
	default: assert(0);
	};
	/*NOTREACHED*/
}


void
process_view::mem_adjust(int dir)
{
//	// make sure we're inited
//	if (size_mem_to_screen_scale_.current() == 0.0)
//		size_mem_to_screen_scale_.tick_set(1.0);

	// try to scale memory either bigger (dir==1), smaller (dir==-1), orjust init stuff (dir==0)
	double change = dir_to_multiplier(dir);
	size_mem_to_screen_scale_.tick_scale(change);

	PV_AUTOSIZE_DEBUG(cout << O_("mem_adjust dir=") << dir << O_(" change = ") << change << O_(" size_mem_to_screen_scale_ = ") << size_mem_to_screen_scale_.current() << endl);
	show_size_ += 2;  // just for debugging
}

void
process_view::mem_target_adjust(int dir)
{
	/*
	 * Recompute our target size ranges.
	 */
	// 1. figure out the size of the screen real-estate
	int available_area = blob::available_area();
	// 2. need to change things?
	if (dir)
		fraction_of_total_screen_area_for_target_size_ *= dir_to_multiplier(dir);
	PV_AUTOSIZE_DEBUG(cout << O_("mem_target_adjust dir=") << dir << O_ (" fraction_of_total_screen_area_for_target_size_ =") << fraction_of_total_screen_area_for_target_size_ << endl);
	show_size_ += 2;
	int desired_area = int(available_area * fraction_of_total_screen_area_for_target_size_);
	size_low_water_ = int(desired_area * dir_to_multiplier(-1));
	size_high_water_ = int(desired_area * dir_to_multiplier(1));
	// make things larger or smaller
	if (dir)
		mem_adjust(dir);
	else if (dir == 0) { // initializing
		/*
		 * It can take MANY iterations to "get it right",
		 * so if there's no default, jump to the right place.
		 */
		double target = double(desired_area) / double(memory_sum_t::sum_);
		size_mem_to_screen_scale_ = target;
		PV_AUTOSIZE_DEBUG(cout << O_("mem_target_adjust jumping to size_mem_to_screen_scale_=") << size_mem_to_screen_scale_.current() << endl);
	};
}

/*
 * process_view::autosize()
 * Scale the blobs to a reasonable size, if desired.
 *
 * compare sum_ (the cumulative scaled size of all blobs)
 * vs high- and low-watermarks and adjust the scaling factor,
 * if necessary.
 */
void
process_view::autosize()
{
	if (!allow_autosize)
		return;

	if (size_low_water_ < 0) {
		PV_AUTOSIZE_DEBUG(cout << O_("mem_target_adjust(0);\n"));
		mem_target_adjust(0);  // compute watermarks, first pass through
	}
	if (size_mem_to_screen_scale_.changed()) {
		blob::size_mem_to_screen_commit();
		size_mem_to_screen_scale_.commit();
	};

	if (show_size_) {
		PV_AUTOSIZE_DEBUG(cout << O_("limits: sum: ") << memory_sum_t::sum_ << " in [" << size_low_water_ << "," << size_high_water_ << "]\n");
		show_size_--;
	};
	if (memory_sum_t::sum_ < size_low_water_) {
		mem_adjust(1);
		blob::splash(_("mem grow\n"));
	} else if (memory_sum_t::sum_ > size_high_water_) {
		mem_adjust(-1);
		blob::splash(_("mem shrink\n"));
	};
}

int
process_view::pid()
{
	// This function can't be inlined because of recursion between
	// the class definitions.
	return pm_->pid();
}

int
process_view::uid()
{
	// This function can't be inlined because of recursion between
	// the class definitions.
	return pm_->uid_;
}

const_str
process_view::cmd()
{
	// This function can't be inlined because of recursion between
	// the class definitions.
	return pm_->cmd_;
}
