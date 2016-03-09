
/*
 * main.cc
 * Copyright (C) 1999-2001 by John Heidemann
 * $Id: main.cc,v 1.34 2004/12/17 19:59:03 johnh Exp $
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
#include "process_scan.hh"

#include <unistd.h>
#include <stdlib.h>
#include <time.h>

#include <iostream>
#include <string>


#include "main.hh"

#ifdef USE_TCL_BLOB
#include "tcl_blob.hh"
#endif /* USE_TCL_BLOB */

#ifdef USE_TEXT_BLOB
#include "text_blob.hh"
#endif /* USE_TEXT_BLOB */


int filter_good_uid = 0,
	filter_good_pid = 0;
bool allow_autosize = true;
bool lava_debug = false;
bool lava_wm_sticky = true;
const char *lava_default_geometry = NULL;
proc_filter_style_enum filter_style = FILTER_NOTHING;
explicit_change_tracking<vm_style_enum> report_vm(ERROR_MEM);  // resources set it
string default_resources = "";


void
usage()
{
	cout << _("usage: lavaps\n\tUse the about menu for more details,\n\tor look at the man page with \"man lavaps\".\n\n");
	exit(1);
}

void
die(const char *s)
{
	cerr << s;
	exit(1);
}

int
lava_random(int limit)
{
	static int inited = 0;
	if (!inited) {
		inited = 1;
		srandom(time(NULL));
	};
	if (limit == 0)
		return 0;
	else return random() % limit;
}

#ifdef HAVE_POPT_H
enum popt_options_enum {
	OPTION_GEOMETRY = 1,
	OPTION_DEBUG,
	OPTION_STICKY_ON,
	OPTION_STICKY_OFF,
};
static void popt_parse_options_callback (poptContext              ctx,
                                    enum poptCallbackReason  reason,
                                    const struct poptOption *opt,
                                    const char              *arg,
                                    void                    *data);
struct poptOption popt_options[] = {
	{
		NULL, 
		'\0', 
		POPT_ARG_CALLBACK,
		(void*)popt_parse_options_callback, 
		0, 
		NULL, 
		NULL
	},
	{
		"geometry",
		'\0',
		POPT_ARG_STRING,
		NULL,
		OPTION_GEOMETRY,
		N_("X geometry specification (see \"X\" man page), can be specified once per window to be opened."),
		N_("GEOMETRY")
	},
	{
		"debug",
		'd',
		POPT_ARG_NONE,
		NULL,
		OPTION_DEBUG,
		N_("Enable debugging output."),
		NULL,
	},
	{
		"sticky",
		'\0',
		POPT_ARG_NONE,
		NULL,
		OPTION_STICKY_ON,
		N_("Make the lava lamp \"sticky\", appearing in all workspaces."),
		NULL,
	},
	{
		"no-sticky",
		'\0',
		POPT_ARG_NONE,
		NULL,
		OPTION_STICKY_OFF,
		N_("Make the lava lamp \"sticky\", appearing in all workspaces."),
		NULL,
	},
	POPT_TABLEEND
};

/* This code is copied from gnome-terminal.  Popt--Yuck! */
void
popt_parse_options_callback (poptContext ctx, enum poptCallbackReason reason,
			     const struct poptOption *opt, const char *arg,
			     void *data)
{
	assert(reason == POPT_CALLBACK_REASON_OPTION);
	switch (opt->val & POPT_ARG_MASK) {
	case OPTION_GEOMETRY:
		if (arg == NULL) {
			die (N_("--geometry option requires argument giving X-style geometry specification.\n"));
		};
		lava_default_geometry = strndup(arg, 1024); // leaks memory, but probably doesn't matter
		break;
	case OPTION_DEBUG:
		lava_debug = true;
		break;
	case OPTION_STICKY_ON:
		lava_wm_sticky = true;
		break;
	case OPTION_STICKY_OFF:
		lava_wm_sticky = false;
		break;
	default:
		die("popt_parse_options_callback: unhandled option.\n");
	};
}

#endif /* HAVE_POPT_H */


void
parse_args(int argc, char **argv)
{
#ifdef USE_GTK_BLOB
	// with gtk, we assume the gnome_program_init will handle options
	return;
#endif /* USE_GTK_BLOB */
#if 0
	int c;

        while ((c = getopt(argc, argv, "?")) != EOF)
                switch (c) {
		default:
			usage();
			break;
                };
#endif /* 0 */
	int i = 0;
	for (i = 1; i < argc; i++) {
		char *opt = argv[i];
		if (opt[0] == '-' && opt[1] == '-')
			opt++;   // handle -- same as - options
		char *optarg = NULL;
		if (i+1 < argc)
			optarg = argv[i+1];
		if (strcmp(argv[i], O_("-display")) == 0 && optarg) {
			(void) setenv(O_("DISPLAY"), optarg, 1);
			i++;
		} else if (strcmp(argv[i], O_("-geometry")) == 0 && optarg) {
			default_resources += O_("lavaps.geometry: ");
			default_resources += optarg;
			default_resources += "\n";
			i++;
			// save it for gtk version
			lava_default_geometry = optarg;
		} else if (strcmp(argv[i], O_("-swallowed")) == 0) {
			default_resources += O_("lavaps.swallowed: false\n");
			i++;
		} else {
			usage();
			exit(1);
		};
	};
}

int
main(int argc, char **argv)
{
	parse_args(argc, argv);

	filter_good_uid = getuid();
	// do way early initialization so we can drop setuid/setgid before doing any tcl code
	process_scan::init();
	blob::init(argc, argv);
	blob::mainloop();
}
