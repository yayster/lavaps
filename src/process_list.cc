
/*
 * process_list.cc
 * Copyright (C) 1999-2001 by John Heidemann
 * $Id: process_list.cc,v 1.30 2004/12/10 19:47:27 johnh Exp $
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

#include <unistd.h>
#include <time.h>

#include <iostream>
#include <algorithm> // for for_each
#include <utility>  // for pair

#include "main.hh"
#include "process_list.hh"
#include "process_scan.hh"


#if 0
#define PL_DEBUG(x) x
#else /* ! 0 */
#define PL_DEBUG(x) 
#endif /* 0 */




// who'd'a thunk it:
// (sigh...egcs-2.90.29 won't let me make this local to process_list::reset_founds()
struct reset_found_f : public unary_function<pair<const pid_t,process_model*>,void> {
	void operator()(pair<const pid_t,process_model*> p) { p.second->found_ = false; }
};

void
process_list::reset_founds()
{
	ENTRY_TRACE(__FILE__,__LINE__);
	for_each(procs_.begin(), procs_.end(), reset_found_f());
}


struct schedule_reap_unfound_f : public unary_function<pair<const pid_t,process_model*>,void> {
	schedule_reap_unfound_f(process_list *pl) : pl_(pl) {}
	void operator()(pair<const pid_t,process_model*> p) { if (!p.second->found_) pl_->death(p.second); }
	process_list *pl_;
};

struct actually_reap_f : public unary_function<process_model*,void> {
	actually_reap_f(process_list::process_list_map_t *procs_p) : procs_p_(procs_p) {}
	void operator()(process_model* pm) { procs_p_->erase(pm->pid()); delete pm; }
	process_list::process_list_map_t *procs_p_;
};

void
process_list::reap_unfounds()
{
	ENTRY_TRACE(__FILE__,__LINE__);
	pms_to_reap_.clear();
	// figure out what to reap
	for_each(procs_.begin(), procs_.end(), schedule_reap_unfound_f(this));
	// now, reap it
	for_each(pms_to_reap_.begin(), pms_to_reap_.end(), actually_reap_f(&procs_));
	pms_to_reap_.clear();
}


void
process_list::birth(process_model *pm, time_t now)
{
	ENTRY_TRACE(__FILE__,__LINE__);
	// assert( procs_[pid] doesn't exist)
	procs_[pm->pid()] = pm;
	common(pm, now);
	pm->init_view();
	PL_DEBUG(cout << pm->pid() << O_(" birth\n"));
}

void
process_list::life(process_model *pm, time_t now)
{
	ENTRY_TRACE(__FILE__,__LINE__);
	common(pm, now);
	pm->update_view();
}

void
process_list::common(process_model *pm, time_t now)
{
	ENTRY_TRACE(__FILE__,__LINE__);
	pm->found_ = true;
	pm->age(now);
	pm->check_inmem_pct();
}

void
process_list::death(process_model *pm)
{
	ENTRY_TRACE(__FILE__,__LINE__);
	PL_DEBUG(cout << pm->pid() << O_(" death\n"));
	// don't do this:
	// procs_.erase(pm->pid());
	// delete pm;
	//
	// instead, do this:
	pms_to_reap_.push_back(pm);
	// why? Perhaps erasing pm while in the map breaks the iteration
	// over the map; instead, queue them to delete and toss them afterwards.
	// (Original way worked for me, but not Mikhail Teterin.)
}


struct dump_f : public unary_function<pair<const pid_t,process_model*>,void> {
	void operator()(pair<const pid_t,process_model*> p) {
		p.second->dump();
	}
};

void
process_list::dump()
{
	ENTRY_TRACE(__FILE__,__LINE__);
	for_each(procs_.begin(), procs_.end(), dump_f());
}


void
process_list::scan()
{
	ENTRY_TRACE(__FILE__,__LINE__);

	process_scan *scanner = process_scan::open_platform();

	time_t now = time(NULL);
	reset_founds();
	while (scanner->next()) {
		if (filter_style == FILTER_BY_PID && scanner->cur_pid() != filter_good_pid)  // for debugging
			continue;
		if (filter_style == FILTER_BY_UID && scanner->cur_uid() != filter_good_uid)
			continue;
		process_list_map_t::iterator pi = procs_.find(scanner->cur_pid());
		if (pi != procs_.end()) {
			process_model *pm = pi->second;
			scanner->life(pm);
			pm->found_ = true;
			life(pm, now);   // do generic stuff
		} else {
			process_model *pm = scanner->birth();
			pm->found_ = true;
			birth(pm, now);
		};
	};
	delete scanner;
	reap_unfounds();

	// If we intended to switch real<=>virtual mem,
	// we've now done so, so forget about the change.
	report_vm.commit();

	// autosize, if necessary
	process_view::autosize();
}
