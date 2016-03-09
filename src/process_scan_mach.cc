
/*
 * process_scan_mach.cc
 * Copyright (C) 1999-2003 by John Heidemann
 * $Id: process_scan_mach.cc,v 1.5 2003/12/01 01:05:25 johnh Exp $
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
 * Mac OS X code by Kevin Geiss <kevin@desertsol.com> 6/18/2003
 */

#include "config.h"

#ifdef USE_PROCESS_SCAN_MACH

extern "C" {
#include <sys/sysctl.h>
// Mach stuff.
#include <mach/shared_memory_server.h>
#include <mach/mach.h>
#include <mach/mach_error.h>
#include <mach/policy.h>
#include <mach/task_info.h>
#include <mach/thread_info.h>
#include <mach/time_value.h>
#include <mach/mach_traps.h>
#include <mach/mach_init.h>
#include <mach/task.h>
#include <mach/thread_act.h>
#include <mach/thread_info.h>
#include <mach/vm_map.h>
}

#include "process_scan.hh"
#include "main.hh"

class process_scan_mach : public process_scan
{
 private:
	process_scan_mach(const process_scan_mach&);
	process_scan_mach& operator=(const process_scan_mach&);

 protected:
	enum // Named constants.
	{
		B_PER_KB = 1024,
		MSECS_PER_SEC = 1000,
		USECS_PER_SEC = 1000000,
		USECS_PER_MSEC = USECS_PER_SEC / MSECS_PER_SEC,
	};
      
	void adjust_mem_sizes( const mach_port_t& task ); // Helper.
	void fetch_thread_cpu_times( const mach_port_t& task );

	struct kinfo_proc *processes_;
	int num_procs_;
	struct kinfo_proc *cur_proc_;
	struct task_basic_info basic_info_;
	int stime_;
	int utime_;

	char *extract_cmd();
	int extract_pid();
	int extract_uid();
	int extract_nice();
	int extract_utime_msec();
	int extract_stime_msec();
	int extract_starttime();
	int extract_size_kb();
	int extract_rssize_kb();

 public:
	static void init();
	process_scan_mach();
	virtual ~process_scan_mach();

	virtual bool next();
	virtual process_model *birth();
	virtual void life(process_model *pm);

	virtual int cur_pid() { return extract_pid(); };
	virtual int cur_uid() { return extract_uid(); };
};

void
process_scan::init()
{
	process_scan_mach::init();
}

void
process_scan_mach::init()
{
}

process_scan *
process_scan::open_platform()
{
	return (process_scan*)new process_scan_mach();
}

process_scan_mach::process_scan_mach(): processes_(0), num_procs_(0),
					cur_proc_(0),
					stime_(0), utime_(0)
{
	// ``Management Information Base'' (MIB) style name for the state.
        // See sysctl(3) in Mac OS X.
	const int NAMELEN = 4;
	int name[NAMELEN];
	name[0] = CTL_KERN;
	name[1] = KERN_PROC;
	name[2] = KERN_PROC_ALL;
	name[3] = 0;
	size_t buf_bytes;

	// Find out how much space we will need for the process info.
	assert( sysctl( name, NAMELEN, 0, &buf_bytes, 0, 0 ) >= 0 );

	// Allocate space for the process info we will get from sysctl.
	// The system tries to round up, so hopefully this will be big enough.
	processes_ = (kinfo_proc*)new char[ buf_bytes ];

	// Save the old size. sysctl updates the 'oldlenp' parameter with
	// the number of bytes sysctl copied to the buffer.
	int old_buf_bytes = buf_bytes;
	// Get info about all processes.
	// In the 'ps' implementation, they say that sysctl may fail if 
	// the system load is high. The man page doesn't mention
	// that possibility. But it's easy to implement a retry loop,
	// so better safe than sorry. The man page does say that access
	// to the sysctl system call is serialized, so maybe there is
	// a condition under which the lock contention gets too high.
	// This also gives us an opportunity to make sure our
	// pre-allocated buffer was big enough.
	const int NUM_TRIES = 100;
	int code = -1;
	for ( int tries = 0; (tries < NUM_TRIES) && (code != 0); tries++ )
	{
		buf_bytes = old_buf_bytes;
		code = sysctl( name, NAMELEN, processes_, &buf_bytes, 0, 0 );
		if ( code < 0 )
		{
			if ( errno != ENOMEM )
				sleep(1); // Hopefully things will work later.
			else // Our buffer isn't big enough.
			{
				// Ask for the length again.
				assert( sysctl( name, NAMELEN,
						0, &buf_bytes,
						0, 0 ) >= 0 );
				// Get a new buffer.
				delete[] (char*)processes_;
				processes_ = (kinfo_proc*)new char[ buf_bytes ];
				old_buf_bytes = buf_bytes;
			}
		}
	}
	if ( code != 0 ) // We never did get success.
	{
		perror(_("Failure calling sysctl"));
		assert( 0 );
	}

	// Determine how many processes there are, using the updated number
	// returned from the last sysctl.
	num_procs_ = buf_bytes / sizeof(kinfo_proc);

	// Make sure we have decent data.
	assert(num_procs_ > 0);
	assert(processes_ != 0);

	cur_proc_ = &processes_[-1];
	// Now cur_proc_ is guaranteed non-NULL from here on out.
	// => no need to assert() it in extract_*().
}

process_scan_mach::~process_scan_mach()
{
	if ( processes_ != 0 ) delete[] (char*)processes_;
	processes_ = cur_proc_ = 0;
	num_procs_ = 0;
}

bool
process_scan_mach::next()
{
	++cur_proc_;
	return cur_proc_ < &processes_[num_procs_];
}

process_model *
process_scan_mach::birth()
{
	process_model *pm = new process_model(extract_pid());

	// Fill in init-time only bits.
	pm->set_uid(extract_uid());
	pm->set_start_time(extract_starttime());

	// Fill in everything else.
	life(pm);

	return pm;
}

void
process_scan_mach::life(process_model *pm)
{
	// Convert the pid to a mach task.
	mach_port_t task;
	if ( task_for_pid(mach_task_self(), pm->pid(), &task) == KERN_SUCCESS )
        {
		// Get the basic info, including the cmd, mem usage, cpu usage.
		mach_msg_type_number_t task_info_outCnt = TASK_BASIC_INFO_COUNT;
		if ( task_info( task,
				TASK_BASIC_INFO,
				(int*)&basic_info_,
				&task_info_outCnt ) == KERN_SUCCESS )
		{
			pm->set_nice(extract_nice());
	        	pm->set_cmd( extract_cmd() );

			adjust_mem_sizes( task );

			// Now set the memory sizes, after adjustment.
			pm->set_virtual_size(extract_size_kb());
			pm->set_resident(extract_rssize_kb());
			
			// Fetch the cpu times from all threads.
			fetch_thread_cpu_times( task );

			// Set the cpu time info.
			pm->set_utime(extract_utime_msec());
			pm->set_stime(extract_stime_msec());
		}
	}
}

void
process_scan_mach::adjust_mem_sizes( const mach_port_t& task )
{
	// The task may need its virtual size adjusted by 2 segments,
	// if the 'split libraries' are mapped into the task's virtual
	// address space.
	vm_address_t address = GLOBAL_SHARED_TEXT_SEGMENT;
	vm_size_t size;
	vm_region_basic_info_data_64_t info;
	mach_msg_type_number_t infoCnt = VM_REGION_BASIC_INFO_COUNT_64;
	mach_port_t object_name;
	if ( vm_region_64(	task,
				&address,
				&size,
				VM_REGION_BASIC_INFO,
				(vm_region_info_t)&info,
				&infoCnt,
				&object_name ) == KERN_SUCCESS )
	{
		if (	info.reserved &&
			(size == (SHARED_TEXT_REGION_SIZE)) &&
			(basic_info_.virtual_size >
				(SHARED_TEXT_REGION_SIZE +
				SHARED_DATA_REGION_SIZE)) )
		{
			basic_info_.virtual_size -= 
				(SHARED_TEXT_REGION_SIZE +
				SHARED_DATA_REGION_SIZE);
		}
	}
}

void
process_scan_mach::fetch_thread_cpu_times( const mach_port_t& task )
{
	thread_port_array_t act_list;
	mach_msg_type_number_t act_listCnt;

	// Get a list of all the task's threads.
	if ( task_threads( task, &act_list, &act_listCnt ) != KERN_SUCCESS )
		return;

	utime_ = stime_ = 0;
	thread_basic_info thread_info_out;
	mach_msg_type_number_t thread_info_outCnt;
	// Get info on each thread.
	for ( int i = 0; i < act_listCnt; i++ )
	{
		thread_info_outCnt = THREAD_BASIC_INFO_COUNT;
		if ( thread_info( act_list[i],
					THREAD_BASIC_INFO,
					(integer_t*)&thread_info_out,
					&thread_info_outCnt ) == KERN_SUCCESS )
		{

			time_value_t t = thread_info_out.user_time;
			utime_ += t.seconds * MSECS_PER_SEC +
					t.microseconds / USECS_PER_MSEC;

			t = thread_info_out.system_time;
			stime_ += t.seconds * MSECS_PER_SEC +
					t.microseconds / USECS_PER_MSEC;
		}
	}
	// Tell mach to deallocate the memory it allocated for us.
	vm_deallocate( mach_task_self(),
			(vm_address_t)act_list,
			act_listCnt * sizeof(thread_port_array_t) );
}

char *process_scan_mach::extract_cmd()
{
	return cur_proc_->kp_proc.p_comm;
}
int process_scan_mach::extract_pid()
{
	return cur_proc_->kp_proc.p_pid;
}
int process_scan_mach::extract_uid()
{
	//return cur_proc_->kp_eproc.e_pcred.p_ruid;
	return cur_proc_->kp_eproc.e_ucred.cr_uid;
}
int process_scan_mach::extract_nice()
{
	return cur_proc_->kp_proc.p_nice;
}
int process_scan_mach::extract_utime_msec()
{
	return utime_ +
		basic_info_.user_time.seconds * MSECS_PER_SEC +
		basic_info_.user_time.microseconds / USECS_PER_MSEC;
}
int process_scan_mach::extract_stime_msec()
{
	return stime_ +
		basic_info_.system_time.seconds * MSECS_PER_SEC +
		basic_info_.system_time.microseconds / USECS_PER_MSEC;
}
int process_scan_mach::extract_starttime()
{
	return cur_proc_->kp_proc.p_starttime.tv_sec;
}
int process_scan_mach::extract_size_kb()
{
	return basic_info_.virtual_size / B_PER_KB;
}
int process_scan_mach::extract_rssize_kb()
{
	return basic_info_.resident_size / B_PER_KB;
}

#endif /* USE_PROCESS_SCAN_MACH */
