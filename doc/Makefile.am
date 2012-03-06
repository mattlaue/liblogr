EXTRA_DIST = Doxyfile.in

noinst_DATA = doxygen-build.stamp

doxygen-build.stamp: Doxyfile $(top_srcdir)/src/*.h
	doxygen
	touch doxygen-build.stamp

clean-local:
	rm -rf @VERSION@
	rm -f doxygen-build.stamp
