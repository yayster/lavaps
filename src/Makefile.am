## Process this file with automake to produce Makefile.in

#
# Copyright (C) 1999-2003 by John Heidemann <johnh@isi.edu>
# $Id: Makefile.am,v 1.53 2004/12/16 05:13:03 johnh Exp $
#

SUBDIRS= @PLATFORM_SUBDIRS@

DOCDIR=../doc
PODPRUNE=$(DOCDIR)/podprune
POD2HELP=$(DOCDIR)/pod2help

bin_PROGRAMS = lavaps
lavaps_cc_sources = \
	main.cc main.hh \
	process_view.cc	process_model.cc process_list.cc \
	process_view.hh	process_model.hh process_list.hh \
	process_scan_linux_proc.cc process_scan_bsd.cc \
	process_scan_solaris.cc process_scan_solaris_2_5.cc \
	process_scan_irix.cc process_scan_aix.cc \
	process_scan_mach.cc \
	process_scan.hh	\
	scaled_blob.cc scaled_blob.hh \
	blob.cc tcl_blob.cc tcl_blob_shape.cc text_blob.cc \
	blob.hh tcl_blob.hh text_blob.hh \
	gtk_blob.cc gtk_blob.hh \
	gtk_blob_splash.cc gtk_blob_splash.hh \
	gtk_blob_conf.cc gtk_blob_conf.hh \
	gtk_blob_base.cc gtk_blob_base.hh \
	gtk_xpms.cc \
	gtk_lava_help.hh gtk_lava_help.cc \
	gtk_compat.cc gtk_compat.hh \
	tcl_blob_code.cc \
	dmalloc_support.cc entry_trace.cc entry_trace.hh \
	const_str.hh \
	change_tracking.hh \
	user.cc user.hh \
	compat.cc compat.hh \
	lava_signal.cc lava_signal.hh \
	color.cc
lavaps_SOURCES = $(lavaps_cc_sources)
# OMIT_DEPENDENCIES becomes an error (!) in automake-1.6.3
# OMIT_DEPENDENCIES=linux/proc/readproc.h

lavaps_tcl_sources = \
	lava.tcl lava_help.tcl help.tcl resource.tcl
lavaps_xpms = \
	IMAGES/gripper_2_16.xpm IMAGES/gripper_2_32.xpm IMAGES/gripper_2_64.xpm \
	IMAGES/menu_2_16.xpm IMAGES/menu_2_32.xpm IMAGES/menu_2_64.xpm \
	IMAGES/icon_16.xpm  \
	IMAGES/icon_32.xpm  \
	IMAGES/icon_64.xpm \
	IMAGES/icon_128.xpm 
EXTRA_lavaps_SOURCES = \
	$(lavaps_tcl_sources) \
	$(lavaps_xpms)

# according to http://www.gnome.org/projects/gconf/
schemadir   = @GCONF_SCHEMA_FILE_DIR@
schema_DATA = lavaps.schemas
#  xxx: my automake seems to miss these
GCONF_SCHEMA_CONFIG_SOURCE=xml::${sysconfdir}/gconf/gconf.xml.defaults
GCONFTOOL=@GCONFTOOL@

EXTRA_DIST=\
	$(schema_DATA) \
	linux/proc/Makefile \
	linux/proc/alloc.c \
	linux/proc/compare.c \
	linux/proc/devname.c \
	linux/proc/devname.h \
	linux/proc/ksym.c \
	linux/proc/output.c \
	linux/proc/ps.h \
	linux/proc/psdata.h \
	linux/proc/pwcache.c \
	linux/proc/readproc.c \
	linux/proc/readproc.h \
	linux/proc/signals.c \
	linux/proc/signals.h \
	linux/proc/signames.h \
	linux/proc/status.c \
	linux/proc/sysinfo.c \
	linux/proc/sysinfo.h \
	linux/proc/tree.h \
	linux/proc/version.c \
	linux/proc/version.h \
	linux/proc/whattime.c \
	linux/proc/whattime.h \
	freebsd/port/Makefile.in \
	freebsd/port/pkg/COMMENT \
	freebsd/port/pkg/DESCR \
	freebsd/port/pkg/PLIST


# ENTRY_TRACE_FLAGS=-DUSE_ENTRY_TRACE
ENTRY_TRACE_FLAGS=
LAVAPS_GXX_FLAGS=@LAVAPS_GXX_FLAGS@
LAVAPS_STL_NAMESPACE_FLAGS=-DLAVAPS_STL_NAMESPACE=@LAVAPS_STL_NAMESPACE@
AM_CXXFLAGS=$(ADDITIONAL_INCLUDES) $(GUI_INCLUDES) $(PLATFORM_INCLUDES) -g $(GCC_FLAGS) $(ENTRY_TRACE_FLAGS) $(LAVAPS_GXX_FLAGS) $(LAVAPS_STL_NAMESPACE_FLAGS) $(X_CFLAGS) -DGNOMELOCALEDIR=\""$(prefix)/share/locale"\"

lavaps_LDADD=$(PLATFORM_LIBS) $(GUI_LIBS) $(ADDITIONAL_LIBS) $(X_LIBS) $(X_PRE_LIBS) $(X_EXTRA_LIBS) -lXext -lX11


CLEANFILES=tcl_blob_code.cc gtk_lava_help.cc

#
# Documentation in its many forms is built from master.pod.
#

lava_help.tcl: $(DOCDIR)/master.pod ../configure.ac
	echo "proc init_help {} { global help" >lava_help.tcl
	$(PERL) $(PODPRUNE) -k LAVAPS DESCRIPTION < $(DOCDIR)/master.pod | $(PERL) $(POD2HELP) about | sed 's/>Lavaps</>Lavaps $(VERSION)</' >>lava_help.tcl
	$(PERL) $(PODPRUNE) -k "CONTROLLING LAVAPS (TCL/TK)" BLOBS < $(DOCDIR)/master.pod | $(PERL) $(POD2HELP) basics >>lava_help.tcl
	$(PERL) $(PODPRUNE) -k "MENUS (TCL/TK)" < $(DOCDIR)/master.pod | $(PERL) $(POD2HELP) menus >>lava_help.tcl
	$(PERL) $(PODPRUNE) -k "RESOURCES (TCL/TK)" < $(DOCDIR)/master.pod | $(PERL) $(POD2HELP) resources >>lava_help.tcl
	$(PERL) $(PODPRUNE) -k COPYRIGHT < $(DOCDIR)/master.pod | $(PERL) $(POD2HELP) copyright >>lava_help.tcl
	$(PERL) $(PODPRUNE) -k "HOW DO I... (TCL/TK)" < $(DOCDIR)/master.pod | $(PERL) $(POD2HELP) howdoi >>lava_help.tcl
	echo "}" >>lava_help.tcl

gtk_lava_help.cc: $(DOCDIR)/master.pod ../configure.ac
	echo '/* this file is auto generated from the makefile */' >gtk_lava_help.cc
	echo '/* DO NOT EDIT DIRECTLY! */' >>gtk_lava_help.cc
	echo '#ifdef USE_GTK_BLOB' >>gtk_lava_help.cc
	echo '#include "gtk_lava_help.hh"' >>gtk_lava_help.cc
	echo 'gtk_lava_help_text *gtk_lava_help_text::first_ = NULL;' >>gtk_lava_help.cc
	$(PERL) $(PODPRUNE) -k LAVAPS DESCRIPTION < $(DOCDIR)/master.pod | $(PERL) $(POD2HELP) -m gtk about | sed 's/>Lavaps</>Lavaps $(VERSION)</' >>gtk_lava_help.cc
	$(PERL) $(PODPRUNE) -k "CONTROLLING LAVAPS" BLOBS < $(DOCDIR)/master.pod | $(PERL) $(POD2HELP) -m gtk basics >>gtk_lava_help.cc
	$(PERL) $(PODPRUNE) -k MENUS < $(DOCDIR)/master.pod | $(PERL) $(POD2HELP) -m gtk menus >>gtk_lava_help.cc
	$(PERL) $(PODPRUNE) -k COPYRIGHT < $(DOCDIR)/master.pod | $(PERL) $(POD2HELP) -m gtk copyright >>gtk_lava_help.cc
	echo '#endif /* USE_GTK_BLOB */' >>gtk_lava_help.cc

#
# tcl stuff
#

tcl_blob.cc: tcl_blob_code.cc $(lavaps_tcl_sources)

tcl_blob_code.cc: $(lavaps_tcl_sources) ../tcl2cc/tcl2cc
	../tcl2cc/tcl2cc lava $(lavaps_tcl_sources) >tcl_blob_code.cc

../tcl2cc/tcl2cc:
	( cd ../tcl2cc; $(MAKE); )

#
# gtk stuff
#

gtk_xpms.cc: Makefile.am
	echo '/* this file is auto generated from the makefile */' >$@
	echo '/* DO NOT EDIT DIRECTLY! */' >>$@
	echo '#include <stdio.h> /* for NULL */' >>$@
	for i in $(lavaps_xpms); do cat $$i | sed 's/static char/static const char/' >>$@; done
	echo "const char **gripper_xpms[] = { gripper_2_16_xpm, gripper_2_32_xpm, gripper_2_64_xpm, NULL };" >>$@
	echo "const char **menu_xpms[] = { menu_2_16_xpm, menu_2_32_xpm, menu_2_64_xpm, NULL };" >>$@
	echo "const char **icon_xpms[] = { icon_16_xpm, icon_32_xpm, icon_64_xpm, icon_128_xpm, NULL };" >>$@


#
# gettext stuff
#
inital_gettext:
	xgettext --copyright-holder="John Heidemann" --debug --keyword=_ --keyword=N_  -o - *.cc *.hh >../po/xx.po

#
# other stuff
#

install-exec-hook:
	test x$(PLATFORM_OWN) != xnone &&  { chown $(PLATFORM_OWN) $(bindir)/lavaps; chmod u+s $(bindir)/lavaps; } || true
	test x$(PLATFORM_GRP) != xnone &&  { chgrp $(PLATFORM_GRP) $(bindir)/lavaps; chmod g+s $(bindir)/lavaps; } || true


