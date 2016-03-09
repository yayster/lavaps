/* this file is auto generated from the makefile */
/* DO NOT EDIT DIRECTLY! */
#ifdef USE_GTK_BLOB
#include "gtk_lava_help.hh"
gtk_lava_help_text *gtk_lava_help_text::first_ = NULL;
static gtk_lava_help_text glht_about(O_("about"),
	N_(
	"\n<big>Lavaps 2.7</big>\n\n"
	"Copyright (C) 1998-2004 by John Heidemann. All rights reserved. Comments to <tt>johnh@isi.edu</tt>, latest version at <tt>http://www.isi.edu/~johnh/SOFTWARE/LAVAPS/</tt>.\n\n"
	"\n<big>Description</big>\n\n"
	"LavaPS is an interactive process-tracking program like ``top'', but with a much different attitude.  Rather than presenting lots of specific info in digital form, it tries to present certain important information in a graphical analog form.  The idea is that you can run it in the background and get a rough idea of what's happening to your system without devoting much concentration to the task.\n\n"
	"LavaPS was inspired by Mark Weiser's idea of calm computing in this paper:\n\n"
	"*  ``The Coming Age of Calm Technology'' by Mark Weiser and John Seely Brown. A revised version of Weiser and Brown. ``Designing Calm Technology'', PowerGrid Journal, v 1.01, <tt>http://powergrid.electriciti.com/1.01</tt> (July 1996). October, 1996.  <tt>http://www.ubiq.com/hypertext/weiser/acmfuture2endnote.htm</tt>.\n\n"
	"(This program dedicated to the memory of M.W.--I hope you would have thought it a good hack.)\n\n"
	"\n\n"
	), N_("Lavaps"));

static gtk_lava_help_text glht_basics(O_("basics"),
	N_(
	"\n<big>Controlling Lavaps</big>\n\n"
	"Basic LavaPS is quite simple. Blobs live and grow corresponding to processes in the system (see <i>\"BLOBS\"</i>). Clicking the left mouse button on a blob shows information about that process. Clicking the right mouse button pops up menus that let you control LavaPS (see <i>\"MENUS\"</i>). The \"base\" at the bottom of the lamp includes icons for the menus and resizing, and allows one to move the lamp around.\n\n"
	"\n<big>Blobs</big>\n\n"
	"LavaPS is all about blobs of virtual, non-toxic lava. Blobs in LavaPS correspond to processes in the system. When a process is started, a new blob is created. When a process exits, the corresponding blob disappears. Blob size is proportional to the amount of memory the process uses. Blob movement is proportional to the amount of time the process runs (if the process never runs, the blob will never move).\n\n"
	"Blobs show several things. First, the basic color (the hue) corresponds to the name of the program which is running.  Emacs is always one color, Netscape another (on my system, blue and yellow). Second, blobs get darker when the process doesn't run. Over time, the process will become nearly black and only  its border will remain colored. Finally, if both physical and virtual memory are shown, then the part of the process will be a slightly different color showing what percentage of the process is not in physical memory.\n\n"
	"There are some more subtle aspects of blob physiology: initial placement is dependent on the process id (blobs appear roughly left to right) and user id (processes for the same user start at the same height, with root's processes at the top).\n\n"
	"Blobs also move along the longer distance of the lamp: if you resize it they may change direction.\n\n"
	"Please don't ask me about the chemical composition of the virtual lava.\n\n"
	"\n\n"
	), N_("Controlling Lavaps"));

static gtk_lava_help_text glht_menus(O_("menus"),
	N_(
	"\n<big>Menus</big>\n\n"
	"The right mouse button pops up menus which control LavaPS, including:\n\n"
	"<b>Proc</b>: control the process under the menu: ``Nice'' it (make it take less CPU), unnice it (the reverse; only works if you're root), or send it a signal. Signals are terminate (the polite way to stop a process; ``terminate'' allows it to clean up after itself), kill (the forcefully way to stop something not listening to terminate), stop and continue (temporarily stop and resume the process), and hang-up (used to restart certain processes). Beware that these commands will be disabled if you don't have privileges to run them, and under some circumstances even ``kill'' won't stop a process.\n\n"
	"<b>Who</b>: track processes by <i>me</i> or by <i>everyone</i> (including root, httpd, etc.).\n\n"
	"<b>What</b>: track process physical or virtual memory or both. Most modern operating systems can keep all of a process in RAM (physical memory), or can let pages that aren't currently used float out to disk (virtual memory). Virtual memory is always a superset of physical memory. You can track either one, or both. When tracking both, virtual memory appears as a different colored strip down the middle of the process blob.\n\n"
	"<b>How</b>: general lava lamp details: sizing, speed, and window manager interaction. Putting too little or too much lava in your lava lamp would make it boring or overflow. LavaPS therefore usually runs with patent-pending lava <i>autosizing</i> where blobs fill about a quarter of the lamp. This feature can be turned off (at your peril) with the How menu. You can also control the desired size of the blobs (when autosizing is enabled) or the absolute size of the blobs (when it's not) with the <i>Grow</i> and <i>Shrink</i> options.\n\n"
	"Also under How, <i>Jumpier</i> and <i>Smoother</i> control the quality of lavaps animation. The smoother the animation, the higher the CPU overhead of lavaps (because of more frequent scans of the process table). Less smooth is more efficient.\n\n"
	"<b>Help</b>: you'll have to figure this one out on your own.\n\n"
	"<b>Quit</b>: this one's even harder than Help.\n\n"
	"\n\n"
	), N_("Menus"));

static gtk_lava_help_text glht_copyright(O_("copyright"),
	N_(
	"\n<big>Copyright</big>\n\n"
	"LavaPS is Copyright (C) 1998-2003 by John Heidemann.\n\n"
	"This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License, version 2, as published by the Free Software Foundation.\n\n"
	"This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.\n\n"
	"You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.\n\n"
	"\n\n"
	), N_("Copyright"));

#endif /* USE_GTK_BLOB */
