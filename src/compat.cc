
/*
 * compat.cc
 * Copyright (C) 2000-2003 by John Heidemann
 * $Id: compat.cc,v 1.8 2003/12/01 01:06:28 johnh Exp $
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
#include <stdlib.h>
#include <string.h>  // for strlen
#include <stdarg.h>  // for va_list etc.
#ifdef HAVE_STDIO_H
#include <stdio.h>  // for snprintf
#endif /* HAVE_STDIO_H */

#include "compat.hh"

// intl
#define O_(String) String


#ifndef HAVE_SNPRINTF
/*
 * This implementation maps overflow into a hard failure.
 */
int
snprintf(char *buf, int size, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);

	/* can't assume vsprintf returns len, since sunos returns char* */
	vsprintf(buf, fmt, ap);
	int n = strlen(buf);
	if (n >= size)
		abort();
	return n;
}
#endif /* HAVE_SNPRINTF */

#ifndef HAVE_SETENV
int
setenv(const char *name, const char *value, int overwrite)
{
	char buf[256];
	snprintf(buf, sizeof(buf), O_("DISPLAY=%s"), value);
	return putenv(buf);
}
#endif /* ! HAVE_SETENV */

