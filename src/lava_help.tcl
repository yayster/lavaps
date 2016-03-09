proc init_help {} { global help
	set help(about) {
<big>Lavaps 2.7</big>

Copyright (C) 1998-2004 by John Heidemann. All rights reserved. Comments to <computer>johnh@isi.edu</computer>, latest version at <computer>http://www.isi.edu/~johnh/SOFTWARE/LAVAPS/</computer>.


<big>Description</big>

LavaPS is an interactive process-tracking program like ``top'', but with a much different attitude.  Rather than presenting lots of specific info in digital form, it tries to present certain important information in a graphical analog form.  The idea is that you can run it in the background and get a rough idea of what's happening to your system without devoting much concentration to the task.

LavaPS was inspired by Mark Weiser's idea of calm computing in this paper:

*  ``The Coming Age of Calm Technology'' by Mark Weiser and John Seely Brown. A revised version of Weiser and Brown. ``Designing Calm Technology'', PowerGrid Journal, v 1.01, <computer>http://powergrid.electriciti.com/1.01</computer> (July 1996). October, 1996.  <computer>http://www.ubiq.com/hypertext/weiser/acmfuture2endnote.htm</computer>.

(This program dedicated to the memory of M.W.--I hope you would have thought it a good hack.)



}
	set help(basics) {
<big>Blobs</big>

LavaPS is all about blobs of virtual, non-toxic lava. Blobs in LavaPS correspond to processes in the system. When a process is started, a new blob is created. When a process exits, the corresponding blob disappears. Blob size is proportional to the amount of memory the process uses. Blob movement is proportional to the amount of time the process runs (if the process never runs, the blob will never move).

Blobs show several things. First, the basic color (the hue) corresponds to the name of the program which is running.  Emacs is always one color, Netscape another (on my system, blue and yellow). Second, blobs get darker when the process doesn't run. Over time, the process will become nearly black and only  its border will remain colored. Finally, if both physical and virtual memory are shown, then the part of the process will be a slightly different color showing what percentage of the process is not in physical memory.

There are some more subtle aspects of blob physiology: initial placement is dependent on the process id (blobs appear roughly left to right) and user id (processes for the same user start at the same height, with root's processes at the top).

Blobs also move along the longer distance of the lamp: if you resize it they may change direction.

Please don't ask me about the chemical composition of the virtual lava.


<big>Controlling Lavaps (tcl/tk)</big>

Basic LavaPS is quite simple. Blobs live and grow corresponding to processes in the system (see "BLOBS"). Clicking the left mouse button on a blob shows information about that process. Clicking the right mouse button pops up menus that let you control LavaPS (see "MENUS").



}
	set help(menus) {
<big>Menus (tcl/tk)</big>

The right mouse button pops up menus which control LavaPS, including:

<italic>Proc</italic>: control the process under the menu: ``Nice'' it (make it take less CPU), unnice it (the reverse; only works if you're root), or send it a signal. Signals are terminate (the polite way to stop a process; ``terminate'' allows it to clean up after itself), kill (the forcefully way to stop something not listening to terminate), stop and continue (temporarily stop and resume the process), and hang-up (used to restart certain processes). Beware that these commands will be disabled if you don't have privileges to run them, and under some circumstances even ``kill'' won't stop a process.

<italic>Who</italic>: track processes by <italic>me</italic> or by <italic>everyone</italic> (including root, httpd, etc.).

<italic>What</italic>: track process physical or virtual memory or both. Most modern operating systems can keep all of a process in RAM (physical memory), or can let pages that aren't currently used float out to disk (virtual memory). Virtual memory is always a superset of physical memory. You can track either one, or both. When tracking both, virtual memory appears as a different colored strip down the middle of the process blob.

<italic>How</italic>: controls blob sizing. Putting too little or too much lava in your lava lamp would make it boring or overflow. LavaPS therefore usually runs with patent-pending lava <italic>autosizing</italic> where blobs fill about a quarter of the lamp. This feature can be turned off (at your peril) with the How menu. You can also control the desired size of the blobs (when autosizing is enabled) or the absolute size of the blobs (when it's not) with the <italic>Grow</italic> and <italic>Shrink</italic> options.

<italic>Help</italic>: you'll have to figure this one out on your own.

<italic>Quit</italic>: this one's even harder than Help.



}
	set help(resources) {
<big>Resources (tcl/tk)</big>

The Tcl/Tk version of LavaPS can configured from X resources (only if they're loaded with xrdb) or with the file <computer>$HOME/.lavapsrc</computer>. In both cases, the format is like:

	lavaps.autosize: false

setting whatever resource you want (in this case autosize) to some value (false). In the <computer>.lavapsrc</computer> file, the ``lavaps'' before the period can be omitted.

The following resources are supported:

<italic>geometry</italic>  (default none). Specifies the initial window location and size in X-style (see X(1)).

<italic>who</italic>  (default me). Whose processes should we be watching, anyway? My processes (set to ``me'') or everyone's (set to... ``everyone''). Can also be the process id of a single process if you're very single-minded.

<italic>what</italic>  (default both). What kind of memory should blob size correspond to, either both, physical, or virtual.

<italic>autosize</italic>  (default true). Keep the blobs at a reasonable size by dynamically changing scaling?

<italic>debug</italic>  (default false).  Enable debugging messages.

<italic>checkInterval</italic>  (default 2000).  How frequently (in milliseconds) should we check to see who's run?  Defaults to 2 seconds which seems ``reasonable'' on my computer; shorten the interval if you want more frequent updates and have a faster computer (or are more tolerant than I am :-).

<italic>shaped</italic>  (default true). Allow lozenge control (see next).

<italic>lozenge</italic>  (default true). Make the lamp lava-lamp (lozenge) shaped using X11 shaped windows. Disabled if shaped is false.

<italic>clicklessInfo</italic>  (default false). If set, process information pops up without clicking. (Not yet fully working.)



}
	set help(copyright) {
<big>Copyright</big>

LavaPS is Copyright (C) 1998-2003 by John Heidemann.

This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License, version 2, as published by the Free Software Foundation.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.



}
	set help(howdoi) {
<big>How Do I... (tcl/tk)</big>

Q: The blobs are as jumpy as little rabbits, <italic>How do I make the animation smoother?</italic>

A: Set the checkInterval resource to a smaller value. <italic>Currently, Resources like checkInterval only work in the Tcl version.</italic>

Q: I'm running LavaPS on my Timex Sinclair and it consumes a lot of CPU, making my editor ed run slowly. <italic>How can I make LavaPS take less CPU?</italic>

A: Set the checkInterval resource to a larger value. <italic>Currently, Resources like checkInterval only work in the Tcl version.</italic>

Q: Lozenge-shaped LavaPS is so cool, but I keep loosing it on my 8000-pixel wide xinerama multi-screen display. <italic>How can I resize lozenge-shaped LavaPS since it doesn't have any title bar?</italic>

A: In the Tcl/Tk version: (1) Read your window manager documentation, most have ways to resize windows other than the title bar (sometimes using a menu).  (2) Set the geometry explicitly with the -geometry command-line option or the geometry resource.  (3) Put the title bar back (unfortunately loosing the lozenge shape) by setting the ``shaped'' resource to ``false''.

In the Gtk version: the base has controls to let you move and resize the lava lamp.



}
}
