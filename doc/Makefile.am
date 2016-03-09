## Process this file with automake to produce Makefile.in

#
# Copyright (C) 1999-2003 by John Heidemann <johnh@isi.edu>
# $Id: Makefile.am,v 1.3 2004/12/10 20:36:01 johnh Exp $
#

SUBDIRS=

man_MANS = lavaps.1

EXTRA_DIST=master.pod \
	podprune pod2help

all: lavaps.1 lavaps_man.html

#
# Documentation in its many forms is built from master.pod.
#

MANUAL_NON_SECTIONS=LAVAPS "WHAT'S NEW" REQUIREMENTS "INSTALLATION DETAILS" AVAILABILITY "RELEASE HISTORY" ISPELL

lavaps.1: master.pod
	$(PERL) podprune $(MANUAL_NON_SECTIONS) <master.pod >lavaps.pod
	$(POD2MAN) --section=1 --release="lavaps-@VERSION@" --center="Lavaps Documentation" lavaps.pod >lavaps.1
	rm lavaps.pod

lavaps_man.html: master.pod
	$(PERL) podprune $(MANUAL_NON_SECTIONS)  <master.pod >lavaps.pod
	$(POD2HTML) --title "lavaps-@VERSION@ manual page" lavaps.pod >lavaps_man.html
	rm lavaps.pod

