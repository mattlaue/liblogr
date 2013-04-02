AUTOMAKE_OPTIONS = subdir-objects
ACLOCAL_AMFLAGS = -I m4

if LOGR_ENABLE_DEBUG
AM_CFLAGS = -g3 -O0 -Wall -Werror
AM_CXXFLAGS = -g3 -O0 -Wall -Werror
else
AM_CFLAGS = -O2
AM_CXXFLAGS = -O2
endif

if LOGR_ENABLE_DOC
DOC_DIR = doc
else
DOC_DIR =
endif

if LOGR_ENABLE_EXAMPLES
EXAMPLES_DIR = examples
else
EXAMPLES_DIR =
endif

SUBDIRS = src man $(EXAMPLES_DIR) $(DOC_DIR)

dist_noinst_SCRIPTS = autogen.sh

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = logr.pc
