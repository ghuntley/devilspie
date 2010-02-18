# Initial blurb
AC_PREREQ(2.52)
AC_INIT(devilspie, 0.22, http://www.burtonini.com/)
AC_CONFIG_SRCDIR(src/devilspie.c)
AM_INIT_AUTOMAKE(AC_PACKAGE_NAME, AC_PACKAGE_VERSION)
AM_CONFIG_HEADER(src/config.h)

# Use AM_SILENT_RULES if present
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

# Honor aclocal flags
ACLOCAL="$ACLOCAL $ACLOCAL_FLAGS"

AC_PROG_CC
AM_PROG_CC_STDC

GNOME_COMPILE_WARNINGS(yes)

# Do the gettext/i18n stuff
AC_PROG_INTLTOOL([0.40])
GETTEXT_PACKAGE=devilspie
AC_SUBST([GETTEXT_PACKAGE])
AC_DEFINE_UNQUOTED([GETTEXT_PACKAGE],["$GETTEXT_PACKAGE"],[Gettext package])
AM_GLIB_GNU_GETTEXT

# Check that we actually have the libraries required to build
PKG_CHECK_MODULES(WNCK,
        glib-2.0 >= 2.9.1
        gdk-2.0
        libwnck-1.0 >= 0.17)
AC_SUBST(WNCK_CFLAGS)
AC_SUBST(WNCK_LIBS)

AC_MSG_CHECKING([whether Unique support is requested])
AC_ARG_ENABLE([unique],
	[AS_HELP_STRING([--enable-unique],
	[Enable Unique (single-instance application) support (default: disabled)])],
	[], [enable_unique=no])
AC_MSG_RESULT([$enable_unique])

if test x"$enable_unique" = "xyes" ; then
	PKG_CHECK_MODULES([UNIQUE], [unique-1.0 >= 1.1.2])
	AC_DEFINE([ENABLE_UNIQUE], [1], [Define whether Unique support is enabled])
fi

# See if we have new libwnck with timestamp arguments
PKG_CHECK_MODULES(WNCK_VER, libwnck-1.0 >= 2.9.92,
		 wnck_new="yes", wnck_new="no")
if test "x$wnck_new" = "xyes"; then
        AC_DEFINE(NEED_TIMESTAMPS,1,[Define to 1 if timestamps are needed to wnck functions])
fi

# Check if we have the set_fullscreen function
AC_CHECK_LIB(wnck-1, wnck_window_set_fullscreen,
            have_fullscreen=1, have_fullscreen=0)
AC_DEFINE_UNQUOTED(HAVE_FULLSCREEN, $have_fullscreen, [set_fullscreen is available])

AC_CHECK_LIB(wnck-1, wnck_window_set_window_type,
            have_set_wintype=1, have_set_wintype=0)
AC_DEFINE_UNQUOTED(HAVE_SET_WINDOW_TYPE, $have_set_wintype, [set_window_type is available])

# Check for the POSIX regexpr functions and headers
AC_CHECK_FUNCS([regcomp regexec regfree])
AC_CHECK_HEADERS([sys/types.h regex.h])

AC_OUTPUT([
devilspie.spec
Makefile
data/Makefile
src/Makefile
po/Makefile.in
tests/Makefile
])

echo "
Devilspie was configured with the following options:

    Source code location:   : $srcdir
    Compiler                : $CC
    Prefix                  : $prefix

    Unique support          : $enable_unique
"
