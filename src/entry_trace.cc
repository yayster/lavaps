
/*
 * entry_trace.cc
 * Copyright (C) 1999-2001 by John Heidemann
 * $Id: entry_trace.cc,v 1.6 2003/04/19 21:58:26 johnh Exp $
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

#ifdef USE_ENTRY_TRACE

#include <iostream>

#include "entry_trace.hh"


struct entry_trace_ring entry_trace::ring_[RING_SIZE_];
int entry_trace::index_;

void
entry_trace::dump()
{
	for (int i = 0; i < RING_SIZE_; i++) {
		cout << i << " " << ring_[i].file_ << ":" << ring_[i].line_ << (i == index_ ? "***" : " ") << endl;
	};
}

static void
entry_trace_dump()
{
	entry_trace::dump();
}

#endif /* USE_ENTRY_TRACE */
