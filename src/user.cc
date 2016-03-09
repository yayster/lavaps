
/*
 * user.cc
 * Copyright (C) 1999-2001 by John Heidemann
 * $Id: user.cc,v 1.3 2001/08/15 15:17:56 johnh Exp $
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

#include <unistd.h>
#include <pwd.h>
#include <sys/types.h>
#include <string.h>

#include <map>

#include "user.hh"


struct ltuid {
	bool operator()(const uid_t u1, const uid_t u2) const {
		return u1 < u2;
	};
};


typedef map<const uid_t, const char *, ltuid> uid_to_name_map_t;
static uid_to_name_map_t uid_to_name_map;

const char *
uid_to_name(const uid_t uid)
{
	uid_to_name_map_t::iterator hit = uid_to_name_map.find(uid);
	if (hit == uid_to_name_map.end()) {
		struct passwd *pwe = getpwuid(uid);
		const char *name = strdup((pwe == NULL) ? "???" : pwe->pw_name);
		uid_to_name_map[uid] = name;
	};
	return uid_to_name_map[uid];
}


