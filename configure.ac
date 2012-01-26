AC_INIT([logr], [1.0.0])
AC_PREREQ([2.67])
AM_INIT_AUTOMAKE([foreign -Wall -Werror])

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])
AC_PROG_CC
AC_PROG_LIBTOOL

dnl syslog header
AC_CHECK_HEADER([syslog.h],,
    [AC_MSG_ERROR([*** cannot find headers for syslog])])

dnl savelog binary
AC_CHECK_PROG(HAVE_SAVELOG, savelog, true, false)
if test "x$HAVE_SAVELOG" = "xfalse"; then
   AC_MSG_ERROR([*** savelog executable not found])
fi

dnl check for pthreads
AC_CHECK_LIB(pthread, pthread_create,
                      AM_CONDITIONAL(HAVE_PTHREADS, true),
                      AM_CONDITIONAL(HAVE_PTHREADS, true))

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

AC_SUBST([LOGR_SO_VERSION], [1:0:1])

AC_OUTPUT([
    Makefile
    src/Makefile
    doc/Makefile doc/Doxyfile
    examples/Makefile
])
AC_OUTPUT

