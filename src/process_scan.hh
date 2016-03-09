/* -*-	Mode:C++ -*- */

/*
 * process_scan.hh
 * Copyright (C) 1999 by John Heidemann
 * $Id: process_scan.hh,v 1.4 2001/10/30 21:26:50 johnh Exp $
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


#ifndef lavaps_process_scan_h
#define lavaps_process_scan_h

#include "process_model.hh"

/*
 * Front end
 * for each platform's approach to reading the process table.
 */

class process_scan {
private:
	process_scan(const process_scan&);
	process_scan& operator=(const process_scan&);

public:
	process_scan() {};
	virtual ~process_scan() {}

	static void init();  // early initialization (before privs are dropped)
	static process_scan *open_platform();  // instantiate the platform version
	virtual bool next() = 0;  // fetch next process, returning false on done

	// these update it
	virtual process_model *birth() = 0;
	virtual void life(process_model *pm) = 0;

	virtual int cur_pid() = 0;
	virtual int cur_uid() = 0;
};

#endif /* lavaps_process_scan_h */

