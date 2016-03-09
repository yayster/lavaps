/* -*-	Mode:C++ -*- */

/*
 * process_scan_solaris.cc
 * Copyright (C) 2000-2003 by John Heidemann
 * $Id: process_scan_solaris.cc,v 1.6 2003/12/01 01:05:39 johnh Exp $
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
 * Contributed from 
 * Alan Coopersmith <Alan.Coopersmith@eng.sun.com>.
 * (Thanks!)
 */

#include "config.h"

#ifdef USE_PROCESS_SCAN_SOLARIS

#include <time.h>
#include <procfs.h>
#include <sys/types.h>
#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>

#include "process_scan.hh"
#include "main.hh"


class process_scan_solaris : public process_scan {
private:
	process_scan_solaris(const process_scan_solaris&);
	process_scan_solaris& operator=(const process_scan_solaris&);

protected:
    	DIR *procdir_;
        psinfo_t procdata_;
        psinfo_t *proc_;

  //	int pages_to_kb(int pages);
	int timestruc_t_to_msec(timestruc_t *time);

public:
	process_scan_solaris();
	virtual ~process_scan_solaris();

	virtual bool next();
	virtual process_model *birth();
	virtual void life(process_model *pm);

        virtual int cur_pid() { assert(proc_ != NULL); return proc_->pr_pid; }
        virtual int cur_uid() { assert(proc_ != NULL); return proc_->pr_uid; }
};



void
process_scan::init()
{
}

process_scan *
process_scan::open_platform()
{
	return (process_scan*)new process_scan_solaris();
}

process_scan_solaris::process_scan_solaris() :
  process_scan()
{
  	if (NULL == (procdir_ = opendir(O_("/proc"))))
		die(_("cannot open /proc"));
}

process_scan_solaris::~process_scan_solaris()
{
	if (proc_) {
		proc_ = NULL;
	};
	if (procdir_) {
		closedir(procdir_);
		procdir_ = NULL;
	};
}

// int
// process_scan_solaris::pages_to_kb(int pages)
// {
// 	ENTRY_TRACE(__FILE__,__LINE__);
// 	static int pages_per_kb = 0;
// 	if (!pages_per_kb)
// 		pages_per_kb = getpagesize() / 1024;
// 	return pages * pages_per_kb;
// }

int
process_scan_solaris::timestruc_t_to_msec(timestruc_t *time)
{
	ENTRY_TRACE(__FILE__,__LINE__);
	return ((time->tv_sec * 1000000) + (time->tv_nsec / 1000));
}

bool
process_scan_solaris::next()
{
  	proc_ = NULL;
	do {
		struct dirent *next_proc = readdir(procdir_);
		if (next_proc == NULL) { // end of directory
			break;
		}
		if (next_proc->d_name[0] != '.') { // skip . & ..
			int status_file;
			char status_file_name[64];

			snprintf(status_file_name, sizeof(status_file_name),
				 O_("/proc/%s/psinfo"), next_proc->d_name);
			status_file = open(status_file_name, O_RDONLY);
			if (status_file >= 0) {
				int bytes_read = read(status_file,&procdata_,sizeof(procdata_));
				if (bytes_read >= (signed int) sizeof(procdata_)) {
					proc_ = &procdata_;
				}
				close(status_file);
			}
		}
	} while (proc_ == NULL);
  	return (proc_ != NULL);
}

process_model *
process_scan_solaris::birth()
{
  	assert(proc_ != NULL);
  	process_model *pm = new process_model(int(proc_->pr_pid));
	// fill in init-time only bits:
  	pm->set_uid(proc_->pr_uid);
  	pm->set_start_time(timestruc_t_to_msec(&(proc_->pr_start)));
	pm->set_cmd(proc_->pr_fname);
  	life(pm);
  	return pm;
}

void
process_scan_solaris::life(process_model *pm)
{
  	assert(proc_ != NULL);
  	pm->set_utime(timestruc_t_to_msec(&(proc_->pr_time)));
//	pm->set_stime(timestruc_t_to_msec(&(proc_->pr_stime)));
  	pm->set_virtual_size(proc_->pr_size);
  	pm->set_resident(proc_->pr_rssize);
	// have to track changing cmd's from exec - not on Solaris
//	pm->set_cmd(proc_->cmd);
  	pm->set_nice(proc_->pr_lwp.pr_nice);

}


#endif /* USE_PROCESS_SCAN_SOLARIS */
