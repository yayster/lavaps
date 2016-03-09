#!/usr/dist/bin/wishx -f

######################################################################
#
# help.tcl
#
# Copyright (C) 1993,1994 by John Heidemann <johnh@ficus.cs.ucla.edu>
# All rights reserved.  See the main dontspace file for a full copyright
# notice.
#
# $Id: help.tcl,v 1.6 2000/01/06 18:19:24 johnh Exp $
#
# from: Id: klondikeMenus.tcl,v 1.17 1994/01/27 18:49:11 johnh Exp
#
######################################################################

#
# menu stuff
#
set rcsid(help.tcl) {$Id: help.tcl,v 1.6 2000/01/06 18:19:24 johnh Exp $}

proc readRelease {} {
	global game
	set catchval [catch {
			set f [open $game(releasefile) r]
			gets $f release
			close $f
			return $release
		} ret]
	if { $catchval == 2 } {
		return $ret
	} else {
		return "unknown-release"
	}
}


proc menuHighScores {} {
	displayHighScores
}

proc menuFinishGame {} {
	endGame "quit"
}

proc menuQuit {} {
	global table
	# quit game in progress, if any
	endGame "quit"
	# bail
	exit 0
}



#----------------------------------------------------------------------
# (stolen from Ousterhout's demo program's mkBasic)
# and hacked to add SGML-ish (actually more MIME-ish) tags.

#
# Put the next window up tiled after the last window to go up.
# Maintain a list of relevant windows in the global variable stackList.
#
proc wmConfig {w title} {
	global stackList

	# debugLog "wmConfig $w $title"
	#
	# configure random wm stuff
	#
	if { [info exists stackList] == 0 } {
		# setup a list with something that should never get removed
		set stackList "."
	}
	# debugLog "\t$stackList"
	#
	# first geometry
	#
	set geometry [wm geometry [lindex $stackList [expr [llength $stackList] - 1]]]
	set geoElems [split $geometry "x+"]
	set geoX [lindex $geoElems 2]
	set geoY [lindex $geoElems 3]
	set neoGeoX [expr $geoX + 50]
	set neoGeoY [expr $geoY + 50]
	wm geometry $w [format "+%d+%d" $neoGeoX $neoGeoY]
	lappend stackList $w
	#
	# cleanup
	#
	wm protocol $w WM_DELETE_WINDOW "unmenuHelp $w"
	#
	# group-hood
	#
	wm group $w .
	#
	# titles
	#
	wm title $w $title
	wm iconname $w $title
	# debugLog "\txxx: stackList $stackList"
}
proc unmenuHelp {w} {
	global stackList tk_version
	# debugLog "unmenuHelp $w"
	# debugLog "\t$stackList"
	if { [set listPos [lsearch -exact $stackList $w]] != -1 } {
		set stackList [lreplace $stackList $listPos $listPos]
	}
	# In 4.0, we die if we destroy $w since
	# tkButtonInvoke wants to do something with $w.ok
	# after this "destroy $w".
	# Do it later to work around the problem.
	#	after 10 "if \[winfo exists $w\] { destroy $w }"
	# (This problem seems to have been fixed by 4.2,
	# and the fix seems to cause problems there).
	destroy $w
	# debugLog "\t$stackList"
}

proc setMenuHelpKeyboardBindings {top target} {
	global tk_version
	if {$tk_version < 4.0} {
		bind $top <Any-Enter> "focus $target"
	} else {
		focus $target
		$target configure -takefocus 1
	}
	bind $target <Any-KeyPress-Return> "unmenuHelp $top"
	bind $target <Any-KeyPress-Escape> "unmenuHelp $top"
	bind $target <Any-KeyPress-KP_Enter> "unmenuHelp $top"
	# mkMenuBindings $target
}


proc menuHelp {topic {titleWord ""}} {
	global help helpWindowList 

	set w ".${topic}"
	#
	# destroy any existing window
	# NEEDSWORK: should just bring it forward.
	#
	catch {unmenuHelp $w}
	#
	# create a new window	
	#
	set tl [toplevel $w]
#	debugLog "toplevel: $tl"
	# debugLog "menuHelp $w"
	# default title
	if { $titleWord == "" } { set titleWord	$topic }
	wmConfig $w "Lava-ps--$titleWord"
	set padValue 8
	button $w.ok -text OK -padx [expr 2*$padValue] -command "unmenuHelp $w"
	# NEEDSWORK: font selection should be configurable.
	#
	# If you use this code elsewhere, please follow two conscious
	# style choices.  First, wide things are hard to read
	# (50 chars is about the most reasonable---consider newspaper
	# columns).  Second, we allow the user to resize the window.
	# (The user should always have control, even to do stupid things.)
	#
	text $w.t \
		-relief raised -bd 2 -yscrollcommand "$w.s set" \
		-setgrid true -wrap word \
		-width 60 -padx $padValue -pady $padValue \
		-font -*-Times-Medium-R-*-140-*
	set defFg [lindex [$w.t configure -foreground] 4]
	set defBg [lindex [$w.t configure -background] 4]
	$w.t tag configure italic -font -*-Times-Medium-I-Normal-*-140-*
	$w.t tag configure computer -font -*-Courier-Medium-R-Normal-*-120-*
	$w.t tag configure big -font -*-Times-Bold-R-Normal-*-180-*
	$w.t tag configure reverse -foreground $defBg -background $defFg

	scrollbar $w.s -relief flat -command "$w.t yview"
	pack append $w $w.ok {bottom} $w.s {right filly} $w.t {expand fill}

	$w.t mark set insert 0.0

	# Set up keyboard bindings.
	setMenuHelpKeyboardBindings $w "$w.ok"

	#
	# Scan the text for tags.
	#
	set t $help($topic)
	while { [regexp -indices {<([^@>]*)>} $t match inds] == 1 } {
		set start [lindex $inds 0]
		set end [lindex $inds 1]

		set keyword [string range $t $start $end]
		# puts stderr "tag $keyword found at $inds"

		# insert the left hand text into the thing
		set oldend [$w.t index end]
		$w.t insert end [string range $t 0 [expr $start-2]]
		purgeAllTags $w.t $oldend insert

		# check for begin/end tag
		if { [string range $keyword 0 0] == "/" } {
			# end region
			set keyword [string trimleft $keyword "/"]
			if { [info exists tags($keyword)] == 0 } {
				error "end tag $keyword without beginning"
			}
			$w.t tag add $keyword $tags($keyword) insert
			# puts stdout "tag $keyword added from $tags($keyword) to [$w.t index insert]"
			unset tags($keyword)
		} else {
			if { [info exists tags($keyword)] == 1 } {
				error "nesting of begin tag $keyword"
			}
			set tags($keyword) [$w.t index insert]
			# puts stdout "tag $keyword begins at [$w.t index insert]"
		}

		# continue with the rest
		set t [string range $t [expr $end+2] end]
	}
	set oldend [$w.t index end]
	$w.t insert end $t
	purgeAllTags $w.t $oldend insert
	#
	# Disable the text so the user can't mess with it.
	#
	$w.t configure -state disabled
}

proc purgeAllTags {w start end} {
	# remote any bogus tags
	# puts stderr "Active tags at $start are [$w tag names $start]"
	foreach tag [$w tag names $start] {
		$w tag remove $tag $start $end
	}
}

proc menuHelpScrollToTag {topic tag} {
	set w ".${topic}"
	
	catch {$w.t yview -pickplace reverse.first}
}


proc debugLog {s} {
	puts $s
}
proc debugLogPause {s} {
	update idletasks
	puts $s
	puts "\[pause\]"
	gets stdin dummy
}

