
/*
 * entry_trace.hh
 * Copyright (C) 1999 by John Heidemann
 * $Id: entry_trace.hh,v 1.6 1999/09/06 18:16:16 johnh Exp $
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

#ifndef lavaps_entry_trace_h
#define lavaps_entry_trace_h


#ifdef USE_ENTRY_TRACE

struct entry_trace_ring {
	const char *file_;
	int line_;
};

class entry_trace {
private:
	static const int RING_SIZE_ = 16;
	static entry_trace_ring ring_[RING_SIZE_];
	static int index_;
public:
	static void enter(const char*f, int l) {
		ring_[index_].file_ = f;
		ring_[index_].line_ = l;
		index_ = (index_+1) % RING_SIZE_;
	};
	static void dump();
};

#define ENTRY_TRACE(f,l) entry_trace::enter(f,l);

#else /* ! USE_ENTRY_TRACE */

#define ENTRY_TRACE(f,l) 

#endif /* USE_ENTRY_TRACE */


#endif /* ! lavaps_entry_trace_h */
