
#
# lava.tcl
# Copyright (C) 1998-2002 by John Heidemann
# $Id: lava.tcl,v 1.54 2003/06/20 23:43:54 johnh Exp $
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License,
# version 2, as published by the Free Software Foundation.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
#

proc init_canvas {} {
	global rdb
	if {[info exists rdb(geometry)]} {
		wm geometry . $rdb(geometry)
	}
	# Set alternates and computed values.
	# if {[info exists rdb(foreground)]} { set rdb(fg) $rdb(foreground) }
	# if {[info exists rdb(background)]} { set rdb(bg) $rdb(background) }

	canvas .c -width 100 -height 200 -bg black
	pack append . .c {expand fill}

	bind . <Configure> "canvas_configure %W %w %h %B C"
	# For me with sawfish, focusin/out events cause my 
	# window to have its border redrawn, so I have to
	# reconfigure things to get the shape back.  Ick.
	bind . <FocusIn> "canvas_configure %W %w %h %B FI"
	bind . <FocusOut> "canvas_configure %W %w %h %B FO"
	bind . <Map> "canvas_configure %W %w %h %B M"

	#
	# canvas support
	#
	.c bind p <Any-ButtonPress-1> "info_click_down %W %x %y %X %Y"
	bind .c <Any-ButtonRelease-1> "info_click_up"
	bind .c <Any-B1-Motion> "info_click_down %W %x %y %X %Y"
	bind .c <Any-ButtonPress-3> "info_click_up; tk_popup .menu %X %Y 0"
	bind .c <Any-ButtonPress-2> "info_click_up; tk_popup .menu %X %Y 0"

	set_rdb_default_boolean clicklessinfo 0
	# clickless info...?  doesn't properly track
	if {$rdb(clicklessinfo)} {
		bind .c <Any-Motion> "info_click_down %W %x %y %X %Y"
	}
}

proc canvas_configure {W w h B why} {
	global old_canvas_w old_canvas_h rdb

	if {"$W" != "."} {
		return
	}

	## I used to suppress non-resizes, but no more because
	## it conflicts with lots of things.
	if {$why != "C"} {
		# Focus in/out don't have size, so fake it.
		set w $old_canvas_w
		set h $old_canvas_h
		set b 0
	}
	. configure -height $h -width $w
	if {![info exists rdb(swallowed)]} {
		set_rdb_default_boolean swallowed 0
	}
	if {$rdb(swallowed)} {
		lava_resize $w $h
	} else {
		lava_resize $w $h [wm frame .]
	}
	set old_canvas_w $w
	set old_canvas_h $h

	splash_update_wh $w $h
}

#
# menu
#
proc init_menu {} {
	global menu rdb
	# Do proc_menu_post on the toplevel menu
	# so that it doesn't keep changing.
	set m [menu .menu -tearoff 0 -postcommand proc_menu_post]

	# this next bit is in progress
	set mw .menu.proc
	menu $mw -tearoff 0
	$mw add command -label "Pid: 0" -foreground white -activeforeground white
	$mw add separator
	$mw add command -label "Nice" -command "send_signal nice"
	set state disabled
	if {[whoami] == "root"} {
		set state normal
	}
	$mw add command -label "Unnice" -command "send_signal unnice" -state $state
	$mw add separator
	$mw add command -label "Terminate (politely)" -command "send_signal TERM"
	$mw add command -label "Kill (forcefully)" -command "send_signal KILL"
	$mw add command -label "Stop (suspend)" -command "send_signal STOP"
	$mw add command -label "Continue (unsuspend)" -command "send_signal CONT"
	$mw add command -label "Hang up" -command "send_signal HUP"
	$m add cascade -label "Proc" -menu $mw

	set mw .menu.who
	menu $mw -tearoff 0
	$mw add radiobutton -label "Me" -variable menu(who) -value me -command "menu_set who me {my jobs\n}"
	$mw add radiobutton -label "Everyone" -variable menu(who) -value everyone -command "menu_set who everyone {all jobs\n}"
	set_rdb_default who me
	set menu(who) $rdb(who)
	menu_set who $menu(who)
	$m add cascade -label "Who" -menu $mw

	set mw .menu.what
	menu $mw -tearoff 0
	$mw add radiobutton -label "Virtual Memory" -variable menu(what) -value virtual -command "menu_set what virtual {virtual mem\n}"
	$mw add radiobutton -label "Physical Memory" -variable menu(what) -value physical -command "menu_set what physical {physical mem\n}"
	$mw add radiobutton -label "Both" -variable menu(what) -value both -command "menu_set what both {both mem\n}"
	set_rdb_default what virtual {virtual physical both}
	set menu(what) $rdb(what)
	menu_set what $rdb(what)
	$m add cascade -label "What" -menu $mw

	set mw .menu.how
	menu $mw -tearoff 0
	$mw add command -label "Shrink" -command {menu_set how shrink "shrink mem\n"}
	$mw add command -label "Grow" -command {menu_set how grow "grow mem\n"}
	#
	set_rdb_default_boolean autosize 1
	set menu(how) $rdb(autosize)
	$mw add checkbutton -label "Autosizing" -variable menu(how) -onvalue 1 -offvalue 0 -command {menu_autosize force}
	#
	set_rdb_default_boolean shaped [lava_menu shaped_window_ok]
	if {$rdb(shaped)} {
		set_rdb_default_boolean lozenge $rdb(shaped)
		set state normal
	} else {
		set rdb(lozenge) 0
		set state disabled
	}
	set menu(lozenge) $rdb(lozenge)
	$mw add checkbutton -label "Lozenge" -variable menu(lozenge) -onvalue 1 -offvalue 0 -command "menu_shape" -state $state
	menu_shape
	$m add cascade -label "How" -menu $mw

	set mw .menu.help
	menu $mw -tearoff 0
	$mw add command -label "About..." -command "menuHelp about"
	$mw add command -label "Basics..." -command "menuHelp basics"
	$mw add command -label "Menus..." -command "menuHelp menus"
	$mw add command -label "Resources..." -command "menuHelp resources"
	$mw add command -label "How do I..." -command "menuHelp howdoi"
	$mw add command -label "Copyright..." -command "menuHelp copyright"
	$m add cascade -label "Help" -menu $mw

	$m add separator
	$m add command -label "Quit" -command "exit 0"
}

proc menu_autosize {status} {
	global menu
	switch -exact $status {
	off {
		if {$menu(how) != 0} {
			set menu(how) 0
			menu_set how 0 "manual resize\n"
		}
	}
	force {
		if {$menu(how)} {
			menu_set how 1 "auto-resize\n"
		} else {
			menu_set how 0 "manual resize\n"
		}
	}
	default {
		die "bad menu_autosize argument"
	}
	}
}

proc menu_shape {} {
	global rdb menu
	if {!$rdb(shaped)} {
		menu_set shape unshaped
	} elseif {$menu(lozenge)} {
		menu_set shape 25 "lozenge\n"
	} else {
		menu_set shape 0 "non-lozenge\n"
	};
		
}

proc menu_set {what token {feedback ""}} {
	global menu
	lava_menu $what $token
	if {$feedback != ""} {
		splash_refresh $feedback
	}
}

proc menu_set_through_menu_array {what {feedback ""}} {
	global menu
	lava_menu $what $menu($what)
	if {$feedback != ""} {
		splash_refresh $feedback
	}
}

proc init_debugging {} {
	global rdb
	set_rdb_default_boolean debug 0
	lava_menu debug $rdb(debug)
}

proc debugging {} {
	global rdb
	return $rdb(debug)
}

#
# process stuff
#
proc send_signal {kind} {
	global proc_menu_pid
	if {[catch {lava_menu signal $kind $proc_menu_pid} dummy]} {
		splash_refresh "Cannot signal\n"
	} else {
		splash_refresh "Signaled\n"
	}
}

proc whoami {} {
	global whoami
	if {![info exists whoami]} {
		set whoami [lava_menu whoami]
	}
	return $whoami
}

proc proc_menu_post {} {
	# ick...find the mouseb by hand
	set rx [winfo pointerx .c]
	set ry [winfo pointery .c]
	set wx [winfo x .]
	set wy [winfo y .]
	set x [expr $rx - $wx]
	set y [expr $ry - $wy]
	# puts "rx: $rx  ry: $ry\nwx: $wx  wy: $wy"
	# now find object
	set id [lindex [proc_id_status_at_xy $x $y] 0]

	# extract stuff
	set text [lava_id_to_info $id]
	foreach {ln} [split $text "\n"] {
		if {[regexp {^([^:]+): +(.*)$} $ln match key value]} {
			set stuff($key) $value
		}
	}
		
	set pid $stuff(pid)
	set user [string trim [lindex [split $stuff(user) " "] 1] "()"]
	set me [whoami]
	set cmd [string range $stuff(cmd) 0 19]

	global proc_menu_pid
	set proc_menu_pid $pid

	# display what we know
	.menu.proc entryconfigure {Pid*} -label "Pid: $pid ($cmd)"
	set blob_color [lindex [.c itemconfigure $id -fill] 4]
	.menu.proc entryconfigure {Pid*} -background $blob_color -activebackground $blob_color
	# enable/disable the menus
	if {$me == 0 || $user == $me} {
		set state normal
	} else {
		set state disabled
	}
	foreach {i} {Nice Unnice {Hang*} {Kill*} {Stop*} {Continue*} {Terminate*}} {
		.menu.proc entryconfigure $i -state $state
	}
}

#
# info boxes
#
proc init_info {} {
	global rdb
	toplevel .info -relief raised -borderwidth 2 -class LavaPS
	# try and coerce fvwm into doing the right thing
	wm transient .info .
	wm overrideredirect .info 1
	wm group .
	wm withdraw .info
	wm iconname .info ""
	raise .info
	# xxx: label height should be equal to text height (and dynamic)
	global info_default_wh
	set info_default_wh {24 6}
	if {[debugging]} {
		set info_default_wh {30 7}
	}
	label .info.color -height 1 -width [lindex $info_default_wh 0] -text " "
	text .info.text -width [lindex $info_default_wh 0] -height [lindex $info_default_wh 1]
	pack .info.color
	pack .info.text
	# pack append .info.color "bottom fillx" .info.text "bottom"
	global info_last_x info_last_y info_last_id
	set info_last_x -1
	set info_last_y -1
	set info_last_id -1
	# tracking
	if {$rdb(clicklessinfo)} {
		bind .info <Any-Motion> "info_click_down_in_info %W %x %y %X %Y"
	}
	bind .info <Any-ButtonPress-3> "tk_popup .menu %X %Y 0"
	bind .info <Any-ButtonPress-2> "tk_popup .menu %X %Y 0"
}

proc info_click_down_in_info {w x y rx ry} {
	puts "icdii: $w $x $y $rx $ry"
	info_click_down $w $x $y $rx $ry
}

# return id of object at x,y
# -1 if nothing, -2 if repeat
proc proc_id_status_at_xy {x y} {
	global info_last_x info_last_y info_last_id

	set try_again 1
	while {$try_again} {
		set id [.c find closest $x $y]

		if {$info_last_id == $id && $info_last_x == $x && $info_last_y == $y} {
			
			return [list $id identical]
		}
		if {$info_last_id == $id} {
			return [list $id same-moved]
		}
		global splash_id
		if {$id == $splash_id} {
			splash_destroy
			set try_again 1
		} else {
			set try_again 0
		}
	}
	set info_last_x $x
	set info_last_y $y
	set info_last_id $id
	return [list $id new]
}

proc info_position {c rx ry} {
	global info_width info_height root_width root_height
	if {![info exists info_width] || $info_width == 1} {
		# we cache all this stuff and assume it never changes (it shouldn't)
		set root_width [winfo screenwidth $c]
		set root_height [winfo screenheight $c]
		update idletasks
		set info_width [winfo width $c]
		set info_height [winfo height $c]
	}
	if {[expr $rx + $info_width] >= $root_width} {
		incr rx -$info_width
	}
	if {[expr $ry + $info_height] >= $root_height} {
		incr ry -$info_height
	}
	wm geometry $c "+$rx+$ry"
	# make sure it's on top (most window managers do this by default,
	# but some, such as apple's, don't, according to Kevin Geiss)
	raise $c
}

proc info_click_down {w x y rx ry} {
	# puts "info_click_down: $w $x $y $rx $ry"
	if {![winfo exists .info]} {
		init_info
	}
	# first, if not in lava window, bail
	if {$w != ".c"} {
		info_position .info $rx $ry
		return
	}
	# next, figure out where we are
	set id_status [proc_id_status_at_xy $x $y]
	set id [lindex $id_status 0]
	set status [lindex $id_status 1]
	if {$id < 0 || $status == "identical"} {
		return
	}
	if {$status == "same-moved"} {
		# move existing window
		info_position .info $rx $ry
		return
	}

	.info.color config -background [lindex [.c itemconfigure $id -fill] 4]

	global env
	set text [lava_id_to_info $id]
#	if {[debugging]} {
#		set text "id: $id\n[lava_id_to_info $id]"
#	} else {
#		set text [lava_id_to_info $id]
#	}
	# xxx: should compute width/height
	global info_default_wh
	.info.text delete 0.0 end
	.info.text insert 0.0 $text
	.info.text config -width [lindex $info_default_wh 0] -height [lindex $info_default_wh 1] -font {-family Helvetica}
	info_position .info $rx $ry
	wm deiconify .info
	# xxx: should detect if window goes off edge and reposition it automatically
}
proc info_click_up {} {
	global info_last_id
	if {![winfo exists .info]} {
		init_info
	}
	set info_last_id -1
	wm withdraw .info
}

#
# intro splash
#
proc splash_pre_init {} {
	global splash_x splash_y
	if {![info exists splash_x]} {
		set splash_x 0
		set splash_y 0
	}
}
proc splash_create {text} {
	global splash_id splash_gray splash_last splash_x splash_y
	#
	splash_pre_init
	set splash_id [.c create text $splash_x $splash_y -anchor center -fill white -text $text -font {-family Helvetica -weight bold}]
	set splash_gray 255
	set splash_last $text
	after 4000 splash_fade
}
proc splash_update_wh {w h} {
	# hacky: put the splash text at the buldge of the lozenge
	splash_pre_init
	global splash_x splash_y
	set old_splash_x $splash_x
	set old_splash_y $splash_y
	set splash_x [expr $w / 2]
	set splash_y [expr $h * 2 / 3]
	global splash_id
	if {$splash_id != "dead"} {
		.c move $splash_id [expr $splash_x - $old_splash_x] [expr $splash_y - $old_splash_y]
	}
}
proc splash_destroy {} {
	global splash_id
	.c delete $splash_id
	set splash_id dead
}
proc splash_refresh {text} {
	global splash_id splash_gray splash_last splash_last_count
	if {$splash_id == "dead"} {
		splash_create $text
		set splash_last ""
		set splash_last_count 0
	} else {
		# reset it's timer and add the text
		set splash_gray 255
		if {"$text" == "$splash_last"} {
			if {$splash_last_count > 1} {
				set len [string length $splash_last_count]
				set e [expr [.c index $splash_id end] - 2 - $len]
				.c dchars $splash_id $e [expr $e + 1 + $len]
			} else {
				set e [expr [.c index $splash_id end] - 1]
			}
			incr splash_last_count
			.c insert $splash_id $e " $splash_last_count\n"
		} else {
			# different text, just insert it
			.c insert $splash_id end $text
			set splash_last $text
			set splash_last_count 1
		}
		# xxx: the user can overflow the text on the screen.
		# Fortunately, this is just a cosmetic problem.
	}
}
proc splash_fade {} {
	global splash_id splash_gray
	incr splash_gray -6
	if {$splash_gray <= 0} {
		splash_destroy
	} else {
		set g [format "%02x" $splash_gray]
		.c itemconf $splash_id -fill "#$g$g$g"
		.c itemconf raise $splash_id
		after 1000 splash_fade
	}
}
proc init_splash {} {
	global splash_id
	set splash_id dead
}

#
# controls:
#
proc make_scale {tag} {
	# scale
}
proc make_scales {} {
	make_scale mem
}

#
# debugging input
#
if {0} {
	fconfigure stdin -blocking false -buffering line
	fileevent stdin readable lava_debug_callback
	proc lava_debug_callback {} {
		gets stdin line
		puts "lava_debug_callback: $line"
		eval $line
	}
	# proc m {} { lava_move }
	# proc g {} { lava_grow }
	# proc p {} { lava_print }
	# proc r {} { lava_reverse }
}

#
# monitoring work
#
proc lava_ticker {cookie} {
	global lava_tick_cookie
	if {$cookie < $lava_tick_cookie} {
		# prevent multiple tickers
		return
	}

	lava_tick

	# splash exists is only in tcl, raise it here
	global splash_id
	if {$splash_id != "dead"} {
		.c raise $splash_id
	}

	global rdb
	incr lava_tick_cookie
	after $rdb(checkinterval) "lava_ticker $lava_tick_cookie"
}
proc lava_ticker_always {} {
	global lava_tick_cookie
	lava_ticker $lava_tick_cookie
}
proc init_ticker {} {
	global lava_tick_cookie rdb
	set_rdb_default checkinterval 2000
	set lava_tick_cookie 0
	# On my 266MHz Pentium, 2000 == ~6% CPU usage to a remote display.
	# must defer first check to set up canvas
	after 10 lava_ticker_always
}


#
# main
#
proc main {} {
	wm iconname . "lavaps"
	wm title . "lavaps"
	tk appname lavaps

	init_splash
	# init_resources must preceed init_debugging
	init_resources [lava_default_resources]
	init_debugging
	init_canvas
	init_help
	init_menu
	init_info

#	splash_refresh "buttons:\nleft: info\nright: menu\n"

	init_ticker
}
