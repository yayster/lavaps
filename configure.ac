
dnl
dnl configure.ac
dnl $Id: configure.ac,v 1.51 2004/12/25 02:35:34 johnh Exp $
dnl
dnl Process this file with autoconf to produce a configure script.
dnl
AC_INIT(lavaps,2.7)
AC_CONFIG_SRCDIR(src/blob.cc)
AM_INIT_AUTOMAKE
AC_PREFIX_PROGRAM(lavaps)
AM_CONFIG_HEADER(config.h)

dnl Checks for programs.
AC_PROG_CC
AC_PROG_CXX
AC_PROG_MAKE_SET
AC_LANG_CPLUSPLUS

AC_PATH_PROGS(PERL,perl5 perl,false)
AC_PATH_PROG(POD2MAN,pod2man,false)
AC_PATH_PROG(POD2TEXT,pod2text,false)
AC_PATH_PROG(POD2HTML,pod2html,false)
if test "x$PERL" = xfalse -o "x$POD2MAN" = xfalse -o "x$POD2TEXT" = xfalse -o "x$POD2HTML" = xfalse
then
	AC_MSG_RESULT([    perl and the pod2x programs are only needed to rebuild the documentation.])
fi

dnl early check for toolkit flavor
AC_PATH_PROG(TCLSH,tclsh,false)
AC_PATH_PROG(PKG_CONFIG,pkg-config,false)

dnl check for recent g++ (and libraries)
dnl (thanks to Ted Faber for finding how to test this)
dnl and Murray Smigel for pointing out that the test fails with 3.0
if test "x$GXX" = "xyes" ; then
AC_MSG_CHECKING(that $CXX is recent)
AC_TRY_COMPILE(,
#if __GNUC__ < 2 || (__GNUC__ == 2 && __GNUC_MINOR__ < 8)
error
#endif
,AC_MSG_RESULT(yes),
AC_MSG_RESULT(no)
AC_MSG_ERROR([[$CXX does not appear to have a version number > 2.7.  lavaps requires a newer version of C++ to compile, please upgrade.  If you have a newer version installed, set the environment variable CXX to refer to it and re-run configure.]])
)
fi

dnl Checks for libraries.
AC_PATH_XTRA

AM_WITH_DMALLOC

dnl
dnl Checks for header files.
dnl check tcl/tk later
dnl

dnl
dnl stl is a pain:
dnl someone decided to use namespaces
dnl
dnl MIPSpro Compiler: Version 7.2.1 on Irix 6.5
dnl wants "use namespace stl"
dnl
dnl The standard seems to be "use namespace std"
dnl (supposedly gcc 3.0 works this way).
dnl
dnl Sigh.
dnl
stl_namespace=no
dnl
if test x$stl_namespace = xno
then
	AC_MSG_CHECKING(if STL works without any namespace)
	AC_TRY_COMPILE(
	#include <list>
	,
		list<int> test;
	,
		AC_MSG_RESULT(yes)
		stl_namespace="none"
	,
		AC_MSG_RESULT(no)
	)
fi
dnl
if test x$stl_namespace = xno
then
	AC_MSG_CHECKING(if STL works with namespace std)
	AC_TRY_COMPILE(
	#include <list>
	using namespace std;
	,
		list<int> test;
	,
		AC_MSG_RESULT(yes)
		stl_namespace=std
	,
		AC_MSG_RESULT(no)
	)
fi
dnl
if test x$stl_namespace = xno
then
	AC_MSG_CHECKING(if STL works with namespace stl)
	AC_TRY_COMPILE(
	#include <list>
	using namespace std;
	,
		list<int> test;
	,
		AC_MSG_RESULT(yes)
		stl_namespace=stl
	,
		AC_MSG_RESULT(no)
	)
fi
if test x$stl_namespace = xno
then
	AC_MSG_ERROR([Test cases failed to compile with STL. Turn on your bat-signal and contact your local autoconf expert. ])
fi
if test x$stl_namespace != xnone
then
	AC_DEFINE(LAVAPS_STL_REQUIRES_NAMESPACE,1,[does stl need a namespace?])
fi
LAVAPS_STL_NAMESPACE=$stl_namespace
AC_SUBST(LAVAPS_STL_NAMESPACE)
dnl I clearly don't get the key arguement to AH_VERBATIM...
AH_BOTTOM([
/* Requires -DLAVAPS_STL_NAMESPACE in the Makefile as well. */
#ifdef LAVAPS_STL_REQUIRES_NAMESPACE
using namespace LAVAPS_STL_NAMESPACE;
#endif /* LAVAPS_STL_NAMESPACE */
])


dnl
dnl Checks for library functions.
dnl
AC_CHECK_HEADERS([stdio.h popt.h])
AC_CHECK_FUNCS(setenv snprintf)

dnl
dnl allow user to add their own stuff
dnl
AC_ARG_WITH(additional-includes, --with-additional-includes=path, ADDITIONAL_INCLUDES=$withval,ADDITIONAL_INCLUDES="")
AC_ARG_WITH(additional-libs, --with-additional-libs=path, ADDITIONAL_LIBS=$withval,ADDITIONAL_LIBS="")
AC_SUBST(ADDITIONAL_INCLUDES)
AC_SUBST(ADDITIONAL_LIBS)


dnl
dnl libraries are below
dnl

dnl System-specific stuff:
AC_CANONICAL_HOST
PLATFORM_SOURCES=""
PLATFORM_INCLUDES=""
PLATFORM_LIBS=""
PLATFORM_SUBDIRS=""
dnl if job must be setuid/setgid
PLATFORM_OWN="none"
PLATFORM_GRP="none"
PLATFORM_DEFAULT_TCLTK_LIB_SUFFIX_TYPE=none
platform=$host
AC_MSG_CHECKING([for OS-specific process-scan code])
dnl *** Please keep platforms alphabetized, execept that Linux goes
dnl *** first since it's simplest and is a good place to copy from.
case "${platform}" in
*-*-*linux*)
	PLATFORM_DEFAULT_TCLTK_LIB_SUFFIX_TYPE=none
	PLATFORM_INCLUDES="-DUSE_PROCESS_SCAN_LINUX_PROC -Ilinux"
	PLATFORM_LIBS="-Llinux/proc -lproc"
	PLATFORM_SUBDIRS=linux
	AC_MSG_RESULT([yes---Linux])
	;;
*-*-*aix*)
	PLATFORM_DEFAULT_TCLTK_LIB_SUFFIX_TYPE=none
	PLATFORM_INCLUDES="-DUSE_PROCESS_SCAN_AIX"
	PLATFORM_SPECIFIC_TCLTK_DL_LIB=""
	PLATFORM_LIBS=""
	PLATFORM_SUBDIRS=""
	AC_MSG_RESULT([yes---AIX])
	;;
*-apple-*darwin*)
	PLATFORM_SPECIFIC_TCLTK_INCLUDES="-I/sw/include -L/sw/lib"
	PLATFORM_DEFAULT_TCLTK_LIB_SUFFIX_TYPE=none
	PLATFORM_INCLUDES="-DUSE_PROCESS_SCAN_MACH"
	PLATFORM_SPECIFIC_TCLTK_DL_LIB=""
	PLATFORM_LIBS=""
	PLATFORM_SUBDIRS=""
	dnl Apparently macs must run setuid root.  Ick.
	PLATFORM_OWN="root"
	AC_MSG_RESULT([yes---Mach/MacOS-X])
	;;
*-*-*bsd*)
	PLATFORM_SPECIFIC_TCLTK_INCLUDES="-I/usr/local/include/tk$TK_DOTTED_HI_VERS -I/usr/local/include/tcl$TCL_DOTTED_HI_VERS -L/usr/local/lib"
	case "${platform}" in
	*-*-*darwin*)
		PLATFORM_DEFAULT_TCLTK_LIB_SUFFIX_TYPE=dot
		TCLTK_LIB_SUFFIX_TYPE=dot
		AC_MSG_ERROR([no---Darwin support not complete yet])
		;;
	*) PLATFORM_DEFAULT_TCLTK_LIB_SUFFIX_TYPE=nodot;;
	esac
	PLATFORM_SPECIFIC_TCLTK_DL_LIB="no"
	PLATFORM_INCLUDES="-DUSE_PROCESS_SCAN_BSD"
	PLATFORM_LIBS="-lkvm"
	PLATFORM_SUBDIRS=""
	PLATFORM_GRP="kmem"
	AC_MSG_RESULT([yes---*BSD])
	dnl
	dnl This next check is a hack to work around a problem
	dnl Ted Faber found in FreeBSD 3.2 with newer versions of egcs.
	dnl
	AC_MSG_CHECKING(whether $CXX can include sys/user.h without -fpermissive)
	AC_TRY_COMPILE(
extern "C" {
#include <stdio.h>
#include <err.h>
#include <string.h>
#include <vis.h>
#include <limits.h>
#include <sys/param.h>
#include <sys/sysctl.h> // KERN_PROC_ALL
#include <sys/types.h>
#include <sys/user.h>
#include <fcntl.h>
#include <kvm.h>
}
,
	int i; /* no real function body */
,
	AC_MSG_RESULT(yes),
	PLATFORM_INCLUDES="$PLATFORM_INCLUDES -fpermissive"
,
	AC_MSG_RESULT(no)
)
	;;
*-*-*irix*)
	PLATFORM_SPECIFIC_TCLTK_LIB_SUFFIX_TYPE=none
	dnl xxx: this stuff is in progress
	PLATFORM_INCLUDES="-DUSE_PROCESS_SCAN_IRIX -I/usr/freeware/lib32"
	PLATFORM_LIBS="-L/usr/lib32 -L/usr/freeware/lib32"
	PLATFORM_SUBDIRS=""
	AC_MSG_RESULT([yes---Irix])
	;;
*-*-*solaris*)
	case `uname -r` in
	3.*|4.*|5.1|5.2|5.3|5.4)
		AC_MSG_ERROR([no---Solaris 2.4 or earlier not supported])
		;;
	5.5*)
		PLATFORM_INCLUDES="-DUSE_PROCESS_SCAN_SOLARIS_2_5"
		AC_MSG_RESULT([yes---Solaris 2.5])
		;;
	*)
		AC_MSG_RESULT([yes---Solaris 2.6 or later])
		PLATFORM_INCLUDES="-DUSE_PROCESS_SCAN_SOLARIS"
		;;
	esac
	PLATFORM_DEFAULT_TCLTK_LIB_SUFFIX_TYPE=none
	PLATFORM_SPECIFIC_TCLTK_DL_LIB=""
	PLATFORM_LIBS=""
	PLATFORM_SUBDIRS=""
	;;
*)
	AC_MSG_ERROR([no
Don't know about your host (${platform})
and so will not know how to read process information.])
	;;
esac

dnl
dnl FIGURE OUT THE TOOLKIT
dnl
GUI_TOOLKIT=none
DEFAULT_GUI_TOOLKIT=none
dnl default GUI is whatever comes LAST!
test $TCLSH != false && DEFAULT_GUI_TOOLKIT=tcltk
test $PKG_CONFIG != false && DEFAULT_GUI_TOOLKIT=gtk
AC_ARG_WITH(tcltk,--with-tcltk to use the tcl/tk GUI, GUI_TOOLKIT=tcltk)
AC_ARG_WITH(gtk,--with-gtk to use the Gtk/Gnome GUI, GUI_TOOLKIT=gtk)
AC_MSG_CHECKING([preferred GUI toolkit])
if test $GUI_TOOLKIT != none
then
	AC_MSG_RESULT([$GUI_TOOLKIT selected])
else
	if test $DEFAULT_GUI_TOOLKIT = none
	then
		AC_MSG_ERROR([no default GUI toolkit; specify --with-tcltk or --with-gtk])
	fi
	GUI_TOOLKIT=$DEFAULT_GUI_TOOLKIT
	AC_MSG_RESULT([$GUI_TOOLKIT defaulted])
fi
AC_SUBST(GUI_TOOLKIT)


dnl
dnl suck in tcl/tk-specific autoconf
dnl
use_gettext=no
if test $GUI_TOOLKIT = tcltk
then
	builtin(include,configure.tcltk.ac)
	GUI_INCLUDES="-DUSE_TCL_BLOB $TCLTK_INCLUDES"
	GUI_LIBS=$TCLTK_LIBS
	use_gettext=no
fi
AM_CONDITIONAL([COND_GT], [test "$use_gettext" = yes])

if test $GUI_TOOLKIT = gtk
then
	builtin(include,configure.gtk.ac)
	GUI_INCLUDES="-DUSE_GTK_BLOB $GTK_INCLUDES"
	GUI_LIBS=$GTK_LIBS
	use_gettext=yes
fi

USE_GETTEXT=$use_gettext
AC_SUBST(USE_GETTEXT)
AC_SUBST(GUI_INCLUDES)
AC_SUBST(GUI_LIBS)

if test x$USE_GETTEXT != xyes
then
	# manually make a po Makefile
	echo 'all install:' >po/Makefile
fi

dnl Older versions of the X11 headers are not compatible with the ANSI C++
dnl prototype checking done by recent egcs/g++ releases.  (Solaris is one
dnl platform known to need the -fpermissive flag for g++)
if test "x$GXX" = "xyes" ; then
	AC_MSG_CHECKING(if we need to add -fpermissive for $CXX to include X11/Xlib.h)
	AC_TRY_COMPILE(
	extern "C" {
	#include <X11/Xlib.h>
	}
	,
	int i; /* no real function body */
	,
	AC_MSG_RESULT(no),
	PLATFORM_INCLUDES="$PLATFORM_INCLUDES -fpermissive"
	AC_MSG_RESULT(yes))
fi

AC_SUBST(PLATFORM_SOURCES)
AC_SUBST(PLATFORM_INCLUDES)
AC_SUBST(PLATFORM_LIBS)
AC_SUBST(PLATFORM_SUBDIRS)
AC_SUBST(PLATFORM_OWN)
AC_SUBST(PLATFORM_GRP)

if test x$GXX = xyes
then
	LAVAPS_GXX_FLAGS="-Wall"
else
	LAVAPS_GXX_FLAGS=""
fi
AC_SUBST(LAVAPS_GXX_FLAGS)


AC_OUTPUT([Makefile
	lavaps.spec
	doc/Makefile
	po/Makefile.in
	src/Makefile
	src/linux/Makefile
	src/freebsd/port/Makefile
	tcl2cc/Makefile
])
