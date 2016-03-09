## Process this file with automake to produce Makefile.in

#
# Copyright (C) 1999-2004 by John Heidemann <johnh@isi.edu>
# $Id: Makefile.am,v 1.54 2004/12/25 02:41:28 johnh Exp $
#

## deep
AUTOMAKE_OPTIONS = 1.0 foreign

PACKAGE = @PACKAGE@
VERSION = @VERSION@

srcdir = @srcdir@
top_srcdir = @top_srcdir@

SUBDIRS = doc po tcl2cc src

DOCDIR=doc

EXTRA_DIST=lavaps.spec.in \
	configure.gtk.ac configure.tcltk.ac \
	intltool-extract.in	\
	intltool-merge.in	\
	intltool-update.in

DISTCLEANFILES =		\
	intltool-extract	\
	intltool-merge		\
	intltool-update		


#
# top-level documentation
#


README: $(DOCDIR)/master.pod
	$(PERL) $(DOCDIR)/podprune -k NAME "WHAT'S NEW" DESCRIPTION REQUIREMENTS AVAILABILITY "RELEASE HISTORY" "TO DOS" COPYRIGHT AUTHOR <$(DOCDIR)/master.pod >README.pod
	$(POD2TEXT) README.pod | fmt >README
	rm README.pod


#
# ports and rpms
#

dist_tar_gz=$(PACKAGE)-$(VERSION).tar.gz

$(dist_tar_gz): dist

rpms: lavaps.spec
	cp $(dist_tar_gz) /usr/src/redhat/SOURCES
	cp lavaps.spec /usr/src/redhat/SPECS
	( cd /usr/src/redhat; rpmbuild -ba SPECS/lavaps.spec; )
	cp /usr/src/redhat/RPMS/i386/lavaps-$(VERSION)-1.i386.rpm .
	cp /usr/src/redhat/SRPMS/lavaps-$(VERSION)-1.src.rpm .

# this is kind of a hack so I can do it from redhat
freebsdport:
	echo "freebsdport is broken since the release 2.5 source reorg until a freebsd person can repair it"
	false
	echo 'MD5 ('$(dist_tar_gz)') = '`md5sum $(dist_tar_gz) | awk '{print $$1}'` >freebsd/port/files/md5
	cd freebsd; tar czvf ../$(PACKAGE)-$(VERSION)-freebsd-port.tar.gz lavaps-freebsd-port/{Makefile,files,pkg/DESCR,pkg/COMMENT,pkg/PLIST}

WEBDIR=/home/johnh/LIB/WWW/SOFTWARE/LAVAPS

release: doc/lavaps_man.html
	@echo "You should have done \"make dist; make rpms; make freebsdport\""
	cp lavaps-$(VERSION)-1.i386.rpm $(WEBDIR)
	cp lavaps-$(VERSION)-1.src.rpm $(WEBDIR)
	cp lavaps-$(VERSION).tar.gz $(WEBDIR)
	echo freebsdport broken cp $(PACKAGE)-$(VERSION)-freebsd-port.tar.gz $(WEBDIR)
	cp doc/lavaps_man.html $(WEBDIR)

vc_checkedout:
	vc_checkedout -e . $(SUBDIRS)

#
# manual depends
#
configure: configure.tcltk.ac configure.gtk.ac configure.ac
Makefile: Makefile.in Makefile.am configure.tcltk.ac configure.gtk.ac configure.ac

