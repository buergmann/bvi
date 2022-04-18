dnl Process this file with autoconf to produce a configure script.

AC_INIT
AC_CONFIG_SRCDIR([bvi.c])
AC_CONFIG_HEADER(config.h)
AC_SYS_LARGEFILE

AC_CANONICAL_HOST
CPPFLAGS="$CPPFLAGS -D_FILE_OFFSET_BITS=64 -D_LARGE_FILES"

dnl On SunOS4 systems you have to use the /usr/5bin/cc compiler
dnl to get the SYSV curses library:
case "$host_os" in
sunos4*)
	AC_MSG_RESULT("using /usr/5bin/cc")
	CC="/usr/5bin/cc"
	ac_objext="o"
	;;
*)
	AC_PROG_CC
	;;
esac


dnl Some curses specials
dnl Solaris needs for tputs 'putc(char)' instead of 'putc(int)'
case "$host_os" in
solaris*)
	CPPFLAGS="${CPPFLAGS} -D_MSE_INT_H"
	AC_DEFINE(NEED_PUTC_CHAR)
	;;
hpux9*) DEFS=${DEFS-"-DMINICURSES"}
	;;
esac


dnl Checks for programs.
AC_PROG_INSTALL

dnl Check for typedefs
AC_CHECK_SIZEOF(int)
AC_CHECK_SIZEOF(long)
AC_CHECK_SIZEOF(long long)
AC_CHECK_SIZEOF(void *)

AC_CHECK_TYPE(size_t, unsigned int)
AC_CHECK_TYPE(off_t, int)

dnl Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS(fcntl.h termcap.h termio.h termios.h unistd.h term.h ncurses/term.h)

dnl Check if ncurses requested
_cppflags="${CPPFLAGS}"
_ldflags="${LDFLAGS}"

AC_ARG_WITH(ncurses,
[  --with-ncurses[=DIR]    path to ncurses],[
  if test "$withval" != "no"; then
    CPPFLAGS="${CPPFLAGS} -I$withval/include -I$withval/include/ncurses"
    LDFLAGS="${LDFLAGS} -L$withval/lib"
  fi
])

AC_CHECK_HEADER([ncurses.h],[
    AC_SEARCH_LIBS([initscr], [ncurses], [
        AC_DEFINE(HAVE_NCURSES_H)
	    AC_SEARCH_LIBS([tputs], [tinfo])],[
        AC_SEARCH_LIBS([initscr], [curses],[
                AC_DEFINE(HAVE_CURSES_H)
                AC_SEARCH_LIBS([tputs], [tinfo])],[
	            AC_MSG_ERROR([bvi requires a curses library])])])
 ],[
    AC_CHECK_HEADER([curses.h],[
    	AC_SEARCH_LIBS([initscr], [curses], [
		    AC_DEFINE(HAVE_CURSES_H)],[
		    AC_MSG_ERROR([bvi requires a curses library])])],[
       AC_CHECK_HEADER([ncursesw/curses.h], [
    	   AC_SEARCH_LIBS([initscr], [ncursesw], [
		       AC_DEFINE(HAVE_NCURSESW_CURSES_H)],[
		       AC_MSG_ERROR([bvi requires a curses library])])],[
		   AC_MSG_ERROR([bvi requires a curses library])])])])

dnl Checks for typedefs, structures, and compiler characteristics.
dnl AC_TYPE_SIZE_T

dnl Checks for library functions.
dnl AC_TYPE_SIGNAL
AC_CHECK_FUNCS(strdup strtol strtoll strerror memmove)
AC_FUNC_FSEEKO
AC_FUNC_MALLOC
AC_FUNC_REALLOC

AC_PROG_INSTALL

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
