AM_CPPFLAGS = -I$(top_srcdir)/src -Werror -Wall

if MING
LDADD = $(top_srcdir)/src/liblogr.la
threads_LDADD = $(LDADD)
else
LDADD = $(top_srcdir)/src/liblogr.la
threads_LDADD = $(LDADD) -lpthread
endif

EXTRA_PROGRAMS = apache file logcat man rotate simple threads

noinst_PROGRAMS = $(EXTRA_PROGRAMS)
