const char *lava_code = "\n\
\n\
proc init_canvas {} {\n\
global rdb\n\
if {[info exists rdb(geometry)]} {\n\
wm geometry . $rdb(geometry)\n\
}\n\
\n\
canvas .c -width 100 -height 200 -bg black\n\
pack append . .c {expand fill}\n\
\n\
bind . <Configure> \"canvas_configure %W %w %h %B C\"\n\
bind . <FocusIn> \"canvas_configure %W %w %h %B FI\"\n\
bind . <FocusOut> \"canvas_configure %W %w %h %B FO\"\n\
bind . <Map> \"canvas_configure %W %w %h %B M\"\n\
\n\
.c bind p <Any-ButtonPress-1> \"info_click_down %W %x %y %X %Y\"\n\
bind .c <Any-ButtonRelease-1> \"info_click_up\"\n\
bind .c <Any-B1-Motion> \"info_click_down %W %x %y %X %Y\"\n\
bind .c <Any-ButtonPress-3> \"info_click_up; tk_popup .menu %X %Y 0\"\n\
bind .c <Any-ButtonPress-2> \"info_click_up; tk_popup .menu %X %Y 0\"\n\
\n\
set_rdb_default_boolean clicklessinfo 0\n\
if {$rdb(clicklessinfo)} {\n\
bind .c <Any-Motion> \"info_click_down %W %x %y %X %Y\"\n\
}\n\
}\n\
\n\
proc canvas_configure {W w h B why} {\n\
global old_canvas_w old_canvas_h rdb\n\
\n\
if {\"$W\" != \".\"} {\n\
return\n\
}\n\
\n\
if {$why != \"C\"} {\n\
set w $old_canvas_w\n\
set h $old_canvas_h\n\
set b 0\n\
}\n\
. configure -height $h -width $w\n\
if {![info exists rdb(swallowed)]} {\n\
set_rdb_default_boolean swallowed 0\n\
}\n\
if {$rdb(swallowed)} {\n\
lava_resize $w $h\n\
} else {\n\
lava_resize $w $h [wm frame .]\n\
}\n\
set old_canvas_w $w\n\
set old_canvas_h $h\n\
\n\
splash_update_wh $w $h\n\
}\n\
\n\
proc init_menu {} {\n\
global menu rdb\n\
set m [menu .menu -tearoff 0 -postcommand proc_menu_post]\n\
\n\
set mw .menu.proc\n\
menu $mw -tearoff 0\n\
$mw add command -label \"Pid: 0\" -foreground white -activeforeground white\n\
$mw add separator\n\
$mw add command -label \"Nice\" -command \"send_signal nice\"\n\
set state disabled\n\
if {[whoami] == \"root\"} {\n\
set state normal\n\
}\n\
$mw add command -label \"Unnice\" -command \"send_signal unnice\" -state $state\n\
$mw add separator\n\
$mw add command -label \"Terminate (politely)\" -command \"send_signal TERM\"\n\
$mw add command -label \"Kill (forcefully)\" -command \"send_signal KILL\"\n\
$mw add command -label \"Stop (suspend)\" -command \"send_signal STOP\"\n\
$mw add command -label \"Continue (unsuspend)\" -command \"send_signal CONT\"\n\
$mw add command -label \"Hang up\" -command \"send_signal HUP\"\n\
$m add cascade -label \"Proc\" -menu $mw\n\
\n\
set mw .menu.who\n\
menu $mw -tearoff 0\n\
$mw add radiobutton -label \"Me\" -variable menu(who) -value me -command \"menu_set who me {my jobs\\n}\"\n\
$mw add radiobutton -label \"Everyone\" -variable menu(who) -value everyone -command \"menu_set who everyone {all jobs\\n}\"\n\
set_rdb_default who me\n\
set menu(who) $rdb(who)\n\
menu_set who $menu(who)\n\
$m add cascade -label \"Who\" -menu $mw\n\
\n\
set mw .menu.what\n\
menu $mw -tearoff 0\n\
$mw add radiobutton -label \"Virtual Memory\" -variable menu(what) -value virtual -command \"menu_set what virtual {virtual mem\\n}\"\n\
$mw add radiobutton -label \"Physical Memory\" -variable menu(what) -value physical -command \"menu_set what physical {physical mem\\n}\"\n\
$mw add radiobutton -label \"Both\" -variable menu(what) -value both -command \"menu_set what both {both mem\\n}\"\n\
set_rdb_default what virtual {virtual physical both}\n\
set menu(what) $rdb(what)\n\
menu_set what $rdb(what)\n\
$m add cascade -label \"What\" -menu $mw\n\
\n\
set mw .menu.how\n\
menu $mw -tearoff 0\n\
$mw add command -label \"Shrink\" -command {menu_set how shrink \"shrink mem\\n\"}\n\
$mw add command -label \"Grow\" -command {menu_set how grow \"grow mem\\n\"}\n\
set_rdb_default_boolean autosize 1\n\
set menu(how) $rdb(autosize)\n\
$mw add checkbutton -label \"Autosizing\" -variable menu(how) -onvalue 1 -offvalue 0 -command {menu_autosize force}\n\
set_rdb_default_boolean shaped [lava_menu shaped_window_ok]\n\
if {$rdb(shaped)} {\n\
set_rdb_default_boolean lozenge $rdb(shaped)\n\
set state normal\n\
} else {\n\
set rdb(lozenge) 0\n\
set state disabled\n\
}\n\
set menu(lozenge) $rdb(lozenge)\n\
$mw add checkbutton -label \"Lozenge\" -variable menu(lozenge) -onvalue 1 -offvalue 0 -command \"menu_shape\" -state $state\n\
menu_shape\n\
$m add cascade -label \"How\" -menu $mw\n\
\n\
set mw .menu.help\n\
menu $mw -tearoff 0\n\
$mw add command -label \"About...\" -command \"menuHelp about\"\n\
$mw add command -label \"Basics...\" -command \"menuHelp basics\"\n\
$mw add command -label \"Menus...\" -command \"menuHelp menus\"\n\
$mw add command -label \"Resources...\" -command \"menuHelp resources\"\n\
$mw add command -label \"How do I...\" -command \"menuHelp howdoi\"\n\
$mw add command -label \"Copyright...\" -command \"menuHelp copyright\"\n\
$m add cascade -label \"Help\" -menu $mw\n\
\n\
$m add separator\n\
$m add command -label \"Quit\" -command \"exit 0\"\n\
}\n\
\n\
proc menu_autosize {status} {\n\
global menu\n\
switch -exact $status {\n\
off {\n\
if {$menu(how) != 0} {\n\
set menu(how) 0\n\
menu_set how 0 \"manual resize\\n\"\n\
}\n\
}\n\
force {\n\
if {$menu(how)} {\n\
menu_set how 1 \"auto-resize\\n\"\n\
} else {\n\
menu_set how 0 \"manual resize\\n\"\n\
}\n\
}\n\
default {\n\
die \"bad menu_autosize argument\"\n\
}\n\
}\n\
}\n\
\n\
proc menu_shape {} {\n\
global rdb menu\n\
if {!$rdb(shaped)} {\n\
menu_set shape unshaped\n\
} elseif {$menu(lozenge)} {\n\
menu_set shape 25 \"lozenge\\n\"\n\
} else {\n\
menu_set shape 0 \"non-lozenge\\n\"\n\
};\n\
\n\
}\n\
\n\
proc menu_set {what token {feedback \"\"}} {\n\
global menu\n\
lava_menu $what $token\n\
if {$feedback != \"\"} {\n\
splash_refresh $feedback\n\
}\n\
}\n\
\n\
proc menu_set_through_menu_array {what {feedback \"\"}} {\n\
global menu\n\
lava_menu $what $menu($what)\n\
if {$feedback != \"\"} {\n\
splash_refresh $feedback\n\
}\n\
}\n\
\n\
proc init_debugging {} {\n\
global rdb\n\
set_rdb_default_boolean debug 0\n\
lava_menu debug $rdb(debug)\n\
}\n\
\n\
proc debugging {} {\n\
global rdb\n\
return $rdb(debug)\n\
}\n\
\n\
proc send_signal {kind} {\n\
global proc_menu_pid\n\
if {[catch {lava_menu signal $kind $proc_menu_pid} dummy]} {\n\
splash_refresh \"Cannot signal\\n\"\n\
} else {\n\
splash_refresh \"Signaled\\n\"\n\
}\n\
}\n\
\n\
proc whoami {} {\n\
global whoami\n\
if {![info exists whoami]} {\n\
set whoami [lava_menu whoami]\n\
}\n\
return $whoami\n\
}\n\
\n\
proc proc_menu_post {} {\n\
set rx [winfo pointerx .c]\n\
set ry [winfo pointery .c]\n\
set wx [winfo x .]\n\
set wy [winfo y .]\n\
set x [expr $rx - $wx]\n\
set y [expr $ry - $wy]\n\
set id [lindex [proc_id_status_at_xy $x $y] 0]\n\
\n\
set text [lava_id_to_info $id]\n\
foreach {ln} [split $text \"\\n\"] {\n\
if {[regexp {^([^:]+): +(.*)$} $ln match key value]} {\n\
set stuff($key) $value\n\
}\n\
}\n\
\n\
set pid $stuff(pid)\n\
set user [string trim [lindex [split $stuff(user) \" \"] 1] \"()\"]\n\
set me [whoami]\n\
set cmd [string range $stuff(cmd) 0 19]\n\
\n\
global proc_menu_pid\n\
set proc_menu_pid $pid\n\
\n\
.menu.proc entryconfigure {Pid*} -label \"Pid: $pid ($cmd)\"\n\
set blob_color [lindex [.c itemconfigure $id -fill] 4]\n\
.menu.proc entryconfigure {Pid*} -background $blob_color -activebackground $blob_color\n\
if {$me == 0 || $user == $me} {\n\
set state normal\n\
} else {\n\
set state disabled\n\
}\n\
foreach {i} {Nice Unnice {Hang*} {Kill*} {Stop*} {Continue*} {Terminate*}} {\n\
.menu.proc entryconfigure $i -state $state\n\
}\n\
}\n\
\n\
proc init_info {} {\n\
global rdb\n\
toplevel .info -relief raised -borderwidth 2 -class LavaPS\n\
wm transient .info .\n\
wm overrideredirect .info 1\n\
wm group .\n\
wm withdraw .info\n\
wm iconname .info \"\"\n\
raise .info\n\
global info_default_wh\n\
set info_default_wh {24 6}\n\
if {[debugging]} {\n\
set info_default_wh {30 7}\n\
}\n\
label .info.color -height 1 -width [lindex $info_default_wh 0] -text \" \"\n\
text .info.text -width [lindex $info_default_wh 0] -height [lindex $info_default_wh 1]\n\
pack .info.color\n\
pack .info.text\n\
global info_last_x info_last_y info_last_id\n\
set info_last_x -1\n\
set info_last_y -1\n\
set info_last_id -1\n\
if {$rdb(clicklessinfo)} {\n\
bind .info <Any-Motion> \"info_click_down_in_info %W %x %y %X %Y\"\n\
}\n\
bind .info <Any-ButtonPress-3> \"tk_popup .menu %X %Y 0\"\n\
bind .info <Any-ButtonPress-2> \"tk_popup .menu %X %Y 0\"\n\
}\n\
\n\
proc info_click_down_in_info {w x y rx ry} {\n\
puts \"icdii: $w $x $y $rx $ry\"\n\
info_click_down $w $x $y $rx $ry\n\
}\n\
\n\
proc proc_id_status_at_xy {x y} {\n\
global info_last_x info_last_y info_last_id\n\
\n\
set try_again 1\n\
while {$try_again} {\n\
set id [.c find closest $x $y]\n\
\n\
if {$info_last_id == $id && $info_last_x == $x && $info_last_y == $y} {\n\
\n\
return [list $id identical]\n\
}\n\
if {$info_last_id == $id} {\n\
return [list $id same-moved]\n\
}\n\
global splash_id\n\
if {$id == $splash_id} {\n\
splash_destroy\n\
set try_again 1\n\
} else {\n\
set try_again 0\n\
}\n\
}\n\
set info_last_x $x\n\
set info_last_y $y\n\
set info_last_id $id\n\
return [list $id new]\n\
}\n\
\n\
proc info_position {c rx ry} {\n\
global info_width info_height root_width root_height\n\
if {![info exists info_width] || $info_width == 1} {\n\
set root_width [winfo screenwidth $c]\n\
set root_height [winfo screenheight $c]\n\
update idletasks\n\
set info_width [winfo width $c]\n\
set info_height [winfo height $c]\n\
}\n\
if {[expr $rx + $info_width] >= $root_width} {\n\
incr rx -$info_width\n\
}\n\
if {[expr $ry + $info_height] >= $root_height} {\n\
incr ry -$info_height\n\
}\n\
wm geometry $c \"+$rx+$ry\"\n\
raise $c\n\
}\n\
\n\
proc info_click_down {w x y rx ry} {\n\
if {![winfo exists .info]} {\n\
init_info\n\
}\n\
if {$w != \".c\"} {\n\
info_position .info $rx $ry\n\
return\n\
}\n\
set id_status [proc_id_status_at_xy $x $y]\n\
set id [lindex $id_status 0]\n\
set status [lindex $id_status 1]\n\
if {$id < 0 || $status == \"identical\"} {\n\
return\n\
}\n\
if {$status == \"same-moved\"} {\n\
info_position .info $rx $ry\n\
return\n\
}\n\
\n\
.info.color config -background [lindex [.c itemconfigure $id -fill] 4]\n\
\n\
global env\n\
set text [lava_id_to_info $id]\n\
global info_default_wh\n\
.info.text delete 0.0 end\n\
.info.text insert 0.0 $text\n\
.info.text config -width [lindex $info_default_wh 0] -height [lindex $info_default_wh 1] -font {-family Helvetica}\n\
info_position .info $rx $ry\n\
wm deiconify .info\n\
}\n\
proc info_click_up {} {\n\
global info_last_id\n\
if {![winfo exists .info]} {\n\
init_info\n\
}\n\
set info_last_id -1\n\
wm withdraw .info\n\
}\n\
\n\
proc splash_pre_init {} {\n\
global splash_x splash_y\n\
if {![info exists splash_x]} {\n\
set splash_x 0\n\
set splash_y 0\n\
}\n\
}\n\
proc splash_create {text} {\n\
global splash_id splash_gray splash_last splash_x splash_y\n\
splash_pre_init\n\
set splash_id [.c create text $splash_x $splash_y -anchor center -fill white -text $text -font {-family Helvetica -weight bold}]\n\
set splash_gray 255\n\
set splash_last $text\n\
after 4000 splash_fade\n\
}\n\
proc splash_update_wh {w h} {\n\
splash_pre_init\n\
global splash_x splash_y\n\
set old_splash_x $splash_x\n\
set old_splash_y $splash_y\n\
set splash_x [expr $w / 2]\n\
set splash_y [expr $h * 2 / 3]\n\
global splash_id\n\
if {$splash_id != \"dead\"} {\n\
.c move $splash_id [expr $splash_x - $old_splash_x] [expr $splash_y - $old_splash_y]\n\
}\n\
}\n\
proc splash_destroy {} {\n\
global splash_id\n\
.c delete $splash_id\n\
set splash_id dead\n\
}\n\
proc splash_refresh {text} {\n\
global splash_id splash_gray splash_last splash_last_count\n\
if {$splash_id == \"dead\"} {\n\
splash_create $text\n\
set splash_last \"\"\n\
set splash_last_count 0\n\
} else {\n\
set splash_gray 255\n\
if {\"$text\" == \"$splash_last\"} {\n\
if {$splash_last_count > 1} {\n\
set len [string length $splash_last_count]\n\
set e [expr [.c index $splash_id end] - 2 - $len]\n\
.c dchars $splash_id $e [expr $e + 1 + $len]\n\
} else {\n\
set e [expr [.c index $splash_id end] - 1]\n\
}\n\
incr splash_last_count\n\
.c insert $splash_id $e \" $splash_last_count\\n\"\n\
} else {\n\
.c insert $splash_id end $text\n\
set splash_last $text\n\
set splash_last_count 1\n\
}\n\
}\n\
}\n\
proc splash_fade {} {\n\
global splash_id splash_gray\n\
incr splash_gray -6\n\
if {$splash_gray <= 0} {\n\
splash_destroy\n\
} else {\n\
set g [format \"%02x\" $splash_gray]\n\
.c itemconf $splash_id -fill \"#$g$g$g\"\n\
.c itemconf raise $splash_id\n\
after 1000 splash_fade\n\
}\n\
}\n\
proc init_splash {} {\n\
global splash_id\n\
set splash_id dead\n\
}\n\
\n\
proc make_scale {tag} {\n\
}\n\
proc make_scales {} {\n\
make_scale mem\n\
}\n\
\n\
if {0} {\n\
fconfigure stdin -blocking false -buffering line\n\
fileevent stdin readable lava_debug_callback\n\
proc lava_debug_callback {} {\n\
gets stdin line\n\
puts \"lava_debug_callback: $line\"\n\
eval $line\n\
}\n\
}\n\
\n\
proc lava_ticker {cookie} {\n\
global lava_tick_cookie\n\
if {$cookie < $lava_tick_cookie} {\n\
return\n\
}\n\
\n\
lava_tick\n\
\n\
global splash_id\n\
if {$splash_id != \"dead\"} {\n\
.c raise $splash_id\n\
}\n\
\n\
global rdb\n\
incr lava_tick_cookie\n\
after $rdb(checkinterval) \"lava_ticker $lava_tick_cookie\"\n\
}\n\
proc lava_ticker_always {} {\n\
global lava_tick_cookie\n\
lava_ticker $lava_tick_cookie\n\
}\n\
proc init_ticker {} {\n\
global lava_tick_cookie rdb\n\
set_rdb_default checkinterval 2000\n\
set lava_tick_cookie 0\n\
after 10 lava_ticker_always\n\
}\n\
\n\
\n\
proc main {} {\n\
wm iconname . \"lavaps\"\n\
wm title . \"lavaps\"\n\
tk appname lavaps\n\
\n\
init_splash\n\
init_resources [lava_default_resources]\n\
init_debugging\n\
init_canvas\n\
init_help\n\
init_menu\n\
init_info\n\
\n\
\n\
init_ticker\n\
}\n\
proc init_help {} { global help\n\
set help(about) {\n\
<big>Lavaps 2.7</big>\n\
\n\
Copyright (C) 1998-2004 by John Heidemann. All rights reserved. Comments to <computer>johnh@isi.edu</computer>, latest version at <computer>http://www.isi.edu/~johnh/SOFTWARE/LAVAPS/</computer>.\n\
\n\
\n\
<big>Description</big>\n\
\n\
LavaPS is an interactive process-tracking program like ``top'', but with a much different attitude.  Rather than presenting lots of specific info in digital form, it tries to present certain important information in a graphical analog form.  The idea is that you can run it in the background and get a rough idea of what's happening to your system without devoting much concentration to the task.\n\
\n\
LavaPS was inspired by Mark Weiser's idea of calm computing in this paper:\n\
\n\
*  ``The Coming Age of Calm Technology'' by Mark Weiser and John Seely Brown. A revised version of Weiser and Brown. ``Designing Calm Technology'', PowerGrid Journal, v 1.01, <computer>http://powergrid.electriciti.com/1.01</computer> (July 1996). October, 1996.  <computer>http://www.ubiq.com/hypertext/weiser/acmfuture2endnote.htm</computer>.\n\
\n\
(This program dedicated to the memory of M.W.--I hope you would have thought it a good hack.)\n\
\n\
\n\
\n\
}\n\
set help(basics) {\n\
<big>Blobs</big>\n\
\n\
LavaPS is all about blobs of virtual, non-toxic lava. Blobs in LavaPS correspond to processes in the system. When a process is started, a new blob is created. When a process exits, the corresponding blob disappears. Blob size is proportional to the amount of memory the process uses. Blob movement is proportional to the amount of time the process runs (if the process never runs, the blob will never move).\n\
\n\
Blobs show several things. First, the basic color (the hue) corresponds to the name of the program which is running.  Emacs is always one color, Netscape another (on my system, blue and yellow). Second, blobs get darker when the process doesn't run. Over time, the process will become nearly black and only  its border will remain colored. Finally, if both physical and virtual memory are shown, then the part of the process will be a slightly different color showing what percentage of the process is not in physical memory.\n\
\n\
There are some more subtle aspects of blob physiology: initial placement is dependent on the process id (blobs appear roughly left to right) and user id (processes for the same user start at the same height, with root's processes at the top).\n\
\n\
Blobs also move along the longer distance of the lamp: if you resize it they may change direction.\n\
\n\
Please don't ask me about the chemical composition of the virtual lava.\n\
\n\
\n\
<big>Controlling Lavaps (tcl/tk)</big>\n\
\n\
Basic LavaPS is quite simple. Blobs live and grow corresponding to processes in the system (see \"BLOBS\"). Clicking the left mouse button on a blob shows information about that process. Clicking the right mouse button pops up menus that let you control LavaPS (see \"MENUS\").\n\
\n\
\n\
\n\
}\n\
set help(menus) {\n\
<big>Menus (tcl/tk)</big>\n\
\n\
The right mouse button pops up menus which control LavaPS, including:\n\
\n\
<italic>Proc</italic>: control the process under the menu: ``Nice'' it (make it take less CPU), unnice it (the reverse; only works if you're root), or send it a signal. Signals are terminate (the polite way to stop a process; ``terminate'' allows it to clean up after itself), kill (the forcefully way to stop something not listening to terminate), stop and continue (temporarily stop and resume the process), and hang-up (used to restart certain processes). Beware that these commands will be disabled if you don't have privileges to run them, and under some circumstances even ``kill'' won't stop a process.\n\
\n\
<italic>Who</italic>: track processes by <italic>me</italic> or by <italic>everyone</italic> (including root, httpd, etc.).\n\
\n\
<italic>What</italic>: track process physical or virtual memory or both. Most modern operating systems can keep all of a process in RAM (physical memory), or can let pages that aren't currently used float out to disk (virtual memory). Virtual memory is always a superset of physical memory. You can track either one, or both. When tracking both, virtual memory appears as a different colored strip down the middle of the process blob.\n\
\n\
<italic>How</italic>: controls blob sizing. Putting too little or too much lava in your lava lamp would make it boring or overflow. LavaPS therefore usually runs with patent-pending lava <italic>autosizing</italic> where blobs fill about a quarter of the lamp. This feature can be turned off (at your peril) with the How menu. You can also control the desired size of the blobs (when autosizing is enabled) or the absolute size of the blobs (when it's not) with the <italic>Grow</italic> and <italic>Shrink</italic> options.\n\
\n\
<italic>Help</italic>: you'll have to figure this one out on your own.\n\
\n\
<italic>Quit</italic>: this one's even harder than Help.\n\
\n\
\n\
\n\
}\n\
set help(resources) {\n\
<big>Resources (tcl/tk)</big>\n\
\n\
The Tcl/Tk version of LavaPS can configured from X resources (only if they're loaded with xrdb) or with the file <computer>$HOME/.lavapsrc</computer>. In both cases, the format is like:\n\
\n\
lavaps.autosize: false\n\
\n\
setting whatever resource you want (in this case autosize) to some value (false). In the <computer>.lavapsrc</computer> file, the ``lavaps'' before the period can be omitted.\n\
\n\
The following resources are supported:\n\
\n\
<italic>geometry</italic>  (default none). Specifies the initial window location and size in X-style (see X(1)).\n\
\n\
<italic>who</italic>  (default me). Whose processes should we be watching, anyway? My processes (set to ``me'') or everyone's (set to... ``everyone''). Can also be the process id of a single process if you're very single-minded.\n\
\n\
<italic>what</italic>  (default both). What kind of memory should blob size correspond to, either both, physical, or virtual.\n\
\n\
<italic>autosize</italic>  (default true). Keep the blobs at a reasonable size by dynamically changing scaling?\n\
\n\
<italic>debug</italic>  (default false).  Enable debugging messages.\n\
\n\
<italic>checkInterval</italic>  (default 2000).  How frequently (in milliseconds) should we check to see who's run?  Defaults to 2 seconds which seems ``reasonable'' on my computer; shorten the interval if you want more frequent updates and have a faster computer (or are more tolerant than I am :-).\n\
\n\
<italic>shaped</italic>  (default true). Allow lozenge control (see next).\n\
\n\
<italic>lozenge</italic>  (default true). Make the lamp lava-lamp (lozenge) shaped using X11 shaped windows. Disabled if shaped is false.\n\
\n\
<italic>clicklessInfo</italic>  (default false). If set, process information pops up without clicking. (Not yet fully working.)\n\
\n\
\n\
\n\
}\n\
set help(copyright) {\n\
<big>Copyright</big>\n\
\n\
LavaPS is Copyright (C) 1998-2003 by John Heidemann.\n\
\n\
This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License, version 2, as published by the Free Software Foundation.\n\
\n\
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.\n\
\n\
You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.\n\
\n\
\n\
\n\
}\n\
set help(howdoi) {\n\
<big>How Do I... (tcl/tk)</big>\n\
\n\
Q: The blobs are as jumpy as little rabbits, <italic>How do I make the animation smoother?</italic>\n\
\n\
A: Set the checkInterval resource to a smaller value. <italic>Currently, Resources like checkInterval only work in the Tcl version.</italic>\n\
\n\
Q: I'm running LavaPS on my Timex Sinclair and it consumes a lot of CPU, making my editor ed run slowly. <italic>How can I make LavaPS take less CPU?</italic>\n\
\n\
A: Set the checkInterval resource to a larger value. <italic>Currently, Resources like checkInterval only work in the Tcl version.</italic>\n\
\n\
Q: Lozenge-shaped LavaPS is so cool, but I keep loosing it on my 8000-pixel wide xinerama multi-screen display. <italic>How can I resize lozenge-shaped LavaPS since it doesn't have any title bar?</italic>\n\
\n\
A: In the Tcl/Tk version: (1) Read your window manager documentation, most have ways to resize windows other than the title bar (sometimes using a menu).  (2) Set the geometry explicitly with the -geometry command-line option or the geometry resource.  (3) Put the title bar back (unfortunately loosing the lozenge shape) by setting the ``shaped'' resource to ``false''.\n\
\n\
In the Gtk version: the base has controls to let you move and resize the lava lamp.\n\
\n\
\n\
\n\
}\n\
}\n\
\n\
\n\
set rcsid(help.tcl) {$Id: help.tcl,v 1.6 2000/01/06 18:19:24 johnh Exp $}\n\
\n\
proc readRelease {} {\n\
global game\n\
set catchval [catch {\n\
set f [open $game(releasefile) r]\n\
gets $f release\n\
close $f\n\
return $release\n\
} ret]\n\
if { $catchval == 2 } {\n\
return $ret\n\
} else {\n\
return \"unknown-release\"\n\
}\n\
}\n\
\n\
\n\
proc menuHighScores {} {\n\
displayHighScores\n\
}\n\
\n\
proc menuFinishGame {} {\n\
endGame \"quit\"\n\
}\n\
\n\
proc menuQuit {} {\n\
global table\n\
endGame \"quit\"\n\
exit 0\n\
}\n\
\n\
\n\
\n\
\n\
proc wmConfig {w title} {\n\
global stackList\n\
\n\
if { [info exists stackList] == 0 } {\n\
set stackList \".\"\n\
}\n\
set geometry [wm geometry [lindex $stackList [expr [llength $stackList] - 1]]]\n\
set geoElems [split $geometry \"x+\"]\n\
set geoX [lindex $geoElems 2]\n\
set geoY [lindex $geoElems 3]\n\
set neoGeoX [expr $geoX + 50]\n\
set neoGeoY [expr $geoY + 50]\n\
wm geometry $w [format \"+%d+%d\" $neoGeoX $neoGeoY]\n\
lappend stackList $w\n\
wm protocol $w WM_DELETE_WINDOW \"unmenuHelp $w\"\n\
wm group $w .\n\
wm title $w $title\n\
wm iconname $w $title\n\
}\n\
proc unmenuHelp {w} {\n\
global stackList tk_version\n\
if { [set listPos [lsearch -exact $stackList $w]] != -1 } {\n\
set stackList [lreplace $stackList $listPos $listPos]\n\
}\n\
destroy $w\n\
}\n\
\n\
proc setMenuHelpKeyboardBindings {top target} {\n\
global tk_version\n\
if {$tk_version < 4.0} {\n\
bind $top <Any-Enter> \"focus $target\"\n\
} else {\n\
focus $target\n\
$target configure -takefocus 1\n\
}\n\
bind $target <Any-KeyPress-Return> \"unmenuHelp $top\"\n\
bind $target <Any-KeyPress-Escape> \"unmenuHelp $top\"\n\
bind $target <Any-KeyPress-KP_Enter> \"unmenuHelp $top\"\n\
}\n\
\n\
\n\
proc menuHelp {topic {titleWord \"\"}} {\n\
global help helpWindowList \n\
\n\
set w \".${topic}\"\n\
catch {unmenuHelp $w}\n\
set tl [toplevel $w]\n\
if { $titleWord == \"\" } { set titleWord	$topic }\n\
wmConfig $w \"Lava-ps--$titleWord\"\n\
set padValue 8\n\
button $w.ok -text OK -padx [expr 2*$padValue] -command \"unmenuHelp $w\"\n\
text $w.t \\\n\
-relief raised -bd 2 -yscrollcommand \"$w.s set\" \\\n\
-setgrid true -wrap word \\\n\
-width 60 -padx $padValue -pady $padValue \\\n\
-font -*-Times-Medium-R-*-140-*\n\
set defFg [lindex [$w.t configure -foreground] 4]\n\
set defBg [lindex [$w.t configure -background] 4]\n\
$w.t tag configure italic -font -*-Times-Medium-I-Normal-*-140-*\n\
$w.t tag configure computer -font -*-Courier-Medium-R-Normal-*-120-*\n\
$w.t tag configure big -font -*-Times-Bold-R-Normal-*-180-*\n\
$w.t tag configure reverse -foreground $defBg -background $defFg\n\
\n\
scrollbar $w.s -relief flat -command \"$w.t yview\"\n\
pack append $w $w.ok {bottom} $w.s {right filly} $w.t {expand fill}\n\
\n\
$w.t mark set insert 0.0\n\
\n\
setMenuHelpKeyboardBindings $w \"$w.ok\"\n\
\n\
set t $help($topic)\n\
while { [regexp -indices {<([^@>]*)>} $t match inds] == 1 } {\n\
set start [lindex $inds 0]\n\
set end [lindex $inds 1]\n\
\n\
set keyword [string range $t $start $end]\n\
\n\
set oldend [$w.t index end]\n\
$w.t insert end [string range $t 0 [expr $start-2]]\n\
purgeAllTags $w.t $oldend insert\n\
\n\
if { [string range $keyword 0 0] == \"/\" } {\n\
set keyword [string trimleft $keyword \"/\"]\n\
if { [info exists tags($keyword)] == 0 } {\n\
error \"end tag $keyword without beginning\"\n\
}\n\
$w.t tag add $keyword $tags($keyword) insert\n\
unset tags($keyword)\n\
} else {\n\
if { [info exists tags($keyword)] == 1 } {\n\
error \"nesting of begin tag $keyword\"\n\
}\n\
set tags($keyword) [$w.t index insert]\n\
}\n\
\n\
set t [string range $t [expr $end+2] end]\n\
}\n\
set oldend [$w.t index end]\n\
$w.t insert end $t\n\
purgeAllTags $w.t $oldend insert\n\
$w.t configure -state disabled\n\
}\n\
\n\
proc purgeAllTags {w start end} {\n\
foreach tag [$w tag names $start] {\n\
$w tag remove $tag $start $end\n\
}\n\
}\n\
\n\
proc menuHelpScrollToTag {topic tag} {\n\
set w \".${topic}\"\n\
\n\
catch {$w.t yview -pickplace reverse.first}\n\
}\n\
\n\
\n\
proc debugLog {s} {\n\
puts $s\n\
}\n\
proc debugLogPause {s} {\n\
update idletasks\n\
puts $s\n\
puts \"\\[pause\\]\"\n\
gets stdin dummy\n\
}\n\
\n\
\n\
\n\
proc set_rdb_default {elem default {acceptable \"\"}} {\n\
global rdb\n\
if {![info exists rdb($elem)]} {\n\
set rdb($elem) $default\n\
return 1\n\
} else {\n\
if {$acceptable != \"\"} {\n\
if {[lsearch -exact $acceptable $rdb($elem)] == -1} {\n\
set rdb($elem) $default\n\
return 1\n\
}\n\
}\n\
}\n\
return 0\n\
}\n\
\n\
proc rdb_boolean {v} {\n\
set v [string tolower $v]\n\
set v [string trim $v { 	}]\n\
set v [string trimleft $v { 	}]\n\
switch -regexp $v {\n\
{(false|off|0|no)} { set vv 0 }\n\
default { set vv 1 }\n\
}\n\
}\n\
\n\
proc set_rdb_default_boolean {elem default} {\n\
global rdb\n\
set_rdb_default $elem $default\n\
set rdb($elem) [rdb_boolean $rdb($elem)]\n\
}\n\
\n\
proc rdb_resource_line {ln {strict 0}} {\n\
global rdb\n\
if {[regexp {^[Ll]ava[Pp][Ss][.*]([_A-Za-z0-9.]+):[ 	]+(.*)$} $ln t key value]} {\n\
set key [string tolower $key]\n\
set rdb($key) $value\n\
} elseif {!$strict && [regexp {^[.*]?([_A-Za-z0-9.]+):[ 	]+(.*)$} $ln t key value]} {\n\
set key [string tolower $key]\n\
set rdb($key) $value\n\
}\n\
}\n\
\n\
\n\
proc rdb_resource_file {fn {strict 0}} {\n\
if {[catch {open $fn r} f]} {\n\
return\n\
}\n\
while {[gets $f ln] >= 0} {\n\
rdb_resource_line $ln $strict\n\
}\n\
close $f\n\
}\n\
\n\
proc rdb_resource_string {str {strict 0}} {\n\
foreach ln [split $str \"\\n\"] {\n\
rdb_resource_line $ln $strict\n\
}\n\
}\n\
\n\
proc init_resources {defres} {\n\
global rdb env\n\
\n\
set rdb(moduledir) \"$env(HOME)/.tk/lavaps\"\n\
set rdb(homemoduledir) \"$env(HOME)/.tk/lavaps\"\n\
set rdb(startupfile) \"$rdb(homemoduledir)/startup.tcl\"\n\
\n\
rdb_resource_file \"|xrdb -query\" 1\n\
rdb_resource_file \"$env(HOME)/.lavapsrc\" 0\n\
if {[file exists $rdb(startupfile)]} { source $rdb(startupfile); }\n\
\n\
rdb_resource_string $defres 0\n\
}\n\
\n\
";
