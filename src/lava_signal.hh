
/*
 * signal.hh
 * Copyright (C) 2000 by John Heidemann
 * $Id: lava_signal.hh,v 1.3 2003/06/14 06:05:07 johnh Exp $
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

extern int lava_named_signal(int pid, const char *sig);
extern int lava_signal(int pid, int sig);
extern int lava_renice(int pid, int niceness);

