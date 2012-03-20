AC_INIT([logr], [1.0.3])
AC_PREREQ([2.65])
AM_INIT_AUTOMAKE([foreign -Wall -Werror])

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])
AC_PROG_CC
AC_PROG_LIBTOOL
AC_HEADER_STDBOOL

AC_CANONICAL_HOST

MINGW32=no
case "${host_os}" in
  *mingw32* )
    MINGW32=yes
    ;;
esac
AM_CONDITIONAL(MINGW, [test "x$MINGW32" = "xyes"])

if test "x$MINGW32" = "xyes" ; then
   AM_CONDITIONAL(MINGW, true)
else
   dnl optional headers
   AC_CHECK_HEADER([syslog.h],AC_DEFINE(HAVE_SYSLOG_H, 1, [...]),)
   AC_CHECK_HEADER([sys/prctl.h],AC_DEFINE(HAVE_PRCTL_H, 1, [...]),)

   dnl savelog binary
   AC_CHECK_PROG(SAVELOG, savelog, yes, no)
   if test "x$SAVELOG" = "xyes" ; then
      AC_DEFINE(HAVE_SAVELOG, 1, [...])
   fi
fi

dnl pthread development files
AC_CHECK_HEADERS([pthread.h],,
    [AC_MSG_ERROR([*** cannot find pthread.h])])

dnl Build documentation with Doxygen
AC_ARG_ENABLE([doc],
AS_HELP_STRING([--enable-doc], [Build the API documentation with Doxygen]),
			       [ac_enable_doc="yes"], [ac_enable_doc="no"])

if test "x$ac_enable_doc" != "xno"; then
    AC_CHECK_PROG(HAVE_DOXYGEN, doxygen, true, false)

    if test "x$HAVE_DOXYGEN" = "xfalse" -a "x$ac_enable_doc" = "xyes"; then
        AC_MSG_ERROR([*** API documentation requested but Doxygen not found])
        ac_enable_doc=yes
    fi
fi
AM_CONDITIONAL(LOGR_ENABLE_DOC, [test "x${ac_enable_doc}" = "xyes"])

dnl Build examples
ac_enable_examples=yes
AC_ARG_ENABLE(examples,
     AS_HELP_STRING([--disable-examples], [Disable examples]),
     [ ac_enable_examples=$enableval ], [ ac_enable_examples=yes] )
AM_CONDITIONAL(LOGR_ENABLE_EXAMPLES, [test "x${ac_enable_examples}" != "xno"])

ac_man_date=`date  +"%B %Y"`
AC_SUBST([LOGR_SO_VERSION], [1:0:1])
AC_SUBST([MAN_DATE], [${ac_man_date}])

AC_OUTPUT([
    Makefile
    logr.pc
    src/Makefile
    doc/Makefile doc/Doxyfile
    man/Makefile man/logr.7
    examples/Makefile
])
AC_OUTPUT

