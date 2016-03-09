
#
# resource.tcl
# Copyright (C) 1998-2000 by John Heidemann
# $Id: resource.tcl,v 1.11 2000/06/04 05:39:37 johnh Exp $
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

# returns boolean: default taken
proc set_rdb_default {elem default {acceptable ""}} {
	global rdb
	if {![info exists rdb($elem)]} {
		# puts "set_rdb_default: for $elem, take default $default"
		set rdb($elem) $default
		return 1
	} else {
		if {$acceptable != ""} {
			# verify it
			if {[lsearch -exact $acceptable $rdb($elem)] == -1} {
				# puts stderr "bad resource value: $elem $rdb($elem)"
				# puts "set_rdb_default: for $elem, take default $default since $rdb($elem) is not acceptable"
				set rdb($elem) $default
				return 1
			}
		}
		# puts "set_rdb_default: for $elem, take resource $rdb($elem)"
	}
	return 0
}

# convert a generic boolean to 0 or 1
proc rdb_boolean {v} {
	set v [string tolower $v]
	set v [string trim $v { 	}]
	set v [string trimleft $v { 	}]
	switch -regexp $v {
	{(false|off|0|no)} { set vv 0 }
	default { set vv 1 }
	}
}

proc set_rdb_default_boolean {elem default} {
	global rdb
	set_rdb_default $elem $default
	# puts "set_rdb_default_boolean: $elem is $rdb($elem) (default $default)"
	set rdb($elem) [rdb_boolean $rdb($elem)]
	# puts "set_rdb_default_boolean: $elem is $rdb($elem) (default $default)"
}

proc rdb_resource_line {ln {strict 0}} {
	global rdb
	if {[regexp {^[Ll]ava[Pp][Ss][.*]([_A-Za-z0-9.]+):[ 	]+(.*)$} $ln t key value]} {
		set key [string tolower $key]
		# puts "rdb: <$key> <$value>"
		set rdb($key) $value
	} elseif {!$strict && [regexp {^[.*]?([_A-Za-z0-9.]+):[ 	]+(.*)$} $ln t key value]} {
		set key [string tolower $key]
		# puts "rdb: <$key> <$value>"
		set rdb($key) $value
	}
}


proc rdb_resource_file {fn {strict 0}} {
	if {[catch {open $fn r} f]} {
		# puts "rdb_resource_file: $fn (cannot open)"
		return
	}
	# puts "rdb_resource_file: $fn"
	while {[gets $f ln] >= 0} {
		rdb_resource_line $ln $strict
	}
	close $f
}

proc rdb_resource_string {str {strict 0}} {
	# puts "rdb_resource_string"
	foreach ln [split $str "\n"] {
		rdb_resource_line $ln $strict
	}
}

proc init_resources {defres} {
	# NEEDSWORK: currently only uses xrdb.
	# Should also look in .Xresources, .Xdefaults.
	global rdb env

	set rdb(moduledir) "$env(HOME)/.tk/lavaps"
	set rdb(homemoduledir) "$env(HOME)/.tk/lavaps"
        set rdb(startupfile) "$rdb(homemoduledir)/startup.tcl"

	rdb_resource_file "|xrdb -query" 1
	rdb_resource_file "$env(HOME)/.lavapsrc" 0
	# the next line is one one line as a hack to make tcl2cc not gobble the source cmd
	if {[file exists $rdb(startupfile)]} { source $rdb(startupfile); }

	rdb_resource_string $defres 0
}

