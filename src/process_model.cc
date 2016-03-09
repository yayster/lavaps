
/*
 * process_model.cc
 * Copyright (C) 1999-2001 by John Heidemann
 * $Id: process_model.cc,v 1.27 2003/07/15 03:32:09 johnh Exp $
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
#include <functional>

#include "process_model.hh"
#include "main.hh"


process_model::process_model(int pid) :
	pid_(pid),
	found_(true)
{
	age(time(NULL), true);
}

process_model::~process_model()
{
	ENTRY_TRACE(__FILE__,__LINE__);
	delete pv_;
}

void
process_model::age(time_t now, bool reinit)
{
	if (reinit || utime_.changed() || stime_.changed()) {
		last_run_time_ = now;
		age_increment_ = 4;
		exponential_age_.tick_set(1);
	} else if (now > next_age_time_) {
		age_increment_ *= 2;
		exponential_age_.tick_incr(1);
	} else exponential_age_.commit();
		

	next_age_time_ = last_run_time_ + age_increment_;
}

int
process_model::cmd_hashify(const char *cmd)
{
	// Hash function is the same as Tcl's.
	// see: generic/tclHash.c::HashString for details.
	// (Basically, hash*9 is good because it's fast and keeps
	// old values in both high bits and low bits.)
	int hash = 0;
	for (const unsigned char *cp = (const unsigned char*)cmd; *cp; cp++)
		hash += (hash << 3) + *cp;
	// we want to use 
	return hash;
}

void
process_model::set_cmd(const char *cmd)
{
	cmd_hash_.tick_set(cmd_hashify(cmd));
	if (cmd_hash_.changed())
		cmd_ = cmd;
}

void
process_model::dump()
{
	cout << int(this) << " "<< pid_ << " " << uid_ << " " << cmd_.c_str() << " " <<
		endl;
}

void
process_model::check_inmem_pct()
{
	if (virtual_size_.changed() || resident_.changed()) {
		int pm = resident_.current();
		int vm = virtual_size_.current();
		if (vm == 0)
			inmem_pct_.tick_set(100);
		else if (pm > vm)
			inmem_pct_.tick_set(100);
		else inmem_pct_.tick_set((pm * 100) / vm);
	} else {
		inmem_pct_.commit();
	};
}
