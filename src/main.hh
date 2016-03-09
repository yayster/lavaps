
/*
 * main.h
 * Copyright (C) 1999-2000 by John Heidemann
 * $Id: main.hh,v 1.37 2004/12/17 19:58:29 johnh Exp $
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

#ifndef lavaps_main_hh
#define lavaps_main_hh

#include <string>
#include <assert.h>

#include "config.h"
#include "compat.hh"

#include "entry_trace.hh"

#include "change_tracking.hh"

#ifdef HAVE_POPT_H
#include <popt.h>
extern struct poptOption popt_options[];
#endif /* HAVE_POPT_H */


extern void die(const char *s);
extern int lava_random(int range);
inline int sign(int a) { return (a < 0) ? -1 : ((a > 0) ? 1 : 0); }

extern void hsb_to_rgb(double H, double S, double B, int maximum, int *r, int *g, int *b);
extern string hsb_to_rgb_string(double H, double S, double B);
extern string rgb8_to_string(int r, int g, int b);
extern string rgb16_to_string(int r, int g, int b);


extern int filter_good_uid,
	filter_good_pid;
extern bool allow_autosize,
	lava_debug,
	lava_wm_sticky;
extern const char *lava_default_geometry;
enum proc_filter_style_enum { FILTER_ERROR, FILTER_BY_PID, FILTER_BY_UID, FILTER_NOTHING };
extern proc_filter_style_enum filter_style;
enum vm_style_enum { ERROR_MEM, VIRTUAL_MEM, PHYSICAL_MEM, BOTH_MEM };
extern explicit_change_tracking<vm_style_enum> report_vm;
extern string default_resources;

#define SHORT_STRING_LEN 128

// pedantic_assert is really just to document conditions
// (while I expect asserts to run at runtime)
#define pedantic_assert(X)

/*
 * gettext
 *
 * O_("") is to explicitly tag text that should not be translated
 * (like deubgging text).
 */
#ifdef HAVE_GETTEXT
#include <gnome.h>
// xxx: all this stuff is in gnome.h // yes gettext
// #include <libintl.h>
// #define _(String) gettext (String)
// #define gettext_noop(String) String
// #define N_(String) gettext_noop (String)
#define O_(String) String

#else /* ! HAVE_GETTEXT */

// no gettext
#define _(String) String
#define N_(String) String
#define O_(String) String
#define textdomain(Domain)
#define bindtextdomain(Package, Directory)
#endif /* HAVE_GETTEXT */

#endif /* ! lavaps_main_hh */
