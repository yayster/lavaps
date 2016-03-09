/* -*-	Mode:C++ -*- */

/*
 * process_scan_bsd.cc
 * Copyright (C) 1999-2001 by John Heidemann
 * $Id: process_scan_bsd.cc,v 1.20 2003/12/01 01:04:48 johnh Exp $
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
 * Process_scan_bsd.cc was originally freebsd-specific bug
 * now should support *bsd (for * >= {Free,Net})
 *
 * This file is generalized to handle several BSD variants.
 * It uses a style different than the process_scan_* style
 * with subclasses for OS-specialization.  Instead here,
 * the extract_* methods provide an abstract API to get
 * stuff out of kvm in version-dependent ways.
 * Ifdefs select which batch of extract_*'s to use.
 * The different method is because IMHO the C++ subclass
 * approach is too verbose (wrt class declarations)
 * for what's really just one-liners.
 */

#include "config.h"

#ifdef USE_PROCESS_SCAN_BSD

#if (defined(__APPLE__) && defined(__MACH__))
#define USE_MAC_OS_X
#warning MacOS code not tested.
#endif


#include <time.h>

extern "C" {
#include <stdio.h>
#include <err.h>
#include <string.h>
#include <vis.h>
#include <limits.h>
#include <sys/param.h>
#include <sys/sysctl.h> // KERN_PROC_ALL
#include <sys/types.h>
#include <sys/user.h>
#include <fcntl.h>
#include <kvm.h>
#include <unistd.h>
#include <sys/time.h> // for rusage
#include <sys/resource.h>

#ifdef USE_MAC_OS_X
#include <mach/task_info.h>
#include <mach/time_value.h>
#endif /* USE_MAC_OS_X */
}

#include "process_scan.hh"
#include "main.hh"


class process_scan_bsd : public process_scan {
private:
	process_scan_bsd(const process_scan_bsd&);
	process_scan_bsd& operator=(const process_scan_bsd&);

protected:
	static kvm_t *kvm_;
	struct kinfo_proc *kp_;
	int nentries_;
	struct kinfo_proc *cur_kp_;

	int pages_to_kb(int pages);
	int ticks_to_msec(int pages);
	int timeval_to_msec(struct timeval *tv);
#ifdef USE_MAC_OS_X
	int time_value_to_msec(time_value_t *time_value);
#endif /* USE_MAC_OS_X */


	// these next suite of functions are all can be bsd-variant-specific
	char *extract_cmd();
	int extract_pid();
	int extract_uid();
	int extract_nice();
	int extract_utime_msec();
	int extract_stime_msec();
	int extract_size_kb();
	int extract_rssize_kb();

public:
	static void init();
	process_scan_bsd();
	virtual ~process_scan_bsd();

	virtual bool next();
	virtual process_model *birth();
	virtual void life(process_model *pm);

	virtual int cur_pid() { return extract_pid(); };
	virtual int cur_uid() { return extract_uid(); };
};


kvm_t *process_scan_bsd::kvm_ = NULL;


void
process_scan::init()
{
	process_scan_bsd::init();
}

void
process_scan_bsd::init()
{
	assert(kvm_ == NULL);
	char errbuf[_POSIX2_LINE_MAX];
	kvm_ = kvm_openfiles(NULL, NULL, NULL, O_RDONLY, errbuf);
	if (!kvm_)
		die(_("error opening kvm (probably the program needs to be setgid kmem)\n"));
	// drop setgid now that we've done the hard work
	if (setgid(getgid()))
		die(_("cannot drop privileges\n"));
}

process_scan *
process_scan::open_platform()
{
	return (process_scan*)new process_scan_bsd();
}

process_scan_bsd::process_scan_bsd() :
	process_scan(),
	kp_(NULL),
	nentries_(0),
	cur_kp_(NULL)
{
	assert(kvm_ != NULL);
	kp_ = kvm_getprocs(kvm_, KERN_PROC_ALL, 0, &nentries_);
	assert(kp_ != NULL);
	assert(nentries_ > 0);
	cur_kp_ = &kp_[-1];
	// cur_kp_ guaranteed non-NULL from here on out
	// => no need to assert() it in extract_*().
}

process_scan_bsd::~process_scan_bsd()
{
	// bsd kvm lib handles overwriting
}

int
process_scan_bsd::pages_to_kb(int pages)
{
	ENTRY_TRACE(__FILE__,__LINE__);
	static int pages_per_kb = 0;
	if (!pages_per_kb) {
		pages_per_kb = getpagesize() / 1024;
		assert(pages_per_kb >= 1); // we should fail on VAXen...
// pages_per_kb = sysconf(_SC_PAGESIZE) / 1024; reqd for netbsd?
	};
	return pages * pages_per_kb;
}

int
process_scan_bsd::ticks_to_msec(int ticks)
{
#ifndef HZ
#define HZ 100
#endif
	ENTRY_TRACE(__FILE__,__LINE__);
	return ticks * 1000 / HZ;
}

int
process_scan_bsd::timeval_to_msec(struct timeval *tv)
{
	return tv->tv_sec * 1000 + (tv->tv_usec / 1000);
}

#ifdef USE_MAC_OS_X
int
process_scan_bsd::time_value_to_msec(time_value_t *time_value)
{
	return time_value->seconds * 1000 + (time_value->microseconds / 1000);
}
#endif /* USE_MAC_OS_X */


#if (__FreeBSD__ < 5) || defined(__NetBSD__) || defined(__OpenBSD__)
// the "old" way
char *process_scan_bsd::extract_cmd() { return cur_kp_->kp_proc.p_comm; }
int process_scan_bsd::extract_pid() { return int(cur_kp_->kp_proc.p_pid); }
int process_scan_bsd::extract_uid() { return int(cur_kp_->kp_eproc.e_pcred.p_ruid); }
int process_scan_bsd::extract_nice() { return int(cur_kp_->kp_proc.p_nice); }
int process_scan_bsd::extract_utime_msec() { return ticks_to_msec(cur_kp_->kp_proc.p_uticks); }
int process_scan_bsd::extract_stime_msec() { return ticks_to_msec(cur_kp_->kp_proc.p_sticks); }
#elif (__FreeBSD__ >= 5)
// FreeBSD 5 cleans things up...
char *process_scan_bsd::extract_cmd() { return cur_kp_->ki_comm; }
int process_scan_bsd::extract_pid() { return int(cur_kp_->ki_pid); }
int process_scan_bsd::extract_uid() { return int(cur_kp_->ki_uid); }
int process_scan_bsd::extract_nice() { return int(cur_kp_->ki_nice); }
int process_scan_bsd::extract_utime_msec() { return timeval_to_msec(&(cur_kp_->ki_rusage.ru_utime)); }
int process_scan_bsd::extract_stime_msec() { return timeval_to_msec(&(cur_kp_->ki_rusage.ru_stime)); }
#elif defined(USE_MAC_OS_X)
// this is NOT done 
char *process_scan_bsd::extract_cmd() { return cur_kp_->kp_proc.p_comm; }
int process_scan_bsd::extract_pid() { return int(cur_kp_->kp_proc.p_pid); }
int process_scan_bsd::extract_uid() { return int(cur_kp_->ki_eproc.e_ucred.cr_uid); }
int process_scan_bsd::extract_nice() { return int(cur_kp_->kp_proc.p_nice); }
int process_scan_bsd::extract_utime_msec() { return time_value_t_to_msec(kur_kp_->times.user_time) + time_value_t_to_msec(cur_kp_->tasks_info.user_time); }
int process_scan_bsd::extract_stime_msec() { return time_value_t_to_msec(kur_kp_->times.system _time) + time_value_t_to_msec(cur_kp_->tasks_info.system_time); }
#else
#error unknown bsd variant
#endif


#ifdef __NetBSD__
#  ifndef UPAGES
#    ifdef USPACE
#      define UPAGES   ( USPACE / sysconf(_SC_PAGESIZE) )
#    else /* !USPACE */
#      error No UPAGES, no USPACE, no fun!
#    endif /* USPACE */
#  endif /* !UPAGES */
#endif /* __NetBSD__ */
#if (__FreeBSD__ < 3) || defined(__NetBSD__) || defined(__OpenBSD__)
int
process_scan_bsd::extract_size_kb()
{
	return pages_to_kb(UPAGES + cur_kp_->kp_eproc.e_vm.vm_tsize
		+ cur_kp_->kp_eproc.e_vm.vm_dsize
		+ cur_kp_->kp_eproc.e_vm.vm_ssize);
}
int process_scan_bsd::extract_rssize_kb() { return pages_to_kb(UPAGES + cur_kp_->kp_eproc.e_vm.vm_rssize); }
#elif (__FreeBSD__ >= 3) && (__FreeBSD__ < 5)
int
process_scan_bsd::extract_size_kb()
{
	// under FreeBSD 3.0 and higher, ps uses this value, which is
	// kept in bytes.
	//
	// xxx: previously this added UPAGES, but that seems broken
	// since it's different units.
	return cur_kp_->kp_eproc.e_vm.vm_map.size/1024;
}
int process_scan_bsd::extract_rssize_kb() { return pages_to_kb(UPAGES + cur_kp_->kp_eproc.e_vm.vm_rssize); }
#elif (__FreeBSD__ >= 5)
int process_scan_bsd::extract_size_kb() { return int(cur_kp_->ki_size / 1024); }
int process_scan_bsd::extract_rssize_kb() { return pages_to_kb(cur_kp_->ki_rssize); }
#elif defined(USE_MAC_OS_X)
int process_scan_bsd::extract_size_kb() { return pages_to_kb(cur_kp_->tasks_info.virtual_size); }
int process_scan_bsd::extract_rssize_kb() { return pages_to_kb(cur_kp_->tasks_info.resident_size); }
#else
#error unknown bsd variant
#endif


bool
process_scan_bsd::next()
 {
	cur_kp_++;
	return cur_kp_ < &kp_[nentries_];
}

process_model *
process_scan_bsd::birth()
{
	process_model *pm = new process_model(extract_pid());
	// fill in init-time only bits:
	pm->set_uid(extract_uid());
	pm->set_nice(extract_nice());
	pm->set_start_time(0);  // xxx: not in bsd?
	life(pm);
	return pm;
}

void
process_scan_bsd::life(process_model *pm)
{
	pm->set_utime(extract_utime_msec());
	pm->set_stime(extract_stime_msec());
	pm->set_virtual_size(extract_size_kb());
	pm->set_resident(extract_rssize_kb());
	pm->set_cmd(extract_cmd());
}


#endif /* USE_PROCESS_SCAN_BSD */
