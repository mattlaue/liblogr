AM_CPPFLAGS = -I$(top_srcdir)/src -Werror -Wall

LDADD = $(top_srcdir)/src/liblogr.la

EXTRA_PROGRAMS = apache file logcat rotate simple

if HAVE_PTHREADS
EXTRA_PROGRAMS += threads
endif

noinst_PROGRAMS = $(EXTRA_PROGRAMS)

threads_LDADD = $(LDADD) -lpthread
