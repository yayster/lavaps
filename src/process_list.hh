/* -*-	Mode:C++ -*- */

/*
 * process_list.hh
 * Copyright (C) 1999 by John Heidemann
 * $Id: process_list.hh,v 1.18 2004/12/10 19:47:35 johnh Exp $
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


#ifndef lavaps_process_list_h
#define lavaps_process_list_h

#include <map>

#include "process_model.hh"


//  we assume sizeof(pid_t) <= sizeof(int).  (only the back-end code really knows about pid_t.

class process_list {

private:
	process_list(const process_list& tb);
	process_list& operator=(const process_list& tb);

protected:
	typedef map<const int, process_model *, less<int> > process_list_map_t;
	process_list_map_t procs_;

	// xxx: why won't egcs let me make this local to reset_founds?
	void reset_founds();
	friend class schedule_reap_unfound_f;
	friend class actually_reap_f;
	void reap_unfounds();
	list<process_model *> pms_to_reap_;

	void birth(process_model *pm, time_t now);
	void life(process_model *pm, time_t now);
	void common(process_model *pm, time_t now);  // common stuff to birth and life
	void death(process_model *);
public:
	process_list() {};
	void scan();
	void dump();
};

#endif /* lavaps_process_list_h */

