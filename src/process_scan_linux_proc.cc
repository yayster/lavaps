/* -*-	Mode:C++ -*- */

/*
 * process_scan_linux_proc.cc
 * Copyright (C) 1999-2003 by John Heidemann
 * $Id: process_scan_linux_proc.cc,v 1.11 2003/12/01 01:05:05 johnh Exp $
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

#ifdef USE_PROCESS_SCAN_LINUX_PROC

#include <time.h>
extern "C" {
#include <proc/readproc.h>
}
#include <asm/param.h>  // for HZ

#include "process_scan.hh"
#include "main.hh"


class process_scan_linux_proc : public process_scan {
private:
	process_scan_linux_proc(const process_scan_linux_proc&);
	process_scan_linux_proc& operator=(const process_scan_linux_proc&);

protected:
	PROCTAB *proctab_;
	proc_t *proc_;

	int pages_to_kb(int pages);
	int ticks_to_msec(int ticks);

public:
	process_scan_linux_proc();
	virtual ~process_scan_linux_proc();

	virtual bool next();
	virtual process_model *birth();
	virtual void life(process_model *pm);

	virtual int cur_pid() { assert(proc_ != NULL); return proc_->pid; }
	virtual int cur_uid() { assert(proc_ != NULL); return proc_->uid; }
};



void
process_scan::init()
{
}

process_scan *
process_scan::open_platform()
{
	// someday may fall back if /proc isn't available
	return (process_scan*)new process_scan_linux_proc();
}

process_scan_linux_proc::process_scan_linux_proc() :
	process_scan(), proctab_(NULL), proc_(NULL)
{
	if (NULL == (proctab_ = openproc(PROC_FILLMEM|PROC_FILLUSR)))
		die(_("cannot open /proc"));
}

process_scan_linux_proc::~process_scan_linux_proc()
{
	if (proc_) {
		freeproc(proc_);
		proc_ = NULL;
	};
	if (proctab_) {
		closeproc(proctab_);
		proctab_ = NULL;
	};
}

int
process_scan_linux_proc::pages_to_kb(int pages)
{
	ENTRY_TRACE(__FILE__,__LINE__);
	static int pages_per_kb = 0;
	if (!pages_per_kb)
		pages_per_kb = getpagesize() / 1024;
	return pages * pages_per_kb;
}

int
process_scan_linux_proc::ticks_to_msec(int ticks)
{
	ENTRY_TRACE(__FILE__,__LINE__);
	return ticks * 1000 / HZ;
}

bool
process_scan_linux_proc::next()
{
	if (proc_)
		freeproc(proc_);
	proc_ = readproc(proctab_, NULL);
	return (proc_ != NULL);
}

process_model *
process_scan_linux_proc::birth()
{
	assert(proc_ != NULL);
	process_model *pm = new process_model(int(proc_->pid));
	// fill in init-time only bits:
	pm->set_uid(proc_->uid);
	pm->set_nice(proc_->nice);  // xxx: should track changes
	pm->set_start_time(proc_->start_time);
	life(pm);
	return pm;
}

void
process_scan_linux_proc::life(process_model *pm)
{
	assert(proc_ != NULL);
	pm->set_utime(ticks_to_msec(proc_->utime));
	pm->set_stime(ticks_to_msec(proc_->stime));
	pm->set_virtual_size(pages_to_kb(proc_->size));
	pm->set_resident(pages_to_kb(proc_->resident));
	// have to track changing cmd's from exec
	pm->set_cmd(proc_->cmd);
}


#endif /* USE_PROCESS_SCAN_LINUX_PROC */
