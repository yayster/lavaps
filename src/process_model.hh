/* -*-	Mode:C++ -*- */

/*
 * process_model.hh
 * Copyright (C) 1999 by John Heidemann
 * $Id: process_model.hh,v 1.26 2003/07/15 03:32:24 johnh Exp $
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


#ifndef lavaps_process_model_h
#define lavaps_process_model_h

#include <unistd.h>  // pid_t

#include "change_tracking.hh"
#include "process_view.hh"
#include "const_str.hh"

class process_model;
class process_view;

/*
 * process_model: a system-independent representation of the
 * interesting information about a process.
 */
class process_model {
	// most of these are to give access to the details
	// cleaner would be to have set/access functions for *each*
	// member variable.
	friend class process_view;

protected:
	process_model(const process_model& tb);
	process_model& operator=(const process_model& tb);

	// used for tracking process age via age()
	time_t last_run_time_,
		next_age_time_;
	int age_increment_;
	change_tracking<int> exponential_age_;

	process_view *pv_;

	// rest is the details:
	int pid_;   // required!
	int uid_;
	// should add tty?  only when we want to use it.
	const_str cmd_;   // program name (just the program, not the whole cmd line)
	change_tracking<int> cmd_hash_;   // hash of cmd name

	// int priority_;   // kernel adjusted prio
	int nice_;       // user-supplied nice value
	time_t start_time_;
	change_tracking<time_t> utime_;  // cumulative user time (in msec)
	change_tracking<time_t> stime_;  // cumulative system time

	// use KB below to avoid unsigned ugliness
	change_tracking<int> virtual_size_;      // total memory size in KiB
	change_tracking<int> resident_;      // resident memory size in KiB (could be partially shared)
	change_tracking<int> inmem_pct_;	// % of process in memory

	static int cmd_hashify(const char *cmd);

public:
	bool found_;   // found this scan

	process_model(int pid);
	~process_model();

	void init_view() { pv_ = new process_view(this); }
	void update_view() { pv_->update(); }

	int age() { return exponential_age_.current(); }
	void age(time_t now, bool reinit = false);

	void set_uid(int a) { uid_ = a; }
	void set_nice(int a) { nice_ = a; }
	void set_start_time(int a) { start_time_ = a; }
	void set_cmd(const char *c);

	void set_utime(int t) { utime_.tick_set(t); }  // times in msec
	void set_stime(int t) { stime_.tick_set(t); }
	void set_virtual_size(int s) { virtual_size_.tick_set(s); }
	void set_resident(int s) { resident_.tick_set(s); }
	// void set_inmem_pct(int s) { inmem_pct_.tick_set(s); }

	void check_inmem_pct();

	int pid() { return pid_; }

	void dump();

	process_view *view() { return pv_; }
};

#endif /* lavaps_process_model_h */

