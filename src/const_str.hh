/* -*-	Mode:C++ -*- */

/*
 * const_str.hh
 * Copyright (C) 1999 by John Heidemann
 * $Id: const_str.hh,v 1.6 2003/06/23 01:35:19 johnh Exp $
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


#ifndef lavaps_const_str_h
#define lavaps_const_str_h

#include <string.h>

class const_str {
protected:
	const char *rep_;
	static const char *safe_strdup(const char *s) { return s ? strdup(s) : NULL; }
	static void safe_free(const char *s) { if (s) free((void*)s); }

public:
	const_str() { rep_ = NULL; }
	const_str(char *s) { rep_ = safe_strdup(s); }
	const_str(const const_str& cs) { rep_ = safe_strdup(cs.c_str()); }

	~const_str() { safe_free(rep_); }

	const const_str& operator=(const const_str& cs) { safe_free(rep_); rep_ = safe_strdup(cs.c_str()); return *this; }
	const const_str& operator=(const char *s) { safe_free(rep_); rep_ = safe_strdup(s); return *this; }

	const bool operator==(const_str& cs) { return strcmp(rep_, cs.c_str()) == 0; };
	const bool operator==(const char *s) { return strcmp(rep_, s) == 0; };

	const int length() const { return strlen(rep_); }
	const char *c_str() const { return rep_; }
};

#endif /* lavaps_const_str_h */
