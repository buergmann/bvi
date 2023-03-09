AC_CHECK_HEADER
(
 [ncurses.h],
 [
    AC_SEARCH_LIBS
    ([initscr], [ncurses],
     [
        AC_DEFINE(HAVE_NCURSES_H)
	    AC_SEARCH_LIBS([tputs], [tinfo])
     ],[
        AC_SEARCH_LIBS
        (
          [initscr], [curses],
          [
                AC_DEFINE(HAVE_CURSES_H)
                AC_SEARCH_LIBS([tputs], [tinfo])
          ],[
	            AC_MSG_ERROR([bvi requires a curses library])
          ]
        )
     ]
    )
 ],[
    AC_CHECK_HEADER
    (
     [curses.h],
     [
    	AC_SEARCH_LIBS
        (
          [initscr], [curses],
          [
		    AC_DEFINE(HAVE_CURSES_H)
          ],[
		    AC_MSG_ERROR([bvi requires a curses library])
          ]
        )
     ],[
# statt:		AC_MSG_ERROR([bvi requires a curses library])
       AC_CHECK_HEADER
       (
        [ncursesw/curses.h],
        [
    	   AC_SEARCH_LIBS
           (
             [initscr], [curses],
             [
		       AC_DEFINE(HAVE_NCURSESW_CURSES_H)
             ],[
		       AC_MSG_ERROR([bvi requires a curses library])
             ]
           )
        ],[
		   AC_MSG_ERROR([bvi requires a curses library])
	    ]
       )
     ]
   )
 ]
)
