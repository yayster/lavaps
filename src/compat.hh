
/*
 * compat.hh
 * Copyright (C) 2000 by John Heidemann
 * $Id: compat.hh,v 1.2 2000/02/18 06:52:55 johnh Exp $
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

#ifndef HAVE_SETENV
extern "C" {
	int setenv(const char *name, const char *value, int overwrite);
}
#endif /* ! HAVE_SETENV */
