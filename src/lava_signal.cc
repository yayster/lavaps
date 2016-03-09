
/*
 * lava_signal.cc
 * Copyright (C) 2000-2003 by John Heidemann
 * $Id: lava_signal.cc,v 1.5 2003/12/01 01:06:34 johnh Exp $
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

/*
 * This file contains an abstraction over the OS's kill/renice functions.
 */

#include "config.h"

#include "main.hh"

#include <sys/time.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <signal.h>

int
lava_signal(int pid, int sig)
{
       return kill(pid, sig);
}

struct signal_mapping {
	char *name; int sig;
};
static struct signal_mapping signal_mapping[] = {
	{ O_("HUP"), SIGHUP },
	{ O_("TERM"), SIGTERM },
	{ O_("KILL"), SIGKILL },
	{ O_("STOP"), SIGSTOP },
	{ O_("CONT"), SIGCONT },
	{ O_("USR1"), SIGUSR1 },
	{ O_("USR2"), SIGUSR2 },
	{ NULL, 0 }
};

int
lava_named_signal(int pid, const char *sig)
{
	int i;
	// map string to numeric signal
	for (i = 0; signal_mapping[i].name; i++) {
		if (strcmp(signal_mapping[i].name, sig) == 0)
			break;
	};
	if (!signal_mapping[i].name)
		return -1;
	return lava_signal(pid, signal_mapping[i].sig);
}

int
lava_renice(int pid, int niceness)
{
       int old_prio = getpriority(PRIO_PROCESS, pid);
       if (old_prio == -1)
	       return -1;
       return setpriority(PRIO_PROCESS, pid, old_prio + niceness);
}

